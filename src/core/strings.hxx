#ifndef CORE_STRINGS_HXX
#define CORE_STRINGS_HXX

#include <string_view>
#include <vector>


namespace core::strings {
    auto split(std::string_view source, std::string_view delimiter) -> std::vector<std::string_view>;
    auto strip(std::string_view str, std::string_view chars = "\n\r\t ") -> std::string_view;
}  // namespace core::strings


#endif  // CORE_STRINGS_HXX
