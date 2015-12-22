#include "TokenTable.h"
#include "TokenTableException.h"
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>

TokenTable::TokenTable() : rows(), subRoutineStack(), addr(0), addrStack()
{ 
	
	subRoutineStack.push(0);// �����б�Ҫ����Ϊ��SearchDefinitionInCurrentLevel��ʱ��Ҫ�õ�topԪ��
	addrStack.push(0);		// �����ƺ�û�б�Ҫ
}


void TokenTable::Locate()
{
	subRoutineStack.push(rows.size());
	addrStack.push(addr);
	addr = 0;
}
// ɾ����ǰ�ӳ���(��������)�ڷ��ű��еļ�¼(�ض�λ)
// �Ժ����Ҫ���ģ�ʹ��Ч��Ϊ��ɾ����¼ֻ����valid�ֶ�
void TokenTable::Relocate()
{
	// �ӵ�ǰ�ֳ������ڴ���ʼ���ң�����������֮��ļ�¼ȫ��ɾ��
	// PS��һ��ʵ�ַ�������reverse_iterator������Ҫ����currentLevel����������������������ķ���
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
	// �ض�λ�ֳ�������±�ջ
	subRoutineStack.pop();
	addr = addrStack.top();
	addrStack.pop();
}

// �����ڵ�ǰ�ӳ������Ƿ���ڶ��壨���ڳ�/����������䣩
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


// ����name�Ķ��崦
TokenTable::iterator TokenTable::SearchDefinition(const Token &token)
{
	// ���ڵ�ǰ��Ѱ�Ҷ���
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
	if(iter != rows.end())	// �ҵ�����
	{
		return iter;
	}
	else					// ��֮ǰ��Ѱ�Ҷ���
	{
		reverse_iterator r_iter(rows.begin() + subRoutineStack.top());	//���������
		if(r_iter != rows.rend())			// ���֮ǰ�㲻�Ƕ���
		{
			int last_level = r_iter->level;	// ��һ���Ϊ��ǰ����ڵĹ��̻����Ĳ��
			while(r_iter != rows.rend()
				&& (   false == r_iter->valid 
					|| r_iter->level > last_level
					|| r_iter->name != token.value.identifier
					)
				)
			{
				// if�����Ŀ���ǣ�ʹ����Ч��last_levelֻ�ܲ��ϼ���
				// Ʃ��  proc1(a){}; proc2(b){proc3(){a=1}};
				// proc1�ڵ�0�㣬����a�ڵ�1��
				// proc2�ڵ�0�㣬����b�ڵ�1��
				// proc3�ڵ�1�㣬��������a�ڵ�2��
				// �ڲ���ʱ��last_level�ȸ�Ϊ1����proc3���ϲ���
				// Ȼ��proc1�Ĳ���aҲ�ڵ�1�㣬����������proc3�е�a�����غϡ�������������last_level������ܴ���bug
				// ����취�ǣ������ϼ�����proc2ʱ�����last_level����Ϊproc2��level����0
				if(true == iter->valid && r_iter->level < last_level)
				{
					last_level = r_iter->level;
				}
				++r_iter;
			}
		}
		if(r_iter != rows.rend())	// ���ҵ��˶���
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// ֮ǰ����Ƕ��㣬��û�в��ҵ�����
		{
			return rows.end();
		}
	}
}

// ����name�Ķ��崦
TokenTable::const_iterator TokenTable::SearchDefinition(const Token &token) const
{
	// ���ڵ�ǰ��Ѱ�Ҷ���
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
	if(iter != rows.end())	// �ҵ�����
	{
		return iter;
	}
	else					// ��֮ǰ��Ѱ�Ҷ���
	{
		const_reverse_iterator r_iter(rows.begin() + subRoutineStack.top());	//���������
		if(r_iter != rows.rend())			// ���֮ǰ�㲻�Ƕ���
		{
			int last_level = r_iter->level;	// ��һ���Ϊ��ǰ����ڵĹ��̻����Ĳ��
			while(r_iter != rows.rend()
				&& (   false == r_iter->valid 
					|| r_iter->level > last_level
					|| r_iter->name != token.value.identifier
					)
				)
			{
				// if�����Ŀ���ǣ�ʹ����Ч��last_levelֻ�ܲ��ϼ���
				// Ʃ��  proc1(a){}; proc2(b){proc3(){a=1}};
				// proc1�ڵ�0�㣬����a�ڵ�1��
				// proc2�ڵ�0�㣬����b�ڵ�1��
				// proc3�ڵ�1�㣬��������a�ڵ�2��
				// �ڲ���ʱ��last_level�ȸ�Ϊ1����proc3���ϲ���
				// Ȼ��proc1�Ĳ���aҲ�ڵ�1�㣬����������proc3�е�a�����غϡ�������������last_level������ܴ���bug
				// ����취�ǣ������ϼ�����proc2ʱ�����last_level����Ϊproc2��level����0
				if(true == iter->valid && r_iter->level < last_level)
				{
					last_level = r_iter->level;
				}
				++r_iter;
			}
		}
		if(r_iter != rows.rend())	// ���ҵ��˶���
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// ֮ǰ����Ƕ��㣬��û�в��ҵ�����
		{
			return rows.end();
		}
	}
}
TokenTable::const_iterator TokenTable::end() const
{
	return rows.end();
}

// ͨ������/�����ĵ����������ع���/�����Ĳ���
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
