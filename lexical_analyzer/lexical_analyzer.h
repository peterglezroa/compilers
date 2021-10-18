#ifndef lexical_analyzer
#define lexical_analyzer

#include <string>
#include <iterator>
#include <algorithm>
#include <map>
#include <list>
#include <exception>
#include <regex>

//#define RULEREGEX "([A-Za-z_-]+) ?-> ?(.*)"
#define EPSILON "''"

class Production {
  public:
    std::string variable;
    std::list<std::string> elements;

    Production(std::string variable) : variable(variable) {}

    std::string toString() {
      std::string str = variable;
      str.append(" -> ");
      for (const std::string elem : elements) {
        str.append(elem);
        str.append(" ");
      }
      str.pop_back();
      return str;
    }
};

class LexicalAnalyzer {
  private:
    std::list<std::string> vars; // Syntathic variables
    std::list<std::string> terms; // Terminals
    std::list<Production> prods; // All productions

    bool has_var(std::string str) {
      /* Returns a boolean that confirms if the received string is part of the
       * synthatic variables */
      return (std::find(vars.begin(), vars.end(), str)!=vars.end());
    }

    bool has_term(std::string str) {
      /* Returns a boolean that confirms if the received string is part of the
       * terminals */
      return (std::find(terms.begin(), terms.end(), str)!=terms.end());
    }

    std::list<Production> productions(std::string var) {
      /* Returns a list of productions where the received string is the
       * variable */
      std::list<Production> list;
      for (Production prod : prods)
        if (var.compare(prod.variable) == 0)
          list.push_back(prod);

      return list;
    }

    std::list<std::string> first(std::string str) {
      std::list<std::string> firsts;

      // First rule: first of a terminal
      if (has_term(str) || str.compare(EPSILON) == 0) firsts.push_back(str);
      else if (has_var(str)) {
        for (const Production production : prods) {
          if (str.compare(production.variable) == 0 &&
              str.compare(production.elements.front()) != 0) {
            // First of the first element of the production
            std::list<std::string> ff = first(production.elements.front());

            // Find if it has epsilon
            if (std::find(ff.begin(), ff.end(), EPSILON) != ff.end() &&
                production.elements.size() > 1) {
              ff.remove(EPSILON);
              // Also add the first of the following element
              ff.splice(ff.end(), first(*std::next(production.elements.begin())));
            }

            firsts.splice(firsts.end(), ff);
          }
        }
      } else {
        fprintf(stderr, "Not part of synthatic variabels or terminals (%s)!\n", str.c_str());
        throw "Not part of synthatic variables or terminals!";
      }

      firsts.sort();
      firsts.unique(); // Clean List
      return firsts;
    }

    std::list<std::string> follow(std::string str) {
      std::list<std::string> follows;
      bool first = true;

      if (!has_var(str) && !has_term(str)) {
        fprintf(stderr, "Not part of synthatic variabels or terminals (%s)!\n", str.c_str());
        throw "Not part of synthatic variables or terminals!";
      }

      for (const Production prod : prods) {
        // First rule
        if (first && prod.variable.compare(str) == 0)
          follows.push_back("$");

        for (auto it = prod.elements.begin(); it != prod.elements.end(); it++) {
          if (it->compare(str) == 0) {
            // See next terminal
            if (it != prod.elements.end() && std::next(it) != prod.elements.end()) {
              // Second rule
              // FIRST of the following terminal
              std::list<std::string> fnext = this->first(*std::next(it));
              follows.insert(follows.end(), fnext.begin(), fnext.end());

              // Third rule
              // If next can be EPSILON, then follow the variable
              if (str.compare(prod.variable) != 0 &&
                  std::find(fnext.begin(), fnext.end(), EPSILON) != fnext.end())
                follows.splice(follows.end(), follow(prod.variable));
            } else if (str.compare(prod.variable) != 0)
              // Third rule
              follows.splice(follows.end(), follow(prod.variable));
          }
        }
        first = false;
      }

      follows.remove(EPSILON);
      follows.sort();
      follows.unique();
      return follows;
    }

    bool drifts_epsilon(std::list<std::string> elements) {
      for (const std::string elem : elements)
        if (elem.compare(EPSILON) != 0) {
          std::list<std::string> firsts = first(elem);
          if (std::find(firsts.begin(), firsts.end(), EPSILON) == firsts.end())
            return false;
        }
      return true;
    }

  public:
    LexicalAnalyzer() {}

    void clear() { vars.clear(); terms.clear(); prods.clear(); }

    bool parse(std::list<std::string> productions) {
      bool all = true;
      for (std::string production : productions)
        if (!parse(production))
          all = false;
      return all;
    }

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
      Production prod = Production(variable);

      // Remove variable from terminal list if it was inside
      terms.remove(variable);

      production.erase(0, pos+4); // Erase no terminal to only leave rule

      // Iterate through words in the production
      while ((pos = production.find(' ')) != std::string::npos) {
        terminal = production.substr(0, pos);
        if (!has_var(terminal))
          terms.push_back(terminal);
        prod.elements.push_back(terminal);
        production.erase(0, pos+1);
      }
      terms.push_back(production); // last word
      prod.elements.push_back(production);

      prods.push_back(prod);
      terms.remove(EPSILON);
      terms.sort();
      terms.unique();
      return true;
    }

    bool validString(std::string) {return false;}

    std::string toString() {
      std::string str = "";
      for (Production prod : prods) {
        str.append(prod.toString());
        str.append("\n");
      }
      str.pop_back();
      return str;
    }

    bool is_ll() {
      for (std::string var : vars) {
        std::list<Production> var_prods = productions(var);
        if (var_prods.size() >= 2) {
          for (
            auto it1 = var_prods.begin();
            it1 != var_prods.end() && std::next(it1) != var_prods.end();
            it1++
          ) {
            for (auto it2 = std::next(it1); it2 != var_prods.end(); it2++) {
              // First rule
              // FIRST(prod1) intersection FIRST(prod2) must be empty.
              std::list<std::string> first_prod1 = first(it1->elements.front());
              std::list<std::string> first_prod2 = first(it2->elements.front());
              std::list<std::string> intersection;
              first_prod1.sort();
              first_prod2.sort();
              std::set_intersection(
                first_prod1.begin(), first_prod1.end(),
                first_prod2.begin(), first_prod2.end(),
                std::back_inserter(intersection)
              );

              if (!intersection.empty())
                return false;

              // Second Rule
              // Only one drifts to EPSILON
              if (drifts_epsilon(it1->elements) && drifts_epsilon(it2->elements))
                return false;

              // Third Rule
              // FIRST(prod1) intersection FOLLOW(var) = EPSILON &&
              // FIRST(prod2) intersection FOLLOW(var) = EPSILON
              std::list<std::string> follow_var = follow(var);
              follow_var.sort();
              std::set_intersection(
                first_prod1.begin(), first_prod1.end(),
                follow_var.begin(), follow_var.end(),
                std::back_inserter(intersection)
              );
              std::set_intersection(
                first_prod2.begin(), first_prod2.end(),
                follow_var.begin(), follow_var.end(),
                std::back_inserter(intersection)
              );
              if (!intersection.empty())
                return false;
            }
          }
        }
      }
      return true;
    }


    const std::list<std::string> getVariables() const { return vars;}

    const std::list<std::string> getTerminals() const { return terms; }

    std::list<std::string> getFirst(std::string str) { return first(str); }

    std::list<std::string> getFollow(std::string str) { return follow(str); }
};

#endif
