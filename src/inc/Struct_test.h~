#include "Struct.hpp"

/** Struct functional and non regression test. */

void Struct_test()
{
  // Internal data storage test
  {
    Struct value;
    value["u"] = "ok", value["v"] = (String) value["u"] + "ay", value[0] = "a", value[1] = "b";
    Struct tuple;
    tuple.set("d", "okay").set("b", true).set("a", 1.234).set("b", 666).set("c", false).set("e", true).del("e");
    value.set("w", tuple);
    Struct list;
    list.add("et").add("de").add("un");
    value.set("l", list);
    String result = "{\"u\": \"ok\", \"v\": \"okay\", \"w\": {\"d\": \"okay\", \"b\": \"666\", \"a\": \"1.234\", \"c\": \"false\"}, \"l\": [\"et\", \"de\", \"un\"], \"0\": \"a\", \"1\": \"b\"}";
    assume(((String) value) == result, "illegal-state", "value != result =>\n\t%s\t!=\n\t%s\n", ((String) value).c_str(), result.c_str());
    // - value.save("stdout", "plain");
  }
  // Casting test
  {
    Struct value;
    value["a"] = 3.1416;
    assume((double) value["a"] == 3.1416, "illegal-state", "bad (double) casting");
  }
  // In/Out parsing test
  {
    Struct value;
    String input = "{ 1 = A, b = [2, 3], a = 1, 0 = Z, 10 = 'M', c= \"ah que \\/ \t oui\", 2 = B}";
    value.reset(input);
    String result = "{\"b\": [\"2\", \"3\"], \"a\": \"1\", \"c\": \"ah que / \\t oui\", \"0\": \"Z\", \"1\": \"A\", \"2\": \"B\", \"3\": \"\", \"4\": \"\", \"5\": \"\", \"6\": \"\", \"7\": \"\", \"8\": \"\", \"9\": \"\", \"10\": \"M\"}";
    assume(((String) value) == result, "illegal-state", "value != result =>\n\t%s\t!=\n\t%s\n", ((String) value).c_str(), result.c_str());
    String file = "/tmp/test.json";
    value.save(file);
    Struct value2;
    value2.load(file);
    assume(value == value2, "illegal-state", "value != value2 =>\n\t%s\t!=\n\t%s\n", ((String) value).c_str(), ((String) value2).c_str());
    // - system("json-glib-validate /tmp/test.json");
  }
}
