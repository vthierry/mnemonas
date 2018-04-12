// This allows to test Struct.js
// Usage : cat Struct.js Struct_test.js | node -

const fs = require("fs");
var input = fs.readFileSync("Struct_j=.j=").toString();
//var input =  "what = \n AA = TT\n BB = WW\nUU = XX\n= WW\n= ZZ1\n et le reste\n = toto\n = titi\n= ZZ2\n et le reste\n toto\n titi\n";
var data = Struct.string2data(input);
var output = Struct.data2string(data);
if (input != output) {
  console.log("Upss :" + 
	      "\n--------------------------------------------------------------\n" +
	      input +
	      "\n!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=\n" +
	      output +
	      "\n--------------------------------------------------------------\n");
  process.exit(-1);
 }

 
