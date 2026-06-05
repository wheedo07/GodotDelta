#pragma once

#include<cstdint>
#include<filesystem>
#include<optional>
namespace gddelta::pck {
    struct EmbeddedPckInfo {
        enum class ExecutableType {
            pe,
            elf,
        };
        
        std::uint64_t section_header_offset = 0;
        std::uint64_t embedded_offset = 0;
        std::uint64_t embedded_size = 0;
        std::uint64_t actual_offset = 0;
        std::uint64_t actual_size = 0;
        std::uint32_t section_bit_size = 32;
        ExecutableType executable_type = ExecutableType::elf;
    };
    
    class EmbeddedPckHandler {
        public:
            [[nodiscard]] static bool is_probable_windows_executable(const std::filesystem::path& path);
            [[nodiscard]] static std::optional<EmbeddedPckInfo> find_embedded_pck(const std::filesystem::path& path);
            static void embed_pck_into_executable(
                const std::filesystem::path& base_executable,
                const std::filesystem::path& source_pck,
                const std::filesystem::path& output_executable
            );
            static void fixup_embedded_executable_headers(
                const std::filesystem::path& template_executable,
                const std::filesystem::path& output_executable
            );
    };
}