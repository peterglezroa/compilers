#include <iterator>
#include <list>
#include <string>
#include <vector>
#include "lexical_analyzer.h"

bool has_same_strings(const std::list<std::string> list, const std::vector<std::string> vector) {
    bool found = false;
    if (list.size() != vector.size())
        return false;
    for (std::string str : vector)
        if (std::find(list.begin(), list.end(), str) == list.end())
            return false;
    return true;
}

void test(const LexicalAnalyzer analyzer, std::vector<std::string> vars, std::vector<std::string> terms) {
    fprintf(stdout, "( ");
    for (std::string str : analyzer.getVariables())
        fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, ") and ( ");
    for (std::string str : vars)
        fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, "): ");
    (has_same_strings(analyzer.getVariables(), vars))?
        fprintf(stdout, "\033[1;32mCORRECT\033[0m\n"):
        fprintf(stdout, "\033[1;31mINCORRECT\033[0m\n");

    fprintf(stdout, "( ");
    for (std::string str : analyzer.getVariables())
        fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, ") and ( ");
    for (std::string str : terms)
        fprintf(stdout, "\033[1;35m%s \033[0m", str.c_str());
    fprintf(stdout, "): ");
    (has_same_strings(analyzer.getTerminals(), terms))?
        fprintf(stdout, "\033[1;32mCORRECT\033[0m\n"):
        fprintf(stdout, "\033[1;31mINCORRECT\033[0m\n");
}

int main(int argc, char *argv[]) {
    LexicalAnalyzer analyzer = LexicalAnalyzer();


    std::vector<std::string> test1p = {"E", "EPrime", "T", "TPrime", "F"};
    std::vector<std::string> test1t = {"+", "*", "(", ")", "id"};
    analyzer.parse("E -> T EPrime");
    analyzer.parse("EPrime -> + T EPrime");
    analyzer.parse("EPrime -> ''");
    analyzer.parse("T -> F TPrime");
    analyzer.parse("TPrime -> * F TPrime");
    analyzer.parse("TPrime -> ''");
    analyzer.parse("F -> ( E )");
    analyzer.parse("F -> id");
    test(analyzer, test1p, test1t);
    fprintf(stdout, "\n");

    analyzer.clear();

    std::vector<std::string> test2p = {"E", "T", "F"};
    std::vector<std::string> test2t = {"+", "*", "(", ")", "id"};
    analyzer.parse("E -> E + T");
    analyzer.parse("E -> T");
    analyzer.parse("T -> T * F");
    analyzer.parse("T -> F");
    analyzer.parse("F -> id");
    analyzer.parse("F -> ( E )");
    test(analyzer, test2p, test2t);
    fprintf(stdout, "\n");

    analyzer.clear();

    std::vector<std::string> test3p = {"A", "B", "C", "D"};
    std::vector<std::string> test3t = {"a", "b"};
    analyzer.parse("A -> a A");
    analyzer.parse("A -> b A");
    analyzer.parse("A -> a B");
    analyzer.parse("B -> b C");
    analyzer.parse("C -> b D");
    analyzer.parse("D -> ''");
    test(analyzer, test3p, test3t);
    fprintf(stdout, "\n");

    analyzer.clear();

    std::vector<std::string> test4p = {"bexpr", "bterm", "bfactor"};
    std::vector<std::string> test4t = {"or", "and", "not", "(", ")", "true", "false"};
    analyzer.parse("bexpr -> bexpr or bterm");
    analyzer.parse("bexpr -> bterm");
    analyzer.parse("bterm -> bterm and bfactor");
    analyzer.parse("bterm -> bfactor");
    analyzer.parse("bfactor -> not bfactor");
    analyzer.parse("bfactor -> ( bexpr )");
    analyzer.parse("bfactor -> true");
    analyzer.parse("bfactor -> false");
    test(analyzer, test4p, test4t);
    fprintf(stdout, "\n");

    analyzer.clear();

    std::vector<std::string> test5p = {"S", "A", "APrime"};
    std::vector<std::string> test5t = {"a", "b", "c", "d"};
    analyzer.parse("S -> A a");
    analyzer.parse("S -> b");
    analyzer.parse("A -> b d APrime");
    analyzer.parse("A -> APrime");
    analyzer.parse("APrime -> c APrime");
    analyzer.parse("APrime -> a d APrime");
    analyzer.parse("APrime -> ''");
    test(analyzer, test5p, test5t);


    // Test first and follows
    return 0;
}
