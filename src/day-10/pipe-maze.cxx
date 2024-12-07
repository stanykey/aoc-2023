#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
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
#include <unordered_set>
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
    const auto OFFSETS_MAPPING = std::unordered_map<char, std::pair<Coordinate, Coordinate>>{
        {'|', {{-1, 0}, {+1, 0}}},  // north <-> south
        {'-', {{0, -1}, {0, +1}}},  // west  <-> east
        {'L', {{-1, 0}, {0, +1}}},  // north <-> east
        {'J', {{-1, 0}, {0, -1}}},  // north <-> west
        {'7', {{+1, 0}, {0, -1}}},  // south <-> west
        {'F', {{+1, 0}, {0, +1}}},  // south <-> east
    };

    auto get_tile_directions(char tile) -> std::pair<Coordinate, Coordinate> {
        if (OFFSETS_MAPPING.contains(tile)) {
            return OFFSETS_MAPPING.at(tile);
        }
        return {};
    }

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

    auto check_step(const Grid& grid, Coordinate from, Coordinate to) -> bool {
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
        auto [first, second] = get_tile_directions(grid[to.row][to.col]);
        return !((from != to + first) && (from != to + second));
    };

    auto find_start_tile(const Grid& grid, Coordinate start) -> char {
        if (check_step(grid, start, {start.row - 1, start.col}) && check_step(grid, start, {start.row + 1, start.col})) {
            return '|';
        }

        if (check_step(grid, start, {start.row, start.col - 1}) && check_step(grid, start, {start.row, start.col + 1})) {
            return '-';
        }

        if (check_step(grid, start, {start.row - 1, start.col}) && check_step(grid, start, {start.row, start.col + 1})) {
            return 'L';
        }

        if (check_step(grid, start, {start.row - 1, start.col}) && check_step(grid, start, {start.row, start.col - 1})) {
            return 'J';
        }

        if (check_step(grid, start, {start.row + 1, start.col}) && check_step(grid, start, {start.row, start.col + 1})) {
            return 'F';
        }

        if (check_step(grid, start, {start.row + 1, start.col}) && check_step(grid, start, {start.row, start.col - 1})) {
            return '7';
        }

        return '.';
    }

    auto count_numbers_of_step(const Grid& grid) -> std::size_t {
        // BFS:
        // track of visited spaces and distance we visited them
        auto visited = std::unordered_map<Coordinate, std::int64_t>{};
        auto queue   = std::queue<std::pair<Coordinate, std::int64_t>>{};
        queue.emplace(find_start_position(grid), 0);
        visited[queue.front().first] = 0;

        const auto try_move = [&](Coordinate from, Coordinate destination, std::int64_t distance) {
            if (not check_step(grid, from, destination)) {
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
                try_move(coord, {coord.row + 1, coord.col}, distance + 1);
                try_move(coord, {coord.row - 1, coord.col}, distance + 1);
                try_move(coord, {coord.row, coord.col + 1}, distance + 1);
                try_move(coord, {coord.row, coord.col - 1}, distance + 1);
                continue;
            }

            // there is nothing to do in empty spaces, but we should never step into a space
            if (grid[coord.row][coord.col] == '.') {
                continue;
            }

            // try to visit the two directions the current shape connects to
            const auto new_distance    = distance + 1;
            const auto [first, second] = get_tile_directions(grid[coord.row][coord.col]);
            if (try_move(coord, coord + first, new_distance) || try_move(coord, coord + second, new_distance)) {
                return new_distance;
            }
        }
        return 0;
    }

    auto count_enclosed_tiles(const Grid& grid) -> std::size_t {
        auto       visited    = std::unordered_map<Coordinate, std::int64_t>{};
        auto       queue      = std::queue<std::pair<Coordinate, std::int64_t>>{};
        const auto start      = find_start_position(grid);
        const auto start_tile = find_start_tile(grid, start);
        queue.emplace(start, 0);
        visited[start] = 0;

        const auto get_tile = [&](Coordinate pos) {
            const auto tile = grid[pos.row][pos.col];
            if (tile == 'S') {
                return start_tile;
            }
            return tile;
        };

        while (!queue.empty()) {
            const auto [coord, distance] = queue.front();
            queue.pop();

            const auto tile = get_tile(coord);
            if (tile == '.') {
                continue;  // Skip empty spaces
            }

            // Process both directions for the current tile
            const auto [first, second] = get_tile_directions(tile);
            const auto next_distance   = distance + 1;

            const auto try_move = [&](Coordinate next) {
                if (check_step(grid, coord, next) && !visited.contains(next)) {
                    visited[next] = next_distance;
                    queue.emplace(next, next_distance);
                }
            };

            try_move(coord + first);
            try_move(coord + second);
        }


        // Prepare for the expanded BFS to count reachable tiles outside the loop
        auto expanded_visited = std::unordered_set<Coordinate>{};
        auto expanded_queue   = std::queue<Coordinate>{};

        expanded_queue.emplace(0, 0);  // Start BFS from the top-left corner outside the grid
        expanded_visited.emplace(0, 0);

        const auto can_expand = [&](Coordinate pos) {
            // out of bounds check
            if ((pos.row < 0) || (pos.col < 0) || (pos.row >= grid.size() * 2) || (pos.col >= grid[0].size() * 2)) {
                return false;
            }

            // avoid revisiting coordinates
            if (expanded_visited.contains(pos)) {
                return false;
            }

            // check if the position is part of the loop (original grid coordinates)
            if (pos.row % 2 != 0 && pos.col % 2 != 0) {
                const auto original_pos = Coordinate{
                    .row = (pos.row - 1) / 2,
                    .col = (pos.col - 1) / 2,
                };
                return !visited.contains(original_pos);
            }

            // vertical case: even row, odd column
            if (pos.row % 2 == 0 && pos.col % 2 != 0) {
                const auto north = Coordinate{
                    .row = (pos.row - 2) / 2,
                    .col = (pos.col - 1) / 2,
                };

                const auto south = Coordinate{
                    .row = pos.row / 2,
                    .col = (pos.col - 1) / 2,
                };

                if (!(visited.contains(north) && visited.contains(south))) {
                    return true;
                }

                // check if the north and south connect through this space
                const auto north_directions = get_tile_directions(get_tile(north));
                const auto south_directions = get_tile_directions(get_tile(south));
                return !(
                    (north == south + south_directions.first || north == south + south_directions.second)
                    && (south == north + north_directions.first || south == north + north_directions.second)
                );
            }

            // horizontal case: odd row, even column
            if (pos.row % 2 != 0 && pos.col % 2 == 0) {
                const auto east = Coordinate{
                    .row = (pos.row - 1) / 2,
                    .col = pos.col / 2,
                };

                const auto west = Coordinate{
                    .row = (pos.row - 1) / 2,
                    .col = (pos.col - 2) / 2,
                };

                if (!(visited.contains(east) && visited.contains(west))) {
                    return true;
                }

                // check if the west and east connect through this space
                const auto east_directions = get_tile_directions(get_tile(east));
                const auto west_directions = get_tile_directions(get_tile(west));
                return !(
                    (east == west + west_directions.first || east == west + west_directions.second)
                    && (west == east + east_directions.first || west == east + east_directions.second)
                );
            }

            return true;  // allow expansion in other cases
        };

        while (not expanded_queue.empty()) {
            const auto coord = expanded_queue.front();
            expanded_queue.pop();

            const auto try_expand = [&](Coordinate next) {
                if (can_expand(next)) {
                    expanded_visited.emplace(next);
                    expanded_queue.push(next);
                }
            };

            try_expand({coord.row - 1, coord.col});
            try_expand({coord.row + 1, coord.col});
            try_expand({coord.row, coord.col - 1});
            try_expand({coord.row, coord.col + 1});
        }

        auto encircled = std::ssize(grid) * std::ssize(grid[0]);
        encircled -= std::ranges::count_if(expanded_visited, [](Coordinate pos) {
            return (pos.row % 2 != 0) && (pos.col % 2 != 0);
        });
        encircled -= std::ssize(visited);

        return encircled;
    }
}  // namespace

int main() {
    const auto path      = std::string{"input.data"};
    const auto maze_data = core::io::read_file(path, true);
    const auto maze_grid = core::strings::split(maze_data, "\n");

    const auto steps_numbers = count_numbers_of_step(maze_grid);
    std::cout << std::format("The farthest point is {} steps away\n", steps_numbers);

    const auto enclosed_tiles = count_enclosed_tiles(maze_grid);
    std::cout << std::format("There are {} tiles enclosed by loop\n", enclosed_tiles);

    return 0;
}
