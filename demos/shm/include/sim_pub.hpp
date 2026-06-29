#pragma once 

#include <string>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>

#include "shm.hpp"

#include "shm_helper.hpp"

template<typename _Type>
class ShmPub {
 public:
    bool Init(const std::string& shm_name, size_t capicity = 10) {
        static_assert(std::is_trivially_copyable<_Type>::value,                                                                                                                                              
                     "shared memory message must be trivially copyable"); //可以被简单复制                                                                                                                                   
       static_assert(std::is_standard_layout<_Type>::value,                                                                                                                                                  
                     "shared memory message must be standard layout");  // 是类似c布局简单格式
        int shm_fd = shm_open(normallize_shm_name(shm_name).c_str(), O_RDWR | O_CREAT, 0644);
        if (shm_fd < 0) {
            std::cerr << "shm open and create failed : " << shm_name << " error is " << strerror(errno) << std::endl;
            return false;
        }

        // 要求地址必须是alignof(得到一个对象需要按照什么地址倍数才能使用对其)可以整除的，普通的new实际上是保证了alignof
        // mmap返回的首地址比如   shm = 0x10000000 通常是page对齐的一般的2 4 8 16都可以对其
        // int ret = ftruncate(shm_fd, sizeof(ShmHeader) + sizeof(_Type) * 10);
        // align_up 是一个“小工具函数”：把某个字节偏移量向上补齐到指定对齐值的整数倍，第一个参数待对齐的原始值，待对齐的值

        size_t shm_align_size = align_up(sizeof(ShmHeader), alignof(ShmChunk<_Type>));
        size_t shm_size = shm_align_size + sizeof(ShmChunk<_Type>) * capicity;
        int ret = ftruncate(shm_fd, shm_size);
        if (ret == EINVAL) {
            return false;
        }

        // 必须是共享
        char* shm_user = (char*) mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        
        // 失败返回是(void*)-1
        if (MAP_FAILED == shm_user) {
            std::cout << "mmap request failed" << std::endl;
            return false;
        }

        shm_head_ = new (shm_user) ShmHeader();
        if (nullptr == shm_head_) {
            std::cout << "mmap request shm_head_ failed" << std::endl;
            return false;
        }

        data_head_ = new (shm_user + shm_align_size) ShmChunk<_Type>();
        if (nullptr == data_head_) {
            std::cout << "mmap request data_head_ failed" << std::endl;
            return false;
        }
        
        std::cout << "shm init success" << std::endl;
        return true;
    }

    /**
     * @brief send one msg
     */
    bool publish(const _Type& msg) {
        return false;
    }



 private:
    int shm_fd{-1};
    ShmHeader* shm_head_{nullptr};
    ShmChunk<_Type>* data_head_{nullptr};
    size_t shm_capicity_;


};