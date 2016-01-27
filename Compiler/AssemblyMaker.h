#ifndef ASSEMBLYMAKER_H
#define ASSEMBLYMAKER_H

#include "TokenTable.h"
#include "Quaternary.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class AssemblyMaker
{
public:
	AssemblyMaker(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const vector<string> &stringtable) throw();
	void Print(std::ostream &out) const throw();
	bool Print(const string &filename) const throw();

	bool Assemble() throw();
private:
	AssemblyMaker(const AssemblyMaker &) throw();

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif