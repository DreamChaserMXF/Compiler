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
	// һ���������������ΪINTEGER��CHAR�����̵���������ΪVOID�����泣������������ΪVOID
	// ������Ų�˳���ǰ�������ת���ϸ�˳������ģ�������߼��ж�Ҳ��������һ��
	// VOID����ת��ΪCHAR��INTEGER
	// CHAR����ת��ΪINTEGER��������ת��ΪVOID
	// INTEGER����ת��ΪCHAR��VOID
	enum DecorateType{VOID = 0, CHAR = 1, INTEGER = 2};	

	TokenTableItem(string name, ItemType item_type, DecorateType decorate_type, int value, int level, int defineline, int addr, int quaternary_address = 0) throw();
	void AddUsedLine(int line_number) throw();
//	ItemType GetItemType() const throw();			// ���iterָ��ķ��ű��е�item type
//	DecorateType GetDecorateType() const throw();	// ���iterָ��ķ��ű��е�decorate type
	string toString() const throw();

// ����о�������ΪpublicΪ��
// ��Ϊ�Ͼ��Ƿ��ű��ᾭ������
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