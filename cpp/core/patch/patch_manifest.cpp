#include "patch_manifest.h"
#include<fstream>
#include<stdexcept>
using namespace gddelta::patch;

void PatchManifestWriter::write(
    const workspace::WorkspaceDiff& diff,
    const std::filesystem::path& base_dir,
    const std::filesystem::path& modified_dir,
    const std::filesystem::path& output_path
) const {
    std::ofstream output(output_path, std::ios::trunc);
    if(!output) {
        throw std::runtime_error("Failed to open patch manifest for writing: " + output_path.string());
    }
    
    output
    << "base_dir=" << base_dir.generic_string() << "\n"
    << "modified_dir=" << modified_dir.generic_string() << "\n"
    << "added=" << diff.added.size() << "\n"
    << "modified=" << diff.modified.size() << "\n"
    << "removed=" << diff.removed.size() << "\n"
    << "\n[added]\n";
    
    for(const auto& file : diff.added) {
        output << file.relative_path << "\n";
    }
    
    output << "\n[modified]\n";
    for (const auto& file : diff.modified) {
        output << file.relative_path << "\n";
    }
    
    output << "\n[removed]\n";
    for (const auto& relative_path : diff.removed) {
        output << relative_path << "\n";
    }
}