#include "AssemblyMaker.h"
#include <iostream>
#include <fstream>
using std::cout;
using std::endl;

AssemblyMaker::AssemblyMaker(const vector<Quaternary> &quaternarytable, const TokenTable &tokentable, const vector<string> &stringtable) throw()
	: quaternarytable_(quaternarytable), tokentable_(tokentable), stringtable_(stringtable), assemble_buffer()
{
}

void AssemblyMaker::Print(std::ostream &out) const throw()
{
	out << assemble_buffer.str();
}
bool AssemblyMaker::Print(const string &filename) const throw()
{
	std::ofstream out(filename);
	if(!out.is_open())
	{
		cout << "Cannot open file " << filename << endl;
		return false;
	}
	Print(out);
	return true;
}

bool AssemblyMaker::Assemble() throw()
{
	assemble_buffer.str("");
	Head();
	StackSegment();
	DataSegment();
	CodeBeginSegment();
	MainFunction();
	// TODO traverse the tokentable
	for(TokenTable::const_iterator c_iter = tokentable_.begin();
		c_iter != tokentable_.end(); ++c_iter)
	{
		if(TokenTableItem::PROCEDURE == c_iter->itemtype_)
		{
			OtherProcedure(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
		}
		else if(TokenTableItem::FUNCTION == c_iter->itemtype_)
		{
			OtherFunction(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
		}
	}
	return true;
}

void AssemblyMaker::Head() throw()
{
	// ����
	assemble_buffer << "TITLE MXF_AssemblyCode\n";
	
	// ָ��ģʽ��32λ����Ѱַģʽ��ƽ̹Ѱַ������Сдģʽ�����У�
	assemble_buffer << "\n.386";
	assemble_buffer << "\n.model flat, stdcall";
	assemble_buffer << "\noption casemap: none\n";
	// �����Ŀ�
	assemble_buffer << "\nincludelib .\\masm32\\lib\\msvcrt.lib";
	assemble_buffer << "\nincludelib .\\masm32\\lib\\kernel32.lib";
	assemble_buffer << "\nincludelib .\\masm32\\include\\msvcrt.inc";
	assemble_buffer << "\nincludelib .\\masm32\\include\\kernel32.inc";
	// ���������������
	assemble_buffer << "\nprintf PROTO C: ptr sbyte, :vararg";
	assemble_buffer << "\nscanf  PROTO C: ptr sbyte, :vararg";
	assemble_buffer << endl;
}
void AssemblyMaker::StackSegment() throw()
{
	assemble_buffer << "\n.STACK" << endl;
}
void AssemblyMaker::DataSegment() throw()
{
	assemble_buffer << "\n.DATA";
	assemble_buffer << "\n    _integer_format    db '%d ', 0";
	assemble_buffer << "\n    _char_format       db '%c ', 0";
	assemble_buffer << "\n    _str_format        db '%s ', 0";
	assemble_buffer << endl;
}
void AssemblyMaker::CodeBeginSegment() throw()
{
	assemble_buffer << "\n.CODE" << endl;
}
void AssemblyMaker::MainFunction() throw()
{
	// ����������
	assemble_buffer << "\n.start:\n";				// ��ʼλ��
	assemble_buffer << "\nmain:  proc far" << endl;	// ��ת��ʽ
	// ������ͷ
	// һ. �ҳ��������б����Ĵ洢�ռ�
	int var_space = GetVariableSpace(tokentable_.begin());;

	//TokenTable::const_iterator c_tokentable_iter = tokentable_.begin();
	//while( tokentable_.end() != c_tokentable_iter
	//	|| TokenTableItem::PROCEDURE == c_tokentable_iter->itemtype_
	//	|| TokenTableItem::FUNCTION == c_tokentable_iter->itemtype_)
	//{
	//	++c_tokentable_iter;
	//}
	//// ��һ������/������addr��Ϊ�����Ĵ洢�ռ䣨��λ��4bytes��
	//int var_space = 0;
	//if(tokentable_.end() == c_tokentable_iter)	// ��ʾ��������ֻ��һ��������
	//{
	//	// ����б����Ļ����ҳ����һ���������ټ����ַ
	//	// ���û�б�����var_space����Ϊ0
	//	if(tokentable_.size() != 0)	
	//	{
	//		const TokenTableItem &item = tokentable_.back();
	//		int base_addr = item.addr_;	// ���һ�������ĵ�ַ
	//		// ���һ������������ļ��㷽ʽ
	//		if(TokenTableItem::ARRAY == item.itemtype_)
	//		{
	//			var_space = base_addr + item.value_;
	//		}
	//		else	// ��ͨ�����ļ��㷽ʽ
	//		{
	//			var_space = base_addr + 1;
	//		}
	//	}
	//}
	//else
	//{
	//	var_space = c_tokentable_iter->addr_;
	//}

	// TODO ��. ��������ջ

	// TODO ��. Ϊ������ջ�Ϸ���ռ�

	// ��. ����Ԫʽ�����ҵ��������Ŀ�ʼ��ַ������Ԫʽ���л��
	vector<Quaternary>::const_reverse_iterator rc_quaternary_iter = quaternarytable_.rbegin();
	while(rc_quaternary_iter != quaternarytable_.rend() 
		&& Quaternary::END != rc_quaternary_iter->type1_)
	{
		++rc_quaternary_iter;
	}
	// ��c_iterָ�����Ԫʽֱ��������������������������Ԫʽ
	for(vector<Quaternary>::const_iterator c_quaternary_iter = rc_quaternary_iter.base();
		c_quaternary_iter != quaternarytable_.end(); ++c_quaternary_iter)
	{
		// TODO ת�������
	}
}

// ��ͨ�����Ļ�����
// var_space�Ǹú����ľֲ������Ŀռ�
// func_begin����Ԫʽ���иú�����BEGIN���ĵ�����
void AssemblyMaker::OtherFunction(int var_space, vector<Quaternary>::const_iterator func_begin) throw()
{
	
}

// ��ͨ���̵Ļ�����
// var_space�Ǹù��̵ľֲ������Ŀռ�
// func_begin����Ԫʽ���иù��̵�BEGIN���ĵ�����
void AssemblyMaker::OtherProcedure(int var_space, vector<Quaternary>::const_iterator proc_begin) throw()
{

}

// ���ع���/�����ľֲ�������ռ�Ŀռ䣨��λ��4bytes��
// c_iterΪ����/�����ڷ��ű��е�λ�õ���һ��λ��
// ����֮������ôҪ������Ϊ�������ڷ��ű���û��λ�ã�ֻ���ṩ��һ��λ��
int AssemblyMaker::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
{
	// ����ù���/����û�оֲ�����
	if(tokentable_.end() == c_iter
		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
	{
		return 0;
	}
	// ��while������ǰ����/�����ľֲ�����
	while( tokentable_.end() != c_iter
		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
	{
		++c_iter;
	}
	// �ҳ����һ���������õ����ַ���ټ���ֲ������ռ䣨��ַ+ĩ�����ռ䣩
	const TokenTableItem &item = (tokentable_.end() == c_iter) ? tokentable_.back() : *(c_iter - 1);
	// ���һ�������������������ͨ���������������ֲ�ͬ�Ŀռ���㷽ʽ
	return item.addr_ + (TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1;
}

// ͨ��ָ����ű�Ĺ���/�����ĵ��������ҵ�������Ԫʽ���ж�Ӧ��BEGIN���ĵ�����
vector<Quaternary>::const_iterator AssemblyMaker::GetProcFuncIterInQuaternaryTable(TokenTable::const_iterator c_iter) const throw()
{
	for(vector<Quaternary>::const_iterator iter = quaternarytable_.begin();
		iter != quaternarytable_.end(); ++iter)
	{
		// ��Ԫʽ��BEGIN����У�dst_��ֵΪ����/�����ڷ��ű��е��±�
		if(Quaternary::BEGIN == iter->op_
			&& iter->dst_ == distance(tokentable_.begin(), c_iter))
		{
			return iter;
		}
	}
	return quaternarytable_.end();
}