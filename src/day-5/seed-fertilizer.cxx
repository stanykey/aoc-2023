#include <core/io.hxx>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>


namespace {
    struct Range {
        std::uint64_t input_offset  = 0;
        std::uint64_t output_offset = 0;
        std::uint64_t size          = 0;

        auto operator<(const Range& other) const -> bool {
            return input_offset < other.input_offset;
        }

        friend auto operator>>(std::istream& stream, Range& range) -> std::istream& {
            return stream >> range.output_offset >> range.input_offset >> range.size;
        }
    };

    struct SeedRange {
        std::uint64_t start = 0;
        std::uint64_t size  = 0;

        auto operator<(const SeedRange& other) const -> bool {
            return start < other.start;
        }

        friend auto operator>>(std::istream& stream, SeedRange& range) -> std::istream& {
            return stream >> range.start >> range.size;
        }
    };

    struct Mapper {
        std::vector<std::uint64_t> seeds;
        std::vector<Range>         seed_to_soil;
        std::vector<Range>         soil_to_fertilizer;
        std::vector<Range>         fertilizer_to_water;
        std::vector<Range>         water_to_light;
        std::vector<Range>         light_to_temperature;
        std::vector<Range>         temperature_to_humidity;
        std::vector<Range>         humidity_to_location;

        friend std::istream& operator>>(std::istream& stream, Mapper& mapper) {
            const auto drop_text = [&stream] {
                while (std::isdigit(stream.peek()) == 0) {
                    stream.ignore();
                }
            };

            const auto read_part = [&stream, &drop_text]<typename T>(std::vector<T>& destination) {
                drop_text();
                destination = core::io::read_sequence<T>(stream);
                std::ranges::sort(destination, std::less<>{});
                stream.clear();
            };

            read_part(mapper.seeds);
            read_part(mapper.seed_to_soil);
            read_part(mapper.soil_to_fertilizer);
            read_part(mapper.fertilizer_to_water);
            read_part(mapper.water_to_light);
            read_part(mapper.light_to_temperature);
            read_part(mapper.temperature_to_humidity);
            read_part(mapper.humidity_to_location);

            return stream;
        }

        static auto convert(std::uint64_t input, const std::vector<Range>& mapper) -> std::uint64_t {
            auto it = std::ranges::upper_bound(mapper, Range{input, 0, 0}, std::less<>{});

            // If there is none, then no conversion
            if (it == mapper.begin()) {
                return input;
            }
            it = std::prev(it);

            // If we are outside the bounds of the relevant mapping, then no conversion
            if ((input - it->input_offset) > it->size) {
                return input;
            }

            return (input - it->input_offset) + it->output_offset;
        }

        [[nodiscard]] auto seed_to_location(std::uint64_t seed) const -> std::uint64_t {
            const auto soil        = convert(seed, seed_to_soil);
            const auto fertilizer  = convert(soil, soil_to_fertilizer);
            const auto water       = convert(fertilizer, fertilizer_to_water);
            const auto light       = convert(water, water_to_light);
            const auto temperature = convert(light, light_to_temperature);
            const auto humidity    = convert(temperature, temperature_to_humidity);
            return convert(humidity, humidity_to_location);
        }
    };


    struct RangeMapper {
        std::vector<SeedRange> seeds;
        std::vector<Range>     seed_to_soil;
        std::vector<Range>     soil_to_fertilizer;
        std::vector<Range>     fertilizer_to_water;
        std::vector<Range>     water_to_light;
        std::vector<Range>     light_to_temperature;
        std::vector<Range>     temperature_to_humidity;
        std::vector<Range>     humidity_to_location;

        friend std::istream& operator>>(std::istream& stream, RangeMapper& mapper) {
            const auto drop_text = [&stream] {
                while (std::isdigit(stream.peek()) == 0) {
                    stream.ignore();
                }
            };

            const auto read_part = [&stream, &drop_text]<typename T>(std::vector<T>& destination) {
                drop_text();
                destination = core::io::read_sequence<T>(stream);
                std::ranges::sort(destination, std::less<>{});
                stream.clear();
            };

            read_part(mapper.seeds);
            read_part(mapper.seed_to_soil);
            read_part(mapper.soil_to_fertilizer);
            read_part(mapper.fertilizer_to_water);
            read_part(mapper.water_to_light);
            read_part(mapper.light_to_temperature);
            read_part(mapper.temperature_to_humidity);
            read_part(mapper.humidity_to_location);

            return stream;
        }

        static auto convert(const std::vector<SeedRange>& input, const std::vector<Range>& map) -> std::vector<SeedRange> {
            auto it = std::ranges::upper_bound(map, Range{input[0].start, 0, 0}, std::less<>{});
            if (it != map.begin()) {
                it = std::prev(it);
            }

            // For each seed range in the input (the ranges are already sorted)
            std::vector<SeedRange> output;
            for (auto [start, size] : input) {
                while (size > 0) {
                    if (it == map.end()) {
                        // No conversion, no more mappings
                        output.push_back({start, size});
                        size = 0;
                    } else if (start < it->input_offset) {
                        // No conversion
                        // (initial part of the range not covered by a mapping)
                        const auto actual = std::min(size, it->input_offset - start);
                        output.push_back({start, actual});
                        start += actual;
                        size -= actual;
                    } else if ((start - it->input_offset) >= it->size) {
                        // The current mapping is no longer relevant
                        ++it;
                    } else {
                        // Actual conversion
                        const auto actual = std::min((it->input_offset + it->size) - start, size);
                        output.push_back({start - it->input_offset + it->output_offset, actual});
                        start += actual;
                        size -= actual;
                    }
                }
            }
            std::ranges::sort(output, std::less<>{});
            return output;
        }

        [[nodiscard]] auto all_seed_locations() const -> std::vector<SeedRange> {
            const auto soil        = convert(seeds, seed_to_soil);
            const auto fertilizer  = convert(soil, soil_to_fertilizer);
            const auto water       = convert(fertilizer, fertilizer_to_water);
            const auto light       = convert(water, water_to_light);
            const auto temperature = convert(light, light_to_temperature);
            const auto humidity    = convert(temperature, temperature_to_humidity);
            return convert(humidity, humidity_to_location);
        }
    };

    auto find_closest_location(const std::string& path) -> std::uint64_t {
        auto       stream = std::ifstream{path};
        const auto mapper = core::io::read<Mapper>(stream);
        if (not stream) {
            throw std::runtime_error("Failed to parse");
        }

        return std::ranges::min(mapper.seeds | std::views::transform([&mapper](std::uint64_t seed) {
                                    return mapper.seed_to_location(seed);
                                }));
    }

    auto find_closest_range_location(const std::string& path) -> std::uint64_t {
        auto       stream = std::ifstream{path};
        const auto mapper = core::io::read<RangeMapper>(stream);
        if (not stream) {
            throw std::runtime_error("Failed to parse");
        }

        return mapper.all_seed_locations().front().start;
    }

}  // namespace

auto main() -> int {
    const auto input_path = std::string{"input.data"};

    const auto closest_location = find_closest_location(input_path);
    std::cout << std::format("The result value is {}\n", closest_location);

    const auto closest_range_location = find_closest_range_location(input_path);
    std::cout << std::format("The result value is {}\n", closest_range_location);

    return 0;
}
