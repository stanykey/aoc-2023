#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace {
    template<std::ranges::random_access_range Range>
    auto shift(Range range) -> void {
        const auto is_rock = [](char symbol) { return symbol != '.'; };
        for (auto destination = std::ranges::find(range, '.'); destination != range.end();) {
            auto source = std::find_if(destination, range.end(), is_rock);
            if (source == range.end()) {
                return;
            }

            if (*source == 'O') {
                // move the rounded rock to the empty space under cubed one
                std::iter_swap(source, destination);
            } else if (*source == '#') {
                // move destination to next position past source
                destination = std::next(source);
            }

            destination = std::find(destination, range.end(), '.');
        }
    }

    auto simulate(const std::vector<std::string_view>& grid, std::size_t count) -> std::vector<char> {
        const auto row_count = grid.size();
        const auto col_count = grid[0].size();

        auto flatten_grid = std::vector<char>{};
        flatten_grid.reserve(row_count * col_count);
        std::ranges::copy(grid | std::views::join, std::back_inserter(flatten_grid));

        const auto make_spin = [&]() {
            for (auto col = 0ul; col < col_count; ++col) {  // north
                shift(flatten_grid | std::views::drop(col) | std::views::stride(col_count));
            }

            for (auto row : flatten_grid | std::views::chunk(col_count)) {  // west
                shift(row);
            }

            for (auto col = 0ul; col < col_count; ++col) {  // south
                shift(flatten_grid | std::views::drop(col) | std::views::stride(col_count) | std::views::reverse);
            }

            for (auto row : flatten_grid | std::views::chunk(col_count)) {  // east
                shift(row | std::views::reverse);
            }
        };

        auto visited = std::unordered_map<std::string, std::int64_t>{};
        auto prev    = std::vector<char>{};
        auto ticks   = 0ll;
        auto done    = false;
        while (!done) {
            visited.emplace(std::string{flatten_grid.begin(), flatten_grid.end()}, ticks);
            ticks++;
            make_spin();

            const auto it = visited.find(std::string{flatten_grid.begin(), flatten_grid.end()});
            if (it != visited.end()) {
                // iterate until a clean loop multiple
                for (auto remaining = (count - ticks) % (ticks - it->second); remaining > 0; remaining--) {
                    make_spin();
                }
                done = true;
            }
        }

        return flatten_grid;
    }

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

    auto find_total_load(const std::vector<std::string_view>& grid, std::size_t simulations_count) -> std::uint64_t {
        const auto row_count              = grid.size();
        const auto col_count              = grid[0].size();
        const auto simulated_flatten_grid = simulate(grid, simulations_count);

        auto weight = 0ull;
        for (auto [idx, row] : simulated_flatten_grid | std::views::chunk(col_count) | std::views::enumerate) {
            weight += std::ranges::count(row, 'O') * (row_count - idx);
        }

        return weight;
    }
}  // namespace


int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto map  = core::io::read_file(path, true);
    const auto grid = core::strings::split(core::strings::strip(map), "\n");

    const auto first_start_time = std::chrono::high_resolution_clock::now();
    const auto total_load       = find_total_load(grid);
    std::cout << std::format("The total load is {}\n", total_load);
    const auto first_time_elapsed = std::chrono::high_resolution_clock::now() - first_start_time;
    std::cout << std::format("Time elapsed {}\n", first_time_elapsed);

    const auto second_start_time           = std::chrono::high_resolution_clock::now();
    const auto simulations                 = 1'000'000'000ul;
    const auto total_load_with_simulations = find_total_load(grid, simulations);
    std::cout << std::format("The total load after {} simulations is {}\n", simulations, total_load_with_simulations);
    const auto second_time_elapsed = std::chrono::high_resolution_clock::now() - second_start_time;
    std::cout << std::format("Time elapsed {}\n", second_time_elapsed);

    return 0;
}
