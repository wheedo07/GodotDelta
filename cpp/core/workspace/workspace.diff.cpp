#include "workspace.h"
using namespace gddelta::workspace;

WorkspaceDiff Workspace::diff(const WorkspaceSnapshot& base, const WorkspaceSnapshot& modified) const {
    WorkspaceDiff result;

    for(const auto &[relative_path, file] : modified.files) {
        const auto it = base.files.find(relative_path);
        if(it == base.files.end()) {
            result.added.push_back(file);
            continue;
        }

        const auto base_file = it->second;
        if(base_file.size != file.size || base_file.content_hash != file.content_hash) {
            result.modified.push_back(file);
        }
    }

    for(const auto &[relative_path, file] : base.files) {
        static_cast<void>(file);
        if(!modified.files.contains(relative_path)) {
            result.removed.push_back(relative_path);
        }
    }

    return result;
}