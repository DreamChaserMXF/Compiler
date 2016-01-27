#include "TokenTableItem.h"
#include <sstream>
#include <vector>
using std::vector;

TokenTableItem::TokenTableItem(string name, ItemType item_type, DecorateType decorate_type, int value, int level, int defineline, int addr) throw()
		: valid_(true), name_(name), itemtype_(item_type), decoratetype_(decorate_type), value_(value), level_(level), defineline_(defineline), usedline_(), addr_(addr)
{ }
void TokenTableItem::AddUsedLine(int line_number) throw()
{
	usedline_.insert(line_number);
}
bool TokenTableItem::CheckItemType(const set<ItemType> legaltypes) const throw()
{
	return (legaltypes.find(itemtype_) != legaltypes.end());
}
TokenTableItem::ItemType TokenTableItem::GetItemType() const throw()
{
	return itemtype_;
}
TokenTableItem::DecorateType TokenTableItem::GetDecorateType() const throw()
{
	return decoratetype_;
}
string TokenTableItem::toString() const throw()
{
	std::ostringstream buf;
	buf << valid_ << "     ";
	buf.width(8);
	buf << std::left << name_ << "    ";
	buf << TokenTableItem::itemTypeToString[itemtype_] << "  ";
	buf << TokenTableItem::decorateTypeToString[decoratetype_] << "    ";
	buf.width(4);
	buf << std::right << value_ << "   ";
	buf.width(4);
	buf << std::left << addr_ << "  ";
	buf.width(4);
	buf << level_ << "  ";
	buf.width(4);
	buf << defineline_ << "  ";
	buf << "{";
	set<int>::const_iterator s_iter = usedline_.begin();
	if(s_iter != usedline_.end())
	{
		buf.width(2);
		buf << *s_iter;
		while(++s_iter != usedline_.end())
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
map<TokenTableItem::ItemType, string> TokenTableItem::InitItemTypeToStringMap() throw()
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
map<TokenTableItem::DecorateType, string> TokenTableItem::InitDecorateTypeToStringMap() throw()
{
	map<DecorateType, string> dsMap;
	dsMap.insert(std::pair<DecorateType, string>(VOID, "VOID   "));
	dsMap.insert(std::pair<DecorateType, string>(INTEGER, "INTEGER"));
	dsMap.insert(std::pair<DecorateType, string>(CHAR, "CHAR   "));
	return dsMap;
}