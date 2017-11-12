#include "Struct.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "s_load.h"
#include "s_save.h"

/////////////////////////////////////////////

///@cond INTERNAL
Struct::Struct()
{
  clear();
}
Struct::~Struct()
{
  clear();
}
///@endcond
Struct::Struct(const Struct& value)
{
  copy(*this, value);
}
Struct::operator String() const
{
  if((values.size() > 0) && (value == ""))
    const_cast < Struct * > (this)->value = asString();
  return value;
}
Struct::Struct(String value)
{
  clear();
  this->value = value;
}
Struct::Struct(const char *value)
{
  clear();
  this->value = value;
}
Struct::Struct(char value)
{
  clear();
  this->value = value;
}
Struct::operator char() const
{
  String value = (String) * this;
  return value == "" ? '\0' : value[0];
}
Struct::operator bool() const
{
  // Interprets an empty value as false and structured value as true
  if(values.size() > 0)
    return true;
  if(value == "")
    return false;
  // Interprets a numerical value as a boolean
  {
    double v = toDouble(value);
    if(!isnan(v))
      return v != 0;
  }
  // Interprets a string value as a boolean
  {
    std::string s = value;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s != "false";
  }
}
Struct::Struct(bool value)
{
  clear();
  this->value = value ? "true" : "false";
}
Struct::operator double() const
{
  return toDouble(value);
}
Struct::Struct(double value)
{
  clear();
  this->value = toName < double > ("%.16g", value);
}
Struct::operator int() const
{
  double v = toDouble(value);
  return isnan(v) ? INT_NAN : (int) rint(v);
}
Struct::operator unsigned int() const
{
  double v = toDouble(value);
  return isnan(v) ? INT_NAN : (unsigned int) rint(v);
}
Struct::Struct(int value)
{
  clear();
  this->value = toName < int > ("%d", value);
}
Struct::Struct(unsigned int value)
{
  clear();
  this->value = toName < unsigned int > ("%u", value);
}
int Struct::toInt(String value)
{
  char *end;
  int r = strtol(value.c_str(), &end, 10);
  return *end == '\0' ? r : INT_NAN;
}
double Struct::toDouble(String value)
{
  char *end;
  double r = strtod(value.c_str(), &end);
  return *end == '\0' ? r : NAN;
}
template < typename T > std::string Struct::toName(const char *format, T value)
{
  static char s[256];
  // - for(int i = 0; i < 256; s[i++] = 0); // used to avoid valgrind erors
  sprintf(s, format, value);
  return s;
}
std::string Struct::toName(int index)
{
  return toName < int > ("%d", index);
}
const Struct Struct::EMPTY;

/////////////////////////////////////////////

void Struct::clear()
{
  for(std::map < std::string, Struct > ::iterator i = values.begin(); i != values.end(); i++)
    i->second.clear();
  values.clear();
  names.clear();
  value = "";
  length = 0;
}
void Struct::copy(Struct& dst, const Struct& src)
{
  dst.clear();
  dst.value = src.value;
  dst.length = src.length;
  for(std::map < std::string, Struct > ::const_iterator i = src.values.begin(); i != src.values.end(); i++)
    copy(dst.values[i->first], i->second);
  for(std::vector < std::string > ::const_iterator i = src.names.begin(); i != src.names.end(); i++)
    dst.names.push_back(*i);
}
void Struct::insert(String name, bool setting)
{
  if(values.find(name) == values.end()) {
    // Manages the names and length
    int l = toInt(name);
    if(l == INT_NAN)
      names.push_back(name);
    else if(l >= length)
      length = l + 1;
    // Resets the corresponding string value
    if(setting)
      this->value = "";
  }
}
void Struct::erase(String name)
{
  if(values.find(name) != values.end()) {
    // Removes the value
    values.erase(name);
    // Manages the names and length
    int l = toInt(name);
    if(l == INT_NAN)
      names.erase(std::remove(names.begin(), names.end(), name), names.end());
    else {
      length = 0;
      for(std::map < std::string, Struct > ::const_iterator i = values.begin(); i != values.end(); i++) {
        int l = toInt(i->first);
        if((l != INT_NAN) && (length < l + 1))
          length = l + 1;
      }
    }
    // Resets the corresponding string value
    this->value = "";
  }
}
void Struct::dump(String header) const
{
  printf("%s value = '%s' count = %d length = %d names = [ ", header.c_str(), value.c_str(), (int) names.size(), length);
  for(std::vector < std::string > ::const_iterator i = names.begin(); i != names.end(); i++)
    printf("'%s' ", (*i).c_str());
  printf("] values = { ");
  for(std::map < std::string, Struct > ::const_iterator i = values.begin(); i != values.end(); i++)
    printf("'%s':«%s»", i->first.c_str(), ((String) i->second).c_str());
  printf("}\n");
}
void Struct::clean()
{
  // Recursively applies on all values
  for(std::map < std::string, Struct > ::iterator i = values.begin(); i != values.end(); i++)
    i->second.clean();
  // Cleans indexed values
  assume(values.size() > 0 || length == 0, "illegal-Struct", "Spurious length %d for an empty or atomic value in '%s'", length, value.c_str());
  for(int i = 0; i < length; i++) {
    String n = toName(i);
    if(values.count(n) > 0) {
      Struct& s = values[n];
      if(s.isEmpty())
        erase(n);
    }
  }
  // Cleans named values
  for(std::vector < std::string > ::iterator i = names.begin(); i != names.end();) {
    String n = *i;
    assume(values.count(n) == 1, "illegal-Struct", "Spurious count %d of named value %s in '%s'\n", values.count(n), n.c_str(), value.c_str());
    Struct& s = values[n];
    if(s.isEmpty())
      erase(n);
    else
      i++;
  }
}
/////////////////////////////////////////////

Struct& Struct::get(String name, Struct value)
{
  if(isEmpty(name))
    set(name, value);
  return get(name);
}
Struct& Struct::get(String name) const
{
  Struct *s = const_cast < Struct * > (this);
  s->insert(name, false);
  return s->values[name];
}
Struct& Struct::set(String name, const Struct& value)
{
  insert(name, true);
  copy(values[name], value);
  return *this;
}
bool Struct::equals(const Struct& value) const
{
  if(isAtomic()) {
    if(value.isAtomic())
      return this->value == value.value;
    else
      return false;
  } else {
    if(value.isAtomic())
      return false;
    else {
      // Calculates the union of both names
      std::set < std::string > names;
      {
        for(std::map < std::string, Struct > ::const_iterator i = values.begin(); i != values.end(); i++)
          names.insert(i->first);
        for(std::map < std::string, Struct > ::const_iterator i = value.values.begin(); i != value.values.end(); i++)
          names.insert(i->first);
      }
      for(std::set < std::string > ::const_iterator i = names.begin(); i != names.end(); i++) {
        if((values.count(*i) == 0) || (value.values.count(*i) == 0))
          return false;
        Struct& s1 = const_cast < Struct & > (*this).values[*i], s2 = const_cast < Struct & > (value).values[*i];
        if(!s1.equals(s2))
          return false;
      }
      return true;
    }
  }
}
void Struct::reset(const char *value, ...)
{
  clear();
  // Performs a sprintf() parsing
  std::string result;
  {
    static const int nchars = 10000;
    static char chars[nchars];
    va_list a;
    va_start(a, value);
    vsnprintf(chars, nchars, value, a);
    va_end(a);
    result = chars;
  }
  // Now reads the String
  if(result != "") {
    // Implements the weak parsing of a JSON structure
    class StructReader {
public:
      void read(Struct& value, String string)
      {
        // Initializes the input buffer
        chars = string.c_str(), index = 0, length = string.length();
        // Clears and set the value
        value.clear();
        read_value(value);
        // Appends the trailer if any
        next_space();
        if(index < length)
          value.add(chars + index);
      }
private:
      // String input buffer, index and length
      const char *chars;
      int index, length;
      std::string word;
      // Reads a value from the string starting at index i and set it
      void read_value(Struct& value)
      {
        next_space();
        switch(chars[index]) {
        case '{':
          read_tuple_value(value);
          break;
        case '[':
          read_list_value(value);
          break;
        default:
          value = read_word();
          break;
        }
      }
      // Reads a word
      String read_word()
      {
        if((chars[index] == '"') || (chars[index] == '\''))
          return read_quoted_word(chars[index]);
        else
          return read_nospace_word();
      }
      // Reads a quoted word
      String read_quoted_word(char quote)
      {
        word = "";
        for(index++; index < length && chars[index] != quote; index++) {
          if((chars[index] == '\\') && (index < length - 1)) {
            index++;
            switch(chars[index]) {
            case '"':
            case '\\':
            case '/':
              word += chars[index];
              break;
            case 'n':
              word += "\n";
              break;
            case 'b':
              word += "\b";
              break;
            case 'r':
              word += "\r";
              break;
            case 't':
              word += "\t";
              break;
            case 'f':
              word += "\f";
              break;
            default:
              word += "\\";
              word += chars[index];
            }
          } else
            word += chars[index];
        }
        if(index < length)
          index++;
        return word;
      }
      // Reads a no-space word
      String read_nospace_word()
      {
        int i0;
        for(i0 = index; index < length && !(isspace(chars[index]) ||
                                            chars[index] == ',' || chars[index] == ';' ||
                                            chars[index] == ':' || chars[index] == '=' ||
                                            chars[index] == '}' || chars[index] == ']'); index++)
        {}
        word = std::string(chars + i0, index - i0);
        return word;
      }
      // Reads a list
      void read_list_value(Struct& value)
      {
        index++;
        while(true) {
          next_space();
          if(index >= length)
            return;
          if(chars[index] == ']') {
            index++;
            return;
          }
          Struct item;
          read_value(item);
          value.set(value.getLength(), item);
          next_space();
          if((index < length) && ((chars[index] == ',') || (chars[index] == ';')))
            index++;
        }
      }
      // Reads a tuple
      void read_tuple_value(Struct& value)
      {
        index++;
        while(true) {
          next_space();
          if(index >= length)
            return;
          if(chars[index] == '}') {
            index++;
            return;
          }
          std::string name = read_word();
          next_space();
          Struct item = true;
          if((index < length) && ((chars[index] == ':') || (chars[index] == '='))) {
            index++;
            read_value(item);
          }
          value.set(name, item);
          next_space();
          if((index < length) && ((chars[index] == ',') || (chars[index] == ';')))
            index++;
        }
      }
      // Shifts until the next non-space char
      void next_space()
      {
        for(; index < length && isspace(chars[index]); index++) {}
      }
    }
    reader;
    reader.read(*this, result);
  }
}
Struct::Struct(int argc, const char *argv[])
{
  clear();
  const char *name = NULL;
  int index = 0;
  bool esc = false;
  for(int i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-") == 0) {
      if(name != NULL) {
        set(name, true);
        name = NULL;
      }
      esc = true;
    } else if((argv[i][0] == '-') && !esc) {
      if(name != NULL)
        set(name, true);
      name = &argv[i][1];
    } else {
      Struct value;
      value.reset(argv[i]);
      if(name == NULL)
        set(index++, value);
      else {
        set(name, value);
        name = NULL;
      }
      esc = false;
    }
  }
  if(name != NULL)
    set(name, true);
}
std::string Struct::asString(String format) const
{
  assume(format == "raw" || format == "plain" || format == "html", "illegal-argument", "in Struct::asString undefined format %s", format.c_str());
  // Implements the string writing of a JSON structure
  class StructWriter {
public:
    // Writes a value in a string, in  compact-raw (mode = 0), 2D-plain-text (mode = 1), html-text (mode = 2).
    void write(std::string& string, const Struct& value, String format = "raw")
    {
      const_cast < Struct & > (value).clean();
      tab = 0, mode = format == "html" ? 2 : format == "plain" ? 1 : 0;
      string = stringHeader();
      write_value(string, value);
      string += stringTrailer();
    }
private:
    // Recursively writes a value
    void write_value(std::string& string, const Struct& value)
    {
      if(value.isAtomic())
        write_word(string, value.value);
      else if(value.getCount() == 0) {
        string += asBeginTag("[");
        for(int i = 0, l = value.getLength() - 1; i <= l; i++) {
          write_value(string, value.get(i));
          string += (i < l ? asNextTag(", ") : asEndTag("]"));
        }
      } else {
        string += asBeginTag("{");
        bool once = true;
        for(Struct::Iterator i(value); i.next();) {
          if(once)
            once = false;
          else
            string += asNextTag(", ");
          write_word(string, i.getName(), true);
          string += asMeta(": ", true);
          write_value(string, value.get(i.getName()));
        }
        for(int i = 0, l = value.getLength() - 1; i <= l; i++) {
          Struct value_i = value.get(i);
          if(!value_i.isEmpty()) {
            if(once)
              once = false;
            else
              string += asNextTag(", ");
            write_word(string, toName(i), true);
            string += asMeta(": ", true);
            write_value(string, value_i);
          }
        }
        string += asEndTag("}");
      }
    }
    void write_word(std::string& string, String value, bool name = false)
    {
      string += asBeginValue("\"", name);
      for(unsigned int i = 0; i < value.length(); i++)
        switch(value[i]) {
        case '"':
        case '\\':
          string += "\\";
          string += value[i];
          break;
        case '\n':
          string += "\\n";
          break;
        case '\b':
          string += "\\b";
          break;
        case '\r':
          string += "\\r";
          break;
        case '\t':
          string += "\\t";
          break;
        case '\f':
          string += "\\f";
          break;
        case '<':
          string += mode == 2 ? "&lt;" : "<";
          break;
        case '&':
          string += mode == 2 ? "&amp;" : "&";
          break;
        default:
          string += value[i];
        }
      string += asEndValue("\"");
    }
    std::string addLn(int t)
    {
      tab += t;
      std::string s;
      if(mode == 1) {
        s += "\n";
        for(int i = 0; i < 2 * tab; i++)
          s += " ";
      }
      return s;
    }
    // HTML and 2D-raw mechanisms
    std::string asBeginTag(String c)
    {
      return mode == 2 ? asMeta(c) + "<div class='struct-block'>" : c + addLn(1);
    }
    std::string asNextTag(String c)
    {
      return mode == 2 ? asMeta(c) + "<br/>" : c + addLn(0);
    }
    std::string asEndTag(String c)
    {
      return mode == 2 ? "</div>" + asMeta(c) : addLn(-1) + c;
    }
    std::string asBeginValue(String c, bool name)
    {
      return mode == 2 ? asMeta(c) + "<span class='" + (name ? "struct-name" : "struct-value") + "'>" : c;
    }
    std::string asEndValue(String c)
    {
      return mode == 2 ? "</span>" + asMeta(c) : c;
    }
    std::string asMeta(String c, bool space = false)
    {
      return mode == 2 ? "<span class='struct-meta-char'>" + (space ? " " + c + " " : c) + "</span>" : mode == 1 && space ? " " + c + " " : c;
    }
    std::string stringHeader()
    {
      return mode == 2 ? "<style>body {background-color:lightgrey}.struct-block{margin-left:20px}.struct-meta-char{color:#330033;font-weight:bold}.struct-name{color:#000066}.struct-value{color:#006600}</style><div class'struct-block'>" : "";
    }
    std::string stringTrailer()
    {
      return mode == 2 ? "</div>" : mode == 1 ? "\n" : "";
    }
    int mode, tab;
  }
  writer;
  std::string string;
  writer.write(string, *this, format);
  return string;
}
void Struct::load(String location)
{
  reset(s_load(location));
}
void Struct::save(String location, String format) const
{
  std::string string = asString(format);
  if(location == "stdout")
    printf("%s", string.c_str());
  else if(location == "stderr")
    fprintf(stderr, "%s", string.c_str());
  else
    s_save(location, string);
}
