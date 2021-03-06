#pragma once

#include <string>
#include <vector>

namespace dc
{

bool contains(const std::string &str, char character);

std::string string_trim_left(const std::string &str);

std::string string_trim_right(const std::string &str);

std::string string_trim(const std::string &str);

std::vector<std::string> string_split(const std::string &str,
                                      const std::string &delimiter);

} // namespace dc
