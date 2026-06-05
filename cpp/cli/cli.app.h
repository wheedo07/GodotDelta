#pragma once

#include "cli.shared.h"
#include "cli.commands.h"
#include<cstdint>
#include<optional>
#include<filesystem>
#include<string_view>
#include<vector>

class CliApplication {
    private:
        cli_internal::CliSupport support_;
        CliCommands commands_;
        struct WatchCommandOptions {
            std::uint64_t interval_ms = 1000;
            std::optional<std::filesystem::path> log_file_path;
        };
        void print_usage();
        bool require_arg_count(int argc, int required_argc);
        std::uint64_t parse_interval_ms(int argc, char **argv, int index);
        WatchCommandOptions parse_watch_options(int argc, char **argv, int index);
        int run_command(std::string_view command, int argc, char **argv);
        std::vector<std::string> collect_input_paths(int start_index, int argc, char **argv);

    public:
        CliApplication();
        int run(int argc, char **argv);
};