#include <core/io.hxx>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace {
    struct Node {
        std::string name;
        std::string left;
        std::string right;
    };

    struct Graph {
        enum class Direction : std::uint8_t {
            Left,
            Right,
        };

        [[nodiscard]] auto pick_next(const std::string& state, Direction direction) const -> std::string_view {
            const auto& node = nodes.at(state);
            return (direction == Direction::Left) ? node.left : node.right;
        }

        friend auto operator>>(std::istream& stream, Graph& graph) -> std::istream& {
            const auto drop_irrelevant = [&] {
                while (stream && (std::isupper(stream.peek()) == 0)) {
                    stream.ignore();
                }
            };

            const auto get = [&] { return static_cast<char>(stream.get()); };

            auto read_location = [&] {
                drop_irrelevant();
                return std::string{get(), get(), get()};
            };

            auto node = Node{};
            while (not stream.eof()) {
                node.name  = read_location();
                node.left  = read_location();
                node.right = read_location();

                graph.nodes.emplace(node.name, node);

                drop_irrelevant();
            };

            return stream;
        }

        std::unordered_map<std::string, Node> nodes;
    };

    auto load_map(const std::string& path) -> std::tuple<std::string, Graph> {
        auto stream = std::ifstream{path};

        auto instructions = core::io::read<std::string>(stream);
        auto graph        = core::io::read<Graph>(stream);

        return {std::move(instructions), std::move(graph)};
    }

    auto count_steps(const std::string& instructions, const Graph& graph) -> std::size_t {
        const auto direction = [&instructions](std::size_t pos) {
            return instructions[pos % instructions.size()] == 'L' ? Graph::Direction::Left : Graph::Direction::Right;
        };

        const auto start = std::string{"AAA"};
        const auto end   = std::string{"ZZZ"};

        auto current = start;
        auto steps   = 0ul;
        while (current != end) {
            current = graph.pick_next(current, direction(steps));
            ++steps;
        }

        return steps;
    }

    auto count_steps_with_ghosts(const std::string& instructions, const Graph& graph) -> std::uint64_t {
        const auto direction = [&instructions](std::size_t pos) {
            return instructions[pos % instructions.size()] == 'L' ? Graph::Direction::Left : Graph::Direction::Right;
        };

        const auto get_node_names = [&graph](char letter) -> std::unordered_set<std::string> {
            auto names_view = graph.nodes
                            | std::views::transform([](const auto& node) -> const std::string& { return node.first; })
                            | std::views::filter([letter](const std::string& name) { return name.back() == letter; });

            return std::unordered_set<std::string>{names_view.begin(), names_view.end()};
        };

        auto       total_steps = std::uint64_t{1};
        const auto ends        = get_node_names('Z');
        for (const auto& ghost : get_node_names('A')) {
            auto current = ghost;
            auto steps   = std::uint64_t{0};
            while (!ends.contains(current)) {
                current = graph.pick_next(current, direction(steps));
                steps++;
            }

            // The end of the loop is aligned with the instructions
            if (steps % instructions.length() != 0) {
                throw std::runtime_error("Unaligned ghost");
            }

            // The ghost is on a proper loop if it returns to this position after steps
            auto state = current;
            for (auto i = 0ul; i < steps; i++) {
                current = graph.pick_next(current, direction(i));
            }

            if (current != state) {
                throw std::runtime_error("The ghost is not on a proper loop");
            }

            total_steps = std::lcm(total_steps, steps);
        }

        return total_steps;
    }

}  // namespace


int main() {
    const auto path                  = std::string{"input.data"};
    const auto [instructions, graph] = load_map(path);

    const auto steps = count_steps(instructions, graph);
    std::cout << std::format("You need {} steps to get from AAA to ZZZ\n", steps);

    const auto ghosts_steps = count_steps_with_ghosts(instructions, graph);
    std::cout << std::format("It takes {} steps to only on nodes that end with Z\n", ghosts_steps);

    return 0;
}
