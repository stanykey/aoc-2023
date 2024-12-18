#include <core/io.hxx>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


namespace {
    // pretty cool idea I found at `Daily bit(e) of C++` blog
    struct Ignore {
        std::string_view str;

        friend auto operator>>(std::istream& stream, const Ignore& ctx) -> std::istream& {
            return stream.ignore(static_cast<std::streamsize>(ctx.str.length()));
        }
    };

    struct Item {
        std::int64_t x = 0;
        std::int64_t m = 0;
        std::int64_t a = 0;
        std::int64_t s = 0;

        [[nodiscard]] auto value() const -> std::int64_t {
            return x + m + a + s;
        }

        friend std::istream& operator>>(std::istream& stream, Item& item) {
            stream >> Ignore{"{x="} >> item.x;
            stream >> Ignore{",m="} >> item.m;
            stream >> Ignore{",a="} >> item.a;
            stream >> Ignore{",s="} >> item.s;

            return stream >> Ignore{"}\n"};
        }
    };

    struct Rule {
        std::function<std::int64_t(const Item&)> project;
        std::function<bool(std::int64_t)>        condition;
        std::string                              destination;

        friend auto operator>>(std::istream& stream, Rule& rule) -> std::istream& {
            // done reading rules
            if (stream.peek() == '}') {
                stream.setstate(std::ios::failbit);
                return stream;
            }

            const auto type              = stream.get();
            const auto is_next_separator = [&]() -> bool { return std::isalnum(stream.peek()) == 0; };
            if (type == 'x' && is_next_separator()) {
                rule.project = &Item::x;
            } else if (type == 'm' && is_next_separator()) {
                rule.project = &Item::m;
            } else if (type == 'a' && is_next_separator()) {
                rule.project = &Item::a;
            } else if (type == 's' && is_next_separator()) {
                rule.project = &Item::s;
            } else {
                // not a condition, a label
                auto label = std::string{static_cast<char>(type)};
                while (stream.peek() != '}') {
                    label.push_back(static_cast<char>(stream.get()));
                }
                rule.project     = {};
                rule.condition   = {};
                rule.destination = label;
                return stream;
            }

            const auto condition = stream.get();
            const auto threshold = core::io::read<std::int64_t>(stream);
            stream >> Ignore{":"};

            auto label = std::string{};
            while (stream.peek() != '}' && stream.peek() != ',') {
                label.push_back(static_cast<char>(stream.get()));
            }
            rule.destination = label;

            if (stream.peek() == ',') {
                stream.ignore();
            }

            if (condition == '<') {
                rule.condition = [threshold](std::int64_t value) { return value < threshold; };
            } else {
                rule.condition = [threshold](std::int64_t value) { return value > threshold; };
            }

            return stream;
        }
    };

    struct Workflow {
        std::string       label;
        std::vector<Rule> rules;

        friend auto operator>>(std::istream& stream, Workflow& workflow) -> std::istream& {
            // workflows end with an empty line
            if (stream.peek() == '\n') {
                stream.ignore();
                stream.setstate(std::ios::failbit);
                return stream;
            }

            auto label = std::string{};
            while (stream.peek() != '{') {
                label.push_back(static_cast<char>(stream.get()));
            }
            workflow.label = label;

            stream >> Ignore{"{"};
            workflow.rules = core::io::read_sequence<Rule>(stream);
            stream.clear();
            return stream >> Ignore{"}\n"};
        }

        [[nodiscard]] auto process(const Item& item) const -> std::string {
            for (const auto& [project, condition, destination] : rules) {
                if (not project or not condition) {
                    return destination;
                }

                if (condition(project(item))) {
                    return destination;
                }
            }

            return {};
        }
    };

    auto load_workflows(std::istream& stream) -> std::unordered_map<std::string, Workflow> {
        auto registry = std::unordered_map<std::string, Workflow>{};
        std::ranges::move(
            std::views::istream<Workflow>(stream) | std::views::transform([](Workflow& workflow) {
                auto label = workflow.label;
                return std::pair(std::move(label), std::move(workflow));
            }),
            std::inserter(registry, registry.end())
        );
        stream.clear();
        return registry;
    }

    auto load_items(std::istream& stream) -> std::vector<Item> {
        return core::io::read_sequence<Item>(stream);
    }

    auto total_value_of_accepted_items(
        const std::unordered_map<std::string, Workflow>& workflows, const std::vector<Item>& items
    ) -> std::int64_t {
        return std::ranges::fold_left(items, 0ll, [&](std::int64_t accumulator, const Item& item) {
            auto current = std::string{"in"};
            while (current != "A" and current != "R") {
                current = workflows.at(current).process(item);
            }

            return (current == "A") ? accumulator + item.value() : accumulator;
        });
    }
}  // namespace

int main() {
    const auto path      = std::filesystem::path{"input.data"};
    auto       stream    = std::ifstream{path};
    const auto workflows = load_workflows(stream);
    const auto items     = load_items(stream);

    std::cout << std::format("The total value of acceted items is {}", total_value_of_accepted_items(workflows, items));
    return 0;
}
