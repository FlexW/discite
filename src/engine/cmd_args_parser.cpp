#include "cmd_args_parser.hpp"
#include "assert.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace
{
using namespace dc;

constexpr auto spacer = "  ";

void show_usage(const ArgsParser &args_parser, const char *program_name)
{
  fmt::print("Usage:\n{} <OPTION>...\n\n{}\nHelp and issue tracker: "
             "https://github.com/FlexW/discite\n© 2022 Felix Weilbach\n",
             program_name,
             args_parser.format_help());
}
} // namespace

namespace dc
{

void ArgsParser::add_option(const Option &option)
{
  const auto option_name_with_prefix = argument_prefix + option.name_;

  if (allowed_options.find(option_name_with_prefix) != allowed_options.end())
  {
    DC_FAIL("Option already added");
  }

  allowed_options[option_name_with_prefix] = option;
}

void ArgsParser::parse_args(int argc, char *argv[])
{
  for (int i = 1; i < argc; ++i)
  {
    const auto argument = argv[i];

    const auto allowed_options_iter = allowed_options.find(argument);
    if (allowed_options_iter != allowed_options.end())
    {
      check_if_mutually_exclusive_option_set(argument);
      const auto allowed_option = allowed_options_iter->second;

      SetOption set_option;
      set_option.type = OptionType::NonValue;
      set_option.name = allowed_option.name_;

      // Check if option requires a value
      if (does_option_require_value(allowed_option))
      {
        set_option.type = OptionType::Value;

        if (argc <= i + 1)
        {
          std::string message;
          message += "Can not read value of option ";
          message += argument;
          throw std::runtime_error(message);
        }

        const auto value = argv[i + 1];
        if (!allowed_option.possible_values_.empty() &&
            std::find(allowed_option.possible_values_.begin(),
                      allowed_option.possible_values_.end(),
                      value) == allowed_option.possible_values_.end())
        {
          std::string possible_values_help;
          bool        first = true;
          for (const auto &possible_value : allowed_option.possible_values_)
          {
            if (!first)
            {
              possible_values_help += ", ";
            }
            first = false;
            possible_values_help += possible_value;
          }
          std::string message;
          message += "Option ";
          message += argument_prefix + allowed_option.name_;
          message += " requires one of the following values: ";
          message += possible_values_help;
          throw std::runtime_error(message);
        }
        set_option.value = value;

        ++i;
      }

      set_options[set_option.name] = set_option;

      continue;
    }

    std::string message;
    message += "Unknown option ";
    message += argument;
    throw std::runtime_error(message);
  }

  check_required_options_have_been_set();
}

bool ArgsParser::is_option_set(const std::string &name) const
{
  return set_options.find(name) != set_options.end();
}

std::optional<std::string>
ArgsParser::value_as_string(const std::string &option_name) const
{
  const auto set_options_iter = set_options.find(option_name);

  if (set_options_iter == set_options.end())
  {
    return {};
  }

  return set_options_iter->second.value;
}

void ArgsParser::format_option(const ArgsParser::Option &option,
                               std::stringstream        &help) const
{
  help << spacer << argument_prefix << option.name_;

  if (option.type_ == OptionType::Value)
  {
    help << " ";
    if (option.type_ == OptionType::Value && !option.possible_values_.empty())
    {
      help << "<";
      bool first_possible_value = true;
      for (const auto &possible_value : option.possible_values_)
      {
        if (!first_possible_value)
        {
          help << ", ";
        }
        first_possible_value = false;
        help << possible_value;
      }
      help << ">";
    }
    else
    {
      help << "VALUE";
    }
  }
  help << "\n" << spacer << spacer << option.description_ << "\n";

  if (option.mutually_exclusive_.size() > 0)
  {
    help << spacer << spacer
         << "This option can not be used with one of the following:\n"
         << spacer << spacer;
    uint32_t i = 0;
    for (const auto &mutually_exclusive_option : option.mutually_exclusive_)
    {
      if (i != 0)
      {
        help << ", ";
      }
      help << mutually_exclusive_option;
    }
    help << "\n";
  }
}

std::string ArgsParser::format_help() const
{
  bool required_option_set = false;

  std::stringstream help;
  help << "Required options:\n";
  for (const auto &p : allowed_options)
  {
    if (p.second.importance_ != OptionImportance::Required)
    {
      continue;
    }
    required_option_set = true;
    const auto option   = p.second;
    format_option(option, help);
  }
  if (required_option_set)
  {
    help << "\n";
  }
  help << "Optional options:\n";
  for (const auto &p : allowed_options)
  {
    if (p.second.importance_ != OptionImportance::Optional)
    {
      continue;
    }
    const auto option = p.second;
    format_option(option, help);
  }

  return help.str();
}

void ArgsParser::check_if_mutually_exclusive_option_set(
    const std::string &argument)
{
  for (const auto &allowed_option : allowed_options)
  {
    for (const auto &mutually_exclusive_option_name :
         allowed_option.second.mutually_exclusive_)
    {
      if (argument_prefix + mutually_exclusive_option_name == argument &&
          is_option_set(allowed_option.second.name_))
      {
        // Error argument is mutually exclusive
        std::string message;
        message += "Option ";
        message += argument_prefix + argument;
        message += "can not be set when option ";
        message += argument_prefix + allowed_option.second.name_;
        message += " is set!";

        throw std::runtime_error(message);
      }
    }
  }
}

bool ArgsParser::does_option_require_value(const Option &allowed_option)
{
  return allowed_option.type_ == OptionType::Value;
}

void ArgsParser::check_required_options_have_been_set()
{
  for (const auto &allowed_option : allowed_options)
  {
    if (allowed_option.second.importance_ == OptionImportance::Required)
    {
      if (set_options.find(allowed_option.second.name_) == set_options.end())
      {
        // Error required Option has not been set
        std::string message;
        message += "Required option ";

        message += argument_prefix;
        message += allowed_option.second.name_;
        message += " has not been set.";
        throw std::runtime_error(message);
      }
    }
  }
}

void parse_and_show_help_on_error(ArgsParser &args_parser,
                                  int         argc,
                                  char       *argv[])
{
  ArgsParser::Option help_option;
  help_option.name_        = "help";
  help_option.description_ = "Display the help";
  help_option.type_        = ArgsParser::OptionType::NonValue;
  help_option.importance_  = ArgsParser::OptionImportance::Optional;
  args_parser.add_option(help_option);

  try
  {
    args_parser.parse_args(argc, argv);
    if (args_parser.is_option_set(help_option.name_))
    {
      show_usage(args_parser, argv[0]);
      std::exit(EXIT_SUCCESS);
    }
  }
  catch (const std::runtime_error &error)
  {
    show_usage(args_parser, argv[0]);
    std::exit(EXIT_FAILURE);
  }
}

} // namespace dc
