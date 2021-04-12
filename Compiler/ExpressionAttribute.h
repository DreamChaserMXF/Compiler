#ifndef EXPRESSIONATTRIBUTE_H
#define EXPRESSIONATTRIBUTE_H

#include "Quaternary.h"

// ������ʾһ�����ʽ������
// addressingmethod_��ʾ�ñ��ʽ���մ洢Ϊ�����������������ű������������ʱ����
// value_��ʾaddressingmethod_�¶�Ӧ��ֵ
// ��addressingmethod_ΪARRAY_ADDRESSING��ʱ��offset��ʾ����Ԫ�ص�ƫ���������±꣩��offset_addressingmethod_��ʾƫ������ȡַ��ʽ
// decoratetype��ʾ�ñ��ʽ����������
class ExpressionAttribute {
public:
    ExpressionAttribute() throw()
        : addressingmethod_(Quaternary::NIL_ADDRESSING)
        , value_(0)
        , offset_addressingmethod_(Quaternary::NIL_ADDRESSING)
        , offset_(0)
        , decoratetype_(TokenTableItem::VOID) {}
    Quaternary::AddressingMethod addressingmethod_;
    int value_;
    Quaternary::AddressingMethod offset_addressingmethod_;
    int offset_;
    TokenTableItem::DecorateType decoratetype_;
    //bool isref_;
};

#endif