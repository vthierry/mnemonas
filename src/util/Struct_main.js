// "Usage : node .../Struct_main.js [j2json|json2j|j2html|j2j|json2json] input-file output-file"

const fs = require("fs");
require("./Struct.js");
if((process.argv.length == 5) &&
   process.argv[2] in { 'json2j' : '', 'j2json' : '', 'j2html' : '', 'j2j' : '', 'json2json' : '' }
   &&
   fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.convert(process.argv[2], fs.readFileSync(process.argv[3]).toString()));
else {
  console.log("Usage : node .../Struct_main.js [j2json|json2j|j2html|j2j|json2json] input-file output-file");
  if(!fs.existsSync(process.argv[3]))
    console.log(" error : file '" + process.argv[3] + "' does not exist");
}
