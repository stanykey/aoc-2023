#include <core/io.hxx>
#include <core/strings.hxx>

#include "core/numbers.hxx"
#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
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

    class HashMap {
    public:
        static constexpr auto SIZE = 256;

        using Bucket  = std::list<std::pair<std::string, std::size_t>>;
        using Storage = std::array<Bucket, SIZE>;

    public:
        auto pop(std::string_view label) -> void {
            storage_[hash(label)].remove_if([&](const auto& pair) { return pair.first == label; });
        }

        auto push(std::string_view label, std::size_t value) -> void {
            const auto key = hash(label);
            const auto it  = std::ranges::find_if(storage_[key], [&](const auto& pair) { return pair.first == label; });
            if (it != storage_[key].end()) {
                it->second = value;
            } else {
                storage_[key].emplace_back(std::string{label}, value);
            }
        }

        [[nodiscard]] auto value() const -> std::size_t {
            auto sum = 0ul;
            for (const auto& [bucket, list] : storage_ | std::views::enumerate) {
                for (const auto& [slot, value] : list | std::views::enumerate) {
                    sum += (bucket + 1) * (slot + 1) * value.second;
                }
            }
            return sum;
        }

    private:
        Storage storage_;
    };

    auto hash_sum(const std::vector<std::string_view>& instructions) -> std::size_t {
        return std::ranges::fold_left(instructions | std::views::transform(hash), 0ul, std::plus<>{});
    }

    auto calc_focusing_power(const std::vector<std::string_view>& instructions) -> std::size_t {
        HashMap hashmap;
        for (const auto& instruction : instructions) {
            const auto delimiter = instruction.find_first_of("=-");
            const auto label     = instruction.substr(0, delimiter);
            if (instruction[delimiter] == '=') {
                const auto value = core::numbers::parse<std::size_t>(instruction.substr(delimiter + 1));
                hashmap.push(label, value);
            } else {
                hashmap.pop(label);
            }
        }
        return hashmap.value();
    }
}  // namespace


int main() {
    const auto path         = std::filesystem::path{"input.data"};
    const auto data         = core::io::read_file(path, true);
    const auto instructions = core::strings::split(core::strings::strip(data), ",");

    std::cout << std::format("The sum of results is {}\n", hash_sum(instructions));
    std::cout << std::format("The focusing power is {}\n", calc_focusing_power(instructions));

    return 0;
}
