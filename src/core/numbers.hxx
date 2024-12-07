#ifndef CORE_NUMBERS_HXX
#define CORE_NUMBERS_HXX

#include <core/strings.hxx>

#include <charconv>
#include <format>
#include <istream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


namespace core::numbers {
    template<typename Number>
    auto parse(std::string_view str) -> Number {
        str = strings::strip(str);

        Number number = 0;
        if (std::from_chars(str.data(), str.data() + str.size(), number).ec != std::errc{}) {
            throw std::runtime_error(std::format("Failed to parse integer from <{}>", str));
        }
        return number;
    }

    template<typename Number>
    auto parse_numbers(std::istream& stream) -> std::vector<Number> {
        auto numbers = std::vector<Number>{};
        std::copy(std::istream_iterator<Number>{stream}, std::istream_iterator<Number>{}, std::back_inserter(numbers));
        return numbers;
    }

    template<typename Number>
    auto parse_numbers(std::string_view record) -> std::vector<Number> {
        auto stream = std::istringstream{std::string{record}};
        return parse_numbers<Number>(stream);
    }

}  // namespace core::numbers

#endif  // CORE_NUMBERS_HXX
