#include "connection-mesh.hxx"

#include <core/io.hxx>

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>


namespace {
    auto count_total_pulses(ConnectionMesh& mesh, std::size_t required_presess) -> std::size_t {
        auto button = Button{};
        for (auto presses = 0ul; presses != required_presess; presses++) {
            button.press(mesh);
            while (mesh.process_signal()) {}
        }

        return mesh.low_signals * mesh.high_signals;
    }

    auto load_diagram(const std::filesystem::path& path) -> ConnectionMesh {
        auto diagram = std::ifstream{path};
        return core::io::read<ConnectionMesh>(diagram);
    }
}  // namespace

int main() {
    const auto path             = std::filesystem::path{"input.data"};
    const auto required_presess = 1000ul;

    auto mesh = load_diagram(path);
    std::cout << std::format(
        "The total number of pulses {} if we press button for {} times\n", count_total_pulses(mesh, required_presess),
        required_presess
    );

    return 0;
}
