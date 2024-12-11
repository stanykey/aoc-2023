#include <core/io.hxx>
#include <core/strings.hxx>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace {
    auto find_total_load(const std::vector<std::string_view>& grid) -> std::uint64_t {
        auto cubed_rock_positions = std::vector<std::int64_t>(grid[0].size(), -1);
        auto rounded_rock_counts  = std::vector<std::size_t>(grid[0].size(), 0);

        auto weight = 0ull;
        for (auto row = 0u; row < grid.size(); row++) {
            for (auto col = 0u; col < grid[row].size(); col++) {
                if (grid[row][col] == 'O') {
                    rounded_rock_counts[col]++;
                } else if (grid[row][col] == '#') {
                    while (rounded_rock_counts[col] > 0) {
                        cubed_rock_positions[col]++;
                        weight += std::ssize(grid) - cubed_rock_positions[col];
                        rounded_rock_counts[col]--;
                    }
                    cubed_rock_positions[col] = row;
                }
            }
        }

        for (auto col = 0ll; col < rounded_rock_counts.size(); col++) {
            while (rounded_rock_counts[col] > 0) {
                cubed_rock_positions[col]++;
                weight += std::ssize(grid) - cubed_rock_positions[col];
                rounded_rock_counts[col]--;
            }
        }

        return weight;
    }

}  // namespace


int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto map  = core::io::read_file(path, true);
    const auto grid = core::strings::split(core::strings::strip(map), "\n");

    const auto start_time = std::chrono::high_resolution_clock::now();
    const auto total_load = find_total_load(grid);
    std::cout << std::format("The total load is {}\n", total_load);
    const auto time_elapsed = std::chrono::high_resolution_clock::now() - start_time;
    std::cout << std::format("Time elapsed {}\n", time_elapsed);

    return 0;
}
