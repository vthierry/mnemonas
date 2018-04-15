const fs = require("fs");

require("./Struct.js");
if((process.argv[2] == "json2j") && (process.argv.length == 5) && fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.json2j(fs.readFileSync(process.argv[3]).toString()));
else if((process.argv[2] == "j2json") && (process.argv.length == 5) && fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.j2json(fs.readFileSync(process.argv[3]).toString()));
else if((process.argv[2] == "j2html") && (process.argv.length == 5) && fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.j2html(fs.readFileSync(process.argv[3]).toString()));
else {
  console.log("Usage : node .../Struct_main.js [j2json|j2html|json2j] input-file output-file");
  if(!fs.existsSync(process.argv[3]))
    console.log(" error : file '" + process.argv[3] + "' does not exist");
}
