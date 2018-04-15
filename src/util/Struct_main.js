const fs = require("fs");

const String = require("./Struct.js");
if((process.argv[2] == "j2json") && (process.argv.length == 5) && fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.j2json(fs.readFileSync(process.argv[3]).toString()));
else if((process.argv[2] == "json2j") && (process.argv.length == 3) && fs.existsSync(process.argv[3]))
  fs.writeFileSync(process.argv[4], Struct.json2j(fs.readFileSync(process.argv[3]).toString()));
else {
  console.log("Usage : node .../Struct_main.js [j2json|json2j] input-file output-file");
  if(!fs.existsSync(process.argv[3]))
    console.log(" error : file '" + process.argv[3] + "' does not exist");
  console.log("You called : ``node .../Struct_main.js " + process.argv[0] + " " + process.argv[1] + " " + process.argv[2]);
}
