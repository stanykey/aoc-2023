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

    const auto MAPPING = Mapping{
        {'2', Card::TWO},   {'3', Card::THREE}, {'4', Card::FOUR}, {'5', Card::FIVE}, {'6', Card::SIX},
        {'7', Card::SEVEN}, {'8', Card::EIGHT}, {'9', Card::NINE}, {'T', Card::TEN},  {'J', Card::JACK},
        {'Q', Card::QUEEN}, {'K', Card::KING},  {'A', Card::ACE},
    };


    Combination determine_combination(const Hand& hand) {
        auto frequency = std::unordered_map<Card, std::uint8_t>{};
        for (const auto& card : hand) {
            frequency[card]++;
        }

        auto counts = std::vector<std::uint8_t>{};
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

    class Player {
    public:
        Player(Hand hand, std::size_t bid)
            : hand_{hand}
            , bid_{bid}
            , combo_{determine_combination(hand_)} {}

        [[nodiscard]] auto hand() const -> const Hand& {
            return hand_;
        }

        [[nodiscard]] auto bid() const -> std::size_t {
            return bid_;
        }

        [[nodiscard]] auto combo() const -> Combination {
            return combo_;
        }

        auto operator<(const Player& other) const -> bool {
            if (combo() == other.combo()) {
                return hand() < other.hand();
            }

            return combo() < other.combo();
        }

    private:
        Hand        hand_;
        std::size_t bid_;
        Combination combo_;
    };
}  // namespace


namespace {
    auto parse_hand(std::string_view hand_str, const Mapping& mapping) -> Hand {
        auto hand = Hand{};
        std::transform(hand_str.cbegin(), hand_str.cend(), hand.begin(), [&mapping](char card) { return mapping.at(card); });
        return hand;
    }

    auto load_players(const std::string& path, const Mapping& mapping) -> std::vector<Player> {
        auto players = std::vector<Player>{};

        auto stream = std::ifstream{path};
        auto record = std::string{};
        while (std::getline(stream, record)) {
            const auto delimiter = record.find(' ');
            const auto hand      = parse_hand({record.data(), delimiter}, mapping);
            const auto bid       = core::numbers::parse<std::size_t>(record.data() + delimiter);
            players.emplace_back(hand, bid);
        }

        std::ranges::sort(players, std::less<>{});
        return players;
    }

    auto get_total_score(const std::string& path, const Mapping& mapping) -> std::size_t {
        const auto players = load_players(path, mapping);

        auto result = 0ul;
        for (auto i = 0u; i != players.size(); i++) {
            result += players[i].bid() * (i + 1);
        }

        return result;
    }
}  // namespace


int main() {
    const auto records_path = std::string{"input.data"};

    const auto score = get_total_score(records_path, MAPPING);
    std::cout << std::format("The total score is {}\n", score);

    return 0;
}
