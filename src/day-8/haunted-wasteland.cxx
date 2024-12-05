#include <core/io.hxx>

#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


namespace {
    struct Node {
        std::string name;
        std::string left;
        std::string right;

        static auto parse(std::string_view record) -> Node {
            if (record.empty()) {
                throw std::invalid_argument{"Empty line encountered while parsing Node"};
            }

            // Validate positions
            const auto comma_pos       = record.find(',');
            const auto left_paren_pos  = record.find('(');
            const auto right_paren_pos = record.find(')');
            const auto positions       = std::unordered_set{comma_pos, left_paren_pos, right_paren_pos};
            if (positions.contains(std::string_view::npos)) {
                throw std::invalid_argument{std::format("Malformed node input: {}", record)};
            }

            // Extract parts safely
            return {
                .name  = std::string{record.substr(0, record.find(' '))},
                .left  = std::string{record.substr(left_paren_pos + 1, comma_pos - left_paren_pos - 1)},
                .right = std::string{record.substr(comma_pos + 2, right_paren_pos - comma_pos - 2)},
            };
        }
    };

    class Network {
        using Map = std::unordered_map<std::string_view, const Node*>;

    public:
        Network(std::string instructions, std::vector<Node> nodes)
            : instructions_{std::move(instructions)}
            , nodes_{std::move(nodes)} {
            build_map();
        }

        static auto load(std::istream& stream) -> Network {
            auto instructions = core::io::read_line(stream);
            auto empty_line   = core::io::read_line(stream);

            auto nodes = std::vector<Node>{};
            auto line  = std::string{};
            while (std::getline(stream, line) && !line.empty()) {
                nodes.emplace_back(Node::parse(line));
            }
            return {std::move(instructions), std::move(nodes)};
        }

        [[nodiscard]] auto instructions() const -> std::string_view {
            return instructions_;
        }

        [[nodiscard]] auto nodes() const -> const std::vector<Node>& {
            return nodes_;
        }

        [[nodiscard]] auto distance(std::string_view from, std::string_view to) const -> std::optional<std::size_t> {
            auto distance    = 0ul;
            auto instruction = 0ul;
            auto current     = from;
            while (current != to) {
                const auto next = map_.find(current);
                if (next == map_.cend()) {
                    return std::nullopt;
                }

                const auto  direction = instructions_[instruction];
                const auto& node      = *next->second;
                current               = (direction == 'L') ? node.left : node.right;
                instruction           = (instruction + 1) % instructions_.size();

                distance++;
            }

            return distance;
        }

    private:
        auto build_map() -> void {
            for (const auto& node : nodes_) {
                map_.emplace(node.name, &node);
            }
        }

    private:
        std::string       instructions_;
        std::vector<Node> nodes_;
        Map               map_;
    };

    auto load_network(const std::string& path) -> Network {
        auto stream = std::ifstream{path};
        return Network::load(stream);
    }
}  // namespace


int main() {
    const auto path    = std::string{"input.data"};
    const auto network = load_network(path);

    const auto from     = std::string_view{"AAA"};
    const auto to       = std::string_view{"ZZZ"};
    const auto distance = network.distance(from, to);
    std::cout << std::format("Distance from AAA to ZZZ is {}\n", distance.value_or(0));

    return 0;
}
