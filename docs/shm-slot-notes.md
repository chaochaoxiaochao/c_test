# Shared Memory Slot Notes

These notes capture the design discussion for the shared-memory publish/subscribe demo.

## Slot Layout

A shared-memory queue should usually store `Slot<T>[capacity]`, not a bare `T[capacity]`.

```cpp
template <typename T>
struct ShmSlot {
    std::atomic<uint64_t> seq;
    T data;
};
```

`Slot<T>` is one message cell in the ring buffer. It keeps the payload plus per-message metadata.

The whole shared-memory block can be shaped like this:

```text
| ShmHeader | padding | ShmSlot<T>[capacity] |
```

`ShmHeader` stores global queue metadata, such as magic, version, capacity, element size, type hash, and the global write sequence.

## Why `seq` Exists

`seq` is the message sequence number for a slot.

If `capacity == 10`:

```text
seq 0  -> slot[0]
seq 1  -> slot[1]
...
seq 9  -> slot[9]
seq 10 -> slot[0]
```

So `slot.seq` tells the subscriber which message is currently stored in that slot.

It is useful for:

- Detecting whether the slot contains the expected message.
- Detecting dropped or overwritten messages when the subscriber is slow.
- Acting as the publish-complete synchronization flag.

A common publish order is:

```cpp
slot.data = msg;
slot.seq.store(seq, std::memory_order_release);
```

A common subscribe order is:

```cpp
auto seen = slot.seq.load(std::memory_order_acquire);
if (seen == expected_seq) {
    T msg = slot.data;
}
```

The `release`/`acquire` pair makes the payload write visible before the subscriber accepts the sequence update.

## Alignment Rule

`mmap` returns a page-aligned base address, so placing `ShmHeader` at the beginning is normally safe.

The next object after the header still needs manual alignment because `sizeof(ShmHeader)` might not be a multiple of `alignof(Slot<T>)`.

Use an aligned offset:

```cpp
inline std::size_t align_up(std::size_t value, std::size_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}

using Slot = ShmSlot<T>;
std::size_t slots_offset = align_up(sizeof(ShmHeader), alignof(Slot));
std::size_t shm_size = slots_offset + sizeof(Slot) * capacity;
```

Only the first slot needs manual alignment. Later slots are naturally aligned because `sizeof(Slot)` includes any padding required for arrays of `Slot`.

## Copy Publish vs Loan Publish

Start with copy publish:

```cpp
bool publish(const T& msg);
```

This is simpler and is enough to validate the ring buffer layout, sequence handling, and subscriber logic.

Later, add zero-copy loan publish:

```cpp
auto loan = pub.loan();
loan->field = value;
loan.publish();
```

Loan publish avoids one copy, but it also needs stricter slot lifetime rules: free slot acquisition, commit/cancel behavior, slow subscriber handling, overwrite policy, and cleanup when a loan is abandoned.

## Message Type Rule

For the first version, restrict `T` to simple shared-memory-safe types:

```cpp
static_assert(std::is_trivially_copyable_v<T>);
static_assert(std::is_standard_layout_v<T>);
```

Avoid direct pointers and STL containers inside `T`, such as `std::string`, `std::vector`, `std::map`, `std::shared_ptr`, and raw `T*` fields. Their internal addresses are process-local and are usually invalid in another process.

For variable-size data, store the bytes inside shared memory and refer to them with `offset + size`, not process-local pointers.

## Current Plan / TODO

The first implementation should stay deliberately small: a basic sequence-tagged overwrite ring. The goal is to prove the shared-memory layout, cross-manager open path, publish path, read path, and close/unlink behavior before adding stronger concurrency policies.

### Version 1: `BasicSeqRing`

Semantics:

- Fixed slot mapping: `slot_index = seq % capacity`.
- `Publish(const T& msg)` never blocks and overwrites the target slot.
- `TryRead(seq, out)` succeeds only when the target slot currently stores that exact `seq`.
- Slow readers may miss messages after wrap-around.
- This version does not protect a reader from a concurrent writer overwriting the same slot while `data` is being copied. It is a layout/API validation step and a useful low-contention/basic strategy, not the final safe concurrent reader policy.

Slot state:

```cpp
template <typename T>
struct ShmSlot {
    std::atomic<std::uint64_t> seq;
    T data;
};
```

Initialize empty slots with a sentinel, not `0`:

```cpp
static constexpr std::uint64_t kEmptySeq = UINT64_MAX;
slot.seq.store(kEmptySeq, std::memory_order_relaxed);
```

`0` is the first real published sequence, so using `0` for empty slots can make `TryRead(0, out)` succeed before any publish.

### Read / Write API

Keep the manager primitive API explicit:

```cpp
bool Open(const std::string& name, std::size_t capacity = 10);
bool Close();
bool Publish(const T& msg);
bool TryRead(std::uint64_t seq, T* out) const;
std::uint64_t NextSeq() const;
bool IsCreator() const;
```

`Publish` flow for `BasicSeqRing`:

```cpp
const auto seq = header->write_seq.fetch_add(1, std::memory_order_relaxed);
auto& slot = slots[seq % capacity];
slot.data = msg;
slot.seq.store(seq, std::memory_order_release);
```

`TryRead` flow for `BasicSeqRing`:

```cpp
auto& slot = slots[seq % capacity];
auto seen = slot.seq.load(std::memory_order_acquire);
if (seen != seq) {
    return false;
}
*out = slot.data;
return true;
```

The `%` is required. Bitwise `&` only works for power-of-two masks and is wrong for arbitrary capacities such as `30`.

### Pub/Sub Wrapper Roles

Keep `ShmManager<T>` as the low-level primitive. Add thin wrappers later:

- `ShmPub<T>` exposes `Open`, `Publish`, and `Close` only.
- `ShmSub<T>` owns a `ShmManager<T>` plus `next_seq_`.
- First sub API should be pull-based: `PollOnce(T* out)` and `Drain(handler, max_count)`.
- Background callbacks require a polling thread or a cross-process notification primitive; do not put callback loops in `ShmManager<T>`.

### Later Policies

After `BasicSeqRing` is tested, add stronger policies as separate, named strategies:

1. `OverwriteSeqlockRingPolicy`
   - Fixed mapping: `seq % capacity`.
   - Slot state encodes `Writing(seq)` and `Ready(seq)`.
   - Reader checks `state -> data -> state` to detect concurrent overwrite.
   - Writer does not block; reader may fail instead of returning torn data.

2. `DropIfBusyRingPolicy`
   - Fixed mapping: `seq % capacity`.
   - Slot tracks active readers.
   - Writer returns busy instead of overwriting a slot currently being read.

3. `ScanWritableLatestPolicy`
   - Writer scans for any slot with no active readers.
   - Reader normally consumes latest via `latest_slot/latest_seq`.
   - This is not the same as `seq % capacity`; reading arbitrary `seq` needs a search or a `seq -> slot_index` index.

4. `BackpressurePolicy`
   - Header tracks registered readers and their `next_seq`.
   - Writer fails or blocks when the ring is full for the slowest active reader.
   - Requires reader registration, heartbeat, and dead-reader cleanup.

### Tests to Keep Green

- Opening a manager with a mismatched capacity fails and cleans up its partial mapping.
- Reopening with the correct capacity succeeds without reinitializing existing slots.
- `Publish` before `Open` fails.
- Empty slots do not satisfy `TryRead(0, out)` before the first publish.
- `Publish` followed by `TryRead(seq, out)` succeeds across two `ShmManager` instances in the same process.
- Wrap-around overwrites old `seq` values and makes stale reads fail.
