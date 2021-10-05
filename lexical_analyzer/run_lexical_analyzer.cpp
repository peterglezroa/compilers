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
    fprintf(stdout, "\nThe productions are: ");
    std::string lastprod = analyzer.getProductions().back();
    for (std::string const prod : analyzer.getProductions())
        (prod != lastprod)?
            fprintf(stdout, "%s, ", prod.c_str())
        :
            fprintf(stdout, "%s\n", prod.c_str())
        ;

    fprintf(stdout, "The terminals are: ");
    std::string lastterm = analyzer.getTerminals().back();
    for (std::string const term : analyzer.getTerminals())
        (term != lastterm)?
            fprintf(stdout, "%s, ", term.c_str())
        :
            fprintf(stdout, "%s\n", term.c_str())
        ;
    return 0;
}
