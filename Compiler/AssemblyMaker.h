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

	void Head() throw();
	void StackSegment() throw();
	void DataSegment() throw();
	void CodeBeginSegment() throw();
	void MainFunction() throw();
	void OtherFunction(TokenTable::const_iterator c_iter) throw();
	void EndStatement() throw();
	void TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter) const throw();

	int GetVariableSpace(TokenTable::const_iterator c_iter) const throw();
	vector<Quaternary>::const_iterator GetProcFuncIterInQuaternaryTable(TokenTable::const_iterator c_iter) const throw();
	vector<Quaternary>::const_iterator GetFunctionBody(vector<Quaternary>::const_iterator begin_iter) const throw();

	const vector<Quaternary> &quaternarytable_;
	const TokenTable &tokentable_;
	const vector<string> &stringtable_;
	std::ostringstream assemble_buffer;
};

#endif