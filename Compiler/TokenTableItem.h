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

	TokenTableItem(string name, ItemType item_type, DecorateType decorate_type, int value, int level, int defineline, int addr) throw();
	void AddUsedLine(int line_number) throw();
	bool CheckItemType(const set<ItemType> legaltypes) const throw();
	ItemType GetItemType() const throw();			// 获得iter指向的符号表行的item type
	DecorateType GetDecorateType() const throw();	// 获得iter指向的符号表行的decorate type
	string toString() const throw();

private:
	friend class TokenTable;

	static map<ItemType, string> itemTypeToString;
	static map<DecorateType, string> decorateTypeToString;
	static map<ItemType, string> InitItemTypeToStringMap() throw();
	static map<DecorateType, string> InitDecorateTypeToStringMap() throw();

	bool			valid_;
	string			name_;
	ItemType		itemtype_;
	DecorateType	decoratetype_;
	int				value_;
	int				level_;
	int				defineline_;
	set<int>		usedline_;
	int				addr_;
};

#endif