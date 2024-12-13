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
#include <string_view>
#include <unordered_set>
#include <vector>


namespace {
    using Grid = std::vector<std::string_view>;

    struct Coordinate {
        std::int64_t row = 0;
        std::int64_t col = 0;

        auto operator<=>(const Coordinate& other) const = default;

        [[nodiscard]] Coordinate operator+(const Coordinate& other) const {
            return Coordinate{
                .row = row + other.row,
                .col = col + other.col,
            };
        }
    };

    struct Beam {
        Coordinate position;
        Coordinate direction;
        auto       operator<=>(const Beam&) const = default;
    };
}  // namespace

template<>
struct std::hash<Coordinate> {
    auto operator()(const Coordinate& coordinate) const noexcept -> std::size_t {
        const auto first  = std::hash<std::int64_t>{}(coordinate.row);
        const auto second = std::hash<std::int64_t>{}(coordinate.col);
        return first ^ (second << 1);
    }
};

template<>
struct std::hash<Beam> {
    auto operator()(const Beam& beam) const noexcept -> std::size_t {
        const auto first  = std::hash<Coordinate>{}(beam.position);
        const auto second = std::hash<Coordinate>{}(beam.direction);
        return first ^ (second << 1);
    }
};

namespace {
    auto energized_tiles(const Grid& grid, Coordinate start_pos, Coordinate start_dir) -> std::int64_t {
        auto queue     = std::queue<Beam>{};
        auto visited   = std::unordered_set<Beam>{};
        auto energized = std::unordered_set<Coordinate>{};

        const auto can_head_next = [&](const Beam& beam) {
            if (beam.position.row < 0 || beam.position.col < 0) {
                return false;
            }

            if (beam.position.row >= std::ssize(grid)) {
                return false;
            }

            if (beam.position.col >= std::ssize(grid[0])) {
                return false;
            }

            return not visited.contains(beam);
        };

        const auto continue_ray = [&](Coordinate pos, Coordinate dir) {
            pos = pos + dir;
            if (can_head_next({pos, dir})) {
                visited.emplace(pos, dir);
                queue.emplace(pos, dir);
                energized.emplace(pos);
            }
        };

        // prepare for the old good one BFS
        queue.emplace(start_pos, start_dir);
        visited.emplace(start_pos, start_dir);
        energized.emplace(start_pos);
        while (not queue.empty()) {
            const auto [pos, dir] = queue.front();
            queue.pop();

            switch (grid[pos.row][pos.col]) {
                case '.': {  // just continue
                    continue_ray(pos, dir);
                    break;
                }

                case '-': {  // vertical splitter
                    if (dir.row == 0) {
                        continue_ray(pos, dir);
                    } else {
                        continue_ray(pos, {0, -1});
                        continue_ray(pos, {0, 1});
                    }
                    break;
                }

                case '|': {  // horizontal splitter
                    if (dir.col == 0) {
                        continue_ray(pos, dir);
                    } else {
                        continue_ray(pos, {1, 0});
                        continue_ray(pos, {-1, 0});
                    }
                    break;
                }

                case '/': {  // 90-degree turn upward
                    continue_ray(pos, {-dir.col, -dir.row});
                    break;
                }

                case '\\': {  // 90-degree turn downward
                    continue_ray(pos, {dir.col, dir.row});
                    break;
                }
            }
        }

        return std::ssize(energized);
    }

    auto energize_tiles_with_sides(const Grid& grid) -> std::int64_t {
        auto max_result = 0ll;

        const auto grid_rows = std::ssize(grid);
        const auto grid_cols = std::ssize(grid[0]);

        // handle top row (downward)
        for (auto col = 0ll; col < grid_cols; col++) {
            max_result = std::max(max_result, energized_tiles(grid, {0, col}, {1, 0}));
        }

        // handle bottom row (upward)
        for (auto col = 0ll; col < grid_cols; col++) {
            max_result = std::max(max_result, energized_tiles(grid, {grid_rows - 1, col}, {-1, 0}));
        }

        // handle left column (rightward)
        for (auto row = 0ll; row < grid_rows; row++) {
            max_result = std::max(max_result, energized_tiles(grid, {row, 0}, {0, 1}));
        }

        // handle right column (leftward)
        for (auto row = 0ll; row < grid_rows; row++) {
            max_result = std::max(max_result, energized_tiles(grid, {row, grid_cols - 1}, {0, -1}));
        }

        return max_result;
    }
}  // namespace


int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto data = core::io::read_file(path, true);
    const auto map  = core::strings::split(core::strings::strip(data), "\n");

    std::cout << std::format("The number of energized tiles is : {}\n", energized_tiles(map, {0, 0}, {0, 1}));
    std::cout << std::format("The number of energized tiles with sides is : {}\n", energize_tiles_with_sides(map));

    return 0;
}
