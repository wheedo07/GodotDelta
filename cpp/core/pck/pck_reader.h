#pragma once

#include "core/pck/pck_format.h"
#include<filesystem>
#include<fstream>
#include<optional>
#include<string_view>
#include<vector>
namespace gddelta::pck {
    class PckReader {
        private:
            void open_stream(std::ifstream &stream, const std::filesystem::path& path, std::uint64_t pck_start_offset);

            std::filesystem::path path_;
            PckHeader header_ {};
            std::vector<PckEntry> entries_;

        public:
            PckReader() = default;
            void open(const std::filesystem::path& path);
            [[nodiscard]] const std::filesystem::path& path() const noexcept;
            [[nodiscard]] const PckHeader& header() const noexcept;
            [[nodiscard]] const std::vector<PckEntry>& entries() const noexcept;
            [[nodiscard]] std::optional<PckEntry> find_entry(std::string_view pack_path) const;
            [[nodiscard]] std::vector<std::uint8_t> read_entry_data(const PckEntry& entry) const;
    };
}