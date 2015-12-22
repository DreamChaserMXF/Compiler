#include "TokenTable.h"
#include "TokenTableException.h"
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>

TokenTable::TokenTable() : rows(), subRoutineStack(), addr(0), addrStack()
{ 
	
	subRoutineStack.push(0);// 这行有必要，因为在SearchDefinitionInCurrentLevel的时候要用到top元素
	addrStack.push(0);		// 这行似乎没有必要
}


void TokenTable::Locate()
{
	subRoutineStack.push(rows.size());
	addrStack.push(addr);
	addr = 0;
}
// 删除当前子程序(除参数外)在符号表中的记录(重定位)
// 以后可能要更改，使其效果为不删除记录只更改valid字段
void TokenTable::Relocate()
{
	// 从当前分程序的入口处开始查找，将参数声明之后的记录全部删除
	// PS另一种实现方法是用reverse_iterator，但需要传递currentLevel作参数，故用正向迭代器的方法
	iterator iter = rows.begin() + subRoutineStack.top();
	while(iter != rows.end() && TokenTableItem::PARAMETER == iter->itemType)
	{
		++iter;
	}
	while(iter != rows.end())
	{
		iter->valid = false;
		++iter;
	}
	// 重定位分程序入口下标栈
	subRoutineStack.pop();
	addr = addrStack.top();
	addrStack.pop();
}

// 查找在当前子程序中是否存在定义（用于常/变量定义语句）
bool TokenTable::SearchDefinitionInCurrentLevel(const string &name)
{
	iterator iter = rows.begin() + subRoutineStack.top();
	if(iter != rows.end())
	{
		int current_level = iter->level;
		while(iter != rows.end() && (false == iter->valid || iter->level != current_level || iter->name != name))
		{
			++iter;
		}
	}
	return iter != rows.end();
}


// 查找name的定义处
TokenTable::iterator TokenTable::SearchDefinition(const Token &token)
{
	// 先在当前层寻找定义
	iterator iter = rows.begin() + subRoutineStack.top();
	if(iter != rows.end())
	{
		int current_level = iter->level;
		while(iter != rows.end() 
			&& (   false == iter->valid 
				|| iter->level != current_level
				|| iter->name != token.value.identifier
				)
			)
		{
			++iter;
		}
	}
	if(iter != rows.end())	// 找到定义
	{
		return iter;
	}
	else					// 在之前层寻找定义
	{
		reverse_iterator r_iter(rows.begin() + subRoutineStack.top());	//逆向迭代器
		if(r_iter != rows.rend())			// 如果之前层不是顶层
		{
			int last_level = r_iter->level;	// 上一层次为当前层入口的过程或函数的层次
			while(r_iter != rows.rend()
				&& (   false == r_iter->valid 
					|| r_iter->level > last_level
					|| r_iter->name != token.value.identifier
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
				if(true == iter->valid && r_iter->level < last_level)
				{
					last_level = r_iter->level;
				}
				++r_iter;
			}
		}
		if(r_iter != rows.rend())	// 查找到了定义
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// 之前层就是顶层，或没有查找到定义
		{
			return rows.end();
		}
	}
}

// 查找name的定义处
TokenTable::const_iterator TokenTable::SearchDefinition(const Token &token) const
{
	// 先在当前层寻找定义
	const_iterator iter = rows.begin() + subRoutineStack.top();
	if(iter != rows.end())
	{
		int current_level = iter->level;
		while(iter != rows.end() 
			&& (   false == iter->valid 
				|| iter->level != current_level
				|| iter->name != token.value.identifier
				)
			)
		{
			++iter;
		}
	}
	if(iter != rows.end())	// 找到定义
	{
		return iter;
	}
	else					// 在之前层寻找定义
	{
		const_reverse_iterator r_iter(rows.begin() + subRoutineStack.top());	//逆向迭代器
		if(r_iter != rows.rend())			// 如果之前层不是顶层
		{
			int last_level = r_iter->level;	// 上一层次为当前层入口的过程或函数的层次
			while(r_iter != rows.rend()
				&& (   false == r_iter->valid 
					|| r_iter->level > last_level
					|| r_iter->name != token.value.identifier
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
				if(true == iter->valid && r_iter->level < last_level)
				{
					last_level = r_iter->level;
				}
				++r_iter;
			}
		}
		if(r_iter != rows.rend())	// 查找到了定义
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// 之前层就是顶层，或没有查找到定义
		{
			return rows.end();
		}
	}
}
TokenTable::const_iterator TokenTable::end() const
{
	return rows.end();
}

// 通过过程/函数的迭代器，返回过程/函数的参数
vector<TokenTableItem::DecorateType> TokenTable::GetProcFuncParameter(const_iterator iter)
{
	assert(iter != rows.end());
	vector<TokenTableItem::DecorateType> decorate_types;
	++iter;
	while(iter != rows.end() && TokenTableItem::PARAMETER == iter->itemType)
	{
		decorate_types.push_back(iter->decorateType);
		++iter;
	}
	return decorate_types;
}

string TokenTable::toString() const
{
	std::ostringstream buf;
	buf << "Valid Name        ItemType DecorateType Value Addr Level DefLine UsedLine\n";
	for(const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		buf << iter->toString() << '\n';
	}
	return buf.str();
}
void TokenTable::Print(const string &fileName) const
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
void TokenTable::Print(std::ostream &output) const
{
	output << toString();
}

void TokenTable::AddConstItem(Token constIdentifier, TokenTableItem::DecorateType decorateType, int value, int level)
{
	TokenTableItem item(constIdentifier.value.identifier, TokenTableItem::CONST, decorateType, value, level, constIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
void TokenTable::AddVariableItem(Token variableIdentifier, TokenTableItem::DecorateType decorateType, int level)
{
	TokenTableItem item(variableIdentifier.value.identifier, TokenTableItem::VARIABLE, decorateType, 0, level, variableIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
void TokenTable::AddArrayItem(Token arrayIdentifier, TokenTableItem::DecorateType decorateType, int arrayLength, int level)
{
	TokenTableItem item(arrayIdentifier.value.identifier, TokenTableItem::ARRAY, decorateType, arrayLength, level, arrayIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
void TokenTable::AddProcedureItem(Token procedureIdentifier, int level)
{
	TokenTableItem item(procedureIdentifier.value.identifier, TokenTableItem::PROCEDURE, TokenTableItem::VOID, 0, level, procedureIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
void TokenTable::AddFunctionItem(Token functionIdentifier, int level)
{
	TokenTableItem item(functionIdentifier.value.identifier, TokenTableItem::FUNCTION, TokenTableItem::VOID, 0, level, functionIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
void TokenTable::SetParameterCount(const string &proc_func_name, int parameterCount)
{
	reverse_iterator iter = rows.rbegin();
	while(iter->name != proc_func_name)
	{
		++iter;
	}
	iter->value = parameterCount;
}
void TokenTable::SetFunctionReturnType(const string &func_name, TokenTableItem::DecorateType decorateType)
{
	reverse_iterator iter = rows.rbegin();
	while(iter->name != func_name)
	{
		++iter;
	}
	iter->decorateType = decorateType;
}


void TokenTable::AddParameterItem(Token parameterIdentifier, TokenTableItem::DecorateType decorateType, int level)
{
	TokenTableItem item(parameterIdentifier.value.identifier, TokenTableItem::PARAMETER, decorateType, 0, level, parameterIdentifier.lineNumber, addr++);
	rows.push_back(item);
}
