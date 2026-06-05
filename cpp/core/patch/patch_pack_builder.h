#pragma once

#include "core/pck/pck_format.h"
#include "core/workspace/workspace.h"
#include<filesystem>
#include<vector>
namespace gddelta::patch {
    class PatchPackBuilder {
        private:
            workspace::Workspace workspace_;

        public:
            [[nodiscard]] workspace::WorkspaceDiff diff(
                const std::filesystem::path& base_dir,
                const std::filesystem::path& modified_dir
            ) const;
            
            [[nodiscard]] std::vector<pck::PckWriteFile> build_file_list(
                const std::filesystem::path& base_dir,
                const std::filesystem::path& modified_dir
            ) const;
            
            void build_patch_pack(
                const std::filesystem::path& base_dir,
                const std::filesystem::path& modified_dir,
                const std::filesystem::path& output_pck,
                const pck::PckWriteOptions& options = {}
            ) const;
    };
}