#include <core/io.hxx>
#include <core/strings.hxx>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>


namespace {
    auto hash(std::string_view str) -> std::size_t {
        return std::ranges::fold_left(str, std::size_t{0}, [](std::size_t accumulated, char symbol) {
            constexpr auto magic_multiplier = 17ul;
            constexpr auto magic_reminder   = 256ul;
            accumulated += static_cast<int>(symbol);
            accumulated *= magic_multiplier;
            return accumulated % magic_reminder;
        });
    }

    auto hash_sum(const std::vector<std::string_view>& instructions) -> std::size_t {
        return std::ranges::fold_left(instructions | std::views::transform(hash), 0ul, std::plus<>{});
    }
}  // namespace


int main() {
    const auto path         = std::filesystem::path{"input.data"};
    const auto data         = core::io::read_file(path, true);
    const auto instructions = core::strings::split(core::strings::strip(data), ",");

    // for (const auto& instruction : instructions) {
    //     std::cout << std::format("<{}> becomes <{}>\n", instruction, hash(instruction));
    // }

    std::cout << std::format("The sum of results is {} becomes\n", hash_sum(instructions));

    return 0;
}
