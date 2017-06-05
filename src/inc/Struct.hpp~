#ifndef STRUCT_HPP
#define STRUCT_HPP

#include "assume.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

/** Implements a generic minimal iterative structure.
 * Such a value corresponds to:
 * - scalar values: boolean, number, or string;
 * - value list, indexed from <tt>0</tt>, written <tt>[value, ...]</tt>;
 * - parameter set, written <tt>{name = value, ...}</tt>.
 * Key properties of such data structure:
 * - They corresponds to <a href="http://json.org/">JSON</a> structures (i.e. correspond to a data object model).
 * - By construction (value copy when set), they can
 * neither be cyclic (i.e. and infinite tree with a leaf linked to an ancestor),
 * nor have common branches (i.e. two Struct instance sharing the same value in memory).
 * - When a named parameter is set,
 * the previous scalar value if any is erased, while
 * the previous ordered list of value is now considered as value indexed by number converted as name.
 */
class Struct {
  //
  // -1- Scalar value implementation
  //
private:
  std::string value;
public:
  /** Returns this value as a string or the empty string if undefined.
   * @return Either
   * - The string representation of the value if the data is atomic (i.e. corresponds to a boolean, number, or string), or
   * - The strict <a href="http://json.org/">JSON</a> syntax representation of this data-structure.
   */
  operator String() const;
  Struct(String value);
  Struct(const char *value);
  /** Defines a empty struct, for initialization. */
  static const Struct EMPTY;

  /** Returns true if the value is "true", false otherwise.
   * - A numerical value of 0 stands for false and a non-zero numerical value stands for true.
   * @return The boolean <tt>true</tt> value if the value is "true", "True" or "TRUE".
   */
  operator bool() const;
  Struct(bool value);
  /** Returns the data as a floating point number or NAN if undefined. */
  operator double() const;
  Struct(double value);
  // Converts a string a number
  static double toDouble(String value);
  /** Returns the data as an int or 0x10000000 (INT_NAN) if undefined. */
  operator int() const;
  Struct(int value);
  Struct(unsigned int value);
  /** The 0x10000000 (INT_MIN) undefined int value. */
  static const int INT_NAN = INT_MIN;
private:
  // Converts a string a integer
  static int toInt(String value);
  // Converts an integer to a string
  template < typename T > static std::string toName(const char *format, T value);
  static std::string toName(int index);
public:
  /** Returns true if this corresponds to a scalar (boolean, numeric, string) value and false otherwise. */
  bool isAtomic() const
  {
    return values.size() == 0;
  }
  /** Returns true if this corresponds to an empty scalar value corresponding to the empty string and false otherwise. */
  bool isEmpty() const
  {
    return values.size() == 0 && value == "";
  }
  //
  // -2- Iterative value implementation
  //
private:
  // Data internal storage with index comparator
  std::vector < std::string > names;
  std::map < std::string, Struct * > values;
  // Maximal indexed value
  int length;
  // Clears the values
  void clear();
  // Recursively copy the values
  static void copy(Struct *dst, const Struct& src);
  // Inserts a value entry
  void insert(String name, bool setting);
  // Removes a value entry
  void erase(String name);
  // Dumps the internal storage of the struct
  void dump(String header = "") const;
public:
  /** Cleans empty values.
   * - Also verifies the structure integrity.
   */
  void clean();
public:
  // Constructs an empty data structure.
  Struct() {
    clear();
  }
  // Copies a data structure.
  Struct(const Struct &value) {
    copy(this, value);
  }
  // Deletes a data structure.
  ~Struct() {
    clear();
  }

  /** Returns true the value is undefined or corresponds to the empty string and false otherwise.
   * @param name The name or the index of the value.
   */
  bool isEmpty(String name) const
  {
    return values.count(name) == 0 || const_cast < Struct * > (this)->values[name]->isEmpty();
  }
  bool isEmpty(int index) const
  {
    return isEmpty(toName(index));
  }
  /** Returns the reference to a named or indexed value.
   * - Also available though the <tt>struct[name]</tt> subscript operator.
   * @param name The name or the index of the value.
   * @param path If true it allows the construct <tt>get("name1/name..")</tt> to retrieve the value of <tt>name2</tt> of the value of name <tt>name1</tt>, using "/" as separator.
   * @return A reference to the value, creating this entry if not yet defined.
   */
  Struct& get(String name, bool path = false) const;
  Struct& get(int index) const
  {
    return get(toName(index));
  }
  Struct& operator[] (String name) const {
    return get(name, true);
  }
  Struct& operator[] (const char *name) const {
    return get(name, true);
  }
  Struct& operator[] (int index) const {
    return get(index);
  }

  /** Sets a named or indexed value.
   * - Also available though the <tt>struct[name] = value</tt> assignment/subscript operators.
   * @param name The name or the index of the value.
   * @param value The value to set.
   * @param path If true it allows the construct <tt>get("name1/name..")</tt> to retrieve the value of <tt>name2</tt> of the value of name <tt>name1</tt>, using "/" as separator.
   */
  Struct& set(String name, const Struct& value, bool path = false);
  Struct& set(int index, const Struct& value)
  {
    return set(toName(index), value);
  }
  Struct& operator = (const Struct &value) {
    copy(this, value);
    return *this;
  }
  Struct& set(String name)
  {
    return set(name, true);
  }
  Struct& set(int index)
  {
    return set(index, true);
  }
  /** Appends an indexed value. */
  Struct& add(const Struct& value)
  {
    return set(getLength(), value);
  }
  /** Erases a named or indexed value. */
  Struct& del(String name)
  {
    erase(name);
    return *this;
  }
  Struct& del(int index)
  {
    return del(toName(index));
  }
  /** Returns the maximal indexed value in the structure.
   * - It is used in a construct of the form:
   *  <div><tt>for(int i = 0; i < value.getLength(); i++) { Struct &value = value.get(i); ../.. }</tt>.</div>
   */
  int getLength() const
  {
    return length;
  }
  /** Returns the number of named value in the structure. */
  int getCount() const
  {
    return names.size();
  }
  /** Defines an iterator over the struct named fields.
   * - It is used in a construct of the form:
   *  <div><tt>for(Struct::Iterator i(values); i.next();) { String name = i.getName(); Struct &value = i.getValue(); ../.. }</tt>.</div>
   * - It does not iterate over the indexed values, which is realized using a construct of the form:
   *  <div><tt>for(int i = 0; i < values.getLength(); i++) { Struct &value = values[i]; ../.. }</tt>.</div>
   */
  class Iterator {
private:
    const std::vector < std::string >& names;
    const std::map < std::string, Struct * >& values;
    int index;
public:
    /** Constructs an iterator over the struct named fields.
     * @param value The logical-structure to iterate on.
     */
    Iterator(const Struct &value) : names(value.names), values(value.values) {
      index = -1;
    }
    /** Returns the current name if any, else the result is undefined. */
    String getName() const
    {
      return names[index];
    }
    /** Returns the current value if any, else the result is undefined. */
    Struct& getValue() const
    {
      return *values.find(getName())->second;
    }
    /** Returns the current name index, starting from 0 up to getCount()-1. */
    int getIndex() const
    {
      return index;
    }
    /** Returns true if there is another element, enumerating a new element at each call. */
    bool next()
    {
      return ++index < (int) names.size();
    }
  };

  /** Returns true if this structure equals the comparing one.
   * - Also available via the <tt>==</tt> and <tt>!=</tt> operators.
   */
  bool equals(const Struct& value) const;
  bool operator == (const Struct &value) const {
    return equals(value);
  }
  bool operator != (const Struct &value) const {
    return !equals(value);
  }

  //
  // -3- String input/output interface
  //

  /** Saves the data structure as a JSON string, in a file.
   * @param location The file path.
   * - If "stdout" / "stderr", prints the data in the console as stdout or stderr stream.
   * @param format The output format:
   * - "raw" : 1D raw compact format with minimal space.
   * - "plain" : 2D human editable format with tabulation.
   * - "html" : HTML human readable format.
   */
  void save(String location, String format = "raw") const;

  /** Returns the data structure as a a JSON string.
   * @param format The output format:
   * - "raw" : 1D raw compact format with minimal space.
   * - "plain" : 2D human editable format with tabulation.
   * - "html" : HTML human readable format.
   */
  std::string asString(String format = "raw") const;

  /** Resets the data structure from a JSON weak-syntax string.
   * @param value The value given as a string, using weak <a href="http://json.org/">JSON</a> syntax.
   * - By default resets to an empty data structure.
   * - Value is either a string or a "format" a-la printf with the subsequent arguments.
   * - The weak-syntax allows to:
   *   - use either ':' or '=' between name and value
   *   - use either ',' or ';' as item separator
   *   - use either '"' or "'" as quote
   *   - avoid quotes for strings without space or any meta-char ':=,;]}'
   *   - accept string with '\\n' line separators (replaced by the "\\n" sequence), also manage \\b, \\r, \\t, \\f
   *   - set the value 'true' for name without explicit value
   *   - However: '\\uXXXX' unicode string sequences and the '\/' solidus escaped sequence are not managed (i.e., but simply mirrored in the string value)
   */
  void reset(const char *value = "", ...);
  void reset(String value)
  {
    reset(value.c_str());
  }
  /** Resets the data structure from the main() program interface.
   * - Application parameters are defined viawith a syntax of the form:
   * <pre>program [-name [[-] ? value ..]*]*</pre>
   * where <tt>name</tt> is the parameters name with zero or more values the <tt>-</tt> allowing to escape values starting with the <tt>-</tt> char.
   *
   * - It is used in a construct of the form:
   * <pre>int main(int argc, const char* argv[]) {
   *   Struct args(argc, argv);
   *   . . / . .
   *   return EXIT_SUCCESS;
   * }</pre>
   *
   * @param argc The number of arguments.
   * @param argv The arguments.
   */
  Struct(int argc, const char *argv[]);

  /** Resets the data structure from a JSON weak-syntax string stored in a file.
   * @param location The file path.
   */
  void load(String location);
  /** A no operation method, just used to avoid compilation warning in empty callbacks. */
  void nop() const {}
};
#endif
