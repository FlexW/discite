#pragma once

#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class ArgsParser
{
public:
  enum class ValueType
  {
    String
  };

  enum class OptionType
  {
    Value,
    NonValue
  };

  enum class OptionImportance
  {
    Optional,
    Required
  };

  struct Option
  {
    OptionType               type_        = OptionType::NonValue;
    std::string              name_        = "";
    std::string              description_ = "";
    OptionImportance         importance_  = OptionImportance::Optional;
    std::vector<std::string> possible_values_;
    std::vector<std::string> mutually_exclusive_;
  };

  void add_option(const Option &option);

  void parse_args(int argc, char *argv[]);

  bool is_option_set(const std::string &name) const;

  std::optional<std::string>
  value_as_string(const std::string &option_name) const;

  void        format_option(const ArgsParser::Option &option,
                            std::stringstream        &help) const;
  std::string format_help() const;

private:
  const std::string argument_prefix = "--";

  struct SetOption
  {
    std::string name;
    OptionType  type;
    std::string value;
  };

  std::unordered_map<std::string, Option>    allowed_options;
  std::unordered_map<std::string, SetOption> set_options;

  void check_if_mutually_exclusive_option_set(const std::string &argument);

  void check_required_options_have_been_set();

  bool does_option_require_value(const Option &allowed_option);
};

void parse_and_show_help_on_error(ArgsParser &args_parser,
                                  int         argc,
                                  char       *argv[]);
