#include "TokenTableItem.h"
#include <sstream>
#include <vector>
using std::vector;

TokenTableItem::TokenTableItem(string name,
                               ItemType item_type,
                               DecorateType decorate_type,
                               int value,
                               int level,
                               int defineline,
                               int addr,
                               int quaternary_address) throw()
    : valid_(true)
    , name_(name)
    , itemtype_(item_type)
    , decoratetype_(decorate_type)
    , value_(value)
    , level_(level)
    , defineline_(defineline)
    , usedline_()
    , addr_(addr)
    , quaternary_address_(quaternary_address) {}
void TokenTableItem::AddUsedLine(int line_number) throw() { usedline_.insert(line_number); }

string TokenTableItem::toString() const throw() {
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
    if (s_iter != usedline_.end()) {
        buf.width(2);
        buf << *s_iter;
        while (++s_iter != usedline_.end()) {
            buf << ",";
            buf.width(3);
            buf << std::right << *s_iter;
        }
        buf << "}  ";
    } else {
        buf << " }  ";
    }

    return buf.str();
}

// 类型转换矩阵，行下标和列下标为两个操作数的类型，对应的数组元素为两个操作数运算后的结果类型
// 注意，这里矩阵的行列排布严格依赖于DecorateType中的定义顺序
const TokenTableItem::DecorateType TokenTableItem::TypeConversionMatrix[3][3] = {
    /*               VOID                              CHAR                          INTEGER*/
    /* VOID    */ {TokenTableItem::VOID, TokenTableItem::CHAR, TokenTableItem::INTEGER},
    /* CHAR    */ {TokenTableItem::CHAR, TokenTableItem::CHAR, TokenTableItem::INTEGER},
    /* INTEGER */ {TokenTableItem::INTEGER, TokenTableItem::INTEGER, TokenTableItem::INTEGER}};
const char* TokenTableItem::DecorateTypeString[3] = {"void", "char", "integer"};

map<TokenTableItem::ItemType, string> TokenTableItem::itemTypeToString = InitItemTypeToStringMap();
map<TokenTableItem::ItemType, string> TokenTableItem::InitItemTypeToStringMap() throw() {
    map<ItemType, string> isMap;
    isMap.insert(std::pair<ItemType, string>(CONST, "CONST    "));
    isMap.insert(std::pair<ItemType, string>(VARIABLE, "VARIABLE "));
    isMap.insert(std::pair<ItemType, string>(ARRAY, "ARRAY    "));
    isMap.insert(std::pair<ItemType, string>(PROCEDURE, "PROCEDURE"));
    isMap.insert(std::pair<ItemType, string>(FUNCTION, "FUNCTION "));
    isMap.insert(std::pair<ItemType, string>(PARAMETER, "PARAMETER"));
    return isMap;
}

map<TokenTableItem::DecorateType, string> TokenTableItem::decorateTypeToString =
    InitDecorateTypeToStringMap();
map<TokenTableItem::DecorateType, string> TokenTableItem::InitDecorateTypeToStringMap() throw() {
    map<DecorateType, string> dsMap;
    dsMap.insert(std::pair<DecorateType, string>(VOID, "VOID   "));
    dsMap.insert(std::pair<DecorateType, string>(INTEGER, "INTEGER"));
    dsMap.insert(std::pair<DecorateType, string>(CHAR, "CHAR   "));
    return dsMap;
}