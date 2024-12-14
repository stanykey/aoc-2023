#include <core/io.hxx>
#include <core/strings.hxx>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <queue>
#include <string_view>
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

    struct State {
        Coordinate   position;
        Coordinate   direction;
        std::int64_t steps     = 0;
        std::int64_t heat_loss = 0;

        auto operator<(const State& other) const noexcept {
            return heat_loss > other.heat_loss;  // priority queue favors lower heat_loss
        }
    };

    constexpr auto MAX_LINE_LENGTH = 3;
    constexpr auto DIR_COUNT       = 4;
    using Line                     = std::array<std::int64_t, MAX_LINE_LENGTH + 1>;

    constexpr auto EMPTY_LINE = Line{
        std::numeric_limits<std::int64_t>::max(),
        std::numeric_limits<std::int64_t>::max(),
        std::numeric_limits<std::int64_t>::max(),
        std::numeric_limits<std::int64_t>::max(),
    };

    auto left_direction(Coordinate direction) -> Coordinate {
        if (direction.row != 0) {
            return Coordinate{0, direction.row};
        }
        return Coordinate{-direction.col, 0};
    };

    auto right_direction(Coordinate direction) -> Coordinate {
        if (direction.row != 0) {
            return Coordinate{0, -direction.row};
        }
        return Coordinate{direction.col, 0};
    };

    auto to_index(Coordinate direction) -> std::size_t {
        if (direction.row == -1) {
            return 0;
        }

        if (direction.row == 1) {
            return 1;
        }

        if (direction.col == -1) {
            return 2;
        }

        return 3;
    };

    auto find_minimum_heat_loss(const Grid& grid) -> std::size_t {
        const auto is_valid_position = [&](const Coordinate& pos) {
            return pos.row >= 0 && pos.col >= 0 && pos.row < std::ssize(grid) && pos.col < std::ssize(grid[0]);
        };

        const auto get_heat_loss = [&](const Coordinate& pos) { return grid[pos.row][pos.col] - '0'; };

        // Dijkstra's algorithm setup
        auto lines = std::vector(
            grid.size(),
            std::vector(grid[0].size(), std::array<Line, DIR_COUNT>{EMPTY_LINE, EMPTY_LINE, EMPTY_LINE, EMPTY_LINE})
        );
        auto queue = std::priority_queue<State>{};

        // Initialize with starting states (down and right)
        queue.push(State{{0, 0}, {0, 1}, 0, 0});
        queue.push(State{{0, 0}, {1, 0}, 0, 0});

        const auto destination = Coordinate{.row = std::ssize(grid) - 1, .col = std::ssize(grid[0]) - 1};
        while (!queue.empty()) {
            auto [pos, dir, steps, heat_loss] = queue.top();
            if (pos == destination) {
                return heat_loss;
            }
            queue.pop();

            auto& current_heat_loss = lines[pos.row][pos.col][to_index(dir)][steps];
            if (current_heat_loss <= heat_loss) {
                continue;  // Skip worse states
            }

            current_heat_loss = heat_loss;

            // continue in the same direction
            if (steps + 1 <= MAX_LINE_LENGTH && is_valid_position(pos + dir)) {
                queue.push({
                    .position  = pos + dir,
                    .direction = dir,
                    .steps     = steps + 1,
                    .heat_loss = heat_loss + get_heat_loss(pos + dir),
                });
            }

            // turn left
            const auto left_dir = left_direction(dir);
            if (is_valid_position(pos + left_dir)) {
                queue.push({
                    .position  = pos + left_dir,
                    .direction = left_dir,
                    .steps     = 1,
                    .heat_loss = heat_loss + get_heat_loss(pos + left_dir),
                });
            }

            // turn right
            const auto right_dir = right_direction(dir);
            if (is_valid_position(pos + right_dir)) {
                queue.push({
                    .position  = pos + right_dir,
                    .direction = right_dir,
                    .steps     = 1,
                    .heat_loss = heat_loss + get_heat_loss(pos + right_dir),
                });
            }
        }

        return -1;  // No valid path found
    }
}  // namespace


int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto data = core::io::read_file(path, true);
    const auto map  = core::strings::split(core::strings::strip(data), "\n");

    std::cout << std::format("The least heat loss is: {}\n", find_minimum_heat_loss(map));

    return 0;
}
