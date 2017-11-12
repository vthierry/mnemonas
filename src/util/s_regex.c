#include "s_regex.h"
#include "assume.h"

#include <regex>
#include <map>

///@cond INTERNAL

static std::map < unsigned int, std::string > s_regex__error_messages = {
  { std::regex_constants::error_collate, "The expression contained an invalid collating element name." },
  { std::regex_constants::error_ctype, "The expression contained an invalid character class name." },
  { std::regex_constants::error_escape, "The expression contained an invalid escaped character, or a trailing escape." },
  { std::regex_constants::error_backref, "The expression contained an invalid back reference." },
  { std::regex_constants::error_brack, "The expression contained mismatched brackets ([ and ])." },
  { std::regex_constants::error_paren, "The expression contained mismatched parentheses (( and ))." },
  { std::regex_constants::error_brace, "The expression contained mismatched braces ({ and })." },
  { std::regex_constants::error_badbrace, "The expression contained an invalid range between braces ({ and })." },
  { std::regex_constants::error_range, "The expression contained an invalid character range." },
  { std::regex_constants::error_space, "The memory to convert the expression into a finite state machine overflows." },
  { std::regex_constants::error_badrepeat, "The expression contained a repeat specifier (one of *?+{) that was not preceded by a valid regular expression." },
  { std::regex_constants::error_complexity, "The complexity of an attempted match against a regular expression exceeded a pre-set level." },
  { std::regex_constants::error_stack, "The memory to determine whether the regular expression could match the specified character sequence overflows." }
};

///@endcond

std::string s_regex_replace(String string, String regex, String output)
{
#ifdef ON_NEF
  assume(false, "illegal-state", "in s_regex_replace regex not yet implemented with g++ < 4.9");
#endif
  try {
    std::basic_regex < char > e(regex, std::regex_constants::ECMAScript);
    return std::regex_replace(string, e, output);
  }
  catch(std::regex_error & e) {
    assume(false, "illegal-argument", "in std::regex_error : spurious regex '%s' : %s \n", regex.c_str(), s_regex__error_messages[e.code()].c_str());
    return string;
  }
}
bool s_regex_match(String string, String regex)
{
#ifdef ON_NEF
  assume(false, "illegal-state", "in s_regex_march regex not yet implemented with g++ < 4.9");
#endif
  try {
    std::basic_regex < char > e(regex, std::regex_constants::ECMAScript);
    return std::regex_match(string, e);
  }
  catch(std::regex_error & e) {
    assume(false, "illegal-argument", "in std::regex_error : spurious regex '%s' : %s \n", regex.c_str(), s_regex__error_messages[e.code()].c_str());
    return false;
  }
}
