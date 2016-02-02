#include "AssemblyMaker.h"
#include "Print.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <assert.h>
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
	// traverse the tokentable
	for(TokenTable::const_iterator c_iter = tokentable_.begin();
		c_iter != tokentable_.end(); ++c_iter)
	{
		if(TokenTableItem::PROCEDURE == c_iter->itemtype_
			|| TokenTableItem::FUNCTION == c_iter->itemtype_)
		{
			OtherFunction(c_iter);
		}
	}
	EndStatement();
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
	assemble_buffer << "\ninclude .\\masm32\\include\\msvcrt.inc";
	assemble_buffer << "\ninclude .\\masm32\\include\\kernel32.inc";
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
	assemble_buffer << "\n    _string_format     db '%s ', 0";
//	assemble_buffer << "\n    _String0		DB	'123', 0";
	for(size_t i = 0; i < stringtable_.size(); ++i)
	{
		assemble_buffer << "\n    _String" << i << "           db '" << stringtable_[i] << "', 0"; 
	}
	assemble_buffer << endl;
}
void AssemblyMaker::CodeBeginSegment() throw()
{
	assemble_buffer << "\n.CODE" << endl;
}
void AssemblyMaker::MainFunction() throw()
{
	// ����������
	assemble_buffer << "\nstart:\n";				// ��ʼλ��
	assemble_buffer << "\n_main  proc far";		// ��ת��ʽ
	// ������ͷ
	// һ. �ҳ��������оֲ���������ʱ�����Ĵ洢�ռ�
	int var_space = tokentable_.GetVariableSpace(tokentable_.begin());;
	int temp_space = quaternarytable_.front().src2_;
	// ��. ����Ĵ�������������ջ
	// ���õ� eax ebx edx ebp esp
	// esp����ѹջ����
	assemble_buffer << "\n    push    eax"
					<< "\n    push    ebx"
					<< "\n    push    edx";
	assemble_buffer	<< "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
	assemble_buffer << '\n';
	// ��. ����Ԫʽ�����ҵ��������Ŀ�ʼ��ַ������Ԫʽ���л��
	for(vector<Quaternary>::const_iterator q_iter = Quaternary::GetFunctionBody(quaternarytable_.begin());
		Quaternary::END != q_iter->op_;
		++q_iter)
	{
		// ת�������
		TranslateQuaternary(q_iter, 0, var_space, 0);
	}
	//assemble_buffer << "\n    push    offset  _String0"
	//				<< "\n    push    offset  _str_format"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp,   8";
	// ��. ������β����ԭ����
	assemble_buffer << '\n';
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// ��ԭջ��ָ��
					<< "\n    pop     ebp";
	assemble_buffer << "\n    pop     edx"
					<< "\n    pop     ebx"
					<< "\n    pop     eax";
	//assemble_buffer << "\n    pop     0";
	assemble_buffer << "\n    call    ExitProcess";
	assemble_buffer << "\n_main  endp\n" << endl;
}

// ��ͨ�����Ļ�����
// c_iter�Ǹú������ڷ��ű��еı���ĵ�����
void AssemblyMaker::OtherFunction(TokenTable::const_iterator c_iter) throw()
{
	// һ. �ҵ���������Ԫʽ��BEGIN���
	vector<Quaternary>::const_iterator q_iter = GetProcFuncIterInQuaternaryTable(c_iter);
	// ��. �õ�������ռ�Ŀռ䣨��λ��4bytes��
	int para_space = c_iter->value_;
	// ��. �õ��ֲ������Ŀռ䣨��λ��4bytes��
	int var_space = tokentable_.GetVariableSpace(c_iter + 1);
	// ��. �õ���ʱ�����Ŀռ䣨��λ��4bytes��
	int temp_space = q_iter->src2_;
	// ��. �������ͷ����������ջ��Ϊ�ֲ���������ʱ��������ռ䣩
	//assemble_buffer << "\n_" << std::setiosflags(ios::right)<< std::setw(8) << c_iter->name_ << "  proc near";
	//assemble_buffer.width(8);
	//assemble_buffer.setf(std::ios::left);
	assemble_buffer << "\n_" << c_iter->name_ << "  proc near";
	assemble_buffer << "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
	assemble_buffer << '\n';
	// ��. �ҵ�������������Ԫʽ�е����
	//     Ȼ�������Ԫʽ���ɻ����
	for(q_iter = Quaternary::GetFunctionBody(q_iter);
		Quaternary::END != q_iter->op_;
		++q_iter)
	{
		TranslateQuaternary(q_iter, para_space, var_space, c_iter->level_);	
	}
	// ��. �������β
	assemble_buffer << '\n';
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// ��ԭջ��ָ��
					<< "\n    pop     ebp"
					<< "\n    ret";
	//assemble_buffer ;
//	assemble_buffer.width(8);
//	assemble_buffer.setf(std::ios::left);
	//assemble_buffer << "\n_" << std::setiosflags(ios::left)<< std::setw(8) << c_iter->name_ << "  endp\n";
	assemble_buffer << "\n_" << c_iter->name_ << "  endp\n";
}

void AssemblyMaker::EndStatement() throw()
{
	assemble_buffer << "\nend start" << endl;
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

// TODO ������Ԫʽ
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ����ע����ʽ������������Ԫʽ
	assemble_buffer << "\n    ;";
	// ������
	assemble_buffer.width(3);
	assemble_buffer.setf(ios::right);
	assemble_buffer << distance(quaternarytable_.begin(), c_iter) << "  ";
	// ���������
	assemble_buffer.width(9);
	assemble_buffer.setf(ios::right);
	assemble_buffer << Quaternary::OPCodeString[c_iter->op_] << " ";
	// ���������
	PrintOperand(c_iter->method1_, c_iter->src1_, tokentable_, assemble_buffer);
	PrintOperand(c_iter->method2_, c_iter->src2_, tokentable_, assemble_buffer);
	PrintOperand(c_iter->method3_, c_iter->dst_, tokentable_, assemble_buffer);
	// ����Ԫʽ����Ϊ�����
	switch(c_iter->op_)
	{
	case Quaternary::NEG:
		//TranslateNeg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ADD:
		TranslateAdd(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ASG:
		TranslateAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::AASG:
		TranslateArrayAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::WRITE:
		TranslateWrite(c_iter, para_num, var_space, level);
		break;
	case Quaternary::PROC_CALL:
	case Quaternary::FUNC_CALL:
		TranslateCall(c_iter, para_num, var_space, level);
		break;
	default:
		break;
	}
}

// Add�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
void AssemblyMaker::TranslateAdd(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ����һ����������������ʱ
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		// ���صڶ�����������EAX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EAX);
		// ���ϵ�һ��������
		assemble_buffer << "\n    add     eax, " << c_iter->src1_;
	}
	else if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)	// ���ڶ�����������������ʱ
	{
		// ���ص�һ����������EAX
		LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
		// ���ϵڶ���������
		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	}
	else	// ���Ҳ�����������������
	{
		// ���ص�һ����������EAX
		LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
		// ���صڶ�����������EDX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
		// ���
		assemble_buffer << "\n    add     eax, edx";
	}
	// ��EAX�еĽ�����浽Ŀ�Ĳ�����
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);

	//if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ��Ϊ����Ԫʽ����ʱ�й�������������������Ż������Բ����������������������
	//		assert(false);
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ͨ������EAX
	//		LoadVar(c_iter->src2_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ʱ������EAX
	//		LoadTemp(c_iter->src2_, var_space, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// ���ϵ�һ��������
	//	assemble_buffer << "\n    add     eax, " << c_iter->src1_;
	//}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//{
	//	// ���ص�һ����ͨ������EAX
	//	LoadVar(c_iter->src1_, para_num, level, EAX);
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���ϵڶ���������
	//		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ͨ������EDX
	//		LoadVar(c_iter->src2_, para_num, level, EDX);
	//		// EAX��EDX���
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ʱ������EDX
	//		LoadTemp(c_iter->src2_, var_space, EDX);
	//		// EAX��EDX���
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//{
	//	// ���ص�һ����ʱ������EAX
	//	LoadTemp(c_iter->src1_, var_space, EAX);
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���ϵڶ���������
	//		assemble_buffer << "\n    add     eax, " << c_iter->src2_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ͨ������EDX
	//		LoadVar(c_iter->src2_, para_num, level, EDX);
	//		// EAX��EDX���
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method2_)
	//	{
	//		// ���صڶ�����ʱ������EDX
	//		LoadTemp(c_iter->src2_, var_space, EDX);
	//		// EAX��EDX���
	//		assemble_buffer << "\n    add     eax, edx";
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//}
	//else
	//{
	//	assert(false);
	//}
	//// ��EAX�洢��Ŀ�Ĳ���������ͨ��������ʱ������
	//if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)
	//{
	//	StoreVar(c_iter->dst_, para_num, level);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)
	//{
	//	StoreTemp(c_iter->dst_, var_space);
	//}
}

// ��ֵ���
// Դ����������������������ͨ��������ʱ����������Ԫ��
// Ŀ�Ĳ�������������ͨ��������ʱ����
void AssemblyMaker::TranslateAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��Դ������װ�ص�EAX��
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// ��EAX�洢��Ŀ�Ĳ�������
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, 0);
	//if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//	{
	//		// ������������EAX��
	//		assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//	{
	//		// ����ͨ�������ص�EAX��
	//		LoadVar(c_iter->src1_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//	{
	//		// ����ʱ�������ص�EAX��
	//		LoadTemp(c_iter->src1_, var_space, EAX);
	//	}
	//	else if(Quaternary::ARRAY_ADDRESSING == c_iter->method1_)
	//	{
	//		// ������Ԫ�ؼ��ص�EAX��
	//		LoadArray(c_iter->src1_, c_iter->src2_, para_num, level, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// ��EAX�����ݴ洢����ͨ������
	//	StoreVar(c_iter->dst_, para_num, level);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)
	//{
	//	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//	{
	//		// ������������EAX��
	//		assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//	}
	//	else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//	{
	//		// ����ͨ�������ص�EAX��
	//		LoadVar(c_iter->src1_, para_num, level, EAX);
	//	}
	//	else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//	{
	//		// ����ʱ�������ص�EAX��
	//		LoadTemp(c_iter->src1_, var_space, EAX);
	//	}
	//	else if(Quaternary::ARRAY_ADDRESSING == c_iter->method1_)
	//	{
	//		// ������Ԫ�ؼ��ص�EAX��
	//		LoadArray(c_iter->src1_, c_iter->src2_, para_num, level, EAX);
	//	}
	//	else
	//	{
	//		assert(false);
	//	}
	//	// ��EAX�����ݴ洢����ʱ������
	//	StoreTemp(c_iter->dst_, var_space);
	//}
	//else
	//{
	//	assert(false);
	//}
}


// ���鸳ֵ���
// Դ����������������������ͨ��������ʱ����
// Ŀ�Ĳ�����Ϊ����
void AssemblyMaker::TranslateArrayAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// �����������������EAX
	// ����Ĳ����������������飬�ʵ���������Ϊ��������������㣩
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	//if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	//{
	//	// ������������EAX��
	//	assemble_buffer << "\n    mov     eax, " << c_iter->src1_;
	//}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method1_)
	//{
	//	// ����ͨ�������ص�EAX��
	//	LoadVar(c_iter->src1_, para_num, level, EAX);
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method1_)
	//{
	//	// ����ʱ�������ص�EAX��
	//	LoadTemp(c_iter->src1_, var_space, EAX);
	//}
	//else
	//{
	//	assert(false);
	//}

	// ��EAX�����ݴ洢��������
	StoreArray(c_iter->dst_, c_iter->src2_, para_num, level);
	
}


void AssemblyMaker::TranslateWrite(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method3_)		// д������
	{
		if(TokenTableItem::INTEGER == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _integer_format";
		}
		else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _char_format";
		}
		else
		{
			assert(false);
		}
	}
	else if(Quaternary::STRING_ADDRESSING == c_iter->method3_)	// д�ַ���
	{
		assemble_buffer << "\n    push    offset  _String" << c_iter->dst_
						<< "\n    push    offset  _string_format";
	}
	else	// ���ر�����EAX����EAXѹջ�����ظ�ʽ�ַ�����ջ��
	{
		
		LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);// ���ر���
		assemble_buffer << "\n    push    eax";	// EAXѹջ
		// ���ظ�ʽ�ַ���
		if((Quaternary::VARIABLE_ADDRESSING == c_iter->method3_
			|| Quaternary::ARRAY_ADDRESSING == c_iter->method3_)
			&& TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// �ַ��͵�������������Ǳ��������飬����������Ϊ�ַ��ͣ�
		{
			assemble_buffer << "\n    push    offset  _char_format";
		}
		else	// ���͵������������Ϊ��ʱ��������ַ��͵ı���/���飩
		{
			assemble_buffer << "\n    push    offset  _integer_format";
		}
	}
	//else if(Quaternary::VARIABLE_ADDRESSING == c_iter->method3_)	// д��ͨ�������������
	//{
	//	LoadVar(c_iter->dst_, para_num, level, EAX);
	//	assemble_buffer << "\n    push    eax";
	//	if(TokenTableItem::INTEGER == tokentable_.at(c_iter->dst_).decoratetype_)	// ���ͱ���
	//	{
	//		assemble_buffer << "\n    push    offset  _integer_format";
	//	}
	//	else if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// �ַ��ͱ���
	//	{			
	//		assemble_buffer << "\n    push    offset  _char_format";
	//	}
	//}
	//else if(Quaternary::ARRAY_ADDRESSING == c_iter->method3_)	// д�������
	//{
	//	LoadArray(c_iter->dst_, c_iter->offset2_, para_num, level, EAX);
	//	assemble_buffer << "\n    push    eax";
	//	if(TokenTableItem::INTEGER == tokentable_.at(c_iter->dst_).decoratetype_)	// ���ͱ���
	//	{
	//		assemble_buffer << "\n    push    offset  _integer_format";
	//	}
	//	else if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)	// �ַ��ͱ���
	//	{			
	//		assemble_buffer << "\n    push    offset  _char_format";
	//	}
	//}
	//else if(Quaternary::TEMPORARY_ADDRESSING == c_iter->method3_)	// д��ʱ����
	//{
	//	LoadTemp(c_iter->dst_, var_space, EAX);
	//	assemble_buffer << "\n    push    eax"
	//					<< "\n    push    offset  _integer_format";
	//}
	//else
	//{
	//	assert(false);
	//}
	// ����printf����
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
}


// ������Ѷ���Ҫ����display���Ŀ���
// �º�����subfunc_level��ȡֵΪ[1, level + 1]
// ��subfunc_level <= levelʱ�����Ƶ�ǰ������display����ǰsubfunc_level������
// ��subfunc_level == level + 1ʱ������ǰ������display������level����Ƹ��Ӻ������ٽ�EBP��Ϊ�Ӻ����µ�һ��display����
void AssemblyMaker::TranslateCall(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// һ. ��ñ����ú�����level
	int subfunc_level = tokentable_.at(c_iter->dst_).level_;
	// ��. ���ݵ����ߺͱ������ߵ�level����display��
	int offset = 0;
	if(subfunc_level <= level)
	{
		for(int i = 0; i < subfunc_level; ++i)
		{
			offset = 4 * (1 + level - i);
			assemble_buffer << "\n    mov     eax, SS:[ebp + " << offset << "]"
							<< "\n    mov     SS:[esp - " << 4 * (i + 1) <<"], eax";
		}
	}
	else if(subfunc_level == level + 1)
	{
		for(int i = 0; i < level; ++i)
		{
			offset = 4 * (1 + level - i);
			assemble_buffer << "\n    mov     eax, SS:[ebp + " << offset << "]"
							<< "\n    mov     SS:[esp - " << 4 * (i + 1) <<"], eax";
		}
		assemble_buffer << "\n    mov     SS:[esp - " << 4 * subfunc_level <<"], ebp";
	}
	// ��. ����ջ��ָ��
	assemble_buffer << "\n    sub     esp, " << 4 * subfunc_level;
	// ��. call����
	assemble_buffer << "\n    call    _" << tokentable_.at(c_iter->dst_).name_;
	// ��. �ָ�ջ��ָ��
	int subfunc_para_num = tokentable_.GetParameterNum(c_iter->dst_);
	assemble_buffer  << "\n    add     esp, " << 4 * (subfunc_level + subfunc_para_num);
}


// ��������/��ͨ����/�������/��ʱ����װ�ص��Ĵ���reg
// ����ȡַ��ʽ�Ĳ�ͬ�����ò�ͬ��װ�غ���
// �ڶ������������Ƿ��ű�index����������ʱ������index��Ҳ������������ֵ��������Ϊindex_or_value
// ÿ�ε���ʱ����һ����������ȡַ��ʽ�������һ���������Ĵ�����ֵ�����Ǳ�����Ч�ġ������е��õ����嶼��ͬ��
// ����֮�⣬�����������Ĳ���Ϊ��index_or_value
// ��ͨ���������Ĳ���Ϊ��index_or_value, para_num, level
// ������������Ĳ���Ϊ��index_or_value, array_offset, var_space, level
// ��ʱ���������Ĳ���Ϊ��index_or_value, var_space
// �ϸ��˵��ֻҪ�ڵ���ʱ�������������Ĳ���������Ч��ֵ����
// ����һ�����ʱ��������ȷ���������͵���������Ծͽ�����ȫ��������Чֵ
// ��������������Ԫ��ʱ��һ��أ�array_offset��0���ɣ�ʵ������Ӱ�죩
void AssemblyMaker::LoadGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level, enum REGISTER reg) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		LoadImmediate(index_or_value, reg);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		LoadVar(index_or_value, para_num, level, reg);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		LoadArray(index_or_value, array_offset, para_num, level, reg);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		LoadTemp(index_or_value, var_space, reg);
	}
	else
	{
		assert(false);
	}
}

// ������valueװ�ص��Ĵ���reg
void AssemblyMaker::LoadImmediate(int value, enum REGISTER reg) throw()
{
	assemble_buffer << "\n    mov     " << RegisterName[reg] << ", " << value;
}

// ������װ�ص��Ĵ�����
// tokentable_indexΪ�����ڷ��ű��е�λ��
// para_num��ʾ��ǰ�����Ĳ�������
// level��ʾ��ǰ�����ľ�̬��Σ����ڷ��ű��еĲ�Σ�
// reg����װ���ĸ��Ĵ���
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::LoadVar(int tokentable_index, int para_num, int level, enum REGISTER reg) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 0;
		if(n < para_num)	// ˵���ǲ���
		{
			offset = 4 * (1 + level + para_num - n);
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
		}
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		if(n < extern_para_num)	// ˵����������
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// ע�⣺array�������ǲ���
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	�����n��Ϊ��㺯���Ĳ��
void AssemblyMaker::LoadArray(int tokentable_index, int array_offset, int para_num, int level, enum REGISTER reg) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
	}
}
// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::LoadTemp(int index, int var_space, enum REGISTER reg) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    mov     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
}

// ���Ĵ���EAX�����ݴ洢���ڴ���
// ���ܵ��ڴ�����Ϊ����ͨ�����������������ʱ����
// ����ȡַ��ʽ�Ĳ�ͬ�����ò�ͬ�Ĵ洢����
// �ڶ����������ܱ�ʾ���ű��е�index��Ҳ���ܱ�ʾ��ʱ������index����ֻ����Ϊindex
void AssemblyMaker::StoreGeneral(Quaternary::AddressingMethod addressingmethod, int index, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		StoreVar(index, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		StoreArray(index, array_offset, para_num, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		StoreTemp(index, var_space);
	}
	else
	{
		assert(false);
	}
}

// �������ӼĴ���EAX�洢���ڴ�
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::StoreVar(int tokentable_index, int para_num, int level) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �жϱ����Ƿ�Ϊ�����ľֲ�����
	{
		int offset = 0;
		if(n < para_num)	// ˵���ǲ���
		{
			offset = 4 * (1 + level + para_num - n);
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
		}
		assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		if(n < extern_para_num)	// ˵����������
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
		}
		assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
	}
}

// ע�⣺array�������ǲ���
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::StoreArray(int tokentable_index, int array_offset, int para_num, int level) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
	}
}

// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::StoreTemp(int index, int var_space) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
}
// �Ĵ�����
 const char * const AssemblyMaker::RegisterName[2] = 
 {"eax", "edx"};