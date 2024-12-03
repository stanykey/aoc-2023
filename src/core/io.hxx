#ifndef CORE_IO_HXX
#define CORE_IO_HXX

#include <cstddef>
#include <istream>
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
}  // namespace core::io

#endif  // CORE_IO_HXX
