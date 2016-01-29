#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TokenTable.h"
#include "Quaternary.h"

using std::ostream;
using std::ofstream;
using std::vector;
using std::ios;

void PrintOperand(Quaternary::OperandType type, int value, const TokenTable &tokentable, ostream &out) throw();
void PrintQuaternaryVector(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, ostream &out) throw();
bool PrintQuaternaryVector(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const string &filename) throw();

void PrintStringVector(const vector<string> &stringtable, ostream &out) throw();
bool PrintStringVector(const vector<string> &stringtable, const string &filename) throw();