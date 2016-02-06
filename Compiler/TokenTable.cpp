#include "TokenTable.h"
#include "TokenTableException.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>

TokenTable::TokenTable() throw() : rows_(), subroutine_tokentableindex_stack_(), addr_(0), subroutine_tokentableaddr_stack_()
{ 
	
	subroutine_tokentableindex_stack_.push(0);// �����б�Ҫ����Ϊ��SearchDefinitionInCurrentLevel��ʱ��Ҫ�õ�topԪ��
	subroutine_tokentableaddr_stack_.push(0); // �����ƺ�û�б�Ҫ
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
// ɾ����ǰ�ӳ���(��������)�ڷ��ű��еļ�¼(�ض�λ)
// �Ժ����Ҫ���ģ�ʹ��Ч��Ϊ��ɾ����¼ֻ����valid�ֶ�
void TokenTable::Relocate() throw()
{
	// �ӵ�ǰ�ֳ������ڴ���ʼ���ң�����������֮��ļ�¼ȫ��ɾ��
	// PS��һ��ʵ�ַ�������reverse_iterator������Ҫ����currentLevel����������������������ķ���
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
	// �ض�λ�ֳ�������±�ջ
	subroutine_tokentableindex_stack_.pop();
	addr_ = subroutine_tokentableaddr_stack_.top();
	subroutine_tokentableaddr_stack_.pop();
}

// �����ڵ�ǰ�ӳ������Ƿ���ڶ��壨���ڳ�/����������䣩
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


// ����token�Ķ��崦
// ��������Ϊconst�����ͷ�const�������صĵ��������Ͳ�ͬ
TokenTable::iterator TokenTable::SearchDefinition(const Token &token) throw()
{
	// ���ڵ�ǰ��Ѱ�Ҷ���
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
	if(iter != rows_.end())	// �ҵ�����
	{
		return iter;
	}
	else					// ��֮ǰ��Ѱ�Ҷ���
	{
		reverse_iterator r_iter(rows_.begin() + subroutine_tokentableindex_stack_.top());	//���������
		if(r_iter != rows_.rend())			// ���֮ǰ�㲻�Ƕ���
		{
			int last_level = r_iter->level_;	// ��һ���Ϊ��ǰ����ڵĹ��̻����Ĳ��
			while(r_iter != rows_.rend()
				&& (   false == r_iter->valid_ 
					|| r_iter->level_ > last_level
					|| r_iter->name_ != token.value_.identifier
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
				if(true == r_iter->valid_ && r_iter->level_ < last_level)	// bug fixed[if(true == iter->valid_ && r_iter->level_ < last_level)]
				{
					last_level = r_iter->level_;
				}
				++r_iter;
			}
		}
		if(r_iter != rows_.rend())	// ���ҵ��˶���
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// ֮ǰ����Ƕ��㣬��û�в��ҵ�����
		{
			return rows_.end();
		}
	}
}

// ����name�Ķ��崦
// ��������Ϊconst�����ͷ�const�������صĵ��������Ͳ�ͬ
TokenTable::const_iterator TokenTable::SearchDefinition(const Token &token) const throw()
{
	// ���ڵ�ǰ��Ѱ�Ҷ���
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
	if(iter != rows_.end())	// �ҵ�����
	{
		return iter;
	}
	else					// ��֮ǰ��Ѱ�Ҷ���
	{
		const_reverse_iterator r_iter(rows_.begin() + subroutine_tokentableindex_stack_.top());	//���������
		if(r_iter != rows_.rend())			// ���֮ǰ�㲻�Ƕ���
		{
			int last_level = r_iter->level_;	// ��һ���Ϊ��ǰ����ڵĹ��̻����Ĳ��
			while(r_iter != rows_.rend()
				&& (   false == r_iter->valid_ 
					|| r_iter->level_ > last_level
					|| r_iter->name_ != token.value_.identifier
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
				if(true == iter->valid_ && r_iter->level_ < last_level)
				{
					last_level = r_iter->level_;
				}
				++r_iter;
			}
		}
		if(r_iter != rows_.rend())	// ���ҵ��˶���
		{
			iter = r_iter.base() - 1;
			return iter;
		}
		else	// ֮ǰ����Ƕ��㣬��û�в��ҵ�����
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

// ͨ������/�����ĵ����������ع���/�����Ĳ���������
// �������Ե���Ч��ֻ��decoratetype_��isref_
// iterָ����ű��еĹ���/������
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
	// ���ű�������һ�����飬addr_Ҫ��������ĳ�����ʾ����Ĵ���
	addr_ += arrayLength;
}
int TokenTable::AddProcedureItem(Token procedureIdentifier, int level) throw()
{
	// ���ڹ���������ջ�в�ռ�ռ䣬��addr_���ֲ���
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
// ���ù���/�����Ĳ�������
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

// ���ع���/�����ľֲ�������ռ�Ŀռ䣨��λ��4bytes��
// c_iterΪ����/�����ڷ��ű��е�λ�õ���һ��λ��
// ֮������ôҪ������Ϊ�������ڷ��ű���û��λ�ã�ֻ���ṩ��һ��λ��
int TokenTable::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
{
	// ����while������ǰ����/�����Ĳ���������
	while( rows_.end() != c_iter
		&& (TokenTableItem::PARAMETER == c_iter->itemtype_
			|| TokenTableItem::CONST == c_iter->itemtype_)
		)
	{
		++c_iter;
	}
	// ����ù���/����û�оֲ�����
	if(rows_.end() == c_iter
		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
	{
		return 0;
	}
	// ���ڿ϶��б�����
	// ȡ�õ�һ�������ĵ�ַ
	int first_var_addr = c_iter->addr_;
	// ��while������ǰ����/�����ľֲ�����
	while( rows_.end() != c_iter
		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
	{
		++c_iter;
	}
	// �ҳ����һ���������õ����ַ���ټ���ֲ������ռ䣨ĩ������ַ-�ױ�����ַ+ĩ�������ȣ�
	const TokenTableItem &item = ((rows_.end() == c_iter) ? rows_.back() : *(c_iter - 1));
	// ���һ�������������������ͨ���������������ֲ�ͬ�Ŀռ���㷽ʽ
	return item.addr_ - first_var_addr + ((TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1);
}

// �������ű���ĳ��������λ�ã�ȷ�������ڵĺ����Ĳ����ĸ���
int TokenTable::GetParameterNum(int var_index) const throw()
{
	do
	{
		--var_index;
	}while(var_index >= 0
		&& TokenTableItem::PROCEDURE != rows_[var_index].itemtype_
		&& TokenTableItem::FUNCTION != rows_[var_index].itemtype_);
	// �ڷ��ű������Ҳ��������������Ƶģ���ʱֱ�ӷ���0
	if(-1 == var_index)
	{
		return 0;
	}
	else	// �������
	{
		return rows_[var_index].value_;
	}
}