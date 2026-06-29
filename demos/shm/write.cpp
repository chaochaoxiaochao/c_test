#include <iostream>
#include <signal.h>
#include <future>
#include <memory>

#include "shm_pub.hpp"


static std::promise<void> stop_flag;

void signal_handler(int sig) {
    stop_flag.set_value();
}

int main() {
    std::cout << "write start" << std::endl;
    signal(SIGINT, signal_handler);

    auto pub = std::make_shared<ShmPub<int>>();
    
    if(!pub->Init("test_write")) {
        std::cout << "pub shm init failed" << std::endl;
        return -1;
    }

    auto stop_wait = stop_flag.get_future();
    stop_wait.get();
    std::cout << "write stop " << std::endl;
    return 0;
}