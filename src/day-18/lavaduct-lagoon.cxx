#include <core/io.hxx>
#include <core/numbers.hxx>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <queue>
#include <ranges>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>


namespace {
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

        [[nodiscard]] Coordinate operator*(std::int64_t scalar) const {
            return Coordinate{
                .row = row * scalar,
                .col = col * scalar,
            };
        }
    };

    struct BuggedInstruction {
        Coordinate   direction;
        std::int64_t distance = 0;
        std::string  color;

        friend auto operator>>(std::istream& stream, BuggedInstruction& instruction) -> std::istream& {
            while (std::isspace(stream.peek()) != 0) {
                stream.ignore();
            }

            switch (stream.get()) {
                case 'U': instruction.direction = {-1, 0}; break;
                case 'D': instruction.direction = {1, 0}; break;
                case 'L': instruction.direction = {0, -1}; break;
                case 'R': instruction.direction = {0, 1}; break;
            }

            instruction.distance = core::io::read<std::int64_t>(stream);
            instruction.color    = core::io::read<std::string>(stream);

            return stream;
        }
    };

    struct Instruction {
        Coordinate   dig;
        std::int64_t distance = 0;

        friend auto operator>>(std::istream& stream, Instruction& instr) -> std::istream& {
            // skip bugged part
            core::io::read<std::string>(stream);
            core::io::read<std::string>(stream);
            if (not stream) {
                return stream;
            }

            const auto instruction     = core::io::read<std::string>(stream);
            const auto direction_index = 7ul;
            switch (instruction[direction_index]) {
                case '0': instr.dig = {0, 1}; break;
                case '1': instr.dig = {1, 0}; break;
                case '2': instr.dig = {0, -1}; break;
                case '3': instr.dig = {-1, 0}; break;
            }

            const auto distance_length  = 5;
            const auto encoded_distance = std::string_view{instruction.data() + 2, distance_length};
            instr.distance              = core::numbers::parse_hex<std::int64_t>(encoded_distance);

            return stream;
        }
    };

    struct Wall {
        Coordinate top_left;
        Coordinate bottom_right;

        Wall(Coordinate first, Coordinate second)
            : top_left(std::min(first, second))
            , bottom_right(std::max(first, second)) {}

        [[nodiscard]] auto is_horizontal() const -> bool {
            return top_left.row == bottom_right.row;
        }

        [[nodiscard]] auto is_single() const -> bool {
            return top_left == bottom_right;
        }

        auto operator<(const Wall& other) const -> bool {
            // Special sorting to keep horizontal walls in-between the two vertical walls in our order
            if (top_left == other.top_left) {
                if (is_single() && not other.is_single()) {
                    return true;
                }

                if (not is_horizontal() && other.is_horizontal()) {
                    return true;
                }

                return false;
            }
            return top_left < other.top_left;
        }
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

namespace {
    using CoordinateSet   = std::unordered_set<Coordinate>;
    using CoordinateQueue = std::queue<Coordinate>;

    const auto DIRECTIONS = std::array{
        Coordinate{-1, 0},  // down
        Coordinate{1, 0},   // up
        Coordinate{0, -1},  // left
        Coordinate{0, 1},   // right
    };

    auto load_bugged_instructions(const std::filesystem::path& path) -> std::vector<BuggedInstruction> {
        auto stream = std::ifstream{path};
        return core::io::read_sequence<BuggedInstruction>(stream);
    }

    auto load_fixed_instructions(const std::filesystem::path& path) -> std::vector<Instruction> {
        auto stream = std::ifstream{path};
        return core::io::read_sequence<Instruction>(stream);
    }

    auto expand_bounding_box(Coordinate& top_left, Coordinate& bottom_right) {
        top_left.row--;
        top_left.col--;
        bottom_right.row++;
        bottom_right.col++;
    }

    auto lagoon_size(const std::vector<BuggedInstruction>& instructions) -> std::int64_t {
        auto top_left     = Coordinate{};
        auto bottom_right = Coordinate{};
        auto dug_out      = CoordinateSet{};

        auto current = Coordinate{};
        for (const auto& dig : instructions) {
            for (auto steps = 0; steps < dig.distance; ++steps) {
                current = current + dig.direction;
                dug_out.insert(current);
            }

            top_left.row     = std::min(top_left.row, current.row);
            top_left.col     = std::min(top_left.col, current.col);
            bottom_right.row = std::max(bottom_right.row, current.row);
            bottom_right.col = std::max(bottom_right.col, current.col);
        }
        expand_bounding_box(top_left, bottom_right);

        // BFS again (^_^)
        auto outside = CoordinateSet{};
        auto queue   = CoordinateQueue{};
        queue.push(top_left);
        outside.insert(top_left);

        const auto can_step = [&](const Coordinate& coord) -> bool {
            return coord.row >= top_left.row and coord.col >= top_left.col and coord.row <= bottom_right.row
               and coord.col <= bottom_right.col and not outside.contains(coord) and not dug_out.contains(coord);
        };

        while (!queue.empty()) {
            current = queue.front();
            queue.pop();

            for (const auto& dir : DIRECTIONS) {
                if (const auto next = current + dir; can_step(next)) {
                    outside.insert(next);
                    queue.push(next);
                }
            }
        }

        return (bottom_right.row - top_left.row + 1) * (bottom_right.col - top_left.col + 1) - std::ssize(outside);
    }

    auto big_lagoon_size(const std::vector<Instruction>& instructions) -> std::int64_t {
        // kudos for Šimon Tóth (https://simontoth.substack.com/p/daily-bite-of-c-advent-of-code-day-e9d)
        auto walls   = std::multiset<Wall>{};
        auto current = Coordinate{0, 0};
        for (const auto& dig : instructions) {
            const auto next = current + dig.dig * dig.distance;
            walls.emplace(current, next);
            current = next;
        }

        const auto perimeter = std::ranges::fold_left(walls, 0ll, [](std::int64_t accumulated, Wall wall) {
            return accumulated + (wall.bottom_right.row - wall.top_left.row) + (wall.bottom_right.col - wall.top_left.col);
        });

        const auto make_tail = [](const Coordinate& pos) {
            return Wall{
                Coordinate{pos.row + 1, std::numeric_limits<std::int64_t>::min()},
                Coordinate{std::numeric_limits<std::int64_t>::max(), std::numeric_limits<std::int64_t>::max()}
            };
        };

        auto inside = 0ll;
        while (not walls.empty()) {
            // grab a range that spans all lines that are on the current row
            auto begin = walls.begin();
            auto tail  = make_tail(walls.begin()->top_left);
            auto end   = walls.upper_bound(tail);

            auto inside_spaces = 0ll;
            auto line_count    = 0ll;
            auto prev          = Wall{{0, 0}, {0, 0}};
            for (auto it = begin; it != end; ++it) {
                // if we are looking at a horizontal line, adjust the count of lines to the left accordingly to the shape
                if (it != begin && std::prev(it)->top_left.col == it->top_left.col) {
                    // check the shape
                    const auto before = std::prev(it);
                    const auto after  = std::next(it);

                    // closed shape, U or A
                    const auto is_u = before->is_single() && after->is_single();
                    const auto is_a = not(before->is_single() or after->is_single());
                    if (is_u or is_a) {
                        line_count--;
                    }
                } else if (line_count % 2 == 1) {
                    // If we have odd number of lines to the left, we can calculate the
                    // number of spaces from the previous line to the left
                    inside_spaces += std::max(it->top_left.col - prev.bottom_right.col - 1, 0ll);
                }

                line_count++;
                prev = *it;
            }

            // now we need to calculate how many number of rows will the number of inside spaces remain the same
            auto first_row = walls.begin()->top_left.row;
            auto last_row =
                std::ranges::min(std::ranges::subrange(begin, end), std::less<>{}, &Wall::bottom_right).bottom_right.row;
            if (end != walls.end()) {
                last_row = std::min(last_row, end->top_left.row);
            }

            // horizontal lines end on the same line, but we want to always at least advance once
            last_row = std::max(first_row + 1, last_row);
            inside += inside_spaces * (last_row - first_row);

            // now we need to modify the lines. We either remove them,
            // or truncate them so they start on the one line after last_row.
            while (not walls.empty() && walls.begin()->top_left.row == first_row) {
                if (walls.begin()->is_horizontal()) {
                    walls.erase(walls.begin());
                    continue;
                }

                // because we are working with std::set, we cannot modify the value in-place
                auto node                 = walls.extract(walls.begin());
                node.value().top_left.row = last_row;
                walls.insert(std::move(node));
            }
        }

        return inside + perimeter;
    }
}  // namespace

int main() {
    const auto path                = std::filesystem::path{"input.data"};
    const auto bugged_instructions = load_bugged_instructions(path);
    const auto fixed_instructions  = load_fixed_instructions(path);

    std::cout << std::format("The volume of lava is: {}\n", lagoon_size(bugged_instructions));
    std::cout << std::format("The volume of lava is: {}\n", big_lagoon_size(fixed_instructions));

    return 0;
}
