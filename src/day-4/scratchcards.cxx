#include <algorithm>
#include <charconv>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


namespace {
    auto trim(std::string_view str) -> std::string_view {
        while (str.starts_with(' ')) {
            str.remove_prefix(1);
        }

        while (str.ends_with(' ')) {
            str.remove_suffix(1);
        }

        return str;
    }

    auto to_number(std::string_view str) -> std::uint32_t {
        str = trim(str);

        std::uint32_t number = 0;
        if (std::from_chars(str.data(), str.data() + str.size(), number).ec != std::errc{}) {
            throw std::runtime_error(std::format("Failed to parse integer from <{}>", str));
        }
        return number;
    }

    auto make_subview(std::string_view source, char symbol) -> std::string_view {
        const auto symbol_position = source.find(symbol);
        return source.substr(0, symbol_position);
    }

    auto parse_numbers(std::string_view record) -> std::vector<std::uint32_t> {
        auto numbers = std::vector<std::uint32_t>{};
        auto stream  = std::istringstream{std::string{record}};

        std::copy(
            std::istream_iterator<std::uint32_t>{stream}, std::istream_iterator<std::uint32_t>{}, std::back_inserter(numbers)
        );

        return numbers;
    }


    struct Card {
    public:
        static auto load(std::string_view record) -> Card {
            auto card = Card{};

            // Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
            record.remove_prefix(4);  // Remove "Card"

            // extract id
            const auto id = make_subview(record, ':');
            card.id_      = to_number(id);
            record.remove_prefix(id.size() + 1);

            // read winning numbers
            const auto winning_numbers = make_subview(record, '|');
            card.winning_numbers_      = parse_numbers(winning_numbers);
            std::ranges::sort(card.winning_numbers_);
            record.remove_prefix(winning_numbers.size() + 1);

            // read draft numbers
            card.draft_numbers_ = parse_numbers(record);
            std::ranges::sort(card.draft_numbers_);

            return card;
        }

        auto get_points() const -> std::uint32_t {
            if (points_) {
                return points_.value();
            }

            auto matches = std::vector<std::uint32_t>{};
            std::set_intersection(
                winning_numbers_.cbegin(), winning_numbers_.cend(), draft_numbers_.cbegin(), draft_numbers_.cend(),
                std::back_inserter(matches)
            );

            return points_.emplace(1 << (matches.size() - 1));
        }

    private:
        std::uint32_t                        id_{0};
        std::vector<std::uint32_t>           winning_numbers_;
        std::vector<std::uint32_t>           draft_numbers_;
        mutable std::optional<std::uint32_t> points_;
    };


    auto load_cards(const std::string& path) -> std::vector<Card> {
        std::ifstream file{path};
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::vector<Card> cards;

        std::string record;
        while (std::getline(file, record)) {
            cards.emplace_back(Card::load(record));
        }

        return cards;
    }

    auto get_cards_points(const std::vector<Card>& cards) -> std::vector<std::uint32_t> {
        auto scores = std::vector<std::uint32_t>{};
        std::ranges::transform(cards, std::back_inserter(scores), [](const Card& card) { return card.get_points(); });
        return scores;
    }

}  // namespace


auto main() -> int {
    try {
        const auto cards = load_cards("input.data");

        const auto cards_points = get_cards_points(cards);
        const auto total_points = std::reduce(cards_points.cbegin(), cards_points.cend());
        std::cout << std::format("The total points worth is {}\n", total_points);
    } catch (const std::exception& ex) {  // NOLINT: std::exception if fine here
        std::cerr << std::format("Critical error: {}\n", ex.what());
        return 1;
    }

    return 0;
}
