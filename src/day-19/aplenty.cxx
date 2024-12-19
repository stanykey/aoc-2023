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

    template<typename Item, class Project, class Condition, class ConditionMaker>
    struct RuleBase {
        Project     project;
        Condition   condition;
        std::string destination;

        friend auto operator>>(std::istream& stream, RuleBase& rule) -> std::istream& {
            // done reading rules
            if (stream.peek() == '}') {
                stream.setstate(std::ios::failbit);
                return stream;
            }

            const auto type              = stream.get();
            const auto is_next_separator = [&]() -> bool { return std::isalnum(stream.peek()) == 0; };
            if (type == 'x' and is_next_separator()) {
                rule.project = &Item::x;
            } else if (type == 'm' and is_next_separator()) {
                rule.project = &Item::m;
            } else if (type == 'a' and is_next_separator()) {
                rule.project = &Item::a;
            } else if (type == 's' and is_next_separator()) {
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
            const auto threshold = core::io::read<std::uint64_t>(stream);
            stream >> Ignore{":"};

            auto label = std::string{};
            while (stream.peek() != '}' and stream.peek() != ',') {
                label.push_back(static_cast<char>(stream.get()));
            }
            rule.destination = label;

            if (stream.peek() == ',') {
                stream.ignore();
            }

            rule.condition = ConditionMaker{}(condition, threshold);

            return stream;
        }
    };

    template<class Rule>
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
    };

    template<class Workflow>
    auto load_workflows(std::istream& stream) -> std::unordered_map<std::string, Workflow> {
        auto registry = std::unordered_map<std::string, Workflow>{};
        std::ranges::move(
            std::views::istream<Workflow>(stream) | std::views::transform([](Workflow& workflow) {
                auto label = workflow.label;  // NOLINT: clang-tidy doesn't recognize that we do copy here
                return std::pair(std::move(label), std::move(workflow));
            }),
            std::inserter(registry, registry.end())
        );
        stream.clear();
        return registry;
    }
}  // namespace

namespace part_one {
    struct Item {
        std::uint64_t x = 0;
        std::uint64_t m = 0;
        std::uint64_t a = 0;
        std::uint64_t s = 0;

        [[nodiscard]] auto value() const -> std::uint64_t {
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

    using Project   = std::function<std::uint64_t(const Item&)>;
    using Condition = std::function<bool(std::uint64_t)>;

    struct ConditionMaker {
        auto operator()(char condition, std::uint64_t threshold) const -> Condition {
            if (condition == '<') {
                return [threshold](std::uint64_t value) { return value < threshold; };
            }

            return [threshold](std::uint64_t value) { return value > threshold; };
        }
    };

    using Rule = RuleBase<Item, Project, Condition, ConditionMaker>;

    struct Workflow : ::Workflow<Rule> {
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

    auto load_items(std::istream& stream) -> std::vector<Item> {
        return core::io::read_sequence<Item>(stream);
    }

    auto total_value_of_accepted_items(const std::filesystem::path& path) -> std::uint64_t {
        auto       stream    = std::ifstream{path};
        const auto workflows = load_workflows<Workflow>(stream);
        const auto items     = load_items(stream);

        return std::ranges::fold_left(items, 0ull, [&](std::uint64_t accumulator, const Item& item) {
            auto current = std::string{"in"};
            while (current != "A" and current != "R") {
                current = workflows.at(current).process(item);
            }

            return (current == "A") ? accumulator + item.value() : accumulator;
        });
    }
}  // namespace part_one

namespace part_two {
    constexpr auto MAX_RANK = 4000ull;

    struct Range {
        std::uint64_t min = 0;
        std::uint64_t max = 0;

        [[nodiscard]] auto size() const -> std::uint64_t {
            return max - min + 1;
        }
    };

    struct Item {
        Range x{1, MAX_RANK};
        Range m{1, MAX_RANK};
        Range a{1, MAX_RANK};
        Range s{1, MAX_RANK};

        [[nodiscard]] auto combinations() const -> std::uint64_t {
            return x.size() * m.size() * a.size() * s.size();
        }
    };

    using Project   = std::function<Range&(Item&)>;
    using Condition = std::function<std::pair<Range, Range>(Range)>;

    struct ConditionMaker {
        auto operator()(char condition, std::uint64_t threshold) const -> Condition {
            if (condition == '<') {
                return [threshold](Range value) {
                    return std::pair{
                        Range{value.min, std::min(threshold - 1, value.max)},
                        Range{std::max(threshold, value.min), value.max}
                    };
                };
            }
            return [threshold](Range value) {
                return std::pair{
                    Range{std::max(threshold + 1, value.min), value.max}, Range{value.min, std::min(threshold, value.max)}
                };
            };
        }
    };

    using Rule = RuleBase<Item, Project, Condition, ConditionMaker>;


    auto search(
        const std::unordered_map<std::string, Workflow<Rule>>& workflows, std::vector<Item>& accepted,
        const std::string& label, Item value
    ) {
        if (label == "A") {
            accepted.push_back(value);
            return;
        }

        if (label == "R") {
            return;
        }

        for (const auto& [project, condition, destination] : workflows.at(label).rules) {
            // no condition means that we are jumping to a new label
            if (not condition || not project) {
                search(workflows, accepted, destination, value);
                continue;
            }

            // get the "true" and "false" chunks
            auto [jump, next] = condition(project(value));
            if (jump.size() > 0) {
                // If the "true" chunk contains valid values
                auto new_value = value;
                // Update the relevant member with the values
                project(new_value) = jump;
                // recurse
                search(workflows, accepted, destination, new_value);
            }

            // if the "false" chunk doesn't contain any values, we are done
            if (next.size() <= 0) {
                break;  // dead-end
            }

            // otherwise update the relevant member with the "false" values and continue
            project(value) = next;
        }
    }

    auto count_distinct_accepted_combinations(const std::filesystem::path& path) -> std::uint64_t {
        auto       stream    = std::ifstream{path};
        const auto workflows = load_workflows<Workflow<Rule>>(stream);

        auto accepted = std::vector<Item>{};
        search(workflows, accepted, "in", {});
        return std::ranges::fold_left(accepted, 0ull, [](std::uint64_t accumulator, const Item& item) {
            return accumulator + item.combinations();
        });
    }
}  // namespace part_two

int main() {
    const auto path = std::filesystem::path{"input.data"};

    std::cout << std::format("The total value of accepted items is {}\n", part_one::total_value_of_accepted_items(path));
    std::cout << std::format(
        "There will be accepted {} of distinct combinations", part_two::count_distinct_accepted_combinations(path)
    );
    return 0;
}
