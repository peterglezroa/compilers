#include <iterator>
#include <list>
#include <queue>
#include <string>
#include "lexical_analyzer.h"

void compare_lists (
    const std::list<std::string> list1,
    const std::list<std::string> list2
){
    std::queue<std::string> missing; // List of words missing

    // Find items of list 1 in list 2
    for (std::string str : list1)
        if (std::find(list2.begin(), list2.end(), str) == list2.end())
            missing.push(str);

    // Find items of list 2 in list 1
    for (std::string str : list2)
        if (std::find(list1.begin(), list1.end(), str) == list1.end())
            missing.push(str);

    if (missing.size() > 0) {
        fprintf(stdout, "\033[1;31mINCORRECT\033[0m\n\t");

        fprintf(stdout, "( ");
        std::string top = missing.front(); missing.pop();
        for (const std::string str : list1)
            if (str.compare(top) == 0) {
                fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
                if (!missing.empty()) { top = missing.front(); missing.pop(); }
            } else fprintf(stdout, "%s ", str.c_str());
        fprintf(stdout, ")");

        fprintf(stdout, "\t( ");
        for (const std::string str : list2)
            if (str.compare(top) == 0) {
                fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
                if (!missing.empty()) { top = missing.front(); missing.pop(); }
            } else fprintf(stdout, "%s ", str.c_str());
        fprintf(stdout, ")\n");
    } else fprintf(stdout, "\033[1;32mCORRECT\033[0m\n");
}

int main(int argc, char *argv[]) {
    LexicalAnalyzer analyzer = LexicalAnalyzer();

    fprintf(stdout, "===================== TEST 01 =====================\n");
    std::list<std::string> test1p = {"E", "EPrime", "T", "TPrime", "F"};
    std::list<std::string> test1t = {"+", "*", "(", ")", "id"};
    analyzer.parse("E -> T EPrime");
    analyzer.parse("EPrime -> + T EPrime");
    analyzer.parse("EPrime -> ''");
    analyzer.parse("T -> F TPrime");
    analyzer.parse("TPrime -> * F TPrime");
    analyzer.parse("TPrime -> ''");
    analyzer.parse("F -> ( E )");
    analyzer.parse("F -> id");
    fprintf(stdout, "Test synthatic variables: ");
    compare_lists(analyzer.getVariables(), test1p);
    fprintf(stdout, "Test terminals: ");
    compare_lists(analyzer.getTerminals(), test1t);
    fprintf(stdout, "\n");

    analyzer.clear();

    fprintf(stdout, "===================== TEST 02 =====================\n");
    std::list<std::string> test2p = {"E", "T", "F"};
    std::list<std::string> test2t = {"+", "*", "(", ")", "id"};
    analyzer.parse("E -> E + T");
    analyzer.parse("E -> T");
    analyzer.parse("T -> T * F");
    analyzer.parse("T -> F");
    analyzer.parse("F -> id");
    analyzer.parse("F -> ( E )");
    fprintf(stdout, "Test synthatic variables: ");
    compare_lists(analyzer.getVariables(), test2p);
    fprintf(stdout, "Test terminals: ");
    compare_lists(analyzer.getTerminals(), test2t);
    fprintf(stdout, "\n");

    analyzer.clear();

    fprintf(stdout, "===================== TEST 03 =====================\n");
    std::list<std::string> test3p = {"A", "B", "C", "D"};
    std::list<std::string> test3t = {"a", "b"};
    analyzer.parse("A -> a A");
    analyzer.parse("A -> b A");
    analyzer.parse("A -> a B");
    analyzer.parse("B -> b C");
    analyzer.parse("C -> b D");
    analyzer.parse("D -> ''");
    fprintf(stdout, "Test synthatic variables: ");
    compare_lists(analyzer.getVariables(), test3p);
    fprintf(stdout, "Test terminals: ");
    compare_lists(analyzer.getTerminals(), test3t);
    fprintf(stdout, "\n");

    analyzer.clear();

    fprintf(stdout, "===================== TEST 04 =====================\n");
    std::list<std::string> test4p = {"bexpr", "bterm", "bfactor"};
    std::list<std::string> test4t = {"or", "and", "not", "(", ")", "true", "false"};
    analyzer.parse("bexpr -> bexpr or bterm");
    analyzer.parse("bexpr -> bterm");
    analyzer.parse("bterm -> bterm and bfactor");
    analyzer.parse("bterm -> bfactor");
    analyzer.parse("bfactor -> not bfactor");
    analyzer.parse("bfactor -> ( bexpr )");
    analyzer.parse("bfactor -> true");
    analyzer.parse("bfactor -> false");
    fprintf(stdout, "Test synthatic variables: ");
    compare_lists(analyzer.getVariables(), test4p);
    fprintf(stdout, "Test terminals: ");
    compare_lists(analyzer.getTerminals(), test4t);
    fprintf(stdout, "\n");

    analyzer.clear();

    fprintf(stdout, "===================== TEST 05 =====================\n");
    std::list<std::string> test5p = {"S", "A", "APrime"};
    std::list<std::string> test5t = {"a", "b", "c", "d"};
    analyzer.parse("S -> A a");
    analyzer.parse("S -> b");
    analyzer.parse("A -> b d APrime");
    analyzer.parse("A -> APrime");
    analyzer.parse("APrime -> c APrime");
    analyzer.parse("APrime -> a d APrime");
    analyzer.parse("APrime -> ''");
    fprintf(stdout, "Test synthatic variables: ");
    compare_lists(analyzer.getVariables(), test5p);
    fprintf(stdout, "Test terminals: ");
    compare_lists(analyzer.getTerminals(), test5t);
    fprintf(stdout, "\n");


    // Test first and follows
    return 0;
}
