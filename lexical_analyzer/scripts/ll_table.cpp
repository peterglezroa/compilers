#include <iostream>
#include "../lexical_analyzer.h"

#define MAX_RULE_LEN 256

std::string scan_line() {
    char str[256];
    fgets(str, MAX_RULE_LEN, stdin);
    if (strlen(str) > 0 && str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';
    return std::string(str);
}

int main(int argc, char *argv[]) {
    int nrules, ntests;
    std::list<std::string> rules, tests;
    FILE *html, *log = NULL;

    if (argc == 2 || argc == 3) {
      if( (html = fopen(argv[1], "w")) == NULL) {
        fprintf(stderr, "Could not open file to write HTML.\n");
        return -1;
      }

      if (argc == 3)
        if( (log = fopen(argv[2], "w")) == NULL) {
          fprintf(stderr, "Could not open file to write logs.\n");
          return -2;
        }
    } else {
      fprintf(stderr, "usage: %s <HTML file destination>\n", argv[0]);
      return -3;
    }


    LexicalAnalyzer analyzer = LexicalAnalyzer(log);

    // Scan rules
    fscanf(stdin, "%i %i%*c", &nrules, &ntests);
    for (int i = 0; i < nrules; i++) rules.push_back(std::string(scan_line()));

    // Parse rules
    if (!analyzer.parse(rules))
      fprintf(stderr, "Syntax for the rules was rejected! NoTerm -> T T ''\n");

    if (!analyzer.is_ll()) {
      fprintf(stderr, "This is not LL!\n");
      return -4;
    }

    // Set headers
    fprintf(html, "<table>\n\t<tr>\n");
    fprintf(html, "\t\t<th>No Terminal</th>\n");
    for (const std::string t : analyzer.getTerminals())
      fprintf(html, "\t\t<th>%s</th>\n", t.c_str());
    fprintf(html, "\t</tr>\n");

    // Build table
    for (const std::string var : analyzer.getVariables()) {
      fprintf(html, "\t<tr>\n");
      for (const std::string t : analyzer.getTerminals())
        fprintf(html, "\t\t<td>%s</td>\n", analyzer.getProd(var, t).c_str());
      fprintf(html, "\t</tr>\n");
    }
    fprintf(html, "</table>\n");

    // Scan tests
    for (int i = 0; i < ntests; i++) tests.push_back(std::string(scan_line()));

    // Run tests
    int i = 1;
    for (const std::string test : tests) {
      fprintf(html, "<p>Input #%i: ", i);
      (analyzer.validStr(test))?
        fprintf(html, "Yes</p>\n"):
        fprintf(html, "No</p>\n");
      i++;
    }

    fclose(html);
    if(log != NULL) fclose(log);
    return 0;
}
