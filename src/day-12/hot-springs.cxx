#include <core/io.hxx>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>


namespace {
    struct Spring {
        std::string              condition;
        std::vector<std::size_t> damage_sizes;

        friend auto operator>>(std::istream& stream, Spring& spring) -> std::istream& {
            spring.condition = core::io::read<std::string>(stream);

            spring.damage_sizes.clear();
            while ((stream.peek() == ' ') || (stream.peek() == ',')) {
                if (stream.peek() == ',') {
                    stream.ignore();
                }

                spring.damage_sizes.push_back(core::io::read<std::size_t>(stream));
            }

            return stream;
        }
    };

    using MemoKey = std::tuple<std::size_t, std::size_t, std::size_t>;

    struct MemoHash {
        std::size_t operator()(const MemoKey& key) const {
            return std::hash<std::size_t>()(std::get<0>(key)) ^ std::hash<std::size_t>()(std::get<1>(key))
                 ^ std::hash<std::size_t>()(std::get<2>(key));
        }
    };

    auto load_springs(const std::filesystem::path& path) -> std::vector<Spring> {
        auto stream = std::ifstream{path};
        return core::io::read_sequence<Spring>(stream);
    }

    auto find_possible_placements(std::size_t current_offset, std::size_t width, std::string_view condition)
        -> std::vector<std::size_t> {
        auto result = std::vector<std::size_t>{};

        auto start = current_offset;
        while (start <= condition.size() - width) {
            const auto win_start = start;
            const auto win_end   = start + width;

            // check if the window contains only valid characters ('#' or '?')
            auto is_valid_window = true;
            for (std::size_t i = win_start; is_valid_window and (i < win_end); i++) {
                is_valid_window = (condition[i] != '.');
            }

            // check the boundaries of the window
            const auto is_start_ok = (win_start == 0) || (condition[win_start - 1] != '#');
            const auto is_end_ok   = (win_end == condition.size()) || (condition[win_end] != '#');
            if (is_valid_window && is_start_ok && is_end_ok) {
                result.push_back(win_start);
            }

            ++start;
        }

        return result;
    }

    auto compute_all_placements(std::string_view condition, std::span<const std::size_t> damage_sizes)
        -> std::vector<std::vector<std::size_t>> {
        auto placements = std::vector<std::vector<std::size_t>>{};

        auto current_offset = 0ul;
        for (auto size : damage_sizes) {
            placements.push_back(find_possible_placements(current_offset, size, condition));
            if (placements.back().empty()) {
                current_offset = std::ssize(condition);
            } else {
                current_offset = placements.back()[0] + size + 1;
            }
        }

        return placements;
    }

    auto count_valid_arrangements(
        std::size_t current_offset, std::span<const std::size_t> required_positions,
        std::span<const std::size_t> group_sizes, std::span<const std::vector<std::size_t>> group_placements,
        std::unordered_map<MemoKey, std::size_t, MemoHash>& memo
    ) -> std::size_t {
        if (group_sizes.empty()) {
            // if no more groups to place, ensure all required positions are covered
            return (std::ranges::lower_bound(required_positions, current_offset) == required_positions.end()) ? 1 : 0;
        }

        // memoization key
        const auto key = std::make_tuple(current_offset, group_sizes.size(), required_positions.size());
        if (memo.contains(key)) {
            return memo[key];
        }

        auto total_arrangements = std::size_t{0};
        for (const auto& placement_position : group_placements[0]) {
            if (placement_position < current_offset) {
                continue;  // skip overlapping placements
            }

            const auto next_required_position = std::ranges::lower_bound(required_positions, current_offset);
            if (next_required_position != required_positions.end() && placement_position > *next_required_position) {
                break;  // placement doesn't cover the next required position
            }

            // recurse to calculate arrangements for the remaining groups
            total_arrangements += count_valid_arrangements(
                placement_position + group_sizes[0] + 1, required_positions, group_sizes.subspan(1),
                group_placements.subspan(1), memo
            );
        }

        memo[key] = total_arrangements;
        return total_arrangements;
    }

    auto calculate_arrangements_for_spring(const Spring& spring_instance) -> std::size_t {
        // collect required positions (indices of fixed '#')
        auto required_positions = std::vector<std::size_t>{};
        std::ranges::copy_if(
            std::views::iota(std::size_t{0}, spring_instance.condition.size()), std::back_inserter(required_positions),
            [&](std::size_t idx) { return spring_instance.condition[idx] == '#'; }
        );

        const auto group_placements = compute_all_placements(spring_instance.condition, spring_instance.damage_sizes);
        auto       memo             = std::unordered_map<MemoKey, std::size_t, MemoHash>{};
        return count_valid_arrangements(0, required_positions, spring_instance.damage_sizes, group_placements, memo);
    }

    auto calculate_total_arrangements(const std::vector<Spring>& springs) -> std::size_t {
        return std::ranges::fold_left(springs, std::size_t{0}, [](std::size_t total, const Spring& spring_instance) {
            return total + calculate_arrangements_for_spring(spring_instance);
        });
    }

    auto unfold_spring(const Spring& spring) -> Spring {
        Spring unfolded;

        const auto unfold_size = 5;
        // unfold the condition
        for (auto i = 0ul; i < unfold_size; ++i) {
            if (i != 0) {
                unfolded.condition += '?';
            }
            unfolded.condition += spring.condition;
        }

        // unfold the damage sizes
        unfolded.damage_sizes.reserve(spring.damage_sizes.size() * unfold_size);
        for (std::size_t i = 0; i < unfold_size; ++i) {
            unfolded.damage_sizes
                .insert(unfolded.damage_sizes.end(), spring.damage_sizes.begin(), spring.damage_sizes.end());
        }

        return unfolded;
    }

    auto unfold_springs(const std::vector<Spring>& springs) -> std::vector<Spring> {
        auto unfolded_springs = std::vector<Spring>{};

        unfolded_springs.reserve(springs.size());
        for (const auto& original_spring : springs) {
            unfolded_springs.push_back(unfold_spring(original_spring));
        }

        return unfolded_springs;
    }

    auto calculate_total_arrangements_with_unfolding(const std::vector<Spring>& springs) -> std::size_t {
        const auto unfolded_springs = unfold_springs(springs);
        return calculate_total_arrangements(unfolded_springs);
    }
}  // namespace

int main() {
    const auto path    = std::filesystem::path{"input.data"};
    const auto springs = load_springs(path);

    const auto total_arrangements = calculate_total_arrangements(springs);
    std::cout << std::format("There are {} total possible arrangements\n", total_arrangements);

    const auto total_arrangements_unfolded = calculate_total_arrangements_with_unfolding(springs);
    std::cout << std::format("There are {} total possible arrangements after unfolding\n", total_arrangements_unfolded);

    return 0;
}
