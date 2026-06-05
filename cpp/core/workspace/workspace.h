#pragma once

#include<vector>
#include<cstdint>
#include<filesystem>
#include<string>
#include<unordered_map>
namespace gddelta::workspace {
    struct WorkspaceFile {
        std::filesystem::path absolute_path;
        std::string relative_path;
        std::uint64_t size = 0;
        std::uint64_t modified_time = 0;
        std::uint64_t content_hash = 0;
    };
    
    struct WorkspaceSnapshot {
        std::filesystem::path root;
        std::unordered_map<std::string, WorkspaceFile> files;
    };
    
    struct WorkspaceDiff {
        std::vector<WorkspaceFile> added;
        std::vector<WorkspaceFile> modified;
        std::vector<std::string> removed;
    };
    
    class Workspace {
        public:
            [[nodiscard]] WorkspaceDiff diff(const WorkspaceSnapshot& base, const WorkspaceSnapshot& modified) const;
            [[nodiscard]] WorkspaceSnapshot build(const std::filesystem::path& root) const;
    };
}