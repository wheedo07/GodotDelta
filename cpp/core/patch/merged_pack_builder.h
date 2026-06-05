#pragma once

#include "core/pck/pck_format.h"
#include<filesystem>
namespace gddelta::patch {
    class MergedPackBuilder {
        public:
            void build_merged_pack(
                const std::filesystem::path& base_pck,
                const std::filesystem::path& patch_pck,
                const std::filesystem::path& output_pck,
                const pck::PckWriteOptions& options = {}
            ) const;
    };
}