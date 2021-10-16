#ifndef lexical_analyzer
#define lexical_analyzer

#include <string>
#include <iterator>
#include <algorithm>
#include <map>
#include <list>
#include <regex>

//#define RULEREGEX "([A-Za-z_-]+) ?-> ?(.*)"

class LexicalAnalyzer {
    private:
        std::list<std::string> vars; // Syntathic variables
        std::list<std::string> terms; // Terminals

        bool has_var(std::string str) {
            return (std::find(vars.begin(), vars.end(), str)!=vars.end());
        }

        bool has_term(std::string str) {
            return (std::find(terms.begin(), terms.end(), str)!=terms.end());
        }

    public:
        LexicalAnalyzer() {}

        void clear() { vars.clear(); terms.clear(); }

        bool parse(std::string production) {
            /* Parses a given production. If the sintax is valid it returns true, if
             * not it returns false */
            std::regex productionRegex("([A-Za-z_-]+) -> (.*)");
            std::string variable, terminal, *strptr;
            char *cptr;
            size_t pos;

            if (!std::regex_match(production, productionRegex))
                return false;

            // Find the divider between variable and terminals
            pos = production.find(" -> ");

            // Get variable and add it to list if not found
            variable = production.substr(0, pos);
            if (!has_var(variable))
                vars.push_back(variable);

            // Remove variable from terminal list if it was inside
            terms.remove(variable);

            production.erase(0, pos+4); // Erase no terminal to only leave rule

            // Iterate through words in the production
            while ((pos = production.find(' ')) != std::string::npos) {
                terminal = production.substr(0, pos);
                if (!has_var(terminal) && !has_term(terminal))
                    terms.push_back(terminal);
                production.erase(0, pos+1);
            }
            terms.push_back(production); // last word

            terms.unique();
            terms.remove("''");
            return true;
        }

        bool validString(std::string) {return false;}

        const std::list<std::string> getVariables() const { return vars;}

        const std::list<std::string> getTerminals() const { return terms; }
};

#endif
