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
    String result = "{\"u\": \"ok\", \"v\": \"okay\", \"0\": \"a\", \"1\": \"b\", \"w\": {\"d\": \"okay\", \"b\": \"666\", \"a\": \"1.234\", \"c\": \"false\"}, \"l\": [\"et\", \"de\", \"un\"]}";
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
    String result = "{\"1\": \"A\", \"b\": [\"2\", \"3\"], \"a\": \"1\", \"0\": \"Z\", \"10\": \"M\", \"c\": \"ah que / \\t oui\", \"2\": \"B\"}";
    assume(((String) value) == result, "illegal-state", "in Struct_test 3/4: value != result =>\n\t«%s»\t!=\n\t«%s»\n", ((String) value).c_str(), result.c_str());
    String file = "/tmp/test.json";
    value.save(file, "plain");
    Struct value2;
    value2.load(file);
    assume(value == value2, "illegal-state", "in Struct_test 4/4: value != value2 =>\n\t«%s»\t!=\n\t«%s»\n", ((String) value).c_str(), ((String) value2).c_str());
#if 0
    value.save("/tmp/test.json.html", "html");
    system("json-glib-validate /tmp/test.json");
    system("firefox /tmp/test.json.html");
#endif
  }

  // Tests the J= syntax mechanism
#if 1
  {
    system("node ./src/util/Struct_main.js j2json ./src/util/Struct_j.j /tmp/Struct_j_1.json");
    system("node ./src/util/Struct_main.js json2j /tmp/Struct_j_1.json /tmp/Struct_j_1.j");
    system("node ./src/util/Struct_main.js j2json /tmp/Struct_j_1.j /tmp/Struct_j_2.json");
    {
      std::string s0 = s_load("./src/util/Struct_j.j"), s1 = s_load("/tmp/Struct_j_1.j");
      assume(s0 == s1, "illegal-state", "in Struct_test_js the j0: «\n%s» differs from j1: «\n%s»", s0.c_str(), s1.c_str());
    }
    {
      Struct s1, s2;
      s1.load("/tmp/Struct_j_1.json");
      s2.load("/tmp/Struct_j_2.json");
      assume(s1 == s2, "illegal-state", "in Struct_test_js the json1: «\n%s» differs from json2: «\n%s»", ((String) s1).c_str(), ((String) s2).c_str());
    }
  }
#endif
}
///@endcond
