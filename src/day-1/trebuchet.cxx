#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>


namespace {
    auto load_data(const std::string& path) -> std::vector<std::string> {
        std::ifstream file{path};

        std::vector<std::string> data;
        std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(data));
        return data;
    }


    auto extract_digits(const std::string& str) -> std::vector<std::int32_t> {
        // Ordered list of spelled-out digits, longest words first to handle overlaps
        const std::vector<std::tuple<std::string_view, int>> digit_words = {
            {"eight", 8}, {"seven", 7}, {"three", 3}, {"nine", 9}, {"four", 4},
            {"five", 5},  {"six", 6},   {"two", 2},   {"one", 1},
        };

        std::vector<std::int32_t> digits;
        for (auto i = 0ul; i < str.size();) {
            // If no word matched, check for single numeric characters
            if (std::isdigit(str[i]) != 0) {
                digits.push_back(str[i] - '0');  // Convert char to int
                i++;
                continue;
            }

            // Check for spelled-out digits first
            bool found = false;
            for (const auto& [word, value] : digit_words) {
                if (str.substr(i, word.size()) == word) {
                    digits.push_back(value);
                    i += word.size() - 1;  // Move past the matched word, but without one letter due to potential overlaps
                    found = true;
                    break;
                }
            }

            if (!found) {
                i++;
            }
        }

        return digits;
    }


    auto get_calibration_digits(const std::string& str) -> std::optional<std::tuple<std::int32_t, std::int32_t>> {
        auto matches = extract_digits(str);
        if (matches.empty()) {
            // No digits found
            return std::nullopt;
        }

        return std::make_tuple(matches.front(), matches.back());
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
