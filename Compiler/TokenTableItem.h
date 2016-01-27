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

	TokenTableItem(string name, ItemType item_type, DecorateType decorate_type, int value, int level, int defineline, int addr, int quaternary_address = 0) throw();
	void AddUsedLine(int line_number) throw();
	bool CheckItemType(const set<ItemType> legaltypes) const throw();
	ItemType GetItemType() const throw();			// 获得iter指向的符号表行的item type
	DecorateType GetDecorateType() const throw();	// 获得iter指向的符号表行的decorate type
	string toString() const throw();

// 这里感觉还是设为public为好
// 因为毕竟是符号表，会经常访问
	bool			valid_;
	string			name_;
	ItemType		itemtype_;
	DecorateType	decoratetype_;
	int				value_;
	int				level_;
	int				defineline_;
	set<int>		usedline_;
	int				addr_;
	int				quaternary_address_;

private:
	friend class TokenTable;

	static map<ItemType, string> itemTypeToString;
	static map<DecorateType, string> decorateTypeToString;
	static map<ItemType, string> InitItemTypeToStringMap() throw();
	static map<DecorateType, string> InitDecorateTypeToStringMap() throw();
};

#endif