///@cond INTERNAL

void s_regex_test()
{
#ifdef ON_NEF
  return; // regex not yet implemented with g++ < 4.9
#endif
  assume(s_regex_replace("papy", "([ay])", "u") == "pupu", "illegal-state", "in s_regex_test/s_regex_replace bad substitution");
  assume(s_regex_match("papy", "^(p[ay])+$"), "illegal-state", "in s_regex_test/s_regex_match bad match");
}
///@endcond
