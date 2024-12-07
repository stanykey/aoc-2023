#include <core/strings.hxx>

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string_view>
#include <vector>


namespace core::strings {
    auto split(std::string_view source, std::string_view delimiter) -> std::vector<std::string_view> {
        auto parts = std::vector<std::string_view>{};
        std::ranges::transform(std::views::split(source, delimiter), std::back_inserter(parts), [](const auto&& subrange) {
            return std::string_view{subrange.data(), subrange.size()};
        });
        return parts;
    }

    auto strip(std::string_view str, std::string_view chars) -> std::string_view {
        const auto start = str.find_first_not_of(chars);
        if (start == std::string_view::npos) {
            // If all characters are in `chars`, return an empty view
            return {};
        }

        // Find the last character not in `chars` from the end
        const auto end = str.find_last_not_of(chars);
        return str.substr(start, end - start + 1);
    }
}  // namespace core::strings
