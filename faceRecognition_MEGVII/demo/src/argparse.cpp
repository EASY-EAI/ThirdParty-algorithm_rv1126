#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
//#include <exception>

#include "argparse.h"

static void throw_invalid_cmd_args() {
    //throw std::invalid_argument("unknown command argument");
    exit(1);
}

static void throw_invalid_parser() {
    //throw std::invalid_argument("invalid parser");
    exit(1);
}

ArgumentParser::ArgumentParser(const std::string& description)
        : m_description(description) {}

void ArgumentParser::add_keyword_arg(const std::string& short_name,
                                     const std::string& long_name,
                                     const std::string& action, bool required,
                                     const std::string& _default,
                                     const std::string& help) {
    m_keyword_parsers.emplace_back(short_name, long_name, action, required,
                                   _default, help);

    if (m_existing_names.find(long_name.substr(2)) != m_existing_names.end()) {
        printf("duplicate argument: %s\n", long_name.c_str());
        throw_invalid_parser();
    } else {
        m_existing_names.insert(long_name.substr(2));
    }

    if (short_name != "") {
        if (m_existing_names.find(short_name.substr(1)) !=
            m_existing_names.end()) {
            printf("duplicate argument: %s\n", short_name.c_str());
            throw_invalid_parser();
        } else {
            m_existing_names.insert(short_name.substr(1));
        }
    }
}

void ArgumentParser::add_positional_arg(const std::string& name,
                                        const std::string& narg,
                                        const std::string& help) {
    if (m_has_rest_consumer) {
        printf("positional argument after the rest positional argument: %s\n",
               name.c_str());
        throw_invalid_parser();
    }

    if (narg != "1") {
        m_has_rest_consumer = true;
    }

    m_positional_parsers.emplace_back(name, narg, help);
    if (m_existing_names.find(name) != m_existing_names.end()) {
        printf("duplicate argument: %s\n", name.c_str());
        throw_invalid_parser();
    } else {
        m_existing_names.insert(name);
    }
}

ArgumentResult ArgumentParser::parse_args(int argc, char** argv) {
    std::list<const char*> args(argv + 1, argv + argc);

    m_program_name = argv[0];

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        print_help();
        exit(0);
    }

    ArgumentResult result;
    for (auto& p : m_keyword_parsers)
        p.start_parsing(result);
    for (auto& p : m_positional_parsers)
        p.start_parsing(result);

    while (!args.empty()) {
        bool consumed = false;
        for (auto& kp : m_keyword_parsers) {
            if (kp.consume(args)) {
                consumed = true;
                break;
            }
        }

        if (!consumed) {
            if (args.front()[0] == '-') {
                printf("unknown keyword argument: %s\n", args.front());
                throw_invalid_cmd_args();
            }

            for (auto& pp : m_positional_parsers) {
                if (pp.consume(args)) {
                    consumed = true;
                    break;
                }
            }
        }
        if (!consumed) {
            printf("unknown argument %s\n", args.front());
            throw_invalid_cmd_args();
        }
    }

    for (auto& p : m_keyword_parsers)
        p.end_parsing();
    for (auto& p : m_positional_parsers)
        p.end_parsing();

    return result;
}

void ArgumentParser::print_help() const {
    printf("%s\n\n", m_description.c_str());
    printf("Usage:\n");
    printf("  %s", m_program_name.c_str());
    for (auto& p : m_positional_parsers)
        printf(" %s", p.brief_usage().c_str());
    for (auto& p : m_keyword_parsers)
        printf(" %s", p.brief_usage().c_str());
    printf("\n\nArguments:\n");
    for (auto& p : m_positional_parsers)
        p.print_help();
    for (auto& p : m_keyword_parsers)
        p.print_help();
}

ArgumentParser::KeywordParser::KeywordParser(const std::string& short_name,
                                             const std::string& long_name,
                                             const std::string& action,
                                             bool required,
                                             const std::string& _default,
                                             const std::string& help)
        : m_short_name(short_name),
          m_long_name(long_name),
          m_action(action),
          m_required(required),
          m_default(_default),
          m_help(help) {
    if (m_short_name != "") {
        if (m_short_name.find("-") != 0) {
            printf("short name does not start with -: %s\n",
                   m_short_name.c_str());
            throw_invalid_parser();
        }
        if (m_short_name.find("--") == 0) {
            printf("short name should not start with --: %s\n",
                   m_short_name.c_str());
            throw_invalid_parser();
        }
    }

    if (m_required && (m_action == "store_true" || m_action == "store_false")) {
        printf("required argument cannot be store_true or false: %s\n",
               m_long_name.c_str());
        throw_invalid_parser();
    }

    if (m_long_name.find("--") != 0) {
        printf("long name does not start with --: %s\n", m_long_name.c_str());
        throw_invalid_parser();
    }
    m_arg_name = m_long_name.substr(2);

    if (m_action != "" && m_action != "store_true" &&
        m_action != "store_false") {
        printf("invalid action: %s\n", m_action.c_str());
        throw_invalid_parser();
    }
}

void ArgumentParser::KeywordParser::start_parsing(ArgumentResult& result) {
    m_seen = false;
    m_result_buffer = &result;
}

bool ArgumentParser::KeywordParser::consume(std::list<const char*>& args) {
    if (args.front() != m_long_name && args.front() != m_short_name) {
        return false;
    }

    const char* cmd_arg = args.front();
    args.pop_front();

    if (m_seen) {
        printf("duplicate argument: %s\n", cmd_arg);
        throw_invalid_cmd_args();
    }

    m_seen = true;

    if (m_action == "store_true") {
        m_result_buffer->args[m_arg_name] = "1";
    } else if (m_action == "store_false") {
        m_result_buffer->args[m_arg_name] = "0";
    } else {
        if (args.empty()) {
            printf("missing argument: %s\n", cmd_arg);
            throw_invalid_cmd_args();
        }

        const char* val = args.front();
        args.pop_front();
        m_result_buffer->args[m_arg_name] = val;
    }
    return true;
}

void ArgumentParser::KeywordParser::end_parsing() {
    if (m_required && !m_seen) {
        printf("argument is missing: %s\n", m_long_name.c_str());
        throw_invalid_cmd_args();
    }

    if (m_action == "store_true" && !m_seen)
        m_result_buffer->args[m_arg_name] = "0";
    else if (m_action == "store_false" && !m_seen)
        m_result_buffer->args[m_arg_name] = "1";
    else if (!m_seen)
        m_result_buffer->args[m_arg_name] = m_default;
}

void ArgumentParser::KeywordParser::print_help() const {
    if (m_default != "")
        printf("  %s\n    %s (default: %s)\n\n", brief_usage().c_str(),
               m_help.c_str(), m_default.c_str());
    else
        printf("  %s\n    %s\n\n", brief_usage().c_str(), m_help.c_str());
}

std::string ArgumentParser::KeywordParser::brief_usage() const {
    std::string ret;
    if (!m_required)
        ret += "[";

    if (m_short_name != "")
        ret += m_short_name + "/";
    ret += m_long_name;

    if (m_action == "")
        ret += " <" + m_arg_name + ">";

    if (!m_required)
        ret += "]";
    return ret;
}

ArgumentParser::PositionalParser::PositionalParser(const std::string& name,
                                                   const std::string& narg,
                                                   const std::string& help)
        : m_name(name),
          m_narg(narg),
          m_help(help),
          m_seen(false),
          m_result_buffer(nullptr) {
    if (name.find('-') == 0) {
        printf("positional argument should not start with -: %s\n",
               name.c_str());
        throw_invalid_parser();
    }

    if (m_narg != "1" && m_narg != "+" && m_narg != "*") {
        printf("positional argument has invalid narg: %s %s\n", name.c_str(),
               m_narg.c_str());
        throw_invalid_parser();
    }
}

void ArgumentParser::PositionalParser::start_parsing(ArgumentResult& result) {
    m_seen = false;
    m_result_buffer = &result;
}

bool ArgumentParser::PositionalParser::consume(std::list<const char*>& args) {
    if (m_narg == "1" && m_seen) {
        return false;
    }

    const char* cmd_arg = args.front();
    args.pop_front();
    m_seen = true;

    if (m_narg == "1") {
        m_result_buffer->args[m_name] = cmd_arg;
    } else {
        if (m_result_buffer->multiargs.find(m_name) ==
            m_result_buffer->multiargs.end())
            m_result_buffer->multiargs[m_name] = {};
        m_result_buffer->multiargs[m_name].push_back(cmd_arg);
    }
    return true;
}

void ArgumentParser::PositionalParser::end_parsing() {
    if ((m_narg == "1" || m_narg == "+") && !m_seen) {
        printf("missing positional argument: %s\n", m_name.c_str());
        throw_invalid_cmd_args();
    }

    if (m_narg == "*" && !m_seen) {
        m_result_buffer->multiargs[m_name] = {};
    }
}

void ArgumentParser::PositionalParser::print_help() const {
    printf("  %s\n    %s\n\n", brief_usage().c_str(), m_help.c_str());
}

std::string ArgumentParser::PositionalParser::brief_usage() const {
    if (m_narg == "1")
        return "<" + m_name + ">";
    else if (m_narg == "+")
        return "<" + m_name + "> [<" + m_name + "> ...]";
    else if (m_narg == "*")
        return "[<" + m_name + "> ...]";
    return "?";
}
