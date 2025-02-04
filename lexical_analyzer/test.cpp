#include <iterator>
#include <list>
#include <string>
#include "lexical_analyzer.h"

#define DEBUG 1

void print_correct() { fprintf(stdout, "\033[1;32mCORRECT\033[0m\n"); }

void print_incorrect() { fprintf(stdout, "\033[1;31mINCORRECT\033[0m\n"); }

void compare_lists(std::list<std::string> list1, std::list<std::string> list2) {
  std::list<std::string> intersection, subs1, subs2;
  list1.sort();
  list2.sort();
  std::set_intersection (
    list1.begin(), list1.end(), list2.begin(), list2.end(),
    std::back_inserter(intersection)
  );

  if (intersection.size() != list1.size() || intersection.size() != list2.size()) {
    print_incorrect();
    fprintf(stdout, "\t");

    std::set_difference (
      list1.begin(), list1.end(), intersection.begin(), intersection.end(),
      std::back_inserter(subs1)
    );
    fprintf(stdout, "{ ");
    for (const std::string str : intersection)
      fprintf(stdout, "%s ", str.c_str());
    for (const std::string str : subs1)
      fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, "}");

    std::set_difference (
      list2.begin(), list2.end(), intersection.begin(), intersection.end(),
      std::back_inserter(subs2)
    );
    fprintf(stdout, "\t{ ");
    for (const std::string str : intersection)
      fprintf(stdout, "%s ", str.c_str());
    for (const std::string str : subs2)
      fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, "}\n");
  } else print_correct();
}

int main(int argc, char *argv[]) {
  FILE *log = NULL;

  // Passed a log file
  if (argc == 2)
    if ( (log = fopen(argv[1], "w")) == NULL ) {
      fprintf(stderr, "Could not open log file.");
      return -1;
    }

  LexicalAnalyzer analyzer = LexicalAnalyzer(log);

// ================================= TEST 01 =================================
  fprintf(stdout, "===================== TEST 01 =====================\n");
  analyzer.parse({
    "E -> T EPrime",
    "EPrime -> + T EPrime",
    "EPrime -> ''",
    "T -> F TPrime",
    "TPrime -> * F TPrime",
    "TPrime -> ''",
    "F -> ( E )",
    "F -> id"
  });

  // Terminals and not terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"E", "EPrime", "T", "TPrime", "F"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"+", "*", "(", ")", "id"});

  // Firsts
  fprintf(stdout, "Test FIRST(E): ");
  compare_lists(analyzer.getFirst("E"), {"(", "id"});
  fprintf(stdout, "Test FIRST(EPrime): ");
  compare_lists(analyzer.getFirst("EPrime"), {"+", "''"});
  fprintf(stdout, "Test FIRST(T): ");
  compare_lists(analyzer.getFirst("T"), {"(", "id"});
  fprintf(stdout, "Test FIRST(TPrime): ");
  compare_lists(analyzer.getFirst("TPrime"), {"*", "''"});
  fprintf(stdout, "Test FIRST(F): ");
  compare_lists(analyzer.getFirst("F"), {"(", "id"});

  // Follows
  fprintf(stdout, "Test FOLLOW(E): ");
  compare_lists(analyzer.getFollow("E"), {"$", ")"});
  fprintf(stdout, "Test FOLLOW(EPrime): ");
  compare_lists(analyzer.getFollow("EPrime"), {"$", ")"});
  fprintf(stdout, "Test FOLLOW(T): ");
  compare_lists(analyzer.getFollow("T"), {"+", "$", ")"});
  fprintf(stdout, "Test FOLLOW(TPrime): ");
  compare_lists(analyzer.getFollow("TPrime"), {"+", "$", ")"});
  fprintf(stdout, "Test FOLLOW(F): ");
  compare_lists(analyzer.getFollow("F"), {"*", "+", "$", ")"});

  // LL? (Yes)
  fprintf(stdout, "Test LL(1): ");
  (analyzer.is_ll())? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 01 =================================

  analyzer.clear();

// ================================= TEST 02 =================================
  fprintf(stdout, "===================== TEST 02 =====================\n");
  analyzer.parse({
    "E -> E + T",
    "E -> T",
    "T -> T * F",
    "T -> F",
    "F -> id",
    "F -> ( E )"
  });

  // Terminals and not terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"E", "T", "F"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"+", "*", "(", ")", "id"});

  // Firsts
  fprintf(stdout, "Test FIRST(E): ");
  compare_lists(analyzer.getFirst("E"), {"id", "("});
  fprintf(stdout, "Test FIRST(T): ");
  compare_lists(analyzer.getFirst("T"), {"id", "("});
  fprintf(stdout, "Test FIRST(F): ");
  compare_lists(analyzer.getFirst("F"), {"id", "("});

  // Follows
  fprintf(stdout, "Test FOLLOW(E): ");
  compare_lists(analyzer.getFollow("E"), {"$", "+", ")"});
  fprintf(stdout, "Test FOLLOW(T): ");
  compare_lists(analyzer.getFollow("T"), {"*", "$", "+", ")"});
  fprintf(stdout, "Test FOLLOW(F): ");
  compare_lists(analyzer.getFollow("F"), {"*", "$", "+", ")"});

  // LL? (No)
  fprintf(stdout, "Test LL(1): ");
  (!analyzer.is_ll())? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 02 =================================

  analyzer.clear();

// ================================= TEST 03 =================================
  fprintf(stdout, "===================== TEST 03 =====================\n");
  analyzer.parse({
    "A -> a A",
    "A -> b A",
    "A -> a B",
    "B -> b C",
    "C -> b D",
    "D -> ''"
  });

  // Terminals and not Terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"A", "B", "C", "D"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"a", "b"});

  // Firsts
  fprintf(stdout, "Test FIRST(A): ");
  compare_lists(analyzer.getFirst("A"), {"a", "b"});
  fprintf(stdout, "Test FIRST(B): ");
  compare_lists(analyzer.getFirst("B"), {"b"});
  fprintf(stdout, "Test FIRST(C): ");
  compare_lists(analyzer.getFirst("C"), {"b"});
  fprintf(stdout, "Test FIRST(D): ");
  compare_lists(analyzer.getFirst("D"), {"''"});

  // Follows
  fprintf(stdout, "Test FOLLOW(A): ");
  compare_lists(analyzer.getFollow("A"), {"$"});
  fprintf(stdout, "Test FOLLOW(B): ");
  compare_lists(analyzer.getFollow("B"), {"$"});
  fprintf(stdout, "Test FOLLOW(C): ");
  compare_lists(analyzer.getFollow("C"), {"$"});
  fprintf(stdout, "Test FOLLOW(D): ");
  compare_lists(analyzer.getFollow("D"), {"$"});

  // LL? (No)
  fprintf(stdout, "Test LL(1): ");
  (!analyzer.is_ll())? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 03 =================================

  analyzer.clear();

// ================================= TEST 04 =================================
  fprintf(stdout, "===================== TEST 04 =====================\n");
  analyzer.parse({
    "bexpr -> bexpr or bterm",
    "bexpr -> bterm",
    "bterm -> bterm and bfactor",
    "bterm -> bfactor",
    "bfactor -> not bfactor",
    "bfactor -> ( bexpr )",
    "bfactor -> true",
    "bfactor -> false"
  });

  // No terminals and terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"bexpr", "bterm", "bfactor"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"or", "and", "not", "(", ")", "true", "false"});

  // Firsts
  fprintf(stdout, "Test FIRST(bexpr): ");
  compare_lists(analyzer.getFirst("bexpr"), {"not", "(", "true", "false"});
  fprintf(stdout, "Test FIRST(bterm): ");
  compare_lists(analyzer.getFirst("bterm"), {"not", "(", "true", "false"});
  fprintf(stdout, "Test FIRST(bfactor): ");
  compare_lists(analyzer.getFirst("bfactor"), {"not", "(", "true", "false"});

  // Follows
  fprintf(stdout, "Test FOLLOW(bexpr): ");
  compare_lists(analyzer.getFollow("bexpr"), {"$", "or", ")"});
  fprintf(stdout, "Test FOLLOW(bterm): ");
  compare_lists(analyzer.getFollow("bterm"), {"$", "or", ")", "and"});
  fprintf(stdout, "Test FOLLOW(bfactor): ");
  compare_lists(analyzer.getFollow("bfactor"), {"$", "or", ")", "and"});

  // LL? (No)
  fprintf(stdout, "Test LL(1): ");
  (!analyzer.is_ll())? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 04 =================================

  analyzer.clear();

// ================================= TEST 05 =================================
  fprintf(stdout, "===================== TEST 05 =====================\n");
  analyzer.parse({
    "S -> A a",
    "S -> b",
    "A -> b d APrime",
    "A -> APrime",
    "APrime -> c APrime",
    "APrime -> a d APrime",
    "APrime -> ''"
  });

  // No Terminals and Terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"S", "A", "APrime"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"a", "b", "c", "d"});

  // Firsts
  fprintf(stdout, "Test FIRST(S): ");
  compare_lists(analyzer.getFirst("S"), {"b", "c", "a"});
  fprintf(stdout, "Test FIRST(A): ");
  compare_lists(analyzer.getFirst("A"), {"b", "c", "a", "''"});
  fprintf(stdout, "Test FIRST(APrime): ");
  compare_lists(analyzer.getFirst("APrime"), {"c", "a", "''"});

  // Follows
  fprintf(stdout, "Test FOLLOW(S): ");
  compare_lists(analyzer.getFollow("S"), {"$"});
  fprintf(stdout, "Test FOLLOW(A): ");
  compare_lists(analyzer.getFollow("A"), {"a"});
  fprintf(stdout, "Test FOLLOW(APrime): ");
  compare_lists(analyzer.getFollow("APrime"), {"a"});

  // LL? (No)
  fprintf(stdout, "Test LL(1): ");
  (analyzer.is_ll())? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 05 =================================

  analyzer.clear();


// ================================= TEST 06 =================================
  fprintf(stdout, "===================== TEST 06 =====================\n");
  analyzer.parse({
    "goal -> A",
    "A -> ( A )",
    "A -> two",
    "two -> a",
    "two -> b",
  });

  // No Terminals and Terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"goal", "A", "two"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"(", ")", "a", "b"});

  // Firsts
  fprintf(stdout, "Test FIRST(goal): ");
  compare_lists(analyzer.getFirst("goal"), {"(", "a", "b"});
  fprintf(stdout, "Test FIRST(A): ");
  compare_lists(analyzer.getFirst("A"), {"(", "a", "b"});
  fprintf(stdout, "Test FIRST(two): ");
  compare_lists(analyzer.getFirst("two"), {"a", "b"});

  // Follows
  fprintf(stdout, "Test FOLLOW(goal): ");
  compare_lists(analyzer.getFollow("goal"), {"$"});
  fprintf(stdout, "Test FOLLOW(A): ");
  compare_lists(analyzer.getFollow("A"), {")", "$"});
  fprintf(stdout, "Test FOLLOW(two): ");
  compare_lists(analyzer.getFollow("two"), {")", "$"});

  // LL? (Yes)
  fprintf(stdout, "Test LL(1): ");
  (analyzer.is_ll())? print_correct() : print_incorrect();

  // Accept string
  fprintf(stdout, "Test string '( ( a ) )': ");
  (analyzer.validStr("( ( a ) )"))? print_correct() : print_incorrect();

  fprintf(stdout, "Test string '( a ) )': ");
  (!analyzer.validStr("( a ) )"))? print_correct() : print_incorrect();

  fprintf(stdout, "Test string '( ( ( ( ( b ) ) ) ) )': ");
  (analyzer.validStr("( ( ( ( ( b ) ) ) ) )"))? print_correct() : print_incorrect();

  fprintf(stdout, "Test string '( ( ( ( ( a b ) ) ) ) )': ");
  (!analyzer.validStr("( ( ( ( ( a b ) ) ) ) )"))? print_correct() : print_incorrect();
  fprintf(stdout, "\n");
// ================================= TEST 06 =================================

  analyzer.clear();

// ================================= TEST 07 =================================
  fprintf(stdout, "===================== TEST 07 =====================\n");
  analyzer.parse({
    "E -> T X",
    "X -> + E",
    "X -> ''",
    "T -> int Y",
    "T -> ( E )",
    "Y -> * T",
    "Y -> ''",
  });

  // No Terminals and Terminals
  fprintf(stdout, "Test synthatic variables: ");
  compare_lists(analyzer.getVariables(), {"E", "X", "T", "Y"});
  fprintf(stdout, "Test terminals: ");
  compare_lists(analyzer.getTerminals(), {"+", "int", "(", ")", "*"});

  // Firsts
  fprintf(stdout, "Test FIRST(E): ");
  compare_lists(analyzer.getFirst("E"), {"(", "int"});
  fprintf(stdout, "Test FIRST(X): ");
  compare_lists(analyzer.getFirst("X"), {"+", EPSILON});
  fprintf(stdout, "Test FIRST(T): ");
  compare_lists(analyzer.getFirst("T"), {"int", "("});
  fprintf(stdout, "Test FIRST(Y): ");
  compare_lists(analyzer.getFirst("Y"), {"*", EPSILON});

  // Follows
  fprintf(stdout, "Test FOLLOW(E): ");
  compare_lists(analyzer.getFollow("E"), {"$", ")"});
  fprintf(stdout, "Test FOLLOW(X): ");
  compare_lists(analyzer.getFollow("X"), {"$", ")"});
  fprintf(stdout, "Test FOLLOW(T): ");
  compare_lists(analyzer.getFollow("T"), {"+", "$", ")"});
  fprintf(stdout, "Test FOLLOW(Y): ");
  compare_lists(analyzer.getFollow("Y"), {"+", "$", ")"});

  // LL? (Yes)
  fprintf(stdout, "Test LL(1): ");
  (analyzer.is_ll())? print_correct() : print_incorrect();

  // Accept string
  fprintf(stdout, "Test string 'int * ( int + int )': ");
  (analyzer.validStr("int * ( int + int )"))? print_correct() : print_incorrect();

  fprintf(stdout, "\n");
// ================================= TEST 06 =================================

  if (log != NULL) fclose(log);
  return 0;
}
