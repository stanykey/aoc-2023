#include "connection-mesh.hxx"

#include <core/io.hxx>

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>


namespace {
    auto count_total_pulses(ConnectionMesh mesh, std::size_t required_presess) -> std::size_t {
        auto button = Button{};
        for (auto presses = 0ul; presses != required_presess; presses++) {
            button.press(mesh);
            while (mesh.process_signal()) {}
        }

        return mesh.low_signals * mesh.high_signals;
    }

    auto find_minimum_pulses(ConnectionMesh mesh, const std::string& target) -> std::size_t {
        mesh.set_track_connection(target);

        auto button  = Button{};
        auto presses = 0ul;
        auto last    = std::unordered_map<std::string, std::int64_t>{};
        auto loops   = std::unordered_map<std::string, std::int64_t>{};

        const auto not_zero = [](std::int64_t value) { return value != 0; };
        while (loops.size() != 4 || !std::ranges::all_of(loops | std::views::values, not_zero)) {
            button.press(mesh);
            presses++;
            while (mesh.process_signal()) {};

            // keep track of last seen high signal and verify that it is periodic
            for (const auto& trigger : mesh.triggers) {
                if (last[trigger] != 0) {
                    if (loops[trigger] == 0) {
                        loops[trigger] = presses - last[trigger];
                    } else {
                        if (loops[trigger] != (presses - last[trigger])) {
                            throw std::runtime_error("Failed loop");
                        }
                    }
                }
                last[trigger] = presses;
            }
            mesh.triggers.clear();
        }
        return std::ranges::fold_left(loops | std::views::values, 1ul, std::lcm<std::int64_t, std::int64_t>);
    }

    auto load_diagram(const std::filesystem::path& path) -> ConnectionMesh {
        auto diagram = std::ifstream{path};
        return core::io::read<ConnectionMesh>(diagram);
    }
}  // namespace

int main() {
    const auto path = std::filesystem::path{"input.data"};
    const auto mesh = load_diagram(path);

    const auto pulse_count = count_total_pulses(mesh, 1000ul);  // NOLINT: it not magic number :)
    std::cout << std::format("The total number of pulses {} if we press button for 1000 times\n", pulse_count);

    const auto required_pushes = find_minimum_pulses(mesh, "rx");
    std::cout << std::format("We require to press button for {} times to send pulse to 'rx' module\n", required_pushes);

    return 0;
}
