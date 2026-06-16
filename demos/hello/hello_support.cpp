#include "hello_support.hpp"

namespace hello {

std::string message_for(const std::string& suffix) {
    return "hello " + suffix;
}

bool contains_hello(const std::string& message) {
    return message.find("hello") != std::string::npos;
}

}  // namespace hello
