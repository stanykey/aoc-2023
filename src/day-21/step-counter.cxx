#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <queue>
#include <ranges>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>


namespace {
    using Grid = std::vector<std::string_view>;

    struct Coordinate {
        std::int64_t row = 0;
        std::int64_t col = 0;

        auto operator==(const Coordinate& other) const -> bool = default;

        [[nodiscard]] Coordinate operator+(const Coordinate& other) const {
            return Coordinate{
                .row = row + other.row,
                .col = col + other.col,
            };
        }
    };
}  // namespace

template<>
struct std::hash<Coordinate> {
    auto operator()(const Coordinate& coordinate) const noexcept -> std::size_t {
        const auto row_hash = std::hash<std::int64_t>{}(coordinate.row);
        const auto col_hash = std::hash<std::int64_t>{}(coordinate.col);
        return row_hash ^ (col_hash << 1);
    }
};

namespace {
    auto find_start_point(const Grid& grid) -> Coordinate {
        for (auto row = 0; row != grid.size(); row++) {
            for (auto col = 0; col != grid[row].size(); col++) {
                if (grid[row][col] == 'S') {
                    return Coordinate{row, col};
                }
            }
        }
        return {};
    }

    auto find_all_reachable_coorditates(const Grid& grid) -> std::unordered_map<Coordinate, std::size_t> {
        auto visited = std::unordered_map<Coordinate, std::size_t>{};

        const auto can_step = [&](const Coordinate& pos) {
            if (pos.row < 0 or pos.row >= std::ssize(grid)) {
                return false;
            }

            if (pos.col < 0 or pos.col >= std::ssize(grid)) {
                return false;
            }

            if (grid[pos.row][pos.col] == '#') {
                return false;
            }

            return not visited.contains(pos);
        };

        auto queue = std::queue<std::pair<std::size_t, Coordinate>>{};
        queue.emplace(0, find_start_point(grid));
        while (not queue.empty()) {
            const auto [distance, coord] = queue.front();
            queue.pop();

            if (!visited.contains(coord)) {
                visited.emplace(coord, distance);

                for (const auto& dir : std::vector<Coordinate>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                    if (const auto next = coord + dir; can_step(next)) {
                        queue.emplace(distance + 1, next);
                    }
                }
            }
        }

        return visited;
    }

    auto count_reachable_plots(const Grid& grid, std::size_t steps) -> std::size_t {
        const auto visited = find_all_reachable_coorditates(grid);
        return std::ranges::count_if(visited | std::views::values, [steps](auto distance) {
            return distance <= steps and distance % 2 == 0;
        });
    }

    auto count_reachable_plots_on_infinitive_grid(const Grid& grid, std::size_t steps) -> std::size_t {
        // for more details go to
        //   - https://www.reddit.com/r/adventofcode/comments/18nol3m/2023_day_21_a_geometric_solutionexplanation_for/
        //   - https://github.com/villuna/aoc23/wiki/A-Geometric-solution-to-advent-of-code-2023,-day-21

        const auto visited = find_all_reachable_coorditates(grid);

        // clang-format off
        const auto even_corners = std::ranges::count_if(visited | std::views::values, [](auto distance) { return distance % 2 == 0 && distance > 65; });
        const auto odd_corners  = std::ranges::count_if(visited | std::views::values, [](auto distance) { return distance % 2 == 1 && distance > 65; });
        const auto even_full    = std::ranges::count_if(visited | std::views::values, [](auto distance) { return distance % 2 == 0; });
        const auto odd_full     = std::ranges::count_if(visited | std::views::values, [](auto distance) { return distance % 2 == 1; });
        // clang-format on

        const auto n      = (steps - (grid.size() / 2)) / grid.size();
        const auto answer = ((n + 1) * (n + 1)) * odd_full + (n * n) * even_full - (n + 1) * odd_corners + n * even_corners;

        return answer;
    }

}  // namespace

int main() {
    const auto path   = std::filesystem::path{"input.data"};
    const auto data   = core::io::read_file(path, true);
    const auto garden = core::strings::split(core::strings::strip(data), "\n");

    const auto step_count = 64ul;
    const auto plot_count = count_reachable_plots(garden, step_count);
    std::cout << std::format("The Elf could read {} plots in exactly {} steps\n", plot_count, step_count);

    const auto new_step_count = 26'501'365ul;
    const auto new_plot_count = count_reachable_plots_on_infinitive_grid(garden, new_step_count);
    std::cout << std::format("The Elf could read {} plots in exactly {} steps\n", new_plot_count, new_step_count);

    return 0;
}
