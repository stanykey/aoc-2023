#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <vector>


namespace {
    using Schema = std::vector<std::string>;


    constexpr auto DIGITS      = std::string_view{"0123456789"};
    const auto     NON_SYMBOLS = std::set<char>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};


    auto to_number(std::string_view str) -> std::uint32_t {
        std::uint32_t number = 0;
        if (std::from_chars(str.data(), str.data() + str.size(), number).ec != std::errc{}) {
            throw std::runtime_error(std::format("Failed to parse integer from <{}>", str));
        }
        return number;
    }

    auto load_engine_schematic(const std::string& path) -> Schema {
        auto schema = Schema{};

        std::string   line;
        std::ifstream stream{path};
        while (std::getline(stream, line)) {
            schema.emplace_back(line);
        }

        return schema;
    }

    auto find_next_number(std::string_view str, std::size_t offset) -> std::tuple<std::string_view, std::size_t> {
        const auto start = str.find_first_of(DIGITS, offset);
        if (start == std::string_view::npos) {
            return {std::string_view{}, start};
        }

        const auto end = str.find_first_not_of(DIGITS, start);
        return {str.substr(start, end - start), start};
    }

    auto is_part_number(const Schema& schema, std::string_view number, std::size_t row, std::size_t col) -> bool {
        const auto rows          = schema.size();
        const auto number_length = number.size();

        for (auto y = row > 0 ? (row - 1) : 0ul; (y <= row + 1) && (y < rows); y++) {
            const auto& line = schema[y];

            const auto end_col = std::min(col + number_length, line.size());
            for (auto x = (col > 0) ? (col - 1) : 0ul; x <= end_col; ++x) {
                if (y == row && x >= col && x < col + number_length) {
                    continue;  // Skip the number itself
                }

                if ((x < line.size()) && !NON_SYMBOLS.contains(line[x])) {
                    return true;  // Found a symbol
                }
            }
        }

        return false;
    }

    auto get_part_numbers(const Schema& schema) -> std::vector<std::uint32_t> {
        auto part_numbers = std::vector<std::uint32_t>{};

        for (auto i = 0u; i != schema.size(); i++) {
            const auto line = std::string_view{schema[i]};

            auto [number, pos] = find_next_number(line, 0);
            while (!number.empty()) {
                if (is_part_number(schema, number, i, pos)) {
                    part_numbers.emplace_back(to_number(number));
                }

                std::tie(number, pos) = find_next_number(line, pos + number.size() + 1);
            }
        }

        return part_numbers;
    }
}  // namespace


auto main() -> int {
    try {
        const auto schema = load_engine_schematic("input.data");

        const auto part_numbers     = get_part_numbers(schema);
        const auto part_numbers_sum = std::reduce(part_numbers.cbegin(), part_numbers.cend());
        std::cout << std::format("The sum of part numbers in the engine schematic is {}\n", part_numbers_sum);
    } catch (const std::exception& ex) {  // NOLINT: std::exception if fine here
        std::cerr << std::format("Critical error: {}\n", ex.what());
        return 1;
    }

    return 0;
}
