#pragma once 

#include <string>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>

class ShmSub {
 public:
    bool Init(const std::string& shm_name) {
        int shm_fd = shm_open(shm_name.c_str(), O_RDWR, 0);
        if (shm_fd < 0) {
            std::cerr << "shm open failed : " << shm_name << " error is " << strerror(errno) << std::endl;
            return false;
        }

    }
 private:
    int shm_fd{0};
};