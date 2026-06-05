#include "cli.shared.h"
#include "core/pck/pck_embedded.h"
#include "core/pck/pck_reader.h"
#include "core/pck/pck_writer.h"
#include "core/workspace/workspace.h"
#include<chrono>
#include<filesystem>
#include<iostream>
#include<optional>
#include<stdexcept>
#include<thread>

#ifdef _WIN32

#include<windows.h>

#else

#include<spawn.h>
#include<sys/types.h>
#include<unistd.h>
extern char **environ;

#endif

using namespace cli_internal;

namespace {

std::filesystem::path find_ui_executable(const std::filesystem::path& cli_path) {
    const auto cli_dir = std::filesystem::absolute(cli_path).parent_path();
#ifdef _WIN32
    const auto ui_path = cli_dir / "GodotDelta.exe";
    if(std::filesystem::exists(ui_path)) {
        return ui_path;
    }
#else
    for(const auto& candidate_name : {"GodotDelta.x86_64", "GodotDelta"}) {
        const auto ui_path = cli_dir / candidate_name;
        if(std::filesystem::exists(ui_path)) {
            return ui_path;
        }
    }
#endif
    throw std::runtime_error("Failed to find GodotDelta UI executable next to CLI: " + cli_dir.string());
}

std::filesystem::path build_temporary_copy_path(const std::filesystem::path& source_path) {
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    auto temp_path = std::filesystem::temp_directory_path();
    temp_path /= ".gddelta_base_" + source_path.filename().string() + "." + std::to_string(timestamp) + ".tmp";
    return temp_path;
}

}

void CliSupport::launch_ui(const std::filesystem::path& cli_path) const {
    const auto ui_path = find_ui_executable(cli_path);
#ifdef _WIN32
    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    auto command_line = ui_path.wstring();
    if(!CreateProcessW(
        nullptr,
        command_line.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        ui_path.parent_path().wstring().c_str(),
        &startup_info,
        &process_info
    )) {
        throw std::runtime_error("Failed to launch GodotDelta UI: " + ui_path.string());
    }

    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
#else
    pid_t pid = -1;
    auto command = ui_path.string();
    char *argv[] = {command.data(), nullptr};
    const auto result = posix_spawn(
        &pid,
        command.c_str(),
        nullptr,
        nullptr,
        argv,
        environ
    );
    if(result != 0) {
        throw std::runtime_error("Failed to launch GodotDelta UI: " + ui_path.string());
    }
#endif

    std::cout << "Started GodotDelta UI: " << ui_path << "\n";
}

BaseInputPaths CliSupport::resolve_base_input(const std::filesystem::path& base_path) const {
    BaseInputPaths resolved;
    resolved.requested_path = base_path;
    resolved.pack_path = base_path;

    if(base_path.extension() != ".exe") {
        return resolved;
    }

    if(gddelta::pck::EmbeddedPckHandler::find_embedded_pck(base_path).has_value()) {
        return resolved;
    }

    const auto sibling_pck = base_path.parent_path() / (base_path.stem().string() + ".pck");
    if(std::filesystem::exists(sibling_pck)) {
        resolved.pack_path = sibling_pck;
        resolved.uses_external_pack = true;
    }
    return resolved;
}

std::filesystem::path CliSupport::create_temporary_base_copy(const std::filesystem::path& base_pck) const {
    const auto resolved = resolve_base_input(base_pck);
    const auto temp_path = build_temporary_copy_path(resolved.pack_path);
    std::error_code ec;
    std::filesystem::copy_file(resolved.pack_path, temp_path, std::filesystem::copy_options::overwrite_existing, ec);
    if(ec) {
        throw std::runtime_error("Failed to create temporary base copy: " + resolved.pack_path.string());
    }

    return temp_path;
}

gddelta::pck::PckReader CliSupport::open_supported_base_pack(const std::filesystem::path& base_pck) const {
    const auto resolved = resolve_base_input(base_pck);
    gddelta::pck::PckReader base_reader;
    base_reader.open(resolved.pack_path);
    if(base_reader.header().engine_major < 4) {
        throw std::runtime_error(
            "Godot 3.x is not supported. Base pack engine version is " +
            std::to_string(base_reader.header().engine_major) + "." +
            std::to_string(base_reader.header().engine_minor) + "." +
            std::to_string(base_reader.header().engine_patch)
        );
    }
    return base_reader;
}

gddelta::pck::PckWriteOptions CliSupport::build_pack_options_from_base(const std::filesystem::path& base_pck) const {
    const auto temp_copy = create_temporary_base_copy(base_pck);
    gddelta::pck::PckWriteOptions options;
    std::optional<std::string> version_error;
    {
        gddelta::pck::PckReader base_reader;
        base_reader.open(temp_copy);
        if(base_reader.header().engine_major < 4) {
            version_error = 
                "Godot 3.x is not supported. Base pack engine version is " +
                std::to_string(base_reader.header().engine_major) + "." +
                std::to_string(base_reader.header().engine_minor) + "." +
                std::to_string(base_reader.header().engine_patch);
        } else {
            options.format_version = base_reader.header().format_version;
            options.engine_major = base_reader.header().engine_major;
            options.engine_minor = base_reader.header().engine_minor;
            options.engine_patch = base_reader.header().engine_patch;
        }
    }

    std::error_code ec;
    std::filesystem::remove(temp_copy, ec);
    if(version_error.has_value()) {
        throw std::runtime_error(*version_error);
    }

    return options;
}

void CliSupport::copy_runtime_support_files(
    const std::filesystem::path& base_path,
    const std::filesystem::path& sandbox_dir
) const {
    const auto base_dir = base_path.parent_path();
    for(const auto& entry : std::filesystem::recursive_directory_iterator(base_dir)) {
        const auto candidate = entry.path();
        if(candidate == base_path) {
            continue;
        }

        const auto relative_path = std::filesystem::relative(candidate, base_dir);
        const auto destination = sandbox_dir / relative_path;
        std::error_code ec;

        if(entry.is_directory()) {
            std::filesystem::create_directories(destination, ec);
            if(ec) {
                throw std::runtime_error("Failed to create runtime support directory: " + candidate.string());
            }
            continue;
        }

        if(!entry.is_regular_file()) {
            continue;
        }

        std::filesystem::create_directories(destination.parent_path(), ec);
        if(ec) {
            throw std::runtime_error("Failed to prepare runtime support directory: " + destination.parent_path().string());
        }

        std::filesystem::copy_file(candidate, destination, std::filesystem::copy_options::overwrite_existing, ec);
        if(ec) {
            throw std::runtime_error("Failed to copy runtime support file: " + candidate.string());
        }
    }
}

std::filesystem::path CliSupport::create_cleanup_patch(
    const std::filesystem::path& sandbox_dir,
    const gddelta::pck::PckWriteOptions& options
) const {
    std::vector<gddelta::pck::PckWriteFile> removals;
    for (const auto *path : {
        ".gddeltainclude",
        ".gitattributes",
        ".gitignore",
        "export_presets.cfg",
    }) {
        gddelta::pck::PckWriteFile file;
        file.pack_path = path;
        file.removal = true;
        removals.push_back(std::move(file));
    }

    const auto cleanup_patch = sandbox_dir / ".gddelta_cleanup.tmp.pck";
    gddelta::pck::PckWriter writer;
    writer.write_files(removals, cleanup_patch, options);
    return cleanup_patch;
}

void CliSupport::copy_base_into_sandbox(
    const std::filesystem::path& base_pck,
    const std::filesystem::path& sandbox_dir
) const {
    const auto resolved = resolve_base_input(base_pck);
    std::error_code ec;
    std::filesystem::create_directories(sandbox_dir, ec);
    if(ec) {
        throw std::runtime_error("Failed to create sandbox directory: " + sandbox_dir.string());
    }

    const auto sandbox_output = sandbox_dir / resolved.pack_path.filename();
    std::filesystem::copy_file(resolved.pack_path, sandbox_output, std::filesystem::copy_options::overwrite_existing, ec);
    if(ec) {
        throw std::runtime_error("Failed to copy base file into sandbox: " + resolved.pack_path.string());
    }

    CliSupport::copy_runtime_support_files(resolved.pack_path, sandbox_dir);

    std::cout << "Prepared base sandbox " << sandbox_dir << " from " << base_pck << "\n";
}

void CliSupport::watch_workspace_diff(
    const std::filesystem::path& project_dir,
    std::uint64_t interval_ms,
    const std::function<void(const gddelta::workspace::WorkspaceDiff &)>& on_diff
) const {
    gddelta::workspace::Workspace builder;
    auto previous = builder.build(project_dir);
    
    for(;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        const auto current = builder.build(project_dir);
        const auto diff = builder.diff(previous, current);
        previous = current;
        on_diff(diff);
    }
}

void CliSupport::watch_stamp(
    std::uint64_t initial_stamp,
    std::uint64_t interval_ms,
    const std::function<std::uint64_t()>& stamp_provider,
    const std::function<void()>& on_change
) const {
    auto last_stamp = initial_stamp;

    for(;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        const auto next_stamp = stamp_provider();
        if(next_stamp == last_stamp) continue;

        last_stamp = next_stamp;
        on_change();
    }
}

void CliSupport::print_rebuild_paths(
    const std::string& label,
    const std::vector<std::string>& paths
) const {
    std::cout << label << " (" << paths.size() << ")\n";

    constexpr std::size_t max_lines = 8;
    const auto visible_count = std::min(paths.size(), max_lines);
    for(std::size_t i = 0; i < visible_count; ++i) {
        std::cout << "  - " << paths[i] << "\n";
    }

    if(paths.size() > max_lines) {
        std::cout << "  ... and " << (paths.size() - max_lines) << " more\n";
    }
}