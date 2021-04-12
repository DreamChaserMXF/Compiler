#ifndef EXPRESSIONATTRIBUTE_H
#define EXPRESSIONATTRIBUTE_H

#include "Quaternary.h"

// 用来表示一个表达式的属性
// operandtype_表示该表达式最终存储为立即数、常量、符号表变量、还是临时变量
// value_表示operandtype_下对应的值
// 当operandtype_为ARRAY_OPERAND的时候，offset表示数组元素的偏移量（即下标），offset_operandtype_表示偏移量的取址方式
// decoratetype表示该表达式的修饰类型
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