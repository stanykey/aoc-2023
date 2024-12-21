#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>


namespace {
    using Grid = std::vector<std::string_view>;

    struct Coordinate {
        std::int64_t row = 0;
        std::int64_t col = 0;

        auto operator==(const Coordinate& other) const -> bool = default;
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

    auto count_reachable_plots(const Grid& grid, std::size_t steps) -> std::size_t {
        auto next = std::unordered_set<Coordinate>{};
        next.insert(find_start_point(grid));

        const auto try_step = [&](const Coordinate& coord) {
            if (coord.row < 0 or coord.row >= std::ssize(grid)) {
                return;
            }

            if (coord.col < 0 or coord.col >= std::ssize(grid[coord.row])) {
                return;
            }

            // check for block
            if (grid[coord.row][coord.col] == '#') {
                return;
            }

            next.insert(coord);
        };

        auto current = std::unordered_set<Coordinate>{};
        std::ranges::for_each(std::views::iota(0ul, steps), [&](auto) {
            std::exchange(current, next);
            std::exchange(next, {});
            for (const auto& [row, col] : current) {
                try_step({row - 1, col});
                try_step({row + 1, col});
                try_step({row, col - 1});
                try_step({row, col + 1});
            }
        });
        return next.size();
    }
}  // namespace

int main() {
    const auto path   = std::filesystem::path{"input.data"};
    const auto data   = core::io::read_file(path, true);
    const auto garden = core::strings::split(core::strings::strip(data), "\n");

    const auto step_count = 64ul;
    const auto plot_count = count_reachable_plots(garden, step_count);
    std::cout << std::format("The Elf could read {} plots in exactly {} steps\n", plot_count, step_count);

    return 0;
}
