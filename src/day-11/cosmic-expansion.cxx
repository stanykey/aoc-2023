#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>


namespace {
    using Grid = std::vector<std::string_view>;

    struct Coordinate {
        std::int64_t row = 0;
        std::int64_t col = 0;
    };

    auto sum_of_distances(const Grid& image, std::size_t expansion_size) -> std::size_t {
        const auto all_space = [](const auto& range) {
            return std::ranges::all_of(std::get<1>(range), [](char symbol) { return symbol == '.'; });
        };

        const auto column = [&](std::size_t col) {
            return image | std::views::transform([col](std::string_view line) -> char { return line[col]; });
        };

        // get indexes of empty lines and columns
        auto empty_rows = std::vector<std::int64_t>{};
        std::ranges::copy(
            image | std::views::enumerate | std::views::filter(all_space) | std::views::elements<0>,
            std::back_inserter(empty_rows)
        );

        const auto line_length   = std::ssize(image.front());
        auto       empty_columns = std::vector<std::int64_t>{};
        std::ranges::copy(
            // clang-format off
            std::views::iota(0, line_length)
                | std::views::transform(column)
                | std::views::enumerate
                | std::views::filter(all_space)
                | std::views::elements<0>,
            // clang-format on
            std::back_inserter(empty_columns)
        );

        // Distance calculation helper
        // determine the number of empty rows and columns between the two coordinates then calculate the distance,
        // adding the appropriate weight to  the empty rows/columns
        const auto distance = [&](Coordinate first, Coordinate second) {
            const auto columns = std::ranges::distance(
                std::ranges::upper_bound(empty_columns, std::min(first.col, second.col)),
                std::ranges::lower_bound(empty_columns, std::max(first.col, second.col))
            );
            const auto rows = std::ranges::distance(
                std::ranges::upper_bound(empty_rows, std::min(first.row, second.row)),
                std::ranges::lower_bound(empty_rows, std::max(first.row, second.row))
            );
            return (std::abs(first.col - second.col) + (expansion_size - 1) * columns)
                 + (std::abs(first.row - second.row) + (expansion_size - 1) * rows);
        };


        // Transform the map into the list of coordinates of galaxies then fold over the galaxies,
        // for each producing the sum of distances to all previously seen galaxies
        // clang-format off
        auto seen = std::vector<Coordinate>{};
        auto all_galaxies = image
            | std::views::enumerate
            | std::views::transform([](auto&& e) {
                auto&& [row, line] = e;
                return line
                    | std::views::enumerate
                    | std::views::filter([](auto&& entry) {
                          auto&& [_, symbol] = entry;
                          return symbol == '#';
                      })
                    | std::views::transform([row](auto&& entry) {
                          auto&& [col, symbol] = entry;
                          return Coordinate{row, col};
                      });
                })
            | std::views::join;
        // clang-format on

        return std::ranges::fold_left(all_galaxies, std::int64_t{0}, [&](std::int64_t result, Coordinate position) {
            const auto inner_result =
                std::ranges::fold_left(seen, std::int64_t{0}, [&](std::int64_t inner_result, Coordinate inner_position) {
                    return inner_result + distance(position, inner_position);
                });
            seen.push_back(position);
            return result + inner_result;
        });
    }
}  // namespace

int main() {
    const auto path       = std::string{"input.data"};
    const auto image_data = core::io::read_file(path, true);
    const auto image      = core::strings::split(core::strings::strip(image_data), "\n");

    const auto distances_sum = sum_of_distances(image, 2);
    std::cout << std::format("The sum of distances for all galaxies after expansion is {}\n", distances_sum);

    const auto distances_sum_with_old = sum_of_distances(image, 1'000'000);
    std::cout << std::format(
        "The sum of distances for all galaxies in an old universe after expansion is {}\n", distances_sum_with_old
    );

    return 0;
}
