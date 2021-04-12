#ifndef TOKENTABLEITEM_H
#define TOKENTABLEITEM_H

#include <string>
#include <set>
#include <map>
#include <vector>
using std::map;
using std::set;
using std::string;
using std::vector;

class TokenTableItem {
public:
    enum ItemType { CONST, VARIABLE, ARRAY, PROCEDURE, FUNCTION, PARAMETER };
    // 一般变量的修饰类型为INTEGER或CHAR，过程的修饰类型为VOID，字面常量的修饰类型为VOID
    // 这里的排布顺序是按照类型转换严格顺序递增的，程序的逻辑判断也依赖了这一点
    enum DecorateType { VOID = 0, CHAR = 1, INTEGER = 2 };

    TokenTableItem(string name,
                   ItemType item_type,
                   DecorateType decorate_type,
                   bool isref,
                   int value,
                   int level,
                   int addr) throw();
    //void AddUsedLine(int line_number) throw();
    string toString() const throw();

    // 这里感觉还是设为public为好
    // 因为毕竟是符号表，会经常访问
    bool valid_;
    string name_;
    ItemType itemtype_;
    DecorateType decoratetype_;
    bool isref_; // 是否为引用参数
    int value_;
    int level_;
    //int				defineline_;
    //set<int>		usedline_;
    int addr_;


    static const DecorateType TypeConversionMatrix[3][3];
    static const char* DecorateTypeString[3];

private:
    friend class TokenTable;

    static map<ItemType, string> itemTypeToString;
    static map<DecorateType, string> decorateTypeToString;
    static map<ItemType, string> InitItemTypeToStringMap() throw();
    static map<DecorateType, string> InitDecorateTypeToStringMap() throw();
};

#endif