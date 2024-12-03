#include <core/numbers.hxx>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace {
    auto make_subview(std::string_view source, char symbol) -> std::string_view {
        const auto symbol_position = source.find(symbol);
        return source.substr(0, symbol_position);
    }


    struct Card {
    public:
        static auto load(std::string_view record) -> Card {
            auto card = Card{};

            // Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
            record.remove_prefix(4);  // Remove "Card"

            // extract id
            const auto id = make_subview(record, ':');
            card.id_      = core::numbers::parse<std::uint32_t>(id);
            record.remove_prefix(id.size() + 1);

            // read winning numbers
            const auto winning_numbers = make_subview(record, '|');
            card.winning_numbers_      = core::numbers::parse_numbers<std::uint32_t>(winning_numbers);
            std::ranges::sort(card.winning_numbers_);
            record.remove_prefix(winning_numbers.size() + 1);

            // read draft numbers
            card.draft_numbers_ = core::numbers::parse_numbers<std::uint32_t>(record);
            std::ranges::sort(card.draft_numbers_);

            return card;
        }

        auto id() const -> std::uint32_t {
            return id_;
        }

        auto get_points() const -> std::uint32_t {
            return (1 << (get_matches().size() - 1));
        }

        auto get_matches() const -> const std::vector<std::uint32_t>& {
            if (matches_.empty()) {
                std::set_intersection(
                    winning_numbers_.cbegin(), winning_numbers_.cend(), draft_numbers_.cbegin(), draft_numbers_.cend(),
                    std::back_inserter(matches_)
                );
            }
            return matches_;
        }

    private:
        std::uint32_t                      id_{0};
        std::vector<std::uint32_t>         winning_numbers_;
        std::vector<std::uint32_t>         draft_numbers_;
        mutable std::vector<std::uint32_t> matches_;
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

    auto calculate_game_result(const std::vector<Card>& cards) -> std::uint32_t {
        auto counter = std::unordered_map<std::uint32_t, std::uint32_t>{};

        const auto copy_cards = [&](std::size_t id, std::size_t count, std::size_t multiplier) {
            const auto max_id = cards.size() + 1;
            while (id != max_id && count != 0) {
                counter[id] += multiplier;
                id++;
                count--;
            }
        };

        for (const auto& card : cards) {
            const auto& matches = card.get_matches();
            counter[card.id()]++;
            copy_cards(card.id() + 1, matches.size(), counter[card.id()]);
        }

        return std::reduce(counter.cbegin(), counter.cend(), std::uint32_t{0}, [&](std::uint32_t stored, const auto& entry) {
            const auto [id, count] = entry;
            return stored + count;
        });
    }
}  // namespace


auto main() -> int {
    try {
        const auto cards = load_cards("input.data");

        const auto cards_points = get_cards_points(cards);
        const auto total_points = std::reduce(cards_points.cbegin(), cards_points.cend());
        std::cout << std::format("The total points worth is {}\n", total_points);

        const auto scratchcards = calculate_game_result(cards);
        std::cout << std::format("The total amount of scratchcards is {}\n", scratchcards);
    } catch (const std::exception& ex) {  // NOLINT: std::exception if fine here
        std::cerr << std::format("Critical error: {}\n", ex.what());
        return 1;
    }

    return 0;
}
