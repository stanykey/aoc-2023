#include <core/io.hxx>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>


namespace {
    struct Note {
        std::vector<std::uint64_t> rows;
        std::vector<std::uint64_t> cols;

        friend auto operator>>(std::istream& stream, Note& note) -> std::istream& {
            if (stream.peek() == std::char_traits<char>::eof()) {
                return stream;  // no input to process
            }

            note.rows.clear();
            note.cols.clear();

            while (stream) {
                const auto line = core::io::read_line(stream);
                if (line.empty()) {
                    if (stream.eof()) {
                        stream.clear(std::ios::eofbit);
                    }
                    return stream;
                }

                if (note.cols.size() < line.size()) {
                    note.cols.resize(line.size(), 0);
                }

                auto number = 0ull;
                auto column = 0ull;
                for (const auto& symbol : line) {
                    if (symbol == '.') {  // Treat '.' as 0 bit
                        number            = number << 1;
                        note.cols[column] = note.cols[column] << 1;
                    } else {  // Treat '#' as 1 bit
                        number            = (number << 1) | 1;
                        note.cols[column] = (note.cols[column] << 1) | 1;
                    }
                    column++;
                }

                note.rows.push_back(number);
            }

            return stream;
        }

        static auto find_longest_mirror_size(std::span<const std::uint64_t> rng) -> std::uint64_t {
            const auto all_splits =
                std::views::iota(rng.begin(), rng.end()) | std::views::transform([&](auto it) {
                    return std::pair{
                        std::ranges::subrange(rng.begin(), it) | std::views::reverse, std::ranges::subrange(it, rng.end())
                    };
                });

            for (auto&& [prefix, suffix] : all_splits) {
                if (!prefix.empty() and !suffix.empty()) {
                    const auto cmp = std::ranges::mismatch(prefix, suffix);
                    if (cmp.in1 == prefix.end() || cmp.in2 == suffix.end()) {
                        return prefix.size();
                    }
                }
            }
            return 0;
        }

        static auto find_longest_mirror_size_with_one_bit_error(std::span<const uint64_t> rng) -> std::uint64_t {
            const auto all_splits =
                std::views::iota(rng.begin(), rng.end()) | std::views::transform([&](auto it) {
                    return std::pair{
                        std::ranges::subrange(rng.begin(), it) | std::views::reverse, std::ranges::subrange(it, rng.end())
                    };
                });

            const auto count_different_bits = [](const auto& shorter, const auto& longer) {
                return std::inner_product(
                    shorter.begin(), shorter.end(), longer.begin(), 0ull, std::plus<>{},
                    [](std::uint64_t left, std::uint64_t right) { return std::popcount(left ^ right); }
                );
            };

            for (const auto& [prefix, suffix] : all_splits) {
                if (!prefix.empty() and !suffix.empty()) {
                    // clang-format off
                    const auto bit_count = (prefix.size() >= suffix.size())
                        ? count_different_bits(suffix, prefix)
                        : count_different_bits(prefix, suffix)
                    ;
                    // clang-format on

                    if (bit_count == 1) {
                        return prefix.size();
                    }
                }
            }
            return 0;
        }

        [[nodiscard]] auto find_longest_row_mirror_size() const -> std::uint64_t {
            return find_longest_mirror_size(rows);
        }

        [[nodiscard]] auto find_longest_col_mirror_size() const -> std::uint64_t {
            return find_longest_mirror_size(cols);
        }

        [[nodiscard]] auto find_longest_row_mirror_size_with_one_bit_error() const -> std::uint64_t {
            return find_longest_mirror_size_with_one_bit_error(rows);
        }

        [[nodiscard]] auto find_longest_col_mirror_size_with_one_bit_error() const -> std::uint64_t {
            return find_longest_mirror_size_with_one_bit_error(cols);
        }
    };

    auto load_notes(const std::filesystem::path& path) -> std::vector<Note> {
        auto stream = std::ifstream{path};
        return core::io::read_sequence<Note>(stream);
    }

    auto summarize_notes(const std::vector<Note>& notes) -> std::uint64_t {
        return std::ranges::fold_left(notes, 0ull, [](std::uint64_t accumulated, const Note& note) {
            const auto row_weight = 100ull;
            return accumulated + note.find_longest_col_mirror_size() + row_weight * note.find_longest_row_mirror_size();
        });
    }

    auto summarize_notes_with_one_bit_error(const std::vector<Note>& notes) -> std::uint64_t {
        return std::ranges::fold_left(notes, 0ull, [](std::uint64_t accumulated, const Note& note) {
            const auto row_weight = 100ull;
            return accumulated + note.find_longest_col_mirror_size_with_one_bit_error()
                 + row_weight * note.find_longest_row_mirror_size_with_one_bit_error();
        });
    }
}  // namespace


int main() {
    const auto path  = std::filesystem::path{"input.data"};
    const auto notes = load_notes(path);

    const auto summarized = summarize_notes(notes);
    std::cout << std::format("The summarized value of notes is {}\n", summarized);

    const auto summarized_with_one_bit_error = summarize_notes_with_one_bit_error(notes);
    std::cout << std::format("The summarized value with 1-bit error of notes is {}\n", summarized_with_one_bit_error);

    return 0;
}
