#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <sys/mman.h>
#include <unistd.h>

#include <string>

#include "shm_manager.hpp"

using namespace own_shm;

TEST_CASE("open shm", "[test]") {
    const std::string shm_name = "test_shm_" + std::to_string(getpid());
    bool is_create = false;
    int write_fd = create_shm(shm_name, &is_create);
    REQUIRE(write_fd > 0);
    REQUIRE(is_create);

    int read_fd = create_shm(shm_name, &is_create);
    REQUIRE(read_fd > 0);
    REQUIRE(is_create == false);

    REQUIRE(close(write_fd) == 0);
    REQUIRE(shm_unlink(normallize_shm_name(shm_name).c_str()) == 0);
    REQUIRE(close(read_fd) == 0);
}

TEST_CASE("shm_manager", "[simple]") {
    std::string shm_name = "test_simple_" + std::to_string(getpid());
    ShmManager<int> test_manager_write;
    ShmManager<int> test_manager_read;

    REQUIRE(test_manager_write.Publish(1) == false);

    REQUIRE(test_manager_write.Open(shm_name, 30));
    REQUIRE(test_manager_write.isCreate());
    REQUIRE(test_manager_read.Open(shm_name, 20) == false);

    REQUIRE(test_manager_read.Open(shm_name, 30));
    REQUIRE(test_manager_read.isCreate() == false);
    int data = 0;
    REQUIRE(test_manager_read.TryRead(0, &data) == false);

    REQUIRE(test_manager_write.Publish(1));

    uint64_t drq = 0;
    REQUIRE(test_manager_read.TryRead(drq, &data));
    REQUIRE(data == 1);

    REQUIRE(test_manager_read.Close());
    REQUIRE(test_manager_read.TryRead(drq, &data) == false);
    REQUIRE(test_manager_read.Open(shm_name, 30));
    REQUIRE(test_manager_write.Publish(1));

    REQUIRE(test_manager_read.TryRead(1, &data) == true);

    REQUIRE(test_manager_read.Close());
    REQUIRE(test_manager_write.Close());
}

TEST_CASE("shm_manager overwrites old slots", "[simple]") {
    std::string shm_name = "test_overwrite_" + std::to_string(getpid());
    ShmManager<int> manager;

    REQUIRE(manager.Open(shm_name, 2));

    REQUIRE(manager.Publish(10));
    REQUIRE(manager.Publish(20));
    REQUIRE(manager.Publish(30));

    int data = 0;
    REQUIRE(manager.TryRead(0, &data) == false);
    REQUIRE(manager.TryRead(1, &data));
    REQUIRE(data == 20);
    REQUIRE(manager.TryRead(2, &data));
    REQUIRE(data == 30);

    REQUIRE(manager.Close());
}


