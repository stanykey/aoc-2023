#ifndef CORE_IO_HXX
#define CORE_IO_HXX

#include <algorithm>
#include <cstddef>
#include <istream>
#include <ranges>
#include <string>
#include <vector>


namespace core::io {
    template<typename T>
    auto read(std::istream& stream) -> T {
        auto value = T{};
        stream >> value;
        return value;
    }

    template<typename T>
    auto read(std::istream& stream, std::size_t count) -> std::vector<T> {
        auto result = std::vector<T>{};

        result.reserve(count);
        while (count-- && stream) {
            result.emplace_back(read<T>(stream));
        }

        return result;
    }

    template<typename T>
    auto read_sequence(std::istream& stream) -> std::vector<T> {
        auto sequence = std::vector<T>{};
        std::ranges::copy(std::views::istream<T>(stream), std::back_inserter(sequence));
        return sequence;
    }

    template<typename Filter>
    auto read_string(std::istream& stream, Filter filter) -> std::string {
        auto value = std::string{};
        while (stream and filter(stream.peek())) {
            value += static_cast<char>(stream.get());
        }
        return value;
    }

    auto read_line(std::istream& stream) -> std::string;

    auto read_file(const std::string& path, bool as_text) -> std::string;

    template<typename Path>
    auto read_file(const Path& path, bool as_text) -> std::string {
        return read_file(path.string(), as_text);
    }

    auto skip(std::istream& stream, std::string_view ignored) -> std::istream&;

    template<typename Filter>
    auto skip(std::istream& stream, Filter filter) -> std::istream& {
        while (stream and filter(stream.peek())) {
            stream.ignore();
        }
        return stream;
    }
}  // namespace core::io

#endif  // CORE_IO_HXX
