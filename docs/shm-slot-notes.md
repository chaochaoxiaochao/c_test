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
