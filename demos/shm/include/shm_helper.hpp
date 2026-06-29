#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <iostream>

namespace own_shm {

static inline std::string normallize_shm_name(const std::string& name) {
    if (name.empty()) {
        return name;
    }
    if (name.front() == '/') {
        return name;
    }

    return "/" + name;
}

static inline size_t align_up(size_t size, size_t align) noexcept {
    return (size + align - 1) & ~(align - 1);
}

int create_shm(const std::string& shm_name, bool* is_create) {
    int shm_fd;
    while (true) {
        //O_EXCL 是“排他创建”的意思，通常和 O_CREAT 一起用, 不存在创建成功,存在创建失败
        shm_fd = shm_open(normallize_shm_name(shm_name).c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
        if (shm_fd >= 0) { //创建成功   
            fchmod(shm_fd, 0644);
            *is_create = true;
        } else if (errno == EEXIST) { //EEXIST已经存在
            shm_fd = shm_open(normallize_shm_name(shm_name).c_str(), O_RDWR, 0644);
            if (shm_fd < 0 && errno == ENOENT) { //shm又不存在了
                continue;
            }
            *is_create = false;
        } else {
            std::cout << "Open SHM failed" << std::endl;
            return -1;
        }
        break;
    }
    std::cout << "Open SHM success" << std::endl;
    return shm_fd;
}


}