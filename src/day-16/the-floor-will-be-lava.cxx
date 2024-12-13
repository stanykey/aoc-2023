#include <core/io.hxx>
#include <core/strings.hxx>

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
    auto energized_tiles(const Grid& grid) -> std::int64_t {
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
            if (can_head_next(Beam{pos + dir, dir})) {
                visited.emplace(pos + dir, dir);
                queue.emplace(pos + dir, dir);
                energized.emplace(pos + dir);
            }
        };

        // prepare for the old good one BFS
        queue.push(Beam{{0, 0}, {0, 1}});
        visited.insert(Beam{{0, 0}, {0, 1}});
        energized.insert(Coordinate{0, 0});
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
}  // namespace


int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto data = core::io::read_file(path, true);
    const auto map  = core::strings::split(core::strings::strip(data), "\n");

    std::cout << std::format("The number of energized tiles is : {}\n", energized_tiles(map));

    return 0;
}
