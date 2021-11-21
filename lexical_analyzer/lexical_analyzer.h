#ifndef lexical_analyzer
#define lexical_analyzer

#include <algorithm>
#include <exception>
#include <iterator>
#include <list>
#include <map>
#include <regex>
#include <string>
#include <vector>

#define RULEREGEX "([A-Za-z_-]+) ?-> ?(.*)"
#define EPSILON "''"

class Production {
  private:
    std::string variable;
    std::list<std::string> elements;

    friend class LexicalAnalyzer;

  public:
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

    std::string getVariable() const { return variable; }

    std::list<std::string> getElements() const { return elements; }
};

class Variable {
  private:
    std::string name;
    std::list<std::string> first;
    std::list<std::string> follow;
    std::map<std::string, Production*> table;

    friend class LexicalAnalyzer;

  public:
    Variable(std::string name_) : name(name_) {}

    bool operator == (const Variable &v) { return name.compare(v.name) == 0; }

    bool operator != (const Variable &v) { return name.compare(v.name) != 0; }

    bool operator == (const std::string &str) { return str.compare(name) ==0; }

    bool operator != (const std::string &str) { return str.compare(name) !=0; }

    /* Returns if the variable can get to an specific terminal. */
    bool has_term(std::string term) {
      return (table.find(term) != table.end());
    }

    /* Returns a list of the elements of the production that gets to the term */
    std::list<std::string> term_prod(std::string term) const {
      auto it = table.find(term);

      if(it == table.end())
        throw std::runtime_error("Variable can not get to terminal!");

      return it->second->getElements();
    }

    std::string toString() {
      std::string str = name;
      str.append(": FIRST={");
      if(!first.empty()) {
        for (const std::string f : first) { str.append(f); str.append(","); }
        str.pop_back(); // Pop last ','
      }
      str.append("} FOLLOW={");
      if(!follow.empty()) {
        for (const std::string f : follow) { str.append(f); str.append(","); }
        str.pop_back(); // Pop last ','
      }
      str.append("}");

      return str;
    }

    std::list<std::string> getFirst() const { return first; }

    std::list<std::string> getFollow() const { return follow; }
}; 

class LexicalAnalyzer {
  private:
    bool isLL;
    std::list<Variable> vars; // Syntathic variables
    std::list<std::string> terms; // Terminals
    std::list<Production> prods; // All productions

    /* Returns a boolean that confirms if the received string is part of the
     * synthatic variables */
    bool has_var(std::string &str) {
      return std::find(vars.begin(), vars.end(), str) != vars.end();
    }

    /* Returns the pointer to the Variable instance according to a string. If
     * the variable is not found, then it returns NULL */
    Variable * getVar(const std::string str) {
      std::list<Variable>::iterator it = std::find(vars.begin(),vars.end(),str);
      if(it == vars.end()) return NULL;
      return &(*it);
    }

    /* Returns a boolean that confirms if the received string is part of the
     * terminals */
    bool has_term(std::string &str) {
      return std::find(terms.begin(), terms.end(), str) != terms.end();
    }

    /* Returns a list of productions where the received string is the
     * variable */
    std::list<Production> productions(std::string &var) {
      std::list<Production> list;
      for (Production prod : prods)
        if(var == prod.variable) list.push_back(prod);
      return list;
    }

    /* Runs a calculation for returning first of a specific string. It ignores
     * if the firsts are already on memory. Returns the list of firsts. */
    std::list<std::string> calcFirst(std::string str) {
      std::list<std::string> firsts;

      // First rule: first of a terminal
      if (has_term(str) || str.compare(EPSILON) == 0) firsts.push_back(str);
      else if (has_var(str)) {
        for (const Production production : prods) {
          if (str.compare(production.variable) == 0 &&
              str.compare(production.elements.front()) != 0) {
            // First of the first element of the production
            std::list<std::string> ff = calcFirst(production.elements.front());

            // Find if it has epsilon
            if (std::find(ff.begin(), ff.end(), EPSILON) != ff.end() &&
                production.elements.size() > 1) {
              ff.remove(EPSILON);
              // Also add the first of the following element
              ff.splice(ff.end(),
                calcFirst(*std::next(production.elements.begin())));
            }

            firsts.splice(firsts.end(), ff);
          }
        }
      } else {
        fprintf(stderr, "Not part of synthatic variabels or terminals (%s)!\n",
          str.c_str());
        throw std::runtime_error("Not part of variables or terminals!");
      }

      firsts.sort();
      firsts.unique(); // Clean List
      return firsts;
    }

    /* Runs a calculation for returning follow of a specific string. It ignores
     * if the follows are already on memory. Returns the list of follows. */
    std::list<std::string> calcFollow(std::string str) {
      std::list<std::string> follows;
      bool first = true;

      if (!has_var(str) && !has_term(str)) {
        fprintf(stderr, "Not part of synthatic variabels or terminals (%s)!\n",
          str.c_str());
        throw std::runtime_error("Not part of variables or terminals!");
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
              std::list<std::string> fnext = this->calcFirst(*std::next(it));
              follows.insert(follows.end(), fnext.begin(), fnext.end());

              // Third rule
              // If next can be EPSILON, then follow the variable
              if (str.compare(prod.variable) != 0 &&
                  std::find(fnext.begin(), fnext.end(), EPSILON) != fnext.end())
                follows.splice(follows.end(), calcFollow(prod.variable));
            } else if (str.compare(prod.variable) != 0)
              // Third rule
              follows.splice(follows.end(), calcFollow(prod.variable));
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
          std::list<std::string> firsts = calcFirst(elem);
          if (std::find(firsts.begin(), firsts.end(), EPSILON) == firsts.end())
            return false;
        }
      return true;
    }

    /* Runs a calculation to see if this is LL. Ignores if it was already
     * calculated. */
    bool calcIsLL() {
      for (Variable var : vars) {
        std::list<Production> var_prods = productions(var.name);
        if (var_prods.size() >= 2) {
          for (
            auto it1 = var_prods.begin();
            it1 != var_prods.end() && std::next(it1) != var_prods.end();
            it1++
          ) {
            for (auto it2 = std::next(it1); it2 != var_prods.end(); it2++) {
              // First rule
              // FIRST(prod1) intersection FIRST(prod2) must be empty.
              std::list<std::string> firstP1 = getFirst(it1->elements.front());
              std::list<std::string> firstP2 = getFirst(it2->elements.front());
              std::list<std::string> intersection;
              firstP1.sort();
              firstP2.sort();
              std::set_intersection(
                firstP1.begin(), firstP1.end(),
                firstP2.begin(), firstP2.end(),
                std::back_inserter(intersection)
              );

              if (!intersection.empty())
                return false;

              // Second Rule
              // Only one drifts to EPSILON
              if (drifts_epsilon(it1->elements)&&drifts_epsilon(it2->elements))
                return false;

              // Third Rule
              // FIRST(prod1) intersection FOLLOW(var) = EPSILON &&
              // FIRST(prod2) intersection FOLLOW(var) = EPSILON
              var.follow.sort();
              std::set_intersection(
                firstP1.begin(), firstP1.end(),
                var.follow.begin(), var.follow.end(),
                std::back_inserter(intersection)
              );
              std::set_intersection(
                firstP2.begin(), firstP2.end(),
                var.follow.begin(), var.follow.end(),
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

    /* Updates the Variables and if it is LL so we do not need to calculate 
     * so many first, follow, is_ll, and LLTable */
    void update() {
      for (auto it = vars.begin(); it != vars.end(); it++) {
        it->first = calcFirst(it->name);
        it->follow = calcFollow(it->name);
//        var.table = calcTable();
      }
      isLL = calcIsLL();
    }

  public:
    LexicalAnalyzer() { isLL = false; }

    void clear() { vars.clear(); terms.clear(); prods.clear(); }

    /* Parses a given list of productions. If the sintax is valid it returns
     * true, else it returns false.
     * THIS IS MORE EFFICIENT THAN CALLING PARSE FOR EACH PRODUCTION */
    bool parse(std::list<std::string> productions, bool runUpdate = true) {
      bool all = true;
      for (std::string production : productions)
        if (!parse(production, false))
          all = false;
      if(runUpdate) update();
      return all;
    }

    /* Parses a given production. If the sintax is valid it returns true, if
     * not it returns false */
    bool parse(std::string production, bool runUpdate = true) {
      std::regex productionRegex(RULEREGEX);
      std::string variable, terminal, *strptr;
      char *cptr;
      size_t pos;

      // Does not match regex
      if (!std::regex_match(production, productionRegex)) return false;

      // Find the divider between variable and terminals
      pos = production.find(" -> ");

      // Get variable and add it to list if not found
      variable = production.substr(0, pos);
      if (!has_var(variable))
        vars.push_back(Variable(variable));
      Production prod = Production(variable);

      // Remove variable from terminal list if it was inside
      terms.remove(variable);

      // Erase no terminal to only leave rule
      production.erase(0, pos+4);

      // Iterate through words in the production
      while ((pos = production.find(' ')) != std::string::npos) {
        terminal = production.substr(0, pos);
        if(!has_var(terminal)) terms.push_back(terminal);
        prod.elements.push_back(terminal);
        production.erase(0, pos+1);
      }
      terms.push_back(production); // last word
      prod.elements.push_back(production);

      prods.push_back(prod);
      terms.remove(EPSILON);
      terms.sort();
      terms.unique();

      if(runUpdate) update();
      return true;
    }

    bool validString(std::string) {return false;}

    bool is_ll() { return isLL; }

    std::string toString() {
      std::string str = "";
      for (Production prod : prods) {
        str.append(prod.toString());
        str.append("\n");
      }
      str.pop_back();
      return str;
    }

    const std::list<std::string> getVariables() {
      std::list<std::string> variables;
      for(Variable var : vars) variables.push_back(var.name);
      return variables;
    }

    const std::list<std::string> getTerminals() const { return terms; }

    std::list<std::string> getFirst(const std::string &str) {
      Variable *var = getVar(str);
      if (var) return var->getFirst();
      return calcFirst(str);
    }

    std::list<std::string> getFollow(const std::string &str) {
      Variable *var = getVar(str);
      if (var) return var->getFollow();
      return calcFollow(str);
    }
};

#endif
