//
// Created by Siebe Mees on 09/03/2023.
//

#include "DFA.h"
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <stack>
#include <queue>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

vector<pair<string, string>> findSourceStates(const pair<string, string> &statePair, const DFA& dfa) {
    vector<pair<string, string>> sourceStates;
    for (char input : dfa.getAlfabet()) {
        // Zoekt alle fromtopairen op 1 input
        vector<pair<string, string>> fromToPairOnInput;
        map<pair<string, char>, string> transitionFunction = dfa.getTransitionFunction();
        for (const auto& transition : transitionFunction) {
            if (transition.first.second == input) {
                fromToPairOnInput.push_back(make_pair(transition.first.first, transition.second));
            }
        }
        // Check if both states in statePair are present in fromToPairOnInput
        for (int i = 0; i < fromToPairOnInput.size(); ++i) {
            if (fromToPairOnInput[i].second == statePair.first) {
                for (int j = 0; j < fromToPairOnInput.size(); ++j) {
                    if (fromToPairOnInput[j].second == statePair.second) {
                        sourceStates.push_back(make_pair(fromToPairOnInput[i].first, fromToPairOnInput[j].first));
                    }
                }
            }
        }
    }


    for (int i = 0; i < sourceStates.size(); ++i) {
        if (sourceStates[i].first > sourceStates[i].second) {
            swap(sourceStates[i].first, sourceStates[i].second);
        }
    }

    return sourceStates;
}

pair<int, int> DFA::getIndexesForStatePair(pair<string, string>& statePair, const vector<string>& states) {
    int index1 = find(states.begin(), states.end(), statePair.first) - states.begin();
    int index2 = find(states.begin(), states.end(), statePair.second) - states.begin();
    return make_pair(index1, index2-1);
}

vector<vector<bool>> DFA::constructTable(DFA& dfa) {
    // Maak een vector van vectoren om de tabel op te slaan
    vector<vector<bool>> table(dfa.getStates().size()-1, vector<bool>(dfa.getStates().size()-1, false));

    // Sorteer de staten
    vector<string> sortedStates = dfa.getStates();
    sort(sortedStates.begin(), sortedStates.end());
    dfa.setStates(sortedStates);

    // 1. Aankruising accepterende staten
    // Zoek de index van de final states
    vector<int> finalStateIndexes;
    for (const auto& acceptState : dfa.getAcceptStates()) {
        auto it = find(dfa.getStates().begin(), dfa.getStates().end(), acceptState);
        if (it != dfa.getStates().end()) {
            int index = distance(dfa.getStates().begin(), it);
            finalStateIndexes.push_back(index);
        }
    }
    // Markeer alle cellen die overgangen van accepterende naar niet-accepterende staten bevatten
    for (int i = 0; i < table.size(); ++i) {
        for (int j = 0; j <= i; ++j) {
            bool row = find(finalStateIndexes.begin(), finalStateIndexes.end(), i+1) != finalStateIndexes.end();
            bool col = find(finalStateIndexes.begin(), finalStateIndexes.end(), j) != finalStateIndexes.end();
            if ((row && !col) || (!row && col)) {
                table[i][j] = true;
            }
        }
    }


    // 2. Zoek transitie paren
    bool marked = true;
    while (marked) {
        marked = false;
        for (int i = 0; i < table.size(); ++i) {
            for (int j = 0; j <= i; ++j) {
                if (table[i][j]) {
                    // Check if there is a transition from (i, j) to any state (k) on input symbol 'input'
                    vector<pair<string, string>> sourceStates = findSourceStates({dfa.getStates()[j], dfa.getStates()[i + 1]}, dfa);
                    if (!sourceStates.empty()) {
                        for (auto &sourceState: sourceStates) {
                            // Find the index of the source state
                            pair<int, int> indexesSourcePair = getIndexesForStatePair(sourceState, dfa.getStates());
                            if (!table[indexesSourcePair.second][indexesSourcePair.first]) {
                                marked = true;
                            }
                            table[indexesSourcePair.second][indexesSourcePair.first] = true;
                        }
                    }
                }
            }
        }
    }

    dfa.setTable(table);

    return table;
}

DFA::DFA() {}

DFA::DFA(const string inputFile) {
    // inlezen uit file
    ifstream input(inputFile);

    json j;
    input >> j;

    // Access the elements of the "alphabet" array
    string alfabet;
    for (const auto& letter : j["alphabet"]) {
        alfabet += letter;
    }
    setAlfabet(alfabet);

    // Access the elements of the "states" array
    vector<string> states;
    string startState;
    vector<string> acceptStates;
    for (const auto& state : j["states"]) {
        states.push_back(state["name"]);
        if (state["starting"] == true){
            startState = state["name"];
        }
        if (state["accepting"] == true){
            acceptStates.push_back(state["name"]);
        }
    }
    setStates(states);
    setAcceptStates(acceptStates);
    setStartState(startState);

    // Access the elements of the "transitions" array
    // Create a map to hold the transition function
    map<pair<string, char>, string> transitionFunction;
    // Access the elements of the "transitions" array
    for (const auto& transition : j["transitions"]) {
        // Get the "from" state
        string fromState = transition["from"];
        // Get the input character
        string input = transition["input"];
        char inputChar = input[0];
        // Get the "to" state
        string toState = transition["to"];
        // Add the transition to the map
        transitionFunction[make_pair(fromState, inputChar)] += toState;
    }
    setTransitionFunction(transitionFunction);

    setTable(constructTable(*this));
}

bool DFA::accepts(string input) {
    string currentState = startState;
    // Loop over the input
    for (char c : input) {
        // Kijkt of er een transitie bestaat op het staat, input paar
        if (transitionFunction.count({currentState, c}) == 0) {
            // Doe niets, current staat blijft hetzelfde
            currentState = currentState;
        }
        // Als er een transitiefunctie bestaat dan verander je de
        // current staat naar de nieuwe staat volgens de transitiefunctie
        currentState = transitionFunction[{currentState, c}];
    }
    // zoek of we uiteindelijk in een accept state zijn belandt
    return find(acceptStates.begin(), acceptStates.end(), currentState) != acceptStates.end();
}

void DFA::print() {
    // manueel aanmaken
    json j;
    // type
    j["type"] = "DFA";

    // alphabet
    vector<string> alphabetString;
    for (char c : alfabet) {
        alphabetString.push_back(string(1, c));
    }
    j["alphabet"] = alphabetString;

    // states
    json states_array = json::array();
    for (const auto& state : states) {
        json state_obj;
        state_obj["name"] = state;
        state_obj["starting"] = (state == startState);
        state_obj["accepting"] = (find(acceptStates.begin(), acceptStates.end(), state) != acceptStates.end());
        states_array.push_back(state_obj);
    }
    j["states"] = states_array;

    // transitions
    json transitions_array = json::array();
    for (const auto& transition : transitionFunction) {
        json transition_obj;
        transition_obj["from"] = transition.first.first;
        transition_obj["input"] = string(1, transition.first.second);
        transition_obj["to"] = transition.second;
        transitions_array.push_back(transition_obj);
    }
    j["transitions"] = transitions_array;

    // Print JSON object
    cout << setw(4) << j << endl;
}

vector<string> getDFAStatesFromString(const string& state_string) {
    vector<string> nfa_states;
    string state_without_braces = state_string.substr(1, state_string.length() - 2);
    string delimiter = ", ";
    size_t pos = 0;
    while ((pos = state_without_braces.find(delimiter)) != std::string::npos) {
        string nfa_state = state_without_braces.substr(0, pos);
        nfa_states.push_back(nfa_state);
        state_without_braces.erase(0, pos + delimiter.length());
    }
    nfa_states.push_back(state_without_braces);
    return nfa_states;
}

string getStringFromDFAStates(const vector<string>& states) {
    string state_string = "{";
    for (int i = 0; i < states.size(); ++i) {
        state_string += states[i];
        if (i != states.size() - 1) {
            state_string += ", ";
        }
    }
    state_string += "}";
    return state_string;
}



DFA DFA::minimize() {
    // Construeer de tabel
    constructTable(*this);

    DFA newDFA;

    string alfabetString;
    for (char c : alfabet) {
        alfabetString += c;
    }
    newDFA.setAlfabet(alfabetString);

    vector<vector<string>> tableStates;
    bool startStateAdded = false;
    vector<string> startStateNewDFA;
    for (int i = 0; i < table.size(); ++i) {
        for (int j = 0; j <= i; ++j) {
            // check if transition is not marked as true
            if (!table[i][j]) {
                // find source states for this transition
                vector<string> dfaStates = this->getStates();
                vector<string> statePair;
                statePair.push_back(dfaStates[j]);
                statePair.push_back(dfaStates[i+1]);
                // check if statePair is already in tableStates
                if (find(tableStates.begin(), tableStates.end(), statePair) == tableStates.end()) {
                    if (dfaStates[j] == this->getStartState() || dfaStates[i+1] == this->getStartState()) {
                        startStateNewDFA.push_back(dfaStates[j]);
                        startStateNewDFA.push_back(dfaStates[i+1]);
                        newDFA.setStartState(getStringFromDFAStates(startStateNewDFA));
                        startStateAdded = true;
                    }
                    tableStates.push_back(statePair);
                }
            }
        }
    }

    // check if tableStates has a size that is not % 2
    if (tableStates.size() % 2 != 0) {
        // combine all vectors into a single vector
        vector<string> combinedStates;
        for (int i = 0; i < tableStates.size(); ++i) {
            for (int j = 0; j < tableStates[i].size(); ++j) {
                if (find(combinedStates.begin(), combinedStates.end(), tableStates[i][j]) == combinedStates.end()){
                    combinedStates.push_back(tableStates[i][j]);
                }
            }
        }
        // clear the tableStates vector and add the combined vector as a single element
        tableStates.clear();
        tableStates.push_back(combinedStates);
    }



    // Staten
    // Stap 1: Initialiseer de verzameling verwerkte toestanden
    vector<string> processedStates;
    if (!startStateAdded) {
        processedStates.push_back("{"+this->getStartState()+"}");
    } else {
        processedStates.push_back(getStringFromDFAStates(startStateNewDFA));
    }

    // Stap 2: Initialiseer de wachtrij met alle staten van de NFA
    queue<string> stateQueue;
    if (!startStateAdded) {
        stateQueue.push("{"+this->getStartState()+"}");
        newDFA.setStartState("{"+this->getStartState()+"}");
    } else {
        stateQueue.push(getStringFromDFAStates(startStateNewDFA));
    }
    // Stap 3: Verwerk de wachtrij
    while (!stateQueue.empty()) {
        // a. Haal de volgende toestand uit de wachtrij
        vector<string> dfaStates = getDFAStatesFromString(stateQueue.front());
        stateQueue.pop();
        for (const auto& dfaState: dfaStates){
            // b. Genereer de overgangen voor elke invoer van het alfabet van de NFA
            for (char c : alfabet) {
                vector<string> dfaTransition;
                // b' Bereken alle staten die de dfaState kan bereiken
                auto it = transitionFunction.find(make_pair(dfaState, c));
                for (const auto& tableState : tableStates){
                    for (const auto& tableState2 : tableState){
                        if (it->second == tableState2) {
                            dfaTransition = tableState;
                        }
                    }
                }
                if (dfaTransition.empty()) {
                    dfaTransition.push_back(it->second);
                }
                if (!dfaTransition.empty()) {
                    newDFA.addTransition(getStringFromDFAStates(dfaStates), c, getStringFromDFAStates(dfaTransition));
                    // b'' Kijkt in de verzameling van states indien de "to" State al verwerkt is
                    if (find(processedStates.begin(), processedStates.end(), getStringFromDFAStates(dfaTransition)) == processedStates.end()) {
                        stateQueue.push(getStringFromDFAStates(dfaTransition));
                    }
                }
            }
        }
        // c. Markeer de huidige toestand als verwerkt en voeg dazn ook toe aan de staten van de DFA als deze nog niet bestaat natuurlijk
        vector<string> dfaState = newDFA.getStates();
        if (find(dfaState.begin(), dfaState.end(), getStringFromDFAStates(dfaStates)) == dfaState.end()) {
            newDFA.addState(getStringFromDFAStates(dfaStates));
        }
        processedStates.push_back(getStringFromDFAStates(dfaStates));
    }

    for (int i = 0; i < newDFA.getStates().size(); ++i) {

    }

    // Stap 4: Bepaal alle accepterende toestanden in voor de DFA
    vector<string> dfaAcceptStates;
    for (string dfaState : newDFA.getStates()) {
        vector<string> nfaStates = getDFAStatesFromString(dfaState);
        for (string nfaState : nfaStates) {
            if (find(acceptStates.begin(), acceptStates.end(), nfaState) != acceptStates.end()) {
                dfaAcceptStates.push_back(dfaState);
                break;
            }
        }
    }
    newDFA.setAcceptStates(dfaAcceptStates);


    return newDFA;

}

void DFA::printTable() {
    for (int i = 1; i < states.size(); ++i) {
        cout << states[i];
        for (int j = 0; j <= i-1; ++j) {
            cout << "\t";
            if (table[i-1][j]){
                cout << "X";
            } else {
                cout << "-";
            }
        }
        cout << endl;
    }
    for (int j = 0; j < states.size()-1; ++j) {
        cout << "\t" << states[j];
    }
    cout << endl;
}


// Setters
void DFA::setStates(const vector<string> &states) {
    DFA::states = states;
}

void DFA::addState(const std::string &state) {
    DFA::states.push_back(state);
}

void DFA::setAlfabet(const string &alfabet) {
    vector<char> chars;
    for (int i = 0; i < alfabet.size(); ++i) {
        chars.push_back(alfabet[i]);
    }
    DFA::alfabet = chars;
}

void DFA::setTransitionFunction(const map<pair<string, char>, string> &transitionFunction) {
    DFA::transitionFunction = transitionFunction;
}

void DFA::addTransition(const string &fromState, const char &input, const string &toState) {
    DFA::transitionFunction[{fromState, input}] = toState;
}

void DFA::setStartState(const string &startState) {
    DFA::startState = startState;
}

void DFA::setAcceptStates(const vector<string> &acceptStates) {
    DFA::acceptStates = acceptStates;
}

void DFA::addAcceptState(const std::string &state) {
    DFA::acceptStates.push_back(state);
}

const vector<string> &DFA::getStates() const {
    return states;
}

const vector<char> &DFA::getAlfabet() const {
    return alfabet;
}
const map<pair<string, char>, string> &DFA::getTransitionFunction() const {
    return transitionFunction;
}
const vector<string> &DFA::getAcceptStates() const {
    return acceptStates;
}

void DFA::setTable(const vector<vector<bool>> &table) {
    DFA::table = table;
}

bool DFA::isAcceptingState(const std::string &state) const {
    return find(acceptStates.begin(), acceptStates.end(), state) != acceptStates.end();
}

const string &DFA::getStartState() const {
    return startState;
}

const vector<vector<bool>> &DFA::getTable() const {
    return table;
}

bool operator==(DFA &dfa1, DFA &dfa2) {
    // Maak een table DFA aan
    // 1. Voeg de DFA's samen
    // Alfabet
    DFA tableDFA;
    vector<char> alfabetTableDFA;
    for (int i = 0; i < dfa1.getAlfabet().size(); ++i) {
        if (find(alfabetTableDFA.begin(), alfabetTableDFA.end(), dfa1.getAlfabet()[i]) == alfabetTableDFA.end()) {
            alfabetTableDFA.push_back(dfa1.getAlfabet()[i]);
        }
    }
    for (int i = 0; i < dfa2.getAlfabet().size(); ++i) {
        if (find(alfabetTableDFA.begin(), alfabetTableDFA.end(), dfa2.getAlfabet()[i]) == alfabetTableDFA.end()) {
            alfabetTableDFA.push_back(dfa2.getAlfabet()[i]);
        }
    }

    string alfabetDFAsString;
    for (int i = 0; i < alfabetTableDFA.size(); ++i) {
        alfabetDFAsString += alfabetTableDFA[i];
    }
    tableDFA.setAlfabet(alfabetDFAsString);

    // States
    tableDFA.setStartState(dfa1.getStartState());
    vector<string> statesTableDFA;
    for (int i = 0; i < dfa1.getStates().size(); ++i) {
        if (find(statesTableDFA.begin(), statesTableDFA.end(), dfa1.getStates()[i]) == statesTableDFA.end()) {
            statesTableDFA.push_back(dfa1.getStates()[i]);
        }
    }
    for (int i = 0; i < dfa2.getStates().size(); ++i) {
        if (find(statesTableDFA.begin(), statesTableDFA.end(), dfa2.getStates()[i]) == statesTableDFA.end()) {
            statesTableDFA.push_back(dfa2.getStates()[i]);
        }
    }
    tableDFA.setStates(statesTableDFA);
    vector<string> acceptingStatestableDFA;
    for (int i = 0; i < dfa1.getAcceptStates().size(); ++i) {
        if (find(acceptingStatestableDFA.begin(), acceptingStatestableDFA.end(), dfa1.getAcceptStates()[i]) == acceptingStatestableDFA.end()) {
            acceptingStatestableDFA.push_back(dfa1.getAcceptStates()[i]);
        }
    }
    for (int i = 0; i < dfa2.getAcceptStates().size(); ++i) {
        if (find(acceptingStatestableDFA.begin(), acceptingStatestableDFA.end(), dfa2.getAcceptStates()[i]) == acceptingStatestableDFA.end()) {
            acceptingStatestableDFA.push_back(dfa2.getAcceptStates()[i]);
        }
    }
    tableDFA.setAcceptStates(acceptingStatestableDFA);

    // Transitions
    auto dfa1TransitionFunction = dfa1.getTransitionFunction();
    auto dfa2TransitionFunction = dfa2.getTransitionFunction();
    dfa1TransitionFunction.insert(dfa2TransitionFunction.begin(), dfa2TransitionFunction.end());
    for (auto &transition : dfa1TransitionFunction) {
        tableDFA.addTransition(transition.first.first, transition.first.second, transition.second);
    }

    tableDFA.constructTable(tableDFA);

    tableDFA.printTable();

    pair<string, string> startStatesTableDFA  = {dfa1.getStartState(), dfa2.getStartState()};
    pair<int, int> indexesStartstates = tableDFA.getIndexesForStatePair(startStatesTableDFA, tableDFA.getStates());
    for (int i = 0; i < tableDFA.getTable().size(); ++i) {
        for (int j = 0; j <= i; ++j) {
            if (!tableDFA.getTable()[i][j] && i == indexesStartstates.second && j == indexesStartstates.first) {
                return true;
            }
        }
    }
    return false;
}