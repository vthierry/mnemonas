///@cond INTERNAL

void Struct_test()
{
  // Internal data storage test
  {
    Struct value;
    value["u"] = "ok", value["v"] = (String) value["u"] + "ay", value[0] = "a", value[1] = "b";
    Struct tuple;
    tuple.set("d", "okay").set("b", true).set("a", 1.234).set("b", 666).set("c", false).set("e", true).unset("e");
    value.set("w", tuple);
    Struct list;
    list.add("et").add("de").add("un");
    value.set("l", list);
    String result = "{\"u\": \"ok\", \"v\": \"okay\", \"w\": {\"d\": \"okay\", \"b\": \"666\", \"a\": \"1.234\", \"c\": \"false\"}, \"l\": [\"et\", \"de\", \"un\"], \"0\": \"a\", \"1\": \"b\"}";
    assume(((String) value) == result, "illegal-state", "in Struct_test 1/4: value != result =>\n\t«%s»\t!=\n\t«%s»\n", ((String) value).c_str(), result.c_str());
    // - value.save("stdout", "plain");
    // - for(std::vector < std::string >::const_iterator i = value.getNames().begin(); i != value.getNames().end(); i++) printf("> %s\n", (*i).c_str());
  }
  // Casting test
  {
    Struct value;
    value["a"] = 3.1416;
    assume((double) value["a"] == 3.1416, "illegal-state", "in Struct_test 2/4: bad (double) casting");
  }
  // In/Out parsing test
  {
    Struct value;
    String input = "{ 1 = A, b = [2, 3], a = 1, 0 = Z, 10 = 'M', c= \"ah que \\/ \t oui\", 2 = B}";
    value.reset(input);
    String result = "{\"b\": [\"2\", \"3\"], \"a\": \"1\", \"c\": \"ah que / \\t oui\", \"0\": \"Z\", \"1\": \"A\", \"2\": \"B\", \"10\": \"M\"}";
    assume(((String) value) == result, "illegal-state", "in Struct_test 3/4: value != result =>\n\t«%s»\t!=\n\t«%s»\n", ((String) value).c_str(), result.c_str());
    String file = "/tmp/test.json";
    value.save(file, "plain");
    Struct value2;
    value2.load(file);
    assume(value == value2, "illegal-state", "in Struct_test 4/4: value != value2 =>\n\t«%s»\t!=\n\t«%s»\n", ((String) value).c_str(), ((String) value2).c_str());
#if 0
    system("json-glib-validate /tmp/test.json");
    value.save("/tmp/test.json.html", "html");
    system("firefox /tmp/test.json.html");
#endif
  }
  // @todo
  return;
  exit(0);
  // Tests the J= syntax mechanism
  {
    String input = s_load("./src/util/Struct_j=.j=");
    Struct value;
    value.reset(input);
    String output = value.asString("jplain");
    assume(input == output, "illegal-state", "in Struct_test_js 2/2 error in the C/C++ J= implementation");
#if 1
    assume(system("cd ./src/util ; cat Struct.js Struct_test.js | node -") == 0, "illegal-state", "in Struct_test_js 2/2 error in the JavaScript J= implementation");
#endif
  }
}
///@endcond
