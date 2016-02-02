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
	assemble_buffer << "\n    _integer_format_s    db '%d' , 0   ; for scanf";
	assemble_buffer << "\n    _integer_format_p    db '%d ', 0   ; for printf";
	assemble_buffer << "\n    _char_format_s       db '%c' , 0   ; for scanf";
	assemble_buffer << "\n    _char_format_p       db '%c ', 0   ; for printf";
	assemble_buffer << "\n    _string_format       db '%s ', 0";
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
	PrintQuaternaryComment(quaternarytable_, tokentable_, q_iter, assemble_buffer);
	assemble_buffer << "\n_" << c_iter->name_ << distance(tokentable_.begin(), c_iter) <<"  proc near";
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
	// �������صı�ţ�ʹ���ں������У�����ط�����ʱ��ֻҪ��ת�������žͿ�����
	PrintQuaternaryComment(quaternarytable_, tokentable_, q_iter, assemble_buffer);
	assemble_buffer << '\n' << c_iter->name_ << distance(tokentable_.begin(), c_iter) << "_Exit:";
	// �����������
	assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// ��ԭջ��ָ��
					<< "\n    pop     ebp"
					<< "\n    ret";
	//assemble_buffer ;
//	assemble_buffer.width(8);
//	assemble_buffer.setf(std::ios::left);
	//assemble_buffer << "\n_" << std::setiosflags(ios::left)<< std::setw(8) << c_iter->name_ << "  endp\n";
	assemble_buffer << "\n_" << c_iter->name_ << distance(tokentable_.begin(), c_iter) << "  endp\n";
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

// ������Ԫʽ
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	PrintQuaternaryComment(quaternarytable_, tokentable_, c_iter, assemble_buffer);
	// ����Ԫʽ����Ϊ�����
	switch(c_iter->op_)
	{
	case Quaternary::NEG:
		TranslateNeg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ADD:
		TranslateAdd(c_iter, para_num, var_space, level);
		break;
	case Quaternary::SUB:
		TranslateSub(c_iter, para_num, var_space, level);
		break;
	case Quaternary::MUL:
		TranslateMul(c_iter, para_num, var_space, level);
		break;
	case Quaternary::DIV:
		TranslateDiv(c_iter, para_num, var_space, level);
		break;
	case Quaternary::ASG:
		TranslateAssign(c_iter, para_num, var_space, level);
		break;
	case Quaternary::AASG:
		TranslateArrayAssign(c_iter, para_num, var_space, level);
		break;
	
	case Quaternary::JMP:
		TranslateJmp(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JE:
		TranslateJe(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNE:
		TranslateJne(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JG:
		TranslateJg(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNG:
		TranslateJng(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JL:
		TranslateJl(c_iter, para_num, var_space, level);
		break;
	case Quaternary::JNL:
		TranslateJnl(c_iter, para_num, var_space, level);
		break;
	case Quaternary::SETP:
		TranslateSetP(c_iter, para_num, var_space, level);
		break;
	case Quaternary::PROC_CALL:
	case Quaternary::FUNC_CALL:
		TranslateCall(c_iter, para_num, var_space, level);
		break;
	case Quaternary::RET:
		TranslateRet(c_iter, para_num, var_space, level);
		break;
	case Quaternary::STORE:
		TranslateStore(c_iter, para_num, var_space, level);
		break;
	case Quaternary::READ:
		TranslateRead(c_iter, para_num, var_space, level);
		break;
	case Quaternary::WRITE:
		TranslateWrite(c_iter, para_num, var_space, level);
		break;
	case Quaternary::LABEL:
		TranslateLabel(c_iter, para_num, var_space, level);
		break;
	// �����������������
	case Quaternary::BEGIN:
	case Quaternary::END:
	default:
		assert(false);
		break;
	}
}

// dst = -src1[offset]
// Դ��������������ͨ�����������������ʱ����
// ע�⣬���Դ��������������������Ҫװ����EAX��ȡ��������ֱ���ڱ��������ȡ�����ٴ洢���ڴ棬��������ֻ��һ�����ָ��
// Ȼ�������������Ѿ����м��������ʱ���Ż�������������ȡ�������������Դ��������������������
void AssemblyMaker::TranslateNeg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assert(Quaternary::IMMEDIATE_ADDRESSING != c_iter->method1_);	// �����������Կ���
	// װ��Դ��������EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// ȡ��
	assemble_buffer << "\n    neg     eax";
	// ��EAX����ڴ�
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Add�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������expression���м�����������Ż���
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
}

// ����
// ����Դ�������������������飨ָ���ʽ��֧�֣�
// Ҳ���������ǳ������м��������ʱ��Expression��Ż���
void AssemblyMaker::TranslateSub(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// װ�ر�������EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// ���ݼ����Ƿ�Ϊ�����������д���
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		// EAX ��ȥ������
		assemble_buffer << "\n    sub     eax, " << c_iter->src2_;
	}
	else
	{
		// װ�صڶ�����������EDX
		LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
		// EAX ��ȥ EDX
		assemble_buffer << "\n    sub     eax, edx";
	}
	// �洢���
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Mul�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������term���м�����������Ż���
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
// ���ﻹ�ɽ�һ���Ż�����Ϊmulָ����Զ��ڴ���в�������ʵ����ֻ��Ҫ����һ����������EAX����mulָ�����һ�����������ڴ�ռ�ֱ�ӽ��г˷�
// ������ʵ�������Ƚ��鷳�����������ͳһ���ص�EAX��EDX�У���ȥ���˷�
void AssemblyMaker::TranslateMul(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ���ص�һ����������EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// ���صڶ�����������EDX
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	// ���
	assemble_buffer << "\n    mul     edx";
	// ��EAX�еĽ�����浽Ŀ�Ĳ�����
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// Div�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������term���м�����������Ż���
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
// ���Ƴ˷������ﻹ�ɽ�һ���Ż���������ʵ�ֹ��ڸ��ӣ����ݲ��Ż���
// ����Ƚ���Ҫ���ǣ��������ķ�����չ����Ϊ����������EDX��EAX�ֱ��ʾ�ĸ�32λ�͵�32λ����Ҫ�÷�����չ����EDX
// ͬʱҪ�ĳ��з��ŵĳ����������Ų��������븺��ʱ���
// ���˷��е����޷��ŵĳ˷���mul, imul���ƺ�����ࡣ
void AssemblyMaker::TranslateDiv(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ���ص�һ����������EAX
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	// ���صڶ�����������EBX
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EBX);
	// Imp! ����ʱ��EDX�Ǳ������ĸ�λ��Ҫ���з�����չ	
	assemble_buffer << "\n    CDQ";
	// ���
	assemble_buffer << "\n    idiv     ebx";	// Imp! idiv:�з��ŵĳ����������div�Ļ���������Ϊ����ʱ�����������
	// ��EAX�еĽ�����浽Ŀ�Ĳ�����
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// ��ֵ���
// Դ����������������������ͨ��������ʱ����������Ԫ��
// Ŀ�Ĳ�������������ͨ��������ʱ����
void AssemblyMaker::TranslateAssign(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��Դ������װ�ص�EAX��
	LoadGeneral(c_iter->method1_, c_iter->src1_, c_iter->offset2_, para_num, var_space, level, EAX);
	// ��EAX�洢��Ŀ�Ĳ�������
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);	// stupid bug fixed by mxf at 20:17 2/2 2016
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

// ��EAX�����ݴ�������
// ���ں������÷��غ�ȡ�ú����ķ���ֵ
// �����Ŀ��һ������ʱ���������Ż�����Ҳ��������ͨ���������������������������
void AssemblyMaker::TranslateStore(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	StoreGeneral(c_iter->method3_, c_iter->dst_, 0, para_num, var_space, level);
}

// ���뵽dst[offset]
// ע���ķ���û��֧�ֵ����飬��������������֧�֣��������չ�ķ�
// ����֧�֣���ͨ�����������������ʱ������֧�֣���Ϊ����������ʱ�����ж�����
void AssemblyMaker::TranslateRead(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��Ҫ��ȡ�ı����ĵ�ַװ��EAX
	LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX, true);
	// ����ַѹջ
	assemble_buffer << "\n    push    eax";
	// �������ʽ�ַ���ѹջ
	if(TokenTableItem::CHAR == tokentable_.at(c_iter->dst_).decoratetype_)
	{
		assemble_buffer << "\n    push    offset  _char_format_s";
	}
	else
	{
		assemble_buffer << "\n    push    offset  _integer_format_s";	
	}
	assemble_buffer << "\n    call    scanf"
					<< "\n    add     esp, 8";
}

// ���dst[offset]
void AssemblyMaker::TranslateWrite(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method3_)		// д������
	{
		if(TokenTableItem::INTEGER == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _integer_format_p";
		}
		else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
		{
			assemble_buffer << "\n    push    " << c_iter->dst_
							<< "\n    push    offset  _char_format_p";
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
			assemble_buffer << "\n    push    offset  _char_format_p";
		}
		else	// ���͵������������Ϊ��ʱ��������ַ��͵ı���/���飩
		{
			assemble_buffer << "\n    push    offset  _integer_format_p";
		}
	}
	// ����printf����
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
}

// ��������ת
void AssemblyMaker::TranslateJmp(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer << "\n    jmp     " << LabelStringFormat(c_iter->dst_);
}
// ���Ҳ��������ʱ��ת
void AssemblyMaker::TranslateJe(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    je      " << LabelStringFormat(c_iter->dst_);
}
// ���Ҳ���������ʱ��ת
void AssemblyMaker::TranslateJne(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jne     " << LabelStringFormat(c_iter->dst_);
}
// ��>������ת
void AssemblyMaker::TranslateJg(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jg      " << LabelStringFormat(c_iter->dst_);
}
// ��<=������ת
void AssemblyMaker::TranslateJng(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jng     " << LabelStringFormat(c_iter->dst_);
}
// ��<������ת
void AssemblyMaker::TranslateJl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jl      " << LabelStringFormat(c_iter->dst_);
}
// ��<=������ת
void AssemblyMaker::TranslateJnl(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method1_, c_iter->src1_, 0, para_num, var_space, level, EAX);
	LoadGeneral(c_iter->method2_, c_iter->src2_, 0, para_num, var_space, level, EDX);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jnl     " << LabelStringFormat(c_iter->dst_);
}

// ������ѹջ
// ��������������������ͨ���������顢��ʱ����
void AssemblyMaker::TranslateSetP(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	PushGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level);
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
	assemble_buffer << "\n    call    _" << tokentable_.at(c_iter->dst_).name_ << c_iter->dst_;
	// ��. �ָ�ջ��ָ��
	int subfunc_para_num = tokentable_.at(c_iter->dst_).value_;
	assemble_buffer  << "\n    add     esp, " << 4 * (subfunc_level + subfunc_para_num);
}

// ��Ŀ�Ĳ�����װ��EAX�������к�������
void AssemblyMaker::TranslateRet(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);
	// �ҵ������ķ������ǰ�ı��
	string exit_label = FindExitLabel(c_iter);
	assemble_buffer  << "\n    jmp     " << exit_label;
}

void AssemblyMaker::TranslateLabel(vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer  << "\n" << LabelStringFormat(c_iter->dst_) << ":";
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
void AssemblyMaker::LoadGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level, enum REGISTER reg, bool load_addr) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		LoadImmediate(index_or_value, reg);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		LoadVar(index_or_value, para_num, level, reg, load_addr);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		LoadArray(index_or_value, array_offset, para_num, level, reg, load_addr);
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

// ������������ĵ�ַװ�ص��Ĵ�����
// tokentable_indexΪ�����ڷ��ű��е�λ��
// para_num��ʾ��ǰ�����Ĳ�������
// level��ʾ��ǰ�����ľ�̬��Σ����ڷ��ű��еĲ�Σ�
// reg����װ���ĸ��Ĵ���
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::LoadVar(int tokentable_index, int para_num, int level, enum REGISTER reg, bool load_addr) throw()
{
	// ȷ�������ķ�ʽ��װ�ر�������װ�ر����ĵ�ַ
	const char *action = load_addr ? "lea" : "mov";
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�����������������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 0;
		if(n < para_num)	// ˵���ǲ���
		{
			offset = 4 * (1 + level + para_num - n);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp + " << offset << "]";
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
		}
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    " << action << "     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		if(n < extern_para_num)	// ˵����������
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// װ������Ԫ�ػ����ַ���Ĵ���reg
// ע�⣺array�������Ǻ����Ĳ������ʱ�LoadArrayҪ��һ���ж�
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	�����n��Ϊ��㺯���Ĳ��
void AssemblyMaker::LoadArray(int tokentable_index, int array_offset, int para_num, int level, enum REGISTER reg, bool load_addr) throw()
{
	// ȷ�������ķ�ʽ��װ�ر�������װ�ر����ĵ�ַ
	const char *action = load_addr ? "lea" : "mov";
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    " << action << "     ebx, SS:[ebp + " << offset << "]";
		// ��. �������ı���
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    " << action << "     " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
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
			assemble_buffer << "\n    mov     SS:[ebp + " << offset << "], eax";
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     SS:[ebp - " << offset << "], eax";
		}
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
			assemble_buffer << "\n    mov     SS:[ebx + " << offset << "], eax";
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    mov     SS:[ebx - " << offset << "], eax";
		}
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
void AssemblyMaker::PushGeneral(Quaternary::AddressingMethod addressingmethod, int index_or_value, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		PushImmediate(index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		PushVar(index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		PushArray(index_or_value, array_offset, para_num, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		PushTemp(index_or_value, var_space);
	}
	else
	{
		assert(false);
	}
}

// ������valueװ�ص��Ĵ���reg
void AssemblyMaker::PushImmediate(int value) throw()
{
	assemble_buffer << "\n    push    " << value;
}

// ������ѹջ
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::PushVar(int tokentable_index, int para_num, int level) throw()
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
			assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    push    SS:[ebp - " << offset << "]";
		}
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		// ��. �������ı���
		if(n < extern_para_num)	// ˵����������
		{
			int offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    push    SS:[ebx + " << offset << "]";
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    push    SS:[ebx - " << offset << "]";
		}
	}
}

// ������Ԫ��ѹջ
// ע�⣺���鲻�����Ǻ����Ĳ������ʱ�LoadArrayҪ��һ���ж�
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	�����n��Ϊ��㺯���Ĳ��
void AssemblyMaker::PushArray(int tokentable_index, int array_offset, int para_num, int level) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		int offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n   push    SS:[ebp - " << offset << "]";
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    push    SS:[ebp + " << offset << "]";
		// ��. �������ı���
		offset = 4 * (n + 1 + array_offset);
		assemble_buffer << "\n    push    SS:[ebx - " << offset << "]";
	}
}
// ����ʱ����ѹջ
// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::PushTemp(int index, int var_space) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    push    SS:[ebp - " << offset << "]";
}


// ������Ԫʽ����ĳ��ĵ��������ҵ������ڵĺ����ķ�������ǰ�ı��
// �����������ҷ���һ�����ϲ��ҵ�BEGIN��䣬��ú�����Ӧ�ķ��ű�λ�ã���Ҫ��END����а���������Ӧ�ķ��ű�λ��
// ��һ�����²��ҵ�END��䣬���Ҫ��END����а���������Ӧ�ķ��ű�λ��
// ���ַ�����Ҫ�󶼷��ϣ�Ȼ���ڶ��ָ��򵥣��ʲ��õڶ���
string AssemblyMaker::FindExitLabel(vector<Quaternary>::const_iterator c_iter) throw()
{
	// һ. �ҵ���������Ԫʽ���е�END���
	while(Quaternary::END != c_iter->op_)
	{
		++c_iter;
	}
	// ��. �ҵ������ڷ��ű��е��±�
	int tokentable_index = c_iter->dst_;
	// ��. �ҵ�����������
	std::ostringstream buffer;
	// ��. ���غ����ķ�������ǰ�ı��
	buffer << tokentable_.at(tokentable_index).name_ << tokentable_index << "_Exit";
	return buffer.str();
}

// ͨ��label�������label�ַ���
string AssemblyMaker::LabelStringFormat(int label_index)	
{
	std::ostringstream buffer;
	buffer << "_label" << label_index;
	return buffer.str();
}

// �Ĵ�����
 const char * const AssemblyMaker::RegisterName[3] = 
 {"eax", "ebx", "edx"};