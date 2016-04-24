#ifndef EXPRESSIONATTRIBUTE_H
#define EXPRESSIONATTRIBUTE_H

#include "Quaternary.h"

// 用来表示一个表达式的属性
// addressingmethod_表示该表达式最终存储为立即数、常量、符号表变量、还是临时变量
// value_表示addressingmethod_下对应的值
// 当addressingmethod_为ARRAY_ADDRESSING的时候，offset表示数组元素的偏移量（即下标），offset_addressingmethod_表示偏移量的取址方式
// decoratetype表示该表达式的修饰类型
class ExpressionAttribute
{
public:
	ExpressionAttribute() throw() : 
	  addressingmethod_(Quaternary::NIL_ADDRESSING), value_(0), 
		  offset_addressingmethod_(Quaternary::NIL_ADDRESSING), offset_(0), 
		  decoratetype_(TokenTableItem::VOID)
	{}
	Quaternary::AddressingMethod addressingmethod_;
	int value_;
	Quaternary::AddressingMethod offset_addressingmethod_;
	int offset_;
	TokenTableItem::DecorateType decoratetype_;
	//bool isref_;
};

#endif