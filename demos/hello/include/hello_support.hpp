#pragma once

#include <string>

namespace hello {

std::string message_for(const std::string& suffix);
bool contains_hello(const std::string& message);

}  // namespace hello
