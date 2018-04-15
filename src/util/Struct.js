/** Implements the JavaScript read/write a generic minimal iterative structure in J= syntax.
 *
 * @version 0.0.1 
 * @copyright <a href='http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html'>CeCILL-C</a>
 * @author vthierry <thierry.vieville@inria.fr>, Denis Chiron <chiron.denis@gmail.com>
 *
 */
Struct = 
  {
   /** Parses a string in J= syntax and return the related data-structure.
    * @param {string} value The string to parse.
    * @return {object} The parsed data-structure.
    */
   string2data : function(value) {
     // Lexical analysis : builds an input of the form 
     // [ { tab : line-tabulation, label : label-before-equal, string : line-string } ]
     var input = [];     
     for(var index = 0, ll =0; index < value.length; index++) {
       var index0, index1, index2, line = true; tab = 0;
       for(; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++) 
	 switch(value[index]) {
	 case '\t' :
	   tab += 6;
	   break;
	 case ' ' :
	   tab++;
	   break;
	 }
       for(index0 = index; index < value.length && value[index] != '=' && !Struct.isspace(value[index]); index++);
       index1 = index;
       for(; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++){}
       if (value[index] == '=') {
         for(index++; index < value.length && value[index] != '\n' && Struct.isspace(value[index]); index++);
	 for(index2 = index; index < value.length && value[index] != '\n'; index++);
       } else {
	 index1 = index0;
	 for(index2 = index0; index < value.length && value[index] != '\n'; index++); 
	 // Manages multi-line strings
	 if (ll > 0 && input[ll - 1]["tab"] <= tab) {
	   input[ll - 1]["string"] = input[ll - 1]["string"] + "\n" + value.substr(index2, index - index2);
	   line = false;
	 }
       }
       // Manages nested data-structure with a string on the label line
       if (line && ll > 0 && input[ll - 1]["tab"] < tab && input[ll - 1]["string"] != "") {
	 input[ll] = { "tab" : tab, "label" : "title", "string" : input[ll - 1]["string"]};
	 input[ll - 1]["string"] = undefined;
	 ll++;
       }
       if (line) {
	 input[ll] = { "tab" : tab, "label" : value.substr(index0, index1 - index0), "string" : value.substr(index2, index - index2)};
	 ll++;
       }
     }
     //-for(var index = 0; index < input.length; index++) console.log(input[index]);
     var data = {}
     Struct.parse_jvalue(data, input, 0);
     //-console.log(data);
     return data;
   },
   // Syntax analysis : converts the input to a data-structure
   parse_jvalue : function(value, input, index) {
     var tab = input[index]["tab"], length = 0;
     for(; index < input.length && input[index]["tab"] == tab; index++) {
       var index0 = index, item = {};
       if (index + 1 < input.length && tab < input[index + 1]["tab"]) {
	 index = Struct.parse_jvalue(item, input, index + 1);
       } else 
	 item = input[index]["string"];
       if (input[index0]["label"] == "") {
	 value["#"+(length++)] = item;
       } else
	 value[input[index0]["label"]] = item;
     }
     return index - 1;
   },
   isspace : function(c) { return /\s/.test(c); },
   /** Converts a data structure to string in J= syntax.
    * @param {object} data The data-structure to render.
    * @param {string} format The string format, either "plain" or "html".
    * @return {string} A string view of the data-structure.
    */
   data2string : function(data, format = "plain") {
     return (format == "html" ? "<style>body{background-color:lightgrey} .struct-block{margin-left:20px} .struct-meta-char{color:#330033;font-weight:bold} .struct-name{color:#000066} .struct-value{color:#006600} .struct-link{text-decoration: none} .struct-e{font-style: italic} .struct-q{font-family: monospace}</style>\n" : "")+
     Struct.write_value(data, 0, format) + "\n";
   },
   write_value : function(value, tab, format) {
     var string = "";
     if(!(value instanceof Object)) {
       string += Struct.write_word(value, tab, "value", format);
     } else {
        var root = tab == 0, notitle = value["title"] == "" || value["title"] == undefined || root;
	if (!notitle)
	  string += Struct.write_word(value["title"], tab, "value", format);
        for(var label in value) {
	  if (notitle || label != "title") {
            string += Struct.write_word(format == "html" ? "<div class='struct-block'>" : root ? "" : "\n", tab, "line", format);
	    root = false;
	    if (label[0] != '#') {
	      string += Struct.write_word(label, tab, "name", format);
	      string += Struct.write_word(" = ", tab, "meta", format);
	    } else 
	      string += Struct.write_word("= ", tab, "meta", format);
	    string += Struct.write_value(value[label], tab + 1, format);
	    string += format == "html" ? "</div>" :""
	  }
	}
     }
     return string;
   },
   write_word : function(value, tab, type, format) {
      var string = format != "html" ? "" : type == "meta" ? "<span class='struct-meta-char'>" : type == "name" ? "<span class='struct-name'>" : type == "value" ? "<span class='struct-value'>" : "";
      // Applies the [link] and _span_ transform
      if (format == "html") {
	var quoted = false
	for(var i = 0; i < value.length; i++) {
  	  if (value[i] == '"')
	  quoted = !quoted;
	  if (!quoted) {
	    if (value[i] == "[") {
	      value = value.substr(0, i) + value.substr(i).replace(/\[([^\s\]]+)\s+([^\]]*)\]/, "<a class='struct-link' href='$1'>$2</a>");
	      value = value.substr(0, i) + value.substr(i).replace(/\[([^\s\]]+)\]/, "<a class='struct-link' href='$1'>$1</a>");
	      i += value.substr(i).search(/>/);
	    } else if (value[i] == "_") {
	      value = value.substr(0, i) + value.substr(i).replace(/_([^_])_([^_]+)_/, "<span class='struct-$1'>$2</span>");
	      value = value.substr(0, i) + value.substr(i).replace(/_([^_]+)_/, "<span class='struct-e'>$1</span>");
	      i += value.substr(i).search(/>/);	    
	    }
	  }
	}
      }
      // Applies the \n transform
      for(var i = 0; i < value.length; i++) {
	if (value[i] == '\n') {
          string += format == "html" && type == "value" ? "</div>\n<div class='struct-block struct-value'>" : "\n";
          for(var j = 0; j < tab; j++) 
            string += ' ';
	} else 
	  string += value[i];
      }
      string += format != "html" ? "" : type == "meta" || type == "name" || type == "value" ? "</span>" : "";
      return string;
   },
   /** Converts a string in J= syntax to JSON syntax.
    * @param {string} value The string to parse in J= syntax.
    * @return {string} The parsed string in JSON syntax.
    */
   j2json : function(value) {
     return JSON.stringify(Struct.string2data(value));
   },
   /** Converts a string in JSON syntax in J= syntax.
    * @param {string} value The string to parse in JSON syntax.
    * @return {string} The parsed string in J= syntax.
    */
   json2j : function(value) {
     return Struct.data2string(JSON.parse(value));
   },
 };

