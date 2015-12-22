#ifndef TOKENTABLEITEM_H
#define TOKENTABLEITEM_H

#include <string>
#include <set>
#include <map>
#include <vector>
using std::string;
using std::vector;
using std::set;
using std::map;

class TokenTableItem
{
public:
	enum ItemType{CONST, VARIABLE, ARRAY, PROCEDURE, FUNCTION, PARAMETER};
	enum DecorateType{VOID, INTEGER, CHAR};

	TokenTableItem(string name_, ItemType itemType_, DecorateType decorateType_, int value_, int level_, int defLine_, int addr_);
	void AddUsedLine(int lineNumber);
	bool CheckItemType(const set<ItemType> legalTypes) const;
	ItemType GetItemType() const;			// 获得iter指向的符号表行的item type
	DecorateType GetDecorateType() const;	// 获得iter指向的符号表行的decorate type
	string toString() const;

private:
	friend class TokenTable;

	static map<ItemType, string> itemTypeToString;
	static map<DecorateType, string> decorateTypeToString;
	static map<ItemType, string> InitItemTypeToStringMap();
	static map<DecorateType, string> InitDecorateTypeToStringMap();

	bool			valid;
	string			name;
	ItemType		itemType;
	DecorateType	decorateType;
	int				value;
	int				level;
	int				defLine;
	set<int>		usedLine;
	int				addr;
};

#endif