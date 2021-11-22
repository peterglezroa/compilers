#include <iostream>
#include "lexical_analyzer.h"

#define MAX_RULE_LEN 256

std::string scan_line() {
    char str[256];
    fgets(str, MAX_RULE_LEN, stdin);
    if (strlen(str) > 0 && str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';
    return std::string(str);
}

int main(int argc, char *argv[]) {
    int amount;
    std::string rule;
    LexicalAnalyzer analyzer;

    fscanf(stdin, "%i%*c", &amount);
    for (int i = 0; i < amount; i++) {
      rule = scan_line();
      if (!analyzer.parse(std::string(rule)))
        fprintf(stderr, "%s is not a valid rule!", rule.c_str());
    }

    std::list<std::string> vars = analyzer.getVariables();
    for (const std::string var : vars) {
      fprintf(stdout, "%s => FIRST = { ", var.c_str());
      for (const std::string first : analyzer.getFirst(var))
        fprintf(stdout, "\033[1;35m%s\033[0m ", first.c_str());
      fprintf(stdout, "}, FOLLOW = { ");
      for (const std::string follow : analyzer.getFollow(var))
        fprintf(stdout, "\033[1;35m%s\033[0m ", follow.c_str());
      fprintf(stdout, "}\n");
    }

    fprintf(stdout, "LL(1)? ");
    (analyzer.is_ll())?
      fprintf(stdout, "\033[1;32mYES\033[0m\n")
    :
      fprintf(stdout, "\033[1;31mNO\033[0m\n\t")
    ;
    return 0;
}
