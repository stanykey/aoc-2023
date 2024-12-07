#include <core/io.hxx>

#include <fstream>
#include <ios>
#include <istream>
#include <iterator>
#include <string>


namespace core::io {
    auto read_line(std::istream& stream) -> std::string {
        auto line = std::string{};
        std::getline(stream, line);
        return line;
    }

    auto read_file(const std::string& path, bool as_text) -> std::string {
        auto mode   = as_text ? std::ios::in : (std::ios::binary | std::ios::in | std::ios::ate);
        auto stream = std::ifstream{path, mode};
        if (!stream) {
            throw std::ios::failure{"Failed to open file"};
        }

        if (as_text) {
            // Read as text (standard line-by-line or full-stream read)
            return std::string{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
        }

        const auto file_size = stream.tellg();
        stream.seekg(0, std::ios::beg);

        // Read the file content into a string
        auto content = std::string(file_size, '\0');
        if (!stream.read(content.data(), file_size)) {
            throw std::ios::failure{"Failed to read file content"};
        }

        return content;
    }

}  // namespace core::io
