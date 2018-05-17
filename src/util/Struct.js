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
    // Lexical analysis : builds an input of the form
    // [ { tab : line-tabulation, label : label-before-equal, string : line-string } ]
    var input = [];
    for(var index = 0, ll = 0; index < value.length; index++) {
      var index0, index1, index2, line = true;
      tab = 0;
      for(; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++)
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
      index1 = value[index] == '=' ? index-1 : index0;
      // Parses the value 
      for(; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++) {}
      if(value[index] == '=') {
        for(index++; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++) ;
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
    //-for(var index = 0; index < input.length; index++) console.log(input[index]);
    return Struct.parse_jvalue({input: input, index :0});
  },
  // Syntax analysis : converts the input to a data-structure
  parse_jvalue : function(data) {
    var s_value = {}, t_value = [], tab = data.input[data.index]["tab"], size = 0, length = 0;
    for(; data.index < data.input.length && data.input[data.index]["tab"] == tab; data.index++) {
      var index0 = data.index, item;
      if((data.index + 1 < data.input.length) && (tab < data.input[data.index + 1]["tab"])) {
	data.index++;
        item = Struct.parse_jvalue(data);
      } else {
        item = data.input[data.index]["string"];
	if (new RegExp("^(true|false)$").test(item))
	  item = item == "true";
	else if (new RegExp("^[-+]?[0-9]+$").test(item))
	  item = parseInt(item);
	else if (new RegExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$").test(item))
	  item = parseFloat(item);
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
  isspace : function(c) {
    return new RegExp("\\s").test(c);
  },

  /** Converts a data structure to string in J syntax.
   * @param {object} data The data-structure to render.
   * @param {string} format The string format, either "plain" or "html".
   * @return {string} A string view of the data-structure.
   */
  data2string : function(data, format = "plain") {
    return (format == "html" ? "<style>body{background-color:lightgrey} .struct-block{margin-left:20px} .struct-meta-char{color:#330033;font-weight:bold} .struct-name{color:#000066} .struct-value{color:#006600} .struct-link{text-decoration: none} .struct-e{font-style: italic} .struct-q{font-family: monospace}</style>\n" : "") +
           Struct.write_value(data, 0, format) + "\n";
  },
  write_value : function(value, tab, format) {
    var string = "";
    if(!(value instanceof Object)) {
      string += Struct.write_word(String(value), tab, "value", format);
    } else if (value instanceof Array) {
     for(var label = 0; label < value.length; label++) {
       string += Struct.write_word(format == "html" ? "<div class='struct-block'>" : "\n", tab, "line", format);
       string += Struct.write_word("= ", tab, "meta", format);
       string += Struct.write_value(value[label], tab + 1, format);
       string += format == "html" ? "</div>" : ""
      }
    } else {
      var root = tab == 0, notitle = value["title"] == "" || value["title"] == undefined || root;
      if(!notitle)
        string += Struct.write_word(value["title"], tab, "value", format);
      for(var label in value)
        if(notitle || (label != "title")) {
          string += Struct.write_word(format == "html" ? "<div class='struct-block'>" : root ? "" : "\n", tab, "line", format);
          root = false;
          if(label[0] != '#') {
            string += Struct.write_word(label, tab, "name", format);
            string += Struct.write_word(" = ", tab, "meta", format);
          } else
            string += Struct.write_word("= ", tab, "meta", format);
          string += Struct.write_value(value[label], tab + 1, format);
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
  },

  /** Converts a string from a format to another format.
   * @param {string} what The conversion to perform:
   * - 'j2json' : Converts a string from J syntax to JSON syntax.
   * - 'json2j' : Converts a string from JSON syntax to J syntax.
   * - 'j2html' : Converts a string in J syntax to HTML for display.
   * - 'j2j' : Reformats a J syntax string to a nomalized form.
   * - 'json2json' : Reformats a JSON syntax string to a nomalized form.
   * @param {string} value The string to parse in the input format.
   * @return {string} The parsed string in the output format.
   */
  convert : function(what, value) {
    switch(what) {
      case 'j2json' : return JSON.stringify(Struct.string2data(value), null, 2);
      case 'json2j' : return Struct.data2string(JSON.parse(value));
      case 'j2html' : return Struct.data2string(Struct.string2data(value), "html");
      case 'j2j' : return Struct.data2string(Struct.string2data(value));
      case 'json2json' : return JSON.stringify(JSON.parse(value), null, 2);
      default : return value;
    }
  },
};

global.Struct = Struct;
