#include <core/io.hxx>
#include <core/strings.hxx>

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


namespace {
    using Grid = std::vector<std::string_view>;

    struct Coordinate {
        std::int64_t row = 0;
        std::int64_t col = 0;

        auto operator==(const Coordinate& other) const -> bool = default;

        [[nodiscard]] constexpr Coordinate operator+(const Coordinate& other) const {
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
    auto find_start_position(const Grid& grid) -> Coordinate {
        for (auto row = 0ul; row != grid.size(); row++) {
            for (auto col = 0ul; col < grid[0].size(); col++) {
                if (grid[row][col] == 'S') {
                    return {
                        .row = static_cast<std::int64_t>(row),
                        .col = static_cast<std::int64_t>(col),
                    };
                }
            }
        }
        return {};
    }

    auto count_numbers_of_step(std::string_view map) -> std::size_t {
        const auto grid = core::strings::split(core::strings::strip(map), "\n");

        // the shape->offsets mapping
        auto offsets = std::unordered_map<char, std::pair<Coordinate, Coordinate>>{
            {'|', {{-1, 0}, {+1, 0}}},  // north <-> south
            {'-', {{0, -1}, {0, +1}}},  // west  <-> east
            {'L', {{-1, 0}, {0, +1}}},  // north <-> east
            {'J', {{-1, 0}, {0, -1}}},  // north <-> west
            {'7', {{+1, 0}, {0, -1}}},  // south <-> west
            {'F', {{+1, 0}, {0, +1}}},  // south <-> east
        };

        // BFS:
        // track of visited spaces and distance we visited them
        auto visited = std::unordered_map<Coordinate, std::int64_t>{};
        auto queue   = std::queue<std::pair<Coordinate, std::int64_t>>{};
        queue.emplace(find_start_position(grid), 0);
        visited[queue.front().first] = 0;

        auto can_step = [&](Coordinate from, Coordinate to) {
            if (to.row < 0 || to.col < 0) {
                return false;
            }

            if (to.row >= std::ssize(grid)) {
                return false;
            }

            if (to.col >= std::ssize(grid[to.row])) {
                return false;
            }

            // does the destination pipe connect to the source?
            auto [first, second] = offsets[grid[to.row][to.col]];
            return !((from != to + first) && (from != to + second));
        };

        // helper that will return true if we have reached the destination space;
        // otherwise, it will check if the coordinate can be visited and, if yes, insert it into the queue
        auto check_move = [&](Coordinate from, Coordinate destination, std::int64_t distance) {
            if (not can_step(from, destination)) {
                return false;
            }

            const auto it = visited.find(destination);
            if (it != visited.end()) {
                return it->second == distance;
            }

            queue.emplace(destination, distance);
            visited[destination] = distance;
            return false;
        };

        while (not queue.empty()) {
            const auto [coord, distance] = queue.front();
            queue.pop();

            // expand into each direction -  two of the directions must succeed only!
            if (grid[coord.row][coord.col] == 'S') {
                check_move(coord, {coord.row + 1, coord.col}, distance + 1);
                check_move(coord, {coord.row - 1, coord.col}, distance + 1);
                check_move(coord, {coord.row, coord.col + 1}, distance + 1);
                check_move(coord, {coord.row, coord.col - 1}, distance + 1);
                continue;
            }

            // there is nothing to do in empty spaces, but we should never step into a space
            if (grid[coord.row][coord.col] == '.') {
                continue;
            }

            // try to visit the two directions the current shape connects to
            const auto new_distance    = distance + 1;
            const auto [first, second] = offsets[grid[coord.row][coord.col]];
            if (check_move(coord, coord + first, new_distance) || check_move(coord, coord + second, new_distance)) {
                return new_distance;
            }
        }
        return 0;
    }
}  // namespace

int main() {
    const auto path = std::string{"input.data"};
    const auto map  = core::io::read_file(path, true);
    const auto grid = core::strings::split(map, "\n");

    const auto steps_numbers = count_numbers_of_step(map);
    std::cout << std::format("The farthest point is {} steps away\n", steps_numbers);
    return 0;
}
