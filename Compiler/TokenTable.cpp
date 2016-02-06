#include "TokenTable.h"
#include "TokenTableException.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>

TokenTable::TokenTable() throw() : rows_(), subroutine_tokentableindex_stack_(), addr_(0), subroutine_tokentableaddr_stack_()
{ 
	
	subroutine_tokentableindex_stack_.push(0);// 这行有必要，因为在SearchDefinitionInCurrentLevel的时候要用到top元素
	subroutine_tokentableaddr_stack_.push(0); // 这行似乎没有必要
}

TokenTableItem TokenTable::at(int index) const throw(std::out_of_range)
{
	return rows_.at(index);
}

void TokenTable::Locate() throw()
{
	subroutine_tokentableindex_stack_.push(rows_.size());
	subroutine_tokentableaddr_stack_.push(addr_);
	addr_ = 0;
}
// 删除当前子程序(除参数外)在符号表中的记录(重定位)
// 以后可能要更改，使其效果为不删除记录只更改valid字段
void TokenTable::Relocate() throw()
{
	// 从当前分程序的入口处开始查找，将参数声明之后的记录全部删除
	// PS另一种实现方法是用reverse_iterator，但需要传递currentLevel作参数，故用正向迭代器的方法
	iterator iter = rows_.begin() + subroutine_tokentableindex_stack_.top();
	while(iter != rows_.end() && TokenTableItem::PARAMETER == iter->itemtype_)
	{
		++iter;
	}
	while(iter != rows_.end())
	{
		iter->valid_ = false;
		++iter;
	}
	// 重定位分程序入口下标栈
	subroutine_tokentableindex_stack_.pop();
	addr_ = subroutine_tokentableaddr_stack_.top();
	subroutine_tokentableaddr_stack_.pop();
}

// 查找在当前子程序中是否存在定义（用于常/变量定义语句）
bool TokenTable::SearchDefinitionInCurrentLevel(const string &name) throw()
{
	iterator iter = rows_.begin() + subroutine_tokentableindex_stack_.top();
	if(iter != rows_.end())
	{
		int current_level = iter->level_;
		while(iter != rows_.end() && (false == iter->valid_ || iter->level_ != current_level || iter->name_ != name))
		{
			++iter;
		}
	}
	return iter != rows_.end();
}


// 查找token的定义处
// 重载是因为const函数和非const函数返回的迭代器类型不同
TokenTable::iterator TokenTable::SearchDefinition(const Token &token) throw()
{
	// 先在当前层寻找定义
	iterator iter = rows_.begin() + subroutine_tokentableindex_stack_.top();
	if(iter != rows_.end())
	{
		int current_level = iter->level_;
		while(iter != rows_.end() 
			&& (   false == iter->valid_ 
				|| iter->level_ != current_level
				|| iter->name_ != token.value_.identifier
				)
			)
		{
			++iter;
		}
	}
	if(iter != rows_.end())	// 找到定义
	{
		return iter;
	}
	else					// 在之前层寻找定义
	{
		reverse_iterator r_iter(rows_.begin() + subroutine_tokentableindex_stack_.top());	//逆向迭代器
		if(r_iter != rows_.rend())			// 如果之前层不是顶层
		{
			int last_level = r_iter->level_;	// 上一层次为当前层入口的过程或函数的层次
			while(r_iter != rows_.rend()
				&& (   false == r_iter->valid_ 
					|| r_iter->level_ > last_level
					|| r_iter->name_ != token.value_.identifier
					)
				)
			{
				// if语句块的目的是，使得有效的last_level只能不断减少
				// 譬如  proc1(a){}; proc2(b){proc3(){a=1}};
				// proc1在第0层，参数a在第1层
				// proc2在第0层，参数b在第1层
				// proc3在第1层，过程体内a在第2层
				// 在查找时，last_level先赋为1，从proc3向上查找
				// 然而proc1的参数a也在第1层，但作用域与proc3中的a并不重合。导致若不更新last_level，则可能存在bug
				// 解决办法是，当向上检索到proc2时，便把last_level更新为proc2的level，即0
				if(true == r_iter->valid_ && r_iter->level_ < last_level)	// bug fixed[if(true == iter->valid_ && r_iter->level_ < last_level)]
				{
					last_level = r_iter->level_;
				}
				++r_iter;
			}
		}
		if(r_iter != rows_.rend())	// 查找到了定义
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// 之前层就是顶层，或没有查找到定义
		{
			return rows_.end();
		}
	}
}

// 查找name的定义处
// 重载是因为const函数和非const函数返回的迭代器类型不同
TokenTable::const_iterator TokenTable::SearchDefinition(const Token &token) const throw()
{
	// 先在当前层寻找定义
	const_iterator iter = rows_.begin() + subroutine_tokentableindex_stack_.top();
	if(iter != rows_.end())
	{
		int current_level = iter->level_;
		while(iter != rows_.end() 
			&& (   false == iter->valid_ 
				|| iter->level_ != current_level
				|| iter->name_ != token.value_.identifier
				)
			)
		{
			++iter;
		}
	}
	if(iter != rows_.end())	// 找到定义
	{
		return iter;
	}
	else					// 在之前层寻找定义
	{
		const_reverse_iterator r_iter(rows_.begin() + subroutine_tokentableindex_stack_.top());	//逆向迭代器
		if(r_iter != rows_.rend())			// 如果之前层不是顶层
		{
			int last_level = r_iter->level_;	// 上一层次为当前层入口的过程或函数的层次
			while(r_iter != rows_.rend()
				&& (   false == r_iter->valid_ 
					|| r_iter->level_ > last_level
					|| r_iter->name_ != token.value_.identifier
					)
				)
			{
				// if语句块的目的是，使得有效的last_level只能不断减少
				// 譬如  proc1(a){}; proc2(b){proc3(){a=1}};
				// proc1在第0层，参数a在第1层
				// proc2在第0层，参数b在第1层
				// proc3在第1层，过程体内a在第2层
				// 在查找时，last_level先赋为1，从proc3向上查找
				// 然而proc1的参数a也在第1层，但作用域与proc3中的a并不重合。导致若不更新last_level，则可能存在bug
				// 解决办法是，当向上检索到proc2时，便把last_level更新为proc2的level，即0
				if(true == iter->valid_ && r_iter->level_ < last_level)
				{
					last_level = r_iter->level_;
				}
				++r_iter;
			}
		}
		if(r_iter != rows_.rend())	// 查找到了定义
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// 之前层就是顶层，或没有查找到定义
		{
			return rows_.end();
		}
	}
}
TokenTable::const_iterator TokenTable::begin() const throw()
{
	return rows_.begin();
}
TokenTable::const_iterator TokenTable::end() const throw()
{
	return rows_.end();
}
//const TokenTableItem& TokenTable::back() const throw()
//{
//	return rows_.back();
//}
size_t TokenTable::size() const throw()
{
	return rows_.size();
}

// 通过过程/函数的迭代器，返回过程/函数的参数的属性
// 其中属性的有效项只有decoratetype_和isref_
// iter指向符号表中的过程/函数项
vector<ExpressionAttribute> TokenTable::GetProcFuncParameterAttributes(const_iterator iter) throw()
{
	assert(iter != rows_.end());
	vector<ExpressionAttribute> attributes;
	ExpressionAttribute cur_attr;
	++iter;
	while(iter != rows_.end() && TokenTableItem::PARAMETER == iter->itemtype_)
	{
		cur_attr.decoratetype_ = iter->decoratetype_;
		if(iter->isref_)
		{
			cur_attr.addressingmethod_ = Quaternary::REFERENCE_ADDRESSING;
		}
		else
		{
			cur_attr.addressingmethod_ = Quaternary::NIL_ADDRESSING;
		}
		attributes.push_back(cur_attr);
		++iter;
	}
	return attributes;
}

string TokenTable::toString() const throw()
{
	std::ostringstream buf;
	buf << "NO.\tValid Name        ItemType DecorateType Isref Value Addr Level DefLine UsedLine\n";
	for(const_iterator iter = rows_.begin(); iter != rows_.end(); ++iter)
	{
		buf << distance(rows_.begin(), iter) << '\t' << iter->toString() << '\n';
	}
	return buf.str();
}
void TokenTable::Print(const string &fileName) const throw()
{
	std::ofstream outFile(fileName);
	if(!outFile)
	{
		std::cout << "Cannot open file " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}
	Print(outFile);
	outFile.close();
}
void TokenTable::Print(std::ostream &output) const throw()
{
	output << toString();
}

void TokenTable::AddConstItem(Token constIdentifier, TokenTableItem::DecorateType decoratetype_, int value, int level) throw()
{
	TokenTableItem item(constIdentifier.value_.identifier, TokenTableItem::CONST, decoratetype_, false, value, level, constIdentifier.lineNumber_, 0);
	rows_.push_back(item);
}
void TokenTable::AddVariableItem(Token variableIdentifier, TokenTableItem::DecorateType decoratetype_, int level) throw()
{
	TokenTableItem item(variableIdentifier.value_.identifier, TokenTableItem::VARIABLE, decoratetype_, false, 0, level, variableIdentifier.lineNumber_, addr_++);
	rows_.push_back(item);
}
void TokenTable::AddArrayItem(Token arrayIdentifier, TokenTableItem::DecorateType decoratetype_, int arrayLength, int level) throw()
{
	TokenTableItem item(arrayIdentifier.value_.identifier, TokenTableItem::ARRAY, decoratetype_, false, arrayLength, level, arrayIdentifier.lineNumber_, addr_);
	rows_.push_back(item);
	// 符号表中增加一个数组，addr_要加上数组的长度以示数组的存在
	addr_ += arrayLength;
}
int TokenTable::AddProcedureItem(Token procedureIdentifier, int level) throw()
{
	// 由于过程在运行栈中不占空间，故addr_保持不变
	TokenTableItem item(procedureIdentifier.value_.identifier, TokenTableItem::PROCEDURE, TokenTableItem::VOID, false, 0, level, procedureIdentifier.lineNumber_, addr_);
	rows_.push_back(item);
	return rows_.size() - 1;
}
int TokenTable::AddFunctionItem(Token functionIdentifier, int level) throw()
{
	TokenTableItem item(functionIdentifier.value_.identifier, TokenTableItem::FUNCTION, TokenTableItem::VOID, false, 0, level, functionIdentifier.lineNumber_, addr_++);
	rows_.push_back(item);
	return rows_.size() - 1;
}
// 设置过程/函数的参数个数
void TokenTable::SetParameterCount(const string &proc_func_name, int parameterCount) throw()
{
	reverse_iterator iter = rows_.rbegin();
	while(iter->name_ != proc_func_name)
	{
		++iter;
	}
	assert(iter != rows_.rend());
	iter->value_ = parameterCount;
}
void TokenTable::SetFunctionReturnType(const string &func_name, TokenTableItem::DecorateType decoratetype_) throw()
{
	reverse_iterator iter = rows_.rbegin();
	while(iter->name_ != func_name)
	{
		++iter;
	}
	assert(iter != rows_.rend());
	iter->decoratetype_ = decoratetype_;
}

void TokenTable::AddParameterItem(Token parameterIdentifier, TokenTableItem::DecorateType decoratetype_, bool isref, int level) throw()
{
	TokenTableItem item(parameterIdentifier.value_.identifier, TokenTableItem::PARAMETER, decoratetype_, isref, 0, level, parameterIdentifier.lineNumber_, addr_++);
	rows_.push_back(item);
}

// 返回过程/函数的局部变量所占的空间（单位：4bytes）
// c_iter为过程/函数在符号表中的位置的下一个位置
// 之所以这么要求，是因为主函数在符号表中没有位置，只能提供下一个位置
int TokenTable::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
{
	// 先用while跳过当前过程/函数的参数及常量
	while( rows_.end() != c_iter
		&& (TokenTableItem::PARAMETER == c_iter->itemtype_
			|| TokenTableItem::CONST == c_iter->itemtype_)
		)
	{
		++c_iter;
	}
	// 如果该过程/函数没有局部变量
	if(rows_.end() == c_iter
		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
	{
		return 0;
	}
	// 现在肯定有变量了
	// 取得第一个变量的地址
	int first_var_addr = c_iter->addr_;
	// 用while跳过当前过程/函数的局部变量
	while( rows_.end() != c_iter
		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
	{
		++c_iter;
	}
	// 找出最后一个变量，得到其地址，再计算局部变量空间（末变量地址-首变量地址+末变量长度）
	const TokenTableItem &item = ((rows_.end() == c_iter) ? rows_.back() : *(c_iter - 1));
	// 最后一个变量可能是数组或普通变量，所以有两种不同的空间计算方式
	return item.addr_ - first_var_addr + ((TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1);
}

// 给定符号表中某个变量的位置，确定其所在的函数的参数的个数
int TokenTable::GetParameterNum(int var_index) const throw()
{
	do
	{
		--var_index;
	}while(var_index >= 0
		&& TokenTableItem::PROCEDURE != rows_[var_index].itemtype_
		&& TokenTableItem::FUNCTION != rows_[var_index].itemtype_);
	// 在符号表中是找不到主函数的名称的，此时直接返回0
	if(-1 == var_index)
	{
		return 0;
	}
	else	// 正常情况
	{
		return rows_[var_index].value_;
	}
}