#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>


auto load_data(const std::string& path) -> std::vector<std::string> {
    std::ifstream file(path);

    std::vector<std::string> data;
    std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(data));
    return data;
}


constexpr auto find_first_digit(std::string_view str) -> std::int32_t {
    const auto it = std::find_if(str.cbegin(), str.cend(), [](const char ch) { return std::isdigit(ch); });
    return (it == str.cend()) ? 0 : *it - '0';
}


constexpr auto find_last_digit(std::string_view str) -> std::int32_t {
    const auto it = std::find_if(str.crbegin(), str.crend(), [](const char ch) { return std::isdigit(ch); });
    return (it == str.crend()) ? 0 : *it - '0';
}


constexpr auto get_calibration_value(std::string_view data) -> std::int32_t {
    const auto first = find_first_digit(data);
    const auto last  = find_last_digit(data);
    return first * 10 + last;  // NOLINT: no reasonable name for 10 `magic` number
}


auto get_calibration_values(const std::vector<std::string>& data) -> std::vector<std::int32_t> {
    std::vector<std::int32_t> calibrations;
    calibrations.reserve(data.size());
    std::ranges::transform(data, std::back_inserter(calibrations), get_calibration_value);
    return calibrations;
}


auto main() -> int {
    const auto data         = load_data("input.data");
    const auto calibrations = get_calibration_values(data);
    const auto result       = std::reduce(calibrations.cbegin(), calibrations.cend());
    std::cout << std::format("The sum of all calibration values is {}", result);
    return 0;
}
