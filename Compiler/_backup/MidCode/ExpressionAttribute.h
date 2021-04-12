#ifndef EXPRESSIONATTRIBUTE_H
#define EXPRESSIONATTRIBUTE_H

#include "Quaternary.h"

// ������ʾһ�����ʽ������
// operandtype_��ʾ�ñ��ʽ���մ洢Ϊ�����������������ű������������ʱ����
// value_��ʾoperandtype_�¶�Ӧ��ֵ
// ��operandtype_ΪARRAY_OPERAND��ʱ��offset��ʾ����Ԫ�ص�ƫ���������±꣩��offset_operandtype_��ʾƫ������ȡַ��ʽ
// decoratetype��ʾ�ñ��ʽ����������
class ExpressionAttribute {
public:
    ExpressionAttribute() throw()
        : operandtype_(Quaternary::NIL_OPERAND)
        , value_(0)
        , offset_operandtype_(Quaternary::NIL_OPERAND)
        , offset_(0)
        , decoratetype_(TokenTableItem::VOID) {}
    Quaternary::OperandType operandtype_;
    int value_;
    Quaternary::OperandType offset_operandtype_;
    int offset_;
    TokenTableItem::DecorateType decoratetype_;
};

#endif