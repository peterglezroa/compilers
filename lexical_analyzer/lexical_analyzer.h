#ifndef lexical_analyzer
#define lexical_analyzer

#include <algorithm>
#include <exception>
#include <iterator>
#include <list>
#include <map>
#include <regex>
#include <string>

#define RULEREGEX "([A-Za-z_-]+) ?-> ?(.*)"
#define EPSILON "''"

char LABUFFER[255];

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
    int firVer; // Integer used for version control and optimize updates
    std::list<std::string> first;
    int folVer; // Integer used for version control and optimize updates
    std::list<std::string> follow;
    std::map<std::string, Production*> table;

    friend class LexicalAnalyzer;

    /* Function to update the current version of the first list */
    void updateFirst(std::list<std::string> first_, int version) {
      first.clear();
      first = first_;
      firVer = version;
    }

    /* Function to update the current version of the follow list */
    void updateFollow(std::list<std::string> follow_, int version) {
      follow.clear();
      follow = follow_;
      folVer = version;
    }

  public:
    Variable(std::string name_) : name(name_) { firVer=0; folVer=0; }

    bool operator == (const Variable &v) { return name.compare(v.name) == 0; }

    bool operator != (const Variable &v) { return name.compare(v.name) != 0; }

    bool operator == (const std::string &str) { return str.compare(name) ==0; }

    bool operator != (const std::string &str) { return str.compare(name) !=0; }

    /* Returns if the variable can get to an specific terminal. */
    bool hasTerm(std::string term) {
      return std::find(first.begin(), first.end(), term) != first.end();
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

      str.append("}, MAP={");
      if(!table.empty()) {
        std::map<std::string, Production*>::iterator it;
        for (it = table.begin(); it != table.end(); it++) {
          sprintf(LABUFFER, "'%s': %s, ",
            it->first.c_str(), it->second->toString().c_str());
          str.append(LABUFFER);
        }
        str.pop_back(); str.pop_back();
      }
      str.append("}");

      return str;
    }

    std::list<std::string> getFirst() const { return first; }

    std::list<std::string> getFollow() const { return follow; }
}; 

class LexicalAnalyzer {
  private:
    int ver; // Version control
    bool isLL;
    std::list<Variable> vars; // Syntathic variables
    std::list<std::string> terms; // Terminals
    std::list<Production> prods; // All productions

    std::list<std::string> cache;
    FILE *logFile;
    bool logging;

    /* Function to log the activity of the lexical analyzer */
    void log(const char str[]) {
      if (logFile != NULL) fprintf(logFile, "%s", str);
    }

    /* Returns a boolean that confirms if the received string is part of the
     * cache to avoid recursion */
    bool inCache(std::string str) {
      return std::find(cache.begin(), cache.end(), str) != cache.end();
    }

    /* Logs the cache */
    void logCache() {
      if (logging) {
        log("CACHE: ");
        for(std::string e : cache) {
          sprintf(LABUFFER, "%s ", e.c_str());
          log(LABUFFER);
        }
        log("\n");
      }
    }

    /* Returns a boolean that confirms if the received string is part of the
     * synthatic variables */
    bool hasVar(std::string &str) {
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
    bool hasTerm(std::string &str) {
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

    /* Runs a calculation for returning first of a specific string.
     * Returns the list of firsts for an specific string. */
    std::list<std::string> calcFirst(std::string str) {
      std::list<std::string> firsts;
      Variable *v;

      // First rule: first of a terminal
      if (hasTerm(str) || str.compare(EPSILON) == 0) firsts.push_back(str);
      else if ( (v=getVar(str)) != NULL) {
        // Ignores if the version is updated
        if (v->firVer == ver) return v->first;

        // Version is not updated thus needs to update
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

        // Clean list
        firsts.sort();
        firsts.unique();

        // Update var version
        v->updateFirst(firsts, ver);
      } else {
        fprintf(stderr, "Not part of synthatic variabels or terminals (%s)!\n",
          str.c_str());
        throw std::runtime_error("Not part of variables or terminals!");
      }

      return firsts;
    }

    /* Runs a calculation for returning follow of a specific string.
     * Returns the list of follows. */
    std::list<std::string> calcFollow(std::string str) {
      std::list<std::string> follows;
      bool first = true;
      Variable *v = getVar(str);

      if (v == NULL) {
        fprintf(stderr, "Not part of synthatic variables (%s)!\n",
          str.c_str());
        throw std::runtime_error("Not part of variables!");
      }

      // Ignores if the version is updated
      if (v->folVer == ver) return v->follow;

      if (logging) {
        sprintf(LABUFFER, "calcFollow(%s) ", str.c_str()); log(LABUFFER);
        logCache();
      }

      // Follow needs to be updated
      for (const Production prod : prods) {
        // First rule
        if (first && prod.variable.compare(str) == 0) follows.push_back("$");

        for (auto it = prod.elements.begin(); it != prod.elements.end(); it++) {
          if (it->compare(str) == 0) {
            // See next terminal
            if (it!=prod.elements.end() && std::next(it)!=prod.elements.end()) {
              // Second rule
              // FIRST of the following terminal
              std::list<std::string> fnext = this->calcFirst(*std::next(it));
              follows.insert(follows.end(), fnext.begin(), fnext.end());

              // Third rule
              // If next can be EPSILON, then follow the variable
              if (str.compare(prod.variable) != 0 &&
                  std::find(fnext.begin(),fnext.end(),EPSILON) != fnext.end()){
                cache.push_back(str);
                follows.splice(follows.end(), calcFollow(prod.variable));
              }
            } else if (str.compare(prod.variable) != 0 && !inCache(str)) {
              // Third rule: Its the last one
              cache.push_back(str);
              follows.splice(follows.end(), calcFollow(prod.variable));
            }
          }
        }
        first = false;
      }

      cache.remove(str);

      follows.remove(EPSILON);
      follows.sort();
      follows.unique();

      // Update var version
      v->updateFollow(follows, ver);

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

    /* Runs a calculation return the row of the LL table for an specific var. */
    std::map<std::string, Production*> calcTable(const std::string &variable) {
      std::map<std::string, Production*> map;
      std::string front;
      std::list<Production> varProds;
      std::list<Production>::iterator prodIt;
      Variable *var = getVar(variable), *oVar;

      if (!isLL) throw std::runtime_error("Is not LL!");
      else if(var == NULL) throw std::runtime_error("Could not find variable!");
      else {
        for(const std::string term : var->first) {
          for (prodIt = prods.begin(); prodIt != prods.end(); prodIt++) {
            if ((*var) == prodIt->variable) {
              front = prodIt->elements.front();
              if (term.compare(front) == 0 || // Same as Terminal
              (*var != front && (oVar=getVar(front)) && oVar->hasTerm(term)) ){
                // Other variable that has term in first
                map.insert(std::pair<std::string, Production*>(
                  term, &(*prodIt)));
              }
            }
          }
        }
      }
      return map;
    }

    /* Test if the string is valid. */
    bool testStr(std::string str) {
      std::string term, top;
      std::stack<std::string> stack;
      std::list<std::string> pels;
      std::list<std::string>::reverse_iterator rit;
      Variable *v;
      Production* prod;
      size_t pos;

      sprintf(LABUFFER, "\nTesting string '%s'", str.c_str()); log(LABUFFER);

      str.append(" $ ");

      if (!prods.empty()) {
        stack.push("$");
        stack.push(prods.front().variable);
        while ((pos = str.find(' ')) != std::string::npos) {
          term = str.substr(0, pos);

          if (stack.empty()) break;
          top = stack.top();

          if (logging) {
            sprintf(LABUFFER, "\n%s\t|\t%s", top.c_str(), str.c_str());
            log(LABUFFER);
          }

          if (top.compare("$") == 0) break;

          // Same term
          if (top.compare(term) == 0) {
            str.erase(0, pos+1);
            stack.pop();
            if(logging) {
              sprintf(LABUFFER, "\t|\t%s", top.c_str());
              log(LABUFFER);
            }
          // No terminal that can get to term
          } else if( (v=getVar(top)) && v->hasTerm(term)) {
            if(logging) {
              sprintf(LABUFFER, "\t|\t%s", v->table[term]->toString().c_str());
              log(LABUFFER);
            }
            stack.pop();

            pels = v->table[term]->elements;
            for(rit=pels.rbegin(); rit!=pels.rend(); rit++) stack.push(*rit);
          // No terminal that can be epsilon
          } else if(v && v->hasTerm(EPSILON)) {
            if (logging) log("\t|\tEPSILON");
            stack.pop();
          } else break;
        }
        if (logging) log("\n");
        if(str.compare("$ ") == 0 && !stack.empty() &&
            stack.top().compare("$") == 0) return true;
      }
      log("ERROR\n");
      return false;
    }

    /* Updates the Variables and if it is LL so we do not need to calculate 
     * so many first, follow, is_ll, and LLTable */
    void update() {
      ver++;
      sprintf(LABUFFER, "\nUpdating to version %i...\n", ver); log(LABUFFER);
      isLL = calcIsLL();
      (isLL)? log("It's LL\n") : log("It is not LL\n");
      for (auto it = vars.begin(); it != vars.end(); it++) {
        if(it->firVer != ver) it->updateFirst(calcFirst(it->name), ver);
        if(it->folVer != ver) it->updateFollow(calcFollow(it->name), ver);
        if(isLL) it->table = calcTable(it->name);
        sprintf(LABUFFER, "%s\n", it->toString().c_str()); log(LABUFFER);
      }
    }

  public:
    LexicalAnalyzer() { isLL = false; ver = 1; logFile = NULL; logging = false; }

    LexicalAnalyzer(FILE *logFile_): logFile(logFile_) {
      isLL=false; ver=0; logging = true; }

    void clear() {
      log("\nClearing lexical analyzer...\n");
      log("==============================================================\n\n");
      vars.clear();
      terms.clear();
      prods.clear();
    }

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

      sprintf(LABUFFER, "Parsing %s\n", production.c_str()); log(LABUFFER);

      // Find the divider between variable and terminals
      pos = production.find(" -> ");

      // Get variable and add it to list if not found
      variable = production.substr(0, pos);
      if (!hasVar(variable)) vars.push_back(Variable(variable));
      Production prod = Production(variable);

      // Remove variable from terminal list if it was inside
      terms.remove(variable);

      // Erase no terminal to only leave rule
      production.erase(0, pos+4);

      // Iterate through words in the production
      while ((pos = production.find(' ')) != std::string::npos) {
        terminal = production.substr(0, pos);
        if(!hasVar(terminal)) terms.push_back(terminal);
        prod.elements.push_back(terminal);
        production.erase(0, pos+1);
      }
      if(!hasVar(production)) terms.push_back(production); // last word
      prod.elements.push_back(production);

      prods.push_back(prod);
      terms.remove(EPSILON);
      terms.sort();
      terms.unique();

      // Log vars and terms
      if(logging) {
        log("V { ");
        for (Variable var : vars) {
          sprintf(LABUFFER, "%s ", var.name.c_str()); log(LABUFFER);
        }
        log("} T { ");
        for (std::string term : terms) {
          sprintf(LABUFFER, "%s ", term.c_str()); log(LABUFFER);
        }
        log("}\n");
      }

      if(runUpdate) update();
      return true;
    }

    bool validStr(const std::string &str) { return testStr(str); }

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

    std::string getProd(const std::string &v, const std::string &t) {
      Variable *var = getVar(v);
      if (var && var->hasTerm(t))
        return var->table[t]->toString();
      return "";
    }

};

#endif
