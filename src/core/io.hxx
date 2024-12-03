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
    auto read(std::istream& input) -> T {
        T value{};
        input >> value;
        return value;
    }

    template<typename T>
    auto read(std::istream& input, std::size_t count) -> std::vector<T> {
        auto result = std::vector<T>{};

        result.reserve(count);
        while (count--) {
            auto value = read<T>(input);
            result.emplace_back(std::move(value));
        }

        return result;
    }

    template<typename T>
    auto read_sequence(std::istream& input) -> std::vector<T> {
        auto sequence = std::vector<T>{};
        std::ranges::copy(std::views::istream<T>(input), std::back_inserter(sequence));
        return sequence;
    }

    auto read_line(std::istream& input) -> std::string;
}  // namespace core::io

#endif  // CORE_IO_HXX
