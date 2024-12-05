#include <core/io.hxx>
#include <core/numbers.hxx>

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>


namespace {
    auto read_values(std::istream& stream) -> std::vector<std::uint64_t> {
        const auto record = core::io::read_line(stream);
        return core::numbers::parse_numbers<std::uint64_t>({record.data() + record.find(':') + 1});
    }

    auto count_ways_to_beat_record(std::uint64_t duration, std::uint64_t record) -> std::uint64_t {
        auto ways_to_win = 0ul;

        for (int hold_time = 0; hold_time <= duration; ++hold_time) {
            const auto remaining_time = duration - hold_time;
            const auto distance       = hold_time * remaining_time;
            if (distance > record) {
                ways_to_win++;
            }
        }

        return ways_to_win;
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

}  // namespace

auto main() -> int {
    const auto input_path = std::string{"input.data"};

    const auto races_result = get_races_result(input_path);
    std::cout << std::format("The result for multiple races is {}\n", races_result);

    return 0;
}
