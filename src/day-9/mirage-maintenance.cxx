#include <core/numbers.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>
#include <vector>


namespace {
    auto load_histogram(const std::string& path) -> std::vector<std::vector<std::int64_t>> {
        auto stream = std::ifstream{path};

        auto histogram = std::vector<std::vector<std::int64_t>>{};
        auto line      = std::string{};
        while (std::getline(stream, line)) {
            histogram.emplace_back(core::numbers::parse_numbers<std::int64_t>(line));
        }
        return histogram;
    }

    auto sum_of_predictions(const std::vector<std::vector<std::int64_t>>& histogram) -> std::int64_t {
        const auto predict = [&](const std::vector<std::int64_t>& sequence) {
            auto tails = std::vector<std::int64_t>(1, sequence.back());

            auto current   = sequence;
            auto transform = std::views::transform([&](std::size_t i) { return current[i] - current[i - 1]; });
            while (std::ranges::any_of(current, [](std::int64_t value) { return value != 0; })) {
                auto next = std::vector<std::int64_t>{};
                std::ranges::copy(std::views::iota(1ul, current.size()) | transform, std::back_inserter(next));
                tails.push_back(next.back());
                current = std::move(next);
            }
            return std::reduce(tails.cbegin(), tails.cend());
        };

        const auto predictions = histogram | std::views::transform(predict);
        return std::reduce(predictions.begin(), predictions.end());
    }

    auto sum_of_backwards_predictions(const std::vector<std::vector<std::int64_t>>& histogram) -> std::int64_t {
        const auto predict = [&](const std::vector<std::int64_t>& sequence) {
            auto heads = std::vector<std::int64_t>(1, sequence.front());

            auto current   = sequence;
            auto transform = std::views::transform([&](std::size_t i) { return current[i] - current[i - 1]; });
            while (std::ranges::any_of(current, [](std::int64_t value) { return value != 0; })) {
                auto next = std::vector<std::int64_t>{};
                std::ranges::copy(std::views::iota(1ul, current.size()) | transform, std::back_inserter(next));
                heads.push_back(next.front());
                current = std::move(next);
            }

            const auto reversed_heads = heads | std::views::reverse;
            return std::reduce(reversed_heads.begin(), reversed_heads.end(), std::int64_t{0}, [](int acc, int value) {
                return value - acc;
            });
        };

        const auto predictions = histogram | std::views::transform(predict);
        return std::reduce(predictions.begin(), predictions.end());
    }
}  // namespace


int main() {
    const auto path      = std::string{"input.data"};
    const auto histogram = load_histogram(path);

    const auto predictions_sum = sum_of_predictions(histogram);
    std::cout << std::format("the sum of these extrapolated values is {}\n", predictions_sum);

    const auto backwards_predictions_sum = sum_of_backwards_predictions(histogram);
    std::cout << std::format("the sum of these backwards predictions is {}\n", backwards_predictions_sum);
    return 0;
}
