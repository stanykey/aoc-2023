#include <core/numbers.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>


namespace {
    using Schema      = std::vector<std::string>;
    using Coordinate  = std::tuple<std::size_t, std::size_t>;
    using PartNumbers = std::vector<std::uint32_t>;
    using Parts       = std::map<Coordinate, PartNumbers>;


    constexpr auto DIGITS      = std::string_view{"0123456789"};
    const auto     NON_SYMBOLS = std::set<char>{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};


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

    auto is_part_number(const Schema& schema, std::string_view number, std::size_t row, std::size_t col)
        -> std::tuple<bool, std::size_t, std::size_t> {
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
                    return {true, y, x};  // Found a symbol
                }
            }
        }

        return {false, std::string_view::npos, std::string_view::npos};
    }

    auto get_parts(const Schema& schema) -> Parts {
        auto parts = Parts{};

        for (auto i = 0u; i != schema.size(); i++) {
            const auto line = std::string_view{schema[i]};

            auto [number, pos] = find_next_number(line, 0);
            while (!number.empty()) {
                const auto [ok, row, col] = is_part_number(schema, number, i, pos);
                if (ok) {
                    parts[{row, col}].emplace_back(core::numbers::parse<std::uint32_t>(number));
                }

                std::tie(number, pos) = find_next_number(line, pos + number.size() + 1);
            }
        }

        return parts;
    }

    auto get_part_numbers(const Parts& parts) -> PartNumbers {
        auto part_numbers = PartNumbers{};
        for (const auto& [symbol, numbers] : parts) {
            part_numbers.insert(part_numbers.end(), numbers.cbegin(), numbers.cend());
        }
        return part_numbers;
    }

    auto get_gear_ratios(const Schema& schema, const Parts& parts) -> std::vector<std::uint32_t> {
        auto ratios = std::vector<std::uint32_t>{};
        for (const auto& [coordinate, numbers] : parts) {
            const auto [row, col] = coordinate;
            if ((schema[row][col] == '*') && (numbers.size() == 2)) {
                ratios.emplace_back(numbers.front() * numbers.back());
            }
        }

        return ratios;
    }
}  // namespace


auto main() -> int {
    try {
        const auto schema = load_engine_schematic("input.data");

        const auto parts            = get_parts(schema);
        const auto part_numbers     = get_part_numbers(parts);
        const auto part_numbers_sum = std::reduce(part_numbers.cbegin(), part_numbers.cend());
        std::cout << std::format("The sum of part numbers in the engine schematic is {}\n", part_numbers_sum);

        const auto gear_ratios     = get_gear_ratios(schema, parts);
        const auto gear_ratios_sum = std::reduce(gear_ratios.cbegin(), gear_ratios.cend());
        std::cout << std::format("The sum of gear ratios in the engine schematic is {}\n", gear_ratios_sum);
    } catch (const std::exception& ex) {  // NOLINT: std::exception if fine here
        std::cerr << std::format("Critical error: {}\n", ex.what());
        return 1;
    }

    return 0;
}
