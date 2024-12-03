#include <core/io.hxx>

#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>


namespace {
    struct SeedInfo {
        std::uint64_t id          = 0;
        std::uint64_t soil        = 0;
        std::uint64_t fertilizer  = 0;
        std::uint64_t water       = 0;
        std::uint64_t light       = 0;
        std::uint64_t temperature = 0;
        std::uint64_t humidity    = 0;
        std::uint64_t location    = 0;
    };

    using FieldPtr = std::uint64_t SeedInfo::*;


    class Mapper {
    public:
        Mapper(FieldPtr from, FieldPtr to)
            : from_(from)
            , to_(to) {}

        void load_table(std::istream& in) {
            std::string line;
            while (std::getline(in, line) && !line.empty()) {
                std::istringstream data_line(line);

                auto dest_start = core::io::read<std::uint64_t>(data_line);
                auto src_start  = core::io::read<std::uint64_t>(data_line);
                auto range_size = core::io::read<std::uint64_t>(data_line);

                table_.emplace_back(std::make_tuple(src_start, src_start + range_size - 1), dest_start);
            }
        }

        void lookup(std::vector<SeedInfo>& seeds) const {
            for (auto& seed : seeds) {
                const auto from = seed.*from_;
                const auto it =
                    std::find_if(table_.cbegin(), table_.cend(), [&from](const std::tuple<Interval, std::uint64_t>& entry) {
                        const auto& [interval, _] = entry;
                        return std::get<0>(interval) <= from && from <= std::get<1>(interval);
                    });
                if (it == table_.cend()) {
                    seed.*to_ = from;
                } else {
                    const auto& [interval, value] = *it;
                    seed.*to_                     = value + from - std::get<0>(interval);
                }
            }
        }

    private:
        using Interval = std::tuple<std::uint64_t, std::uint64_t>;
        using Table    = std::vector<std::tuple<Interval, std::uint64_t>>;

        Table    table_;
        FieldPtr from_;
        FieldPtr to_;
    };


    auto load_garden_plan(const std::string& data_path) -> std::vector<SeedInfo> {
        auto plan_document = std::ifstream{data_path};

        auto line = core::io::read<std::string>(plan_document);
        if (!line.starts_with("seeds:")) {
            throw std::invalid_argument("invalid input steam");
        }

        auto seeds = std::vector<SeedInfo>{};
        std::transform(
            std::istream_iterator<std::uint64_t>(plan_document), std::istream_iterator<std::uint64_t>(),
            std::back_inserter(seeds), [](std::uint64_t id) { return SeedInfo{.id = id}; }
        );

        auto mappers = std::unordered_map<std::string, Mapper>{
            {"seed-to-soil", Mapper(&SeedInfo::id, &SeedInfo::soil)},
            {"soil-to-fertilizer", Mapper(&SeedInfo::soil, &SeedInfo::fertilizer)},
            {"fertilizer-to-water", Mapper(&SeedInfo::fertilizer, &SeedInfo::water)},
            {"water-to-light", Mapper(&SeedInfo::water, &SeedInfo::light)},
            {"light-to-temperature", Mapper(&SeedInfo::light, &SeedInfo::temperature)},
            {"temperature-to-humidity", Mapper(&SeedInfo::temperature, &SeedInfo::humidity)},
            {"humidity-to-location", Mapper(&SeedInfo::humidity, &SeedInfo::location)},
        };

        plan_document.clear();
        while (std::getline(plan_document, line)) {
            constexpr auto map_suffix = std::string_view{"map:"};
            if (line.ends_with(map_suffix)) {
                const auto name   = line.substr(0, line.size() - (map_suffix.size() + 1));
                auto&      mapper = mappers.at(name);
                mapper.load_table(plan_document);
                mapper.lookup(seeds);
            }
        }

        return seeds;
    }
}  // namespace

auto main() -> int {
    const auto garden_plan = load_garden_plan("input.data");
    const auto result = std::min_element(garden_plan.cbegin(), garden_plan.cend(), [](const auto& lhs, const auto& rhs) {
                            return lhs.location < rhs.location;
                        })->location;
    std::cout << std::format("The result value is {}\n", result);

    return 0;
}
