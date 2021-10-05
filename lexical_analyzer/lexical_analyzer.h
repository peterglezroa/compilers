#ifndef lexical_analyzer
#define lexical_analyzer

#include <string>
#include <iterator>
#include <algorithm>
#include <list>
#include <regex>

//#define RULEREGEX "([A-Za-z_-]+) ?-> ?(.*)"

class LexicalAnalyzer {
    private:
        std::list<std::string> prods;
        std::list<std::string> terms;

        bool has_prod(std::string str) {
            return (std::find(prods.begin(), prods.end(), str)!=prods.end());
        }

        bool has_term(std::string str) {
            return (std::find(terms.begin(), terms.end(), str)!=terms.end());
        }

    public:
        LexicalAnalyzer() {}

        void clear() { prods.clear(); terms.clear(); }

        bool parse(std::string rule) {
            std::regex ruleRegex("([A-Za-z_-]+) -> (.*)");
            /* Parses a given rule. If the sintax is valid it returns true, if
             * not it returns false */
            std::string production, terminal, *strptr;
            char *cptr;
            size_t pos;

            if (!std::regex_match(rule, ruleRegex))
                return false;

            // Find the divider between production and terminals
            pos = rule.find(" -> ");

            // Get production and add it to list if not found
            production = rule.substr(0, pos);
            if (!has_prod(production))
                prods.push_back(production);

            // Remove production from terminal list if it was inside
            terms.remove(production);

            rule.erase(0, pos+4); // Erase production to only leave rule

            // Iterate through words in the rule
            while ((pos = rule.find(' ')) != std::string::npos) {
                terminal = rule.substr(0, pos);
                if (!has_prod(terminal) && !has_term(terminal))
                    terms.push_back(terminal);
                rule.erase(0, pos+1);
            }
            terms.push_back(rule); // last word

            terms.unique();
            terms.remove("''");
            return true;
        }

        bool validString(std::string) {return false;}

        const std::list<std::string> getProductions() const { return prods;}

        const std::list<std::string> getTerminals() const { return terms; }
};

#endif
