#include "cli.commands.h"
#include "cli.shared.h"
#include "core/patch/runtime_patch_resolver.h"
#include "core/patch/patch_pack_builder.h"
#include "core/pck/pck_reader.h"
#include "core/pck/pck_writer.h"
#include "core/workspace/workspace.h"
#include<filesystem>
#include<iostream>

CliCommands::CliCommands(cli_internal::CliSupport& support):
    support_(support) {
}

void CliCommands::inspect_pack(const std::filesystem::path& path) {
    gddelta::pck::PckReader reader;
    reader.open(path);

    const auto header = reader.header();
    std::cout
    << "Path: " << reader.path() << "\n"
    << "Format version: " << header.format_version << "\n"
    << "Engine version: " << header.engine_major << "." << header.engine_minor << "." << header.engine_patch << "\n"
    << "PCK start offset: " << header.pck_start_offset << "\n"
    << "File base: " << header.file_base << "\n"
    << "Directory offset: " << header.directory_offset << "\n"
    << "Entries: " << reader.entries().size() << "\n";
    
    for(const auto& entry : reader.entries()) {
        std::cout << entry.path << " size=" << entry.size << " offset=" << entry.offset << " flags=" << entry.flags << "\n";
    }
}

void CliCommands::create_pack(const std::filesystem::path& source_dir, const std::filesystem::path& output_pck) {
    gddelta::patch::RuntimePatchResolver(source_dir).warn_if_runtime_is_stale();
    gddelta::pck::PckWriter writer;
    writer.write_from_directory(source_dir, output_pck);
    std::cout << "Created " << output_pck << " from " << source_dir << "\n";
}

void CliCommands::diff_workspace(const std::filesystem::path& base_dir, const std::filesystem::path& modified_dir) {
    gddelta::workspace::Workspace builder;

    const auto base = builder.build(base_dir);
    const auto modified = builder.build(modified_dir);
    const auto diff = builder.diff(base, modified);

    std::cout
    << "Added: " << diff.added.size() << "\n"
    << "Modified: " << diff.modified.size() << "\n"
    << "Removed: " << diff.removed.size() << "\n";
    
    for(const auto& file : diff.added) {
        std::cout << "A " << file.relative_path << "\n";
    }
    for(const auto& file : diff.modified) {
        std::cout << "M " << file.relative_path << "\n";
    }
    for(const auto& file : diff.removed) {
        std::cout << "D " << file << "\n";
    }
}

void CliCommands::build_patch_pack(
    const std::filesystem::path& base_dir,
    const std::filesystem::path& modified_dir,
    const std::filesystem::path& output_pck
) {
    gddelta::patch::PatchPackBuilder builder;
    const auto files = builder.build_file_list(base_dir, modified_dir);
    builder.build_patch_pack(base_dir, modified_dir, output_pck);

    std::cout
    << "Created patch PCK " << output_pck
    << " with " << files.size() << " changed entries\n";
}