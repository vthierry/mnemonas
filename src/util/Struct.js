/** Implements the JavaScript read/write a generic minimal iterative structure in J syntax.
 *
 * @version 0.0.1
 * @copyright <a href='http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html'>CeCILL-C</a>
 * @author vthierry <thierry.vieville@inria.fr>, Denis Chiron <chiron.denis@gmail.com>
 *
 */
var Struct = {
  /** Parses a string in J syntax and return the related data-structure.
   * @param {string} value The string to parse.
   * @return {object} The parsed data-structure.
   */
  string2data : function(value) {
    var Reader = {
      read : function(value) {
        // Lexical analysis : builds an input of the form
        // [ { tab : line-tabulation, label : label-before-equal, string : line-string } ]
        var input = [];
	for(var index = 0, ll = 0; index < value.length; index++) {
	  var index0, index1, index2, line = true;
	  tab = 0;
	  for(; index < value.length && value[index] != '\n' && this.isspace(value[index]); index++)
	    switch(value[index]) {
	    case '\t':
	      tab += 6;
	      break;
	    case ' ':
	      tab++;
	      break;
	    }
	  // Parses the label before '='
	  for(index0 = index; index < value.length && value[index] != '=' && value[index] != '\n'; index++) ;
	  index1 = value[index] == '=' ? index - 1 : index0;
	  // Parses the value
	  for(; index < value.length && value[index] != '\n' && this.isspace(value[index]); index++) {}
	  if(value[index] == '=') {
	    for(index++; index < value.length && value[index] != '\n' && this.isspace(value[index]); index++) ;
	    for(index2 = index; index < value.length && value[index] != '\n'; index++) ;
	  } else {
	    index1 = index0;
	    for(index2 = index0; index < value.length && value[index] != '\n'; index++) ;
	    // Manages multi-line strings
	    if((ll > 0) && (input[ll - 1]["tab"] <= tab)) {
	      var index3 = index0 - Math.max(0, tab - input[ll - 1]["tab"] - 1);
	      input[ll - 1]["string"] = input[ll - 1]["string"] + "\n" + value.substr(index3, index - index3);
	      line = false;
	    }
	  }
	  // Manages nested data-structure with a string on the label line
	  if(line && (ll > 0) && (input[ll - 1]["tab"] < tab) && (input[ll - 1]["string"] != "")) {
	    input[ll] = { "tab" : tab, "label" : "title", "string" : input[ll - 1]["string"] };
	    input[ll - 1]["string"] = undefined;
	    ll++;
	  }
	  if(line) {
	    input[ll] = { "tab" : tab, "label" : value.substr(index0, index1 - index0), "string" : value.substr(index2, index - index2) };
	    ll++;
	  }
	}
	// -for(var index = 0; index < input.length; index++) console.log(input[index]);
	return this.parse({ input: input, index :0 });
      },
      // Syntax analysis : converts the input to a data-structure
      parse : function(data) {
        var s_value = {}, t_value = [], tab = data.input[data.index]["tab"], size = 0, length = 0;
	for(; data.index < data.input.length && data.input[data.index]["tab"] == tab; data.index++) {
	  var index0 = data.index, item;
	  if((data.index + 1 < data.input.length) && (tab < data.input[data.index + 1]["tab"])) {
	    data.index++;
	    item = this.parse(data);
	  } else {
	    item = this.string2value(data.input[data.index]["string"]);
	  }
	  if(data.input[index0]["label"] == "") {
	    s_value["#" + (length++)] = item;
	    t_value[t_value.length] = item;
	  } else
	    s_value[data.input[index0]["label"]] = item;
	  size++;
	}
	data.index--;
	return size == length ? t_value : s_value;
      },
      string2value : function(string) {
        if(new RegExp("^(true|false)$").test(string))
	  return string == "true";
	else if(new RegExp("^[-+]?[0-9]+$").test(string))
	  return parseInt(string);
	else if(new RegExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$").test(string))
	  return parseFloat(string);
        else
          return string;
      },
      isspace : function(c) {
        return new RegExp("\\s").test(c);
      }
    };
    return Reader.read(value);
  },
  /** Converts a data structure to string in J syntax.
   * @param {object} data The data-structure to render.
   * @param {string} format The string format, either "plain" or "html".
   * @return {string} A string view of the data-structure.
   */
  data2string : function(data, format = "plain") {
    var Writer = {
      write : function(data, format) {
        return (format == "html" ? "<style>body{background-color:lightgrey} .struct-block{margin-left:20px} .struct-meta-char{color:#330033;font-weight:bold} .struct-name{color:#000066} .struct-value{color:#006600} .struct-link{text-decoration: none} .struct-e{font-style: italic} .struct-q{font-family: monospace}</style>\n" : "") +
        this.write_value(data, 0, format) + "\n";
      },
      write_value : function(value, tab, format) {
        var string = "";
	if(!(value instanceof Object))
	  string += this.write_word(String(value), tab, "value", format);
	else if(value instanceof Array)
	  for(var label = 0; label < value.length; label++) {
	    string += this.write_word(format == "html" ? "<div class='struct-block'>" : "\n", tab, "line", format);
	    string += this.write_word("= ", tab, "meta", format);
	    string += this.write_value(value[label], tab + 1, format);
	    string += format == "html" ? "</div>" : ""
	      }
	else {
	  var root = tab == 0, notitle = value["title"] == "" || value["title"] == undefined || root;
	  if(!notitle)
	    string += this.write_word(value["title"], tab, "value", format);
	  for(var label in value)
	    if(notitle || (label != "title")) {
	      string += this.write_word(format == "html" ? "<div class='struct-block'>" : root ? "" : "\n", tab, "line", format);
	      root = false;
	      if(label[0] != '#') {
		string += this.write_word(label, tab, "name", format);
		string += this.write_word(" = ", tab, "meta", format);
	      } else
		string += this.write_word("= ", tab, "meta", format);
	      string += this.write_value(value[label], tab + 1, format);
	      string += format == "html" ? "</div>" : ""
	  }
	}
	return string;
      },
      write_word : function(value, tab, type, format) {
        var string = format != "html" ? "" : type == "meta" ? "<span class='struct-meta-char'>" : type == "name" ? "<span class='struct-name'>" : type == "value" ? "<span class='struct-value'>" : "";
        // Applies the [link] and _span_ transform
        if(format == "html") {
	  var quoted = false;
	  for(var i = 0; i < value.length; i++) {
	    if(value[i] == '"')
	      quoted = !quoted;
	    if(!quoted) {
	      if(value[i] == "[") {
		value = value.substr(0, i) + value.substr(i).replace(new RegExp("\\[([^\\s\\]]+)\\s+([^\\]]*)\\]"), "<a class='struct-link' href='$1'>$2</a>");
		value = value.substr(0, i) + value.substr(i).replace(new RegExp("\\[([^\\s\\]]+)\\]"), "<a class='struct-link' href='$1'>$1</a>");
		i += value.substr(i).search(">");
	      } else if(value[i] == "_") {
		value = value.substr(0, i) + value.substr(i).replace(new RegExp("_([^_])_([^_]+)_"), "<span class='struct-$1'>$2</span>");
		value = value.substr(0, i) + value.substr(i).replace(new RegExp("_([^_]+)_"), "<span class='struct-e'>$1</span>");
		i += value.substr(i).search(">");
	      }
	    }
	  }
	}
	// Applies the \n transform
	for(var i = 0; i < value.length; i++) {
          if(value[i] == '\n') {
	    string += format == "html" && type == "value" ? "</div>\n<div class='struct-block struct-value'>" : "\n";
	    for(var j = 0; j < tab; j++)
	      string += ' ';
	  } else
	    string += value[i];
	}
	string += format != "html" ? "" : type == "meta" || type == "name" || type == "value" ? "</span>" : "";
	return string;
      }
    };
    return Writer.write(data, format);						  
  },
  /** Parses a data structure from a JSON weak-syntax string.
   * @param {string} value The value given as a string, using weak <a href="http://json.org/">JSON</a> syntax.
   * - The weak-syntax, with respect to the strict <a href="https://www.w3schools.com/js/js_json_intro.asp">JSON-syntax</a>allows to:
   *   - use either ':' or '=' between name and value
   *   - use either ',' or ';' as item separator
   *   - use either '"' or "'" as quote
   *   - avoid quotes for strings without space or any meta-char ':=,;]}'
   *   - accept string with '\\n' line separators (replaced by the "\\n" sequence), also manage \\b, \\r, \\t, \\f
   *   - set the value 'true' for name without explicit value
   *   - However: '\\uXXXX' unicode string sequences and the '\/' solidus escaped sequence are not managed (i.e., but simply mirrored in the string value)
   *
   * In other words the input syntax accepts (i) implicit quote <tt>"</tt> when string reduces to one word, (ii) flexible use of comma <tt>,</tt> when optional, (iii) string on several lines, (iv) considering <tt>true</tt> as implicit value, (v) appending as a raw string trailer chars if any.
   * One consequence is that there is no ``syntax error'' all strings map on a JSON structure (i.e., the exact or closest correct JSON structure, the implicit metric being defined by the parsing algorithm). On the reverse the string output is a human readable indented strict JSON syntax allowing to verify that the input was well-formed.
   * @return {object} The parsed data-structure.
   */
  json2data : function(value) {
    var Reader = {
      string : value, 
      index : 0,    
      read : function() {
        var value = this.read_value();
        this.next_space();
        if(this.index < this.string.length)
          return { value : value, trailer : this.string.substr(this.index) };
	return value;
      },
      read_value : function() {
        this.next_space();
        switch(this.string[this.index]) {
        case '{':
          return this.read_tuple_value();
        case '[':
          return this.read_list_value();
        default:
          return this.string2value(this.read_word(true));
        }
      },
      read_tuple_value : function() {
        var value = {};
        this.index++;
        for(var index0 = -1; index0 != this.index;) { index0 = this.index
          this.next_space();
          if(this.index >= this.string.length)
            return value;
          if(this.string[this.index] == '}') {
            this.index++;
            return value;
          }
          var name = this.read_word();
	  if (name == '')
	    return value;
          this.next_space();
          var item = true;
          if((this.index < this.string.length) && ((this.string[this.index] == ':') || (this.string[this.index] == '='))) {
            this.index++;
            item = this.read_value();
          }
          value[name] = item;
          this.next_space();
          if((this.index < this.string.length) && ((this.string[this.index] == ',') || (this.string[this.index] == ';')))
            this.index++;
        }
	console.log({ bug : "read_tuple_value", value : value, string : "«"+this.string.substr(this.index - 100, 100)+"»|«"+this.string.substr(this.index, 10)+"»" });
      },
      read_list_value : function() {
        var value = [];
        this.index++;
        for(var index0 = -1; index0 != this.index;) { index0 = this.index
          this.next_space();
          if(this.index >= this.string.length)
            return value;
          if(this.string[this.index] == ']') {
            this.index++;
            return value;
          }
          value.push(this.read_value());
          this.next_space();
          if((this.index < this.string.length) && ((this.string[this.index] == ',') || (this.string[this.index] == ';')))
            this.index++;
        }
	console.log({ bug : "read_list_value", value : value, string : "«"+this.string.substr(this.index - 100, 100)+"»|«"+this.string.substr(this.index, 10)+"»" });
      },
      read_word : function(line = false) {
        return this.string[this.index] == '"' || this.string[this.index] == '\'' ? this.read_quoted_word(this.string[this.index]) : this.read_nospace_word(line);
      },
      read_quoted_word : function(quote) {
        var word = "";
        for(this.index++; this.index < this.string.length && this.string[this.index] != quote; this.index++) {
          if((this.string[this.index] == '\\') && (this.index < this.string.length - 1)) {
            this.index++;
            switch(this.string[this.index]) {
            case '"':
            case '\\':
            case '/':
              word += this.string[this.index];
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
              word += this.string[this.index];
            }
          } else
            word += this.string[this.index];
        }
        if(this.index < this.string.length)
          this.index++;
        return word;
      },
      read_nospace_word : function(line = false) {
        var i0;
        for(i0 = this.index; this.index < this.string.length && (line ? this.no_endofline(this.string[this.index]) : this.no_space(this.string[this.index])); this.index++) {}
        return this.string.substr(i0, this.index - i0);
      },
      next_space : function() {
        for(; this.index < this.string.length && this.isspace(this.string[this.index]); this.index++);
      },
      string2value : function(string) {
        if(new RegExp("^(true|false)$").test(string))
	  return string == "true";
	else if(new RegExp("^[-+]?[0-9]+$").test(string))
	  return parseInt(string);
	else if(new RegExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$").test(string))
	  return parseFloat(string);
        else
          return string;
      },
      no_space : function(c) {
        return new RegExp("[^\\s,;:=}\\]]").test(c);
      },
      no_endofline : function(c) {
        return new RegExp("[^\\n,;:=}\\]]").test(c);
      },
      isspace : function(c) {
        return new RegExp("\\s").test(c);
      }
    };
    return Reader.read();
  },
  /** Converts a string from a format to another format.
   * @param {string} what The conversion to perform:
   * - 'j2json' : Converts a string from J syntax to JSON syntax.
   * - 'json2j' : Converts a string from JSON syntax to J syntax.
   * - 'j2html' : Converts a string in J syntax to HTML for display.
   * - 'j2j' : Reformats a J syntax string to a nomalized form.
   * - 'wjson2json' : Reformats a weak JSON syntax string to a normalized form.
   * - 'json2json' : Reformats a JSON syntax string to a normalized form.
   * @param {string} value The string to parse in the input format.
   * @return {string} The parsed string in the output format.
   */
  convert : function(what, value) {
    switch(what) {
    case 'j2json':
      return JSON.stringify(Struct.string2data(value), null, 2);
    case 'json2j':
      return Struct.data2string(JSON.parse(value));
    case 'j2html':
      return Struct.data2string(Struct.string2data(value), "html");
    case 'j2j':
      return Struct.data2string(Struct.string2data(value));
    case 'wjson2json':
      return JSON.stringify(Struct.json2data(value), null, 2);
    case 'json2json':
      return JSON.stringify(JSON.parse(value), null, 2);
    default:
      return value;
    }
  }
};

global.Struct = Struct;
