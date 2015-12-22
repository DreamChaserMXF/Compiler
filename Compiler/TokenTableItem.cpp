#include "TokenTableItem.h"
#include <sstream>
#include <vector>
using std::vector;

TokenTableItem::TokenTableItem(string name_, ItemType itemType_, DecorateType decorateType_, int value_, int level_, int defLine_, int addr_)
		: valid(true), name(name_), itemType(itemType_), decorateType(decorateType_), value(value_), level(level_), defLine(defLine_), usedLine(), addr(addr_)
{ }
void TokenTableItem::AddUsedLine(int lineNumber)
{
	usedLine.insert(lineNumber);
}
bool TokenTableItem::CheckItemType(const set<ItemType> legalTypes) const
{
	return (legalTypes.find(itemType) != legalTypes.end());
}
TokenTableItem::ItemType TokenTableItem::GetItemType() const
{
	return itemType;
}
TokenTableItem::DecorateType TokenTableItem::GetDecorateType() const
{
	return decorateType;
}
string TokenTableItem::toString() const
{
	std::ostringstream buf;
	buf << valid << "     ";
	buf.width(8);
	buf << std::left << name << "    ";
	buf << TokenTableItem::itemTypeToString[itemType] << "  ";
	buf << TokenTableItem::decorateTypeToString[decorateType] << "    ";
	buf.width(4);
	buf << std::right << value << "   ";
	buf.width(4);
	buf << std::left << addr << "  ";
	buf.width(4);
	buf << level << "  ";
	buf.width(4);
	buf << defLine << "  ";
	buf << "{";
	set<int>::const_iterator s_iter = usedLine.begin();
	if(s_iter != usedLine.end())
	{
		buf.width(2);
		buf << *s_iter;
		while(++s_iter != usedLine.end())
		{
			buf << ",";
			buf.width(3);
			buf << std::right << *s_iter;
		}
		buf << "}  ";
	}
	else
	{
		buf << " }  ";
	}
	
	return buf.str();
}


map<TokenTableItem::ItemType, string> TokenTableItem::itemTypeToString = InitItemTypeToStringMap();
map<TokenTableItem::ItemType, string> TokenTableItem::InitItemTypeToStringMap()
{
	map<ItemType, string> isMap;
	isMap.insert(std::pair<ItemType, string>(CONST, "CONST    "));
	isMap.insert(std::pair<ItemType, string>(VARIABLE, "VARIABLE "));
	isMap.insert(std::pair<ItemType, string>(ARRAY, "ARRAY    "));
	isMap.insert(std::pair<ItemType, string>(PROCEDURE, "PROCEDURE"));
	isMap.insert(std::pair<ItemType, string>(FUNCTION, "FUNCTION "));
	isMap.insert(std::pair<ItemType, string>(PARAMETER, "PARAMETER"));
	return isMap;
}

map<TokenTableItem::DecorateType, string> TokenTableItem::decorateTypeToString = InitDecorateTypeToStringMap();
map<TokenTableItem::DecorateType, string> TokenTableItem::InitDecorateTypeToStringMap()
{
	map<DecorateType, string> dsMap;
	dsMap.insert(std::pair<DecorateType, string>(VOID, "VOID   "));
	dsMap.insert(std::pair<DecorateType, string>(INTEGER, "INTEGER"));
	dsMap.insert(std::pair<DecorateType, string>(CHAR, "CHAR   "));
	return dsMap;
}