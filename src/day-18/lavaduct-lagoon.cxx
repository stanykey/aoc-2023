#include <core/io.hxx>

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
#include <queue>
#include <string>
#include <unordered_set>
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

    struct Instruction {
        Coordinate   direction;
        std::int64_t distance = 0;
        std::string  color;

        friend auto operator>>(std::istream& stream, Instruction& instruction) -> std::istream& {
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

    const auto DIRECTIONS = std::array<Coordinate, 4>{
        Coordinate{-1, 0},  // down
        Coordinate{1, 0},   // up
        Coordinate{0, -1},  // left
        Coordinate{0, 1},   // right
    };

    auto load_instructions(const std::filesystem::path& path) -> std::vector<Instruction> {
        auto stream = std::ifstream{path};
        return core::io::read_sequence<Instruction>(stream);
    }

    auto expand_bounding_box(Coordinate& top_left, Coordinate& bottom_right) {
        top_left.row--;
        top_left.col--;
        bottom_right.row++;
        bottom_right.col++;
    }

    auto lagoon_size(const std::vector<Instruction>& instructions) -> std::int64_t {
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
}  // namespace

int main() {
    const auto path         = std::filesystem::path{"input.data"};
    const auto instructions = load_instructions(path);

    std::cout << std::format("The volume of lava is: {}\n", lagoon_size(instructions));

    return 0;
}
