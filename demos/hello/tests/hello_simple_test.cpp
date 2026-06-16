#include <iostream>
#include <string>

int main() {
    const std::string message = "hello simple test";
    if (message.find("hello") == std::string::npos) {
        std::cerr << "expected message to contain hello" << std::endl;
        return 1;
    }

    return 0;
}
