#pragma once

#include "pck_format.h"
#include<filesystem>
#include<vector>
namespace gddelta::pck {
    class PckWriter {
        public:
            void write_from_directory(
                const std::filesystem::path& source_directory,
                const std::filesystem::path& output_pck,
                const PckWriteOptions& options = {}
            );

            void write_files(
                const std::vector<PckWriteFile>& files,
                const std::filesystem::path& output_pck,
                const PckWriteOptions& options = {}
            );
    };
}