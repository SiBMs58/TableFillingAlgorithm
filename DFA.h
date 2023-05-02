//
// Created by Siebe Mees on 09/03/2023.
//

#ifndef SUBSETCONSTRUCTION_DFA_H
#define SUBSETCONSTRUCTION_DFA_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;


class DFA {
private:
    vector<string> states;
    vector<char> alfabet;
    map<pair<string, char>, string> transitionFunction;
    string startState;
    vector<string> acceptStates;

    vector<vector<bool>> table;

public:
    // Default constructor
    DFA();
    // Inputfile constructor
    DFA(const string inputFile);

    bool accepts(string input);

    void print();

    DFA minimize();

    void printTable();

    pair<int, int> getIndexesForStatePair(pair<string, string>& statePair, const vector<string>& states);

    vector<vector<bool>> constructTable(DFA& dfa);

    // Setters
    void setStates(const vector<string> &states);
    void addState(const string &state);
    void setAlfabet(const string &alfabet);
    void setTransitionFunction(const map<pair<string, char>, string> &transitionFunction);
    void addTransition(const string &fromState, const char &input, const string &toState);
    void setStartState(const string &startState);
    void setAcceptStates(const vector<string> &acceptStates);
    void addAcceptState(const string &state);

    void setTable(const vector<vector<bool>> &table);

    // Getters
    const vector<string> &getStates() const;

    const vector<char> &getAlfabet() const;

    const map<pair<string, char>, string> &getTransitionFunction() const;

    const vector<string> &getAcceptStates() const;
    bool isAcceptingState(const string &state) const;

    const string &getStartState() const;

    const vector<vector<bool>> &getTable() const;

    friend bool operator==(DFA& lhs, DFA& rhs);

};


/*int main() {
    DFA dfa("../input-tfa1.json");
    DFA mindfa = dfa.minimize();
    dfa.printTable();
    mindfa.print();
    cout << boolalpha << (dfa == mindfa) << endl;    // zijn ze equivalent? Zou hier zeker moeten. Dit wordt getest in de volgende vraag, maar hiermee kan je al eens proberen
    dfa.print();
    return 0;
}*:

/*int main() {
    DFA dfa1("../input-tfaeq1.json");
    DFA dfa2("../input-tfaeq2.json");
    cout << boolalpha << (dfa1 == dfa2) << endl;
    return 0;
}*/

/*int main() {
    DFA dfa("../input-tfa2.json");
    DFA mindfa = dfa.minimize();
    dfa.printTable();

    //startRedirect();
    mindfa.print();
    //endRedirect();

    // vergelijk json onafhankelijk van volgorde
    //compareJson("labo03");
    return 0;
}*/
#endif //SUBSETCONSTRUCTION_DFA_H
