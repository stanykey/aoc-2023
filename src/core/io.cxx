#include <core/io.hxx>

#include <istream>
#include <string>


namespace core::io {
    auto read_line(std::istream& input) -> std::string {
        auto line = std::string{};
        std::getline(input, line);
        return line;
    }
}  // namespace core::io
