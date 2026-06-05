#pragma once

#include "core/workspace/workspace.h"
#include<filesystem>
namespace gddelta::patch {
    class PatchManifestWriter {
        public:
            void write(
                const workspace::WorkspaceDiff& diff,
                const std::filesystem::path& base_dir,
                const std::filesystem::path& modified_dir,
                const std::filesystem::path& output_path
            ) const;
    };
}