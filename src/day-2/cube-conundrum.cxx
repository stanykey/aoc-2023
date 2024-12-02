#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


namespace {
    constexpr auto RED   = "red";
    constexpr auto GREEN = "green";
    constexpr auto BLUE  = "blue";


    struct Set {
        std::size_t red   = 0;
        std::size_t green = 0;
        std::size_t blue  = 0;
    };

    struct Game {
        std::size_t      id = 0;
        std::vector<Set> sets;
    };


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

    auto extract_game_id(std::string_view str) -> std::size_t {
        std::match_results<std::string_view::const_iterator> match;
        if (std::regex_match(str.cbegin(), str.cend(), match, std::regex{R"(Game (\d+))"})) {
            return std::stoul(match[1].str());
        }
        return 0;
    }

    auto parse_set_record(std::string_view record) -> Set {
        const auto parts   = split(record, ", ");
        const auto pattern = std::regex{R"((\d+)\s+(\w+))"};

        Set set;
        for (const auto part : parts) {
            std::match_results<std::string_view::const_iterator> match;
            std::regex_match(part.cbegin(), part.cend(), match, pattern);

            const auto count = std::stoul(match[1].str());
            const auto color = std::string_view{match[2].first, match[2].second};
            if (color == RED) {
                set.red = count;
            } else if (color == GREEN) {
                set.green = count;
            } else if (color == BLUE) {
                set.blue = count;
            }
        }
        return set;
    }

    auto parse_sets_record(std::string_view record) -> std::vector<Set> {
        std::vector<Set> sets;
        std::ranges::transform(split(record, "; "), std::back_inserter(sets), parse_set_record);
        return sets;
    }

    auto parse_game_record(std::string_view record) -> Game {
        const auto parts = split(record, ": ");
        return {
            .id   = extract_game_id(parts[0]),
            .sets = parse_sets_record(parts[1]),
        };
    }

    auto load_games(const std::string& path) -> std::vector<Game> {
        std::ifstream file{path};
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::vector<Game> games;

        std::string record;
        while (std::getline(file, record)) {
            games.emplace_back(parse_game_record(record));
        }

        return games;
    }

    auto is_valid_game(const Game& game, const Set& set) -> bool {
        return std::ranges::all_of(game.sets, [&set](const Set& subject) {
            return (subject.red <= set.red) && (subject.green <= set.green) && (subject.blue <= set.blue);
        });
    }

    auto get_total_score(const std::vector<Game>& games, const Set& set) -> std::size_t {
        auto valid_games = games | std::views::filter([&set](const Game& game) { return is_valid_game(game, set); });
        return std::reduce(valid_games.begin(), valid_games.end(), std::size_t{0}, [](std::size_t prefix, const Game& game) {
            return prefix + game.id;
        });
    }

    auto get_game_power_score(const Game& game) -> std::size_t {
        Set power_set;
        for (const auto set : game.sets) {
            power_set.red   = std::max(power_set.red, set.red);
            power_set.green = std::max(power_set.green, set.green);
            power_set.blue  = std::max(power_set.blue, set.blue);
        }
        return power_set.red * power_set.green * power_set.blue;
    }

    auto get_total_power_score(const std::vector<Game>& games) -> std::size_t {
        return std::reduce(games.cbegin(), games.cend(), std::size_t{0}, [](std::size_t prefix, const Game& game) {
            return prefix + get_game_power_score(game);
        });
    }

}  // namespace


auto main() -> int {
    try {
        const auto games = load_games("input.data");

        const auto session_set = Set{
            .red   = 12,
            .green = 13,
            .blue  = 14,
        };
        const auto total_score = get_total_score(games, session_set);
        std::cout << std::format("The total score is {}\n", total_score);

        const auto power_score = get_total_power_score(games);
        std::cout << std::format("The total power score is {}\n", power_score);
    } catch (const std::exception& ex) {  // NOLINT: std::exception if fine here
        std::cerr << std::format("Critical error: {}\n", ex.what());
        return 1;
    }

    return 0;
}
