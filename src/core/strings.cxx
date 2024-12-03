#include <core/strings.hxx>

#include <string_view>
#include <vector>


namespace core::strings {
    auto split(std::string_view source, std::string_view delimiter) -> std::vector<std::string_view> {
        std::vector<std::string_view> parts;
        while (!source.empty()) {
            const auto position = source.find(delimiter);

            parts.emplace_back(source.substr(0, position));

            source.remove_prefix(parts.back().size());
            if (!source.empty()) {
                source.remove_prefix(delimiter.size());
            }
        }

        return parts;
    }

    auto trim(std::string_view str) -> std::string_view {
        while (str.starts_with(' ')) {
            str.remove_prefix(1);
        }

        while (str.ends_with(' ')) {
            str.remove_suffix(1);
        }

        return str;
    }
}  // namespace core::strings
