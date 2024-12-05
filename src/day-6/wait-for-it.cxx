#include <core/io.hxx>
#include <core/numbers.hxx>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>


namespace {
    auto read_values(std::istream& stream) -> std::vector<std::uint64_t> {
        const auto record = core::io::read_line(stream);
        return core::numbers::parse_numbers<std::uint64_t>({record.data() + record.find(':') + 1});
    }

    auto read_value(std::istream& stream) -> std::uint64_t {
        const auto record = core::io::read_line(stream);

        auto value = std::string{};
        std::ranges::copy_if(record, std::back_inserter(value), [](char symbol) { return std::isdigit(symbol); });
        return core::numbers::parse<std::uint64_t>(value);
    }

    auto count_ways_to_beat_record(std::uint64_t duration, std::uint64_t record) -> std::uint64_t {
        // Calculate the roots of the quadratic equation
        const auto discriminant      = duration * duration - 4 * record;
        const auto sqrt_discriminant = std::sqrt(discriminant);
        const auto root1             = static_cast<double>(duration) - sqrt_discriminant / 2.0;
        const auto root2             = static_cast<double>(duration) + sqrt_discriminant / 2.0;

        // Count integer solutions in the range (root1, root2)
        const auto lower_bound = static_cast<std::uint64_t>(std::ceil(root1));
        const auto upper_bound = static_cast<std::uint64_t>(std::floor(root2));
        if (lower_bound > upper_bound) {
            return 0;  // No valid integers in the range
        }

        return upper_bound - lower_bound + 1;
    }

    auto get_races_result(const std::string& path) -> std::uint64_t {
        auto stream = std::ifstream{path};

        const auto durations = read_values(stream);
        const auto distances = read_values(stream);

        auto result = 1ul;
        for (auto i = 0ul; i != durations.size(); i++) {
            result *= count_ways_to_beat_record(durations[i], distances[i]);
        }
        return result;
    }

    auto get_race_result(const std::string& path) -> std::uint64_t {
        auto stream = std::ifstream{path};

        const auto duration = read_value(stream);
        const auto distance = read_value(stream);

        return count_ways_to_beat_record(duration, distance);
    }

}  // namespace

auto main() -> int {
    const auto input_path = std::string{"input.data"};

    const auto races_result = get_races_result(input_path);
    std::cout << std::format("The result for multiple races is {}\n", races_result);

    const auto race_result = get_race_result(input_path);
    std::cout << std::format("The result for single races is {}\n", race_result);

    return 0;
}
