#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ArgumentResult {
    /*! \brief The arguments containing at most one value.
     */
    std::unordered_map<std::string, std::string> args;

    /*! \brief The arguments containing more than one values, aka, the
     * positional arguments defined with `narg` being '+' or '*'.
     */
    std::unordered_map<std::string, std::vector<std::string>> multiargs;

    const std::string& operator[](const std::string& key) const {
        return args.at(key);
    }
};

/*! \brief A command-line argument parser that resembles Python argparse
 * library.
 */
class ArgumentParser {
public:
    ArgumentParser() = default;
    ArgumentParser(const std::string& description);

    /*! \brief Add a keyword argument like: --option abc
     *
     * `short_name` is a name starting with "-", like "-o".
     * `long_name` is a name starting with "--", like "--option".
     * `action` is "" for a keyword argument with one value, like "--option
     * foo". `action` is "store_true" for a keyword argument without values,
     * like "--verbose". If `action` is "store_true", the value of the argument
     * will be either "0" (the argument is not given) or "1" (the argument is
     * given). If `required == true`, the parser will report an error when the
     * argument is not given.
     * `_default` is the default value for the argument.
     * `help` is the help message for the argument.
     */
    void add_keyword_arg(const std::string& short_name,
                         const std::string& long_name,
                         const std::string& action = "", bool required = false,
                         const std::string& _default = "",
                         const std::string& help = "");

    /*! \brief Add a positional argument (an argument not specified with a
     * keyword).
     *
     * `name` is the name of the argument.
     * `narg` is the number of values it may contain. Its values can be '1', '+'
     * and '*'. '1' is one and only one value. '+' is one or more values. '*' is
     * zero or more values. Only the last postitional argument can be '*' or
     * '+'.
     */
    void add_positional_arg(const std::string& name, const std::string& narg,
                            const std::string& help);

    /*! \breif Parse the arguments.
     */
    ArgumentResult parse_args(int argc, char** argv);

    /*! \brief Print the help message.
     */
    void print_help() const;

private:
    class KeywordParser {
    public:
        KeywordParser(const std::string& short_name,
                      const std::string& long_name,
                      const std::string& action = "", bool required = false,
                      const std::string& _default = "",
                      const std::string& help = "");

        void start_parsing(ArgumentResult& result);

        bool consume(std::list<const char*>& args);

        void end_parsing();

        void print_help() const;

        std::string brief_usage() const;

    private:
        std::string m_short_name;
        std::string m_long_name;
        std::string m_arg_name;
        std::string m_action;
        bool m_required;
        std::string m_default;
        std::string m_help;

        bool m_seen;
        ArgumentResult* m_result_buffer;
    };

    class PositionalParser {
    public:
        PositionalParser(const std::string& name, const std::string& narg = "1",
                         const std::string& help = "");

        void start_parsing(ArgumentResult& result);

        bool consume(std::list<const char*>& args);

        void end_parsing();

        void print_help() const;

        std::string brief_usage() const;

    private:
        std::string m_name;
        std::string m_narg;
        std::string m_help;

        bool m_seen;
        ArgumentResult* m_result_buffer;
    };

    std::string m_description;
    std::string m_program_name;
    std::unordered_set<std::string> m_existing_names;
    std::vector<KeywordParser> m_keyword_parsers;

    bool m_has_rest_consumer = false;
    std::vector<PositionalParser> m_positional_parsers;
};
