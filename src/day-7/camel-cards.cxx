#include <core/numbers.hxx>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace {
    enum class Card : std::uint8_t {
        JOKER,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        TEN,
        JACK,
        QUEEN,
        KING,
        ACE,
    };

    enum class Combination : std::uint8_t {
        ONE,
        PAIR,
        TWO_PAIR,
        THREE,
        FULL_HOUSE,
        FOUR,
        FIVE,
    };

    constexpr auto HAND_SIZE = 5ul;

    using Hand    = std::array<Card, HAND_SIZE>;
    using Mapping = std::unordered_map<char, Card>;


    class Player {
    public:
        Player(Hand hand, std::size_t bid, Combination combo)
            : hand_{hand}
            , bid_{bid}
            , combo_{combo} {}

        template<class Rules>
        static auto create(Hand hand, std::size_t bid) -> Player {
            return {hand, bid, Rules::determine(hand)};
        }

        [[nodiscard]] auto hand() const -> const Hand& {
            return hand_;
        }

        [[nodiscard]] auto bid() const -> std::size_t {
            return bid_;
        }

        [[nodiscard]] auto combo() const -> Combination {
            return combo_;
        }

    private:
        Hand        hand_;
        std::size_t bid_;
        Combination combo_;
    };

    struct ClassicRules {
        static inline const auto MAPPING = Mapping{
            {'2', Card::TWO},   {'3', Card::THREE}, {'4', Card::FOUR}, {'5', Card::FIVE}, {'6', Card::SIX},
            {'7', Card::SEVEN}, {'8', Card::EIGHT}, {'9', Card::NINE}, {'T', Card::TEN},  {'J', Card::JACK},
            {'Q', Card::QUEEN}, {'K', Card::KING},  {'A', Card::ACE},
        };

        static auto determine(const Hand& hand) -> Combination {
            auto frequency = std::unordered_map<Card, std::size_t>{};
            for (const auto& card : hand) {
                frequency[card]++;
            }

            auto counts = std::vector<std::size_t>{};
            for (const auto& [card, count] : frequency) {
                counts.push_back(count);
            }
            std::ranges::sort(counts, std::greater<>{});

            // NOLINTBEGIN
            switch (counts.front()) {
                case 5: return Combination::FIVE;
                case 4: return Combination::FOUR;
                case 3: return (counts.size() == 2) ? Combination::FULL_HOUSE : Combination::THREE;
                case 2: return (counts.size() == 3) ? Combination::TWO_PAIR : Combination::PAIR;
                default: return Combination::ONE;
            }
            // NOLINTEND
        }

        static auto compare(const Player& lhs, const Player& rhs) -> bool {
            if (lhs.combo() == rhs.combo()) {
                return lhs.hand() < rhs.hand();
            }

            return lhs.combo() < rhs.combo();
        }
    };

    struct JokerRules {
        static inline const auto MAPPING = Mapping{
            {'2', Card::TWO},   {'3', Card::THREE}, {'4', Card::FOUR}, {'5', Card::FIVE}, {'6', Card::SIX},
            {'7', Card::SEVEN}, {'8', Card::EIGHT}, {'9', Card::NINE}, {'T', Card::TEN},  {'J', Card::JOKER},
            {'Q', Card::QUEEN}, {'K', Card::KING},  {'A', Card::ACE},
        };

        static auto determine(const Hand& hand) -> Combination {
            auto frequency = std::unordered_map<Card, std::size_t>{};
            for (const auto& card : hand) {
                frequency[card]++;
            }

            // Handle jokers
            if (frequency.contains(Card::JOKER)) {
                auto jokers = frequency[Card::JOKER];
                frequency.erase(Card::JOKER);
                if (frequency.empty()) {
                    return Combination::FIVE;
                }

                auto max_it = std::max_element(frequency.begin(), frequency.end(), [](const auto& lhs, const auto& rhs) {
                    const auto [left_card, left_count]   = lhs;
                    const auto [right_card, right_count] = rhs;
                    return left_count < right_count;
                });
                max_it->second += jokers;
            }

            const auto marker      = frequency.begin()->second;
            const auto has_triplet = std::any_of(frequency.begin(), frequency.end(), [](const auto& entry) {
                const auto [card, count] = entry;
                return count == 3;
            });

            switch (frequency.size()) {
                case 1: return Combination::FIVE;
                case 2: return (marker == 2 || marker == 3) ? Combination::FULL_HOUSE : Combination::FOUR;
                case 3: return has_triplet ? Combination::THREE : Combination::TWO_PAIR;
                case 4: return Combination::PAIR;
                default: return Combination::ONE;
            }
        }

        static auto compare(const Player& lhs, const Player& rhs) -> bool {
            return ClassicRules::compare(lhs, rhs);
        }
    };
}  // namespace


namespace {
    auto parse_hand(std::string_view hand_str, const Mapping& mapping) -> Hand {
        auto hand = Hand{};
        std::transform(hand_str.cbegin(), hand_str.cend(), hand.begin(), [&mapping](char card) { return mapping.at(card); });
        return hand;
    }

    template<class Rules>
    auto load_players(const std::string& path) -> std::vector<Player> {
        auto players = std::vector<Player>{};

        auto stream = std::ifstream{path};
        auto record = std::string{};
        while (std::getline(stream, record)) {
            const auto delimiter = record.find(' ');
            const auto hand      = parse_hand({record.data(), delimiter}, Rules::MAPPING);
            const auto bid       = core::numbers::parse<std::size_t>(record.data() + delimiter);
            players.emplace_back(Player::create<Rules>(hand, bid));
        }

        std::ranges::sort(players, Rules::compare);
        return players;
    }

    template<class Rules>
    auto get_total_score(const std::string& path) -> std::size_t {
        const auto players = load_players<Rules>(path);

        auto result = 0ul;
        for (auto i = 0u; i != players.size(); i++) {
            result += players[i].bid() * (i + 1);
        }

        return result;
    }
}  // namespace


int main() {
    const auto records_path = std::string{"input.data"};

    const auto classic_score = get_total_score<ClassicRules>(records_path);
    std::cout << std::format("The total score by classic rules is {}\n", classic_score);

    const auto joker_score = get_total_score<JokerRules>(records_path);
    std::cout << std::format("The total score by rules with jokers is {}\n", joker_score);

    return 0;
}
