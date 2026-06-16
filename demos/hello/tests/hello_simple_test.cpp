#include "hello_support.hpp"

#include <iostream>

int main() {
    const auto message = hello::message_for("simple test");
    if (!hello::contains_hello(message)) {
        std::cerr << "expected message to contain hello" << std::endl;
        return 1;
    }

    return 0;
}
