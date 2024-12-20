#include "connection-mesh.hxx"
#include "core/io.hxx"

#include <algorithm>
#include <cctype>
#include <istream>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>


namespace {
    auto read_label(std::istream& stream) -> std::string {
        const auto label = core::io::read_string(stream, [](int symbol) -> bool { return std::isalpha(symbol) != 0; });
        core::io::skip(stream, [](int symbol) -> bool { return symbol != '\n' and std::isalpha(symbol) == 0; });
        return label;
    }

    auto parse_connections(std::istream& stream) -> std::vector<std::string> {
        auto connections = std::vector<std::string>{};
        while (stream && stream.peek() != '\n') {
            connections.push_back(read_label(stream));
        }
        return connections;
    }

    auto parse_connection_mesh(std::istream& stream) -> ConnectionMesh {
        auto mesh         = ConnectionMesh{};
        auto conjunctions = std::vector<std::string>{};
        while (stream) {
            const auto type = stream.peek();
            if (std::isalnum(type) == 0) {
                stream.ignore();
            }

            const auto label       = read_label(stream);
            auto       connections = parse_connections(stream);
            switch (type) {
                case '%': {
                    mesh.modules[label] = FlipFlop{label, Signal::Strength::LOW, std::move(connections)};
                    break;
                }

                case '&': {
                    mesh.modules[label] = Conjunction{label, {}, std::move(connections)};
                    conjunctions.push_back(label);
                    break;
                }

                default: {
                    mesh.modules[label] = Broadcaster{label, std::move(connections)};
                    break;
                }
            }
            stream.ignore();
        }

        for (const auto& label : conjunctions) {
            for (auto& [other_label, other] : mesh.modules) {
                if (std::visit([&](auto& module) { return module.sends_to(label); }, other)) {
                    std::get<Conjunction>(mesh.modules[label]).state[other_label] = Signal::Strength::LOW;
                }
            }
        }

        return mesh;
    }
}  // namespace

auto Button::press(ConnectionMesh& mesh) -> void {
    mesh.send_signal(Signal::Strength::LOW, "button", "broadcaster");
}

auto Broadcaster::receive_signal(ConnectionMesh& mesh, Signal::Strength signal, const std::string&) -> void {
    for (const auto& dest : connections) {
        mesh.send_signal(signal, label, dest);
    }
}

auto Conjunction::receive_signal(ConnectionMesh& mesh, Signal::Strength signal, const std::string& from) -> void {
    state[from] = signal;

    const auto is_high = [](Signal::Strength signal) { return signal == Signal::Strength::HIGH; };
    signal = std::ranges::all_of(state | std::views::values, is_high) ? Signal::Strength::LOW : Signal::Strength::HIGH;
    for (const auto& dest : connections) {
        mesh.send_signal(signal, label, dest);
    }
}

auto FlipFlop::receive_signal(ConnectionMesh& mesh, Signal::Strength signal, const std::string&) -> void {
    if (signal == Signal::Strength::LOW) {
        flip();
        for (const auto& dest : connections) {
            mesh.send_signal(state, label, dest);
        }
    }
}

auto ConnectionMesh::send_signal(Signal::Strength signal, const std::string& from, const std::string& to) -> void {
    switch (signal) {
        case Signal::Strength::HIGH: {
            pending_signals.emplace(Signal::Strength::HIGH, from, to);
            high_signals++;
            break;
        }

        case Signal::Strength::LOW: {
            pending_signals.emplace(Signal::Strength::LOW, from, to);
            low_signals++;
            break;
        }
    }
}

auto ConnectionMesh::process_signal() -> bool {
    if (pending_signals.empty()) {
        return false;
    }

    const auto [signal, from, to] = pending_signals.front();
    pending_signals.pop();

    const auto it = modules.find(to);
    if (it == modules.end()) {
        return true;
    }

    std::visit([&](auto& module) { module.receive_signal(*this, signal, from); }, it->second);
    return true;
}

auto operator>>(std::istream& stream, ConnectionMesh& mesh) -> std::istream& {
    mesh = parse_connection_mesh(stream);
    return stream;
}
