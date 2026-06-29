#pragma once 

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

#include <iostream>
#include <atomic>

#include "shm_helper.hpp"
#include "shm_type.hpp"

namespace own_shm {

/**
 * 管理申请的SHM的类, 明确职责
 * 1. 创建或打开SHM
 * 2. 计算布局,分配布局指针
 * 3. 初始化header和队列的一个个slot
 * 4. 提供最小化的Publish Read语义
 */
template<typename _Type>
class ShmManager {
 public:
    ~ShmManager();
    bool Open(const std::string& name, size_t capacity = 10);
    bool isCreate();
    bool Close();
    
    //
    bool Publish(const _Type& msg);
    bool TryRead(std::uint64_t seq, _Type* out);

 private:
    using Slot = ShmChunk<_Type>;
    static constexpr std::uint32_t kMagic{0x53484D31};
    static constexpr std::uint32_t kVersion{1};
    static constexpr std::uint64_t kEmptySeq = UINT64_MAX;                                                                                                                                                  

    bool ValidateHeader(std::size_t capacity) {
        if (shm_head_ == nullptr) {
            return false;
        }        
        if (shm_head_->magic != kMagic) {
            return false;
        }
        if (shm_head_->version != kVersion) {
            return false;
        }
        if (shm_head_->capacity != capacity) {
            return false;
        }
        if (shm_head_->slot_size != sizeof(Slot)) {
            return false;
        }
        return true;
    }


    std::string shm_name_{""};
    int shm_fd_{-1};
    void* shm_ptr_{nullptr};
    size_t shm_size_{0};

    ShmHeader* shm_head_{nullptr};
    ShmChunk<_Type>* data_head_{nullptr};
    size_t capacity_{0};
    std::atomic_bool is_create_{false};

};

template<typename _Type>
ShmManager<_Type>::~ShmManager() {
    Close();
}

template<typename _Type>
bool ShmManager<_Type>::Open(const std::string& name, size_t capacity) {
    if (shm_fd_ >= 0) {
        return true;
    }
    static_assert(std::is_trivially_copyable<_Type>::value,                                                                                                                                              
                    "shared memory message must be trivially copyable"); //可以被简单复制                                                                                                                                   
    static_assert(std::is_standard_layout<_Type>::value,                                                                                                                                                  
                    "shared memory message must be standard layout");  // 是类似c布局简单格式
    bool is_create{false};
    int fd = create_shm(name, &is_create);
    if (fd < 0) {
        std::cout << "create_shm fd failed" << std::endl;
        return false;
    }
    shm_name_ = name;                                                                                                                                                                                       
    shm_fd_ = fd;                                                                                                                                                                                           
    is_create_.store(is_create); 
    std::cout << "shm create is " << is_create << std::endl;
    // 要求地址必须是alignof(得到一个对象需要按照什么地址倍数才能使用对其)可以整除的，普通的new实际上是保证了alignof
    // mmap返回的首地址比如   shm = 0x10000000 通常是page对齐的一般的2 4 8 16都可以对其
    // int ret = ftruncate(shm_fd, sizeof(ShmHeader) + sizeof(_Type) * 10);
    // align_up 是一个“小工具函数”：把某个字节偏移量向上补齐到指定对齐值的整数倍，第一个参数待对齐的原始值，待对齐的值

    size_t shm_align_size = align_up(sizeof(ShmHeader), alignof(ShmChunk<_Type>));
    shm_size_ = shm_align_size + sizeof(ShmChunk<_Type>) * capacity;
    capacity_ = capacity;
    if (is_create) {
        if (ftruncate(fd, shm_size_) != 0) {                                                                                                                                                                    
            Close();                                                                                                                                                                                              
            return false;                                                                                                                                                                                       
        }
    }

    // 必须是共享
    shm_ptr_= mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // 失败返回是(void*)-1
    if (MAP_FAILED == shm_ptr_) {
        shm_ptr_ = nullptr;                                                                                                                                                                                     
        Close();                                                                                                                                                                                              
        return false;
    }
    uint8_t* shm_user = reinterpret_cast<uint8_t*>(shm_ptr_);


    data_head_ = reinterpret_cast<ShmChunk<_Type>*>(shm_user + shm_align_size); 

    if (is_create) {
        shm_head_ =  new (shm_user) ShmHeader();
        if (nullptr == shm_head_) {
            std::cout << "mmap request shm_head_ failed" << std::endl;
            return false;
        }
        shm_head_->magic = kMagic;
        shm_head_->version = kVersion;
        shm_head_->capacity = capacity;
        shm_head_->slot_size = sizeof(ShmChunk<_Type>);
        shm_head_->write_seq.store(0, std::memory_order_relaxed);                                                                                                                                               


        for (auto index = 0; index < capacity; index++) {
            //每个地址初始化一次
            new (&data_head_[index]) ShmChunk<_Type>();

            // data_head_[index].seq.store(0, std::memory_order_relaxed);
            // 不能写0,因为第一次就是0,会导致误判,kEmptySeq 是一个极大值
            data_head_[index].seq.store(kEmptySeq, std::memory_order_relaxed);
        }
    } else {
        // 读方,只需要内存映射,不能二次初始化
        shm_head_ = reinterpret_cast<ShmHeader*>(shm_user);
        if (!ValidateHeader(capacity)) { 
           Close();                                                                                                                                                                   
           return false;                                                                                                                                                                                   
        } 
    }
    std::cout << "shm init success" << std::endl;
    return true;
}

template<typename _Type>
bool ShmManager<_Type>::isCreate() {
    return is_create_.load();
}

template<typename _Type>
bool ShmManager<_Type>::Close() {
    bool ok = true;
    if (shm_ptr_ != nullptr) {
        if (-1 == munmap(shm_ptr_, shm_size_)) {
            std::cout << " munmap failed " << strerror(errno) << std::endl;
            ok = false;
        }
        shm_ptr_ = nullptr;
    }
    if (shm_fd_ >= 0) {                                                                                                                                                                                 
        if (close(shm_fd_) != 0) {                                                                                                                                                                      
            ok = false;                                                                                                                                                                                 
        }                                                                                                                                                                                               
        shm_fd_ = -1;                                                                                                                                                                                   
    }                                                                                                                                                                                                   
                                                                                                                                                                                                        
    bool expected = true;                                                                                                                                                                               
    if (is_create_.compare_exchange_strong(expected, false)) { 
        //normallize_shm_name                                                                                                                                         
        if (shm_unlink(normallize_shm_name(shm_name_).c_str()) != 0) {                                                                                                                                  
            ok = false;                                                                                                                                                                                 
        }                                                                                                                                                                                               
    }  
    shm_head_ = nullptr;                                                                                                                                                                                
    data_head_ = nullptr;                                                                                                                                                                               
    shm_size_ = 0;                                                                                                                                                                                      
    capacity_ = 0;                                                                                                                                                                                      
    shm_name_.clear();

    return ok;
}

/**
 * 覆盖写, read慢就会覆盖
 */
template<typename _Type>
bool ShmManager<_Type>::Publish(const _Type& msg) {
    if (!shm_head_ || !data_head_ || capacity_ == 0) {
        return false;
    }
    const auto seq = shm_head_->write_seq.fetch_add(1, std::memory_order_relaxed);
    auto& slot = data_head_[seq % capacity_];

    slot.data = msg;                         
    //must memory_order_release                                                                                                                                                           
    slot.seq.store(seq, std::memory_order_release);
    return true;                                                                                                                                                     
}

template<typename _Type>
bool ShmManager<_Type>::TryRead(std::uint64_t seq, _Type* out) {
    if (!out || !shm_head_ || !data_head_ || capacity_ == 0) {                                                                                                                                          
        return false;                                                                                                                                                                                   
    }
    const auto& slot = data_head_[seq % capacity_];
    const auto screen = slot.seq.load(std::memory_order_acquire);
    
    // write也是按照队列一点点写的,如果这里不一致说明要不write还没写到,要不write覆盖了
    // Read是主动非回调实现
    if (seq != screen) {
        return false;
    }

    *out = slot.data;
    return true;
}

}

