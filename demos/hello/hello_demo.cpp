#include "hello_support.hpp"

#include <iostream>

int main() {
    std::cout << hello::message_for("demo") << std::endl;
    return 0;
}
