#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <vector>


namespace {
    auto load_data(const std::string& path) -> std::vector<std::string> {
        std::ifstream file{path};

        std::vector<std::string> data;
        std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(data));
        return data;
    }

    auto get_calibration_digits(const std::string& str) -> std::optional<std::tuple<std::int32_t, std::int32_t>> {
        const auto pattern = std::regex{R"(\d)"};

        auto matches = std::vector<std::string>{};
        auto begin   = std::sregex_iterator{str.cbegin(), str.cend(), pattern};
        for (const auto end = std::sregex_iterator{}; begin != end; ++begin) {
            matches.emplace_back(begin->str());
        }

        if (matches.empty()) {
            // No digits found
            return std::nullopt;
        }

        const auto first_digit = matches.front()[0] - '0';
        const auto last_digit  = matches.back()[0] - '0';
        return std::make_tuple(first_digit, last_digit);
    }


    auto get_calibration_value(const std::string& str) -> std::int32_t {
        const auto digits = get_calibration_digits(str);
        if (!digits) {
            return 0;
        }

        const auto [first, second] = digits.value();
        return first * 10 + second;  // NOLINT: no reasonable name for 10 `magic` number
    }


    auto get_calibration_values(const std::vector<std::string>& data) -> std::vector<std::int32_t> {
        std::vector<std::int32_t> calibrations;
        calibrations.reserve(data.size());
        std::ranges::transform(data, std::back_inserter(calibrations), get_calibration_value);
        return calibrations;
    }
}  // namespace


auto main() -> int {
    const auto data         = load_data("input.data");
    const auto calibrations = get_calibration_values(data);
    const auto result       = std::reduce(calibrations.cbegin(), calibrations.cend());
    std::cout << std::format("The sum of all calibration values is {}", result);
    return 0;
}
