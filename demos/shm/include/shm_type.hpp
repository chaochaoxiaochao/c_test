#pragma once

#include <atomic>

/**
 * @brief 固定字段，管理共享内存
 */
struct ShmHeader {
    std::uint32_t magic; // 对shm布局的魔数定义
    std::uint32_t version; // 以后布局升级用
    std::uint64_t capacity; // ringbuffer slot
    std::uint64_t slot_size; // 用于校验作用
    std::atomic<std::uint64_t> write_seq; //全局,pub一次+1
};

/**
 * @brief 行业通用，通常是对于元素的描述，状态？index？索引等
 */
template<typename _TYPE>
struct ShmChunk {
    //当前chunk放的是第几条消息
    std::atomic<uint64_t> seq; //告诉 reader 当前 slot 里是哪一条消息
    //实际data
    _TYPE data;
};

