#ifndef CONNECTION_MESH_HXX
#define CONNECTION_MESH_HXX

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <queue>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct ConnectionMesh;

struct Signal {
    enum class Strength : std::uint8_t {
        LOW,
        HIGH,
    };

    Strength    signal = Strength::LOW;
    std::string from;
    std::string to;
};

struct FlipFlop {
    std::string              label;
    Signal::Strength         state;
    std::vector<std::string> connections;

    auto flip() -> void {
        switch (state) {
            case Signal::Strength::LOW: state = Signal::Strength::HIGH; break;
            case Signal::Strength::HIGH: state = Signal::Strength::LOW; break;
        }
    }

    auto receive_signal(ConnectionMesh&, Signal::Strength, const std::string& from) -> void;
    auto sends_to(const std::string& label) -> bool {
        return std::ranges::contains(connections, label);
    }
};

struct Conjunction {
    std::string                                       label;
    std::unordered_map<std::string, Signal::Strength> state;
    std::vector<std::string>                          connections;

    auto receive_signal(ConnectionMesh&, Signal::Strength, const std::string& from) -> void;
    auto sends_to(const std::string& label) const -> bool {
        return std::ranges::contains(connections, label);
    }
};

struct Button {
    void press(ConnectionMesh& mesh);
};

struct Broadcaster {
    std::string              label;
    std::vector<std::string> connections;

    auto receive_signal(ConnectionMesh&, Signal::Strength, const std::string& from) -> void;
    auto sends_to(const std::string& label) const -> bool {
        return std::ranges::contains(connections, label);
    }
};

struct ConnectionMesh {
public:
    using Module  = std::variant<Broadcaster, Conjunction, FlipFlop>;
    using Modules = std::unordered_map<std::string, Module>;

public:
    Modules            modules;
    std::queue<Signal> pending_signals;
    std::size_t        low_signals  = 0;
    std::size_t        high_signals = 0;

public:
    auto send_signal(Signal::Strength signal, const std::string& from, const std::string& to) -> void;
    auto process_signal() -> bool;

    friend auto operator>>(std::istream& stream, ConnectionMesh& mesh) -> std::istream&;
};

#endif  // CONNECTION_MESH_HXX
