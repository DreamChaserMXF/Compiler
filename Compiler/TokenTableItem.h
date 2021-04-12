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
    // һ���������������ΪINTEGER��CHAR�����̵���������ΪVOID�����泣������������ΪVOID
    // ������Ų�˳���ǰ�������ת���ϸ�˳������ģ�������߼��ж�Ҳ��������һ��
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

    // ����о�������ΪpublicΪ��
    // ��Ϊ�Ͼ��Ƿ��ű��ᾭ������
    bool valid_;
    string name_;
    ItemType itemtype_;
    DecorateType decoratetype_;
    bool isref_; // �Ƿ�Ϊ���ò���
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