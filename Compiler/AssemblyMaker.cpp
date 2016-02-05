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
	assemble_buffer << "\nincludelib .\\masm32\\lib\\msvcrt.lib      ; for printf & scanf linking";
	assemble_buffer << "\nincludelib .\\masm32\\lib\\kernel32.lib    ; for ExitProcess linking";
	assemble_buffer << "\ninclude .\\masm32\\include\\msvcrt.inc     ; �ƺ�ûɶ�ã�Ӧ����printf��scanf�������ļ�";
	assemble_buffer << "\ninclude .\\masm32\\include\\kernel32.inc   ; for ExitProcess";
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
	assemble_buffer << "\n    _string_format       db '%s', 0";
	for(size_t i = 0; i < stringtable_.size(); ++i)
	{
		assemble_buffer << "\n    _String" << i << "             db ";
		for(string::const_iterator iter = stringtable_[i].begin();
			iter != stringtable_[i].end(); ++iter)
		{
			// ÿ48��������һ�У������Ϊ���ޣ���49�������У��������һ��0������֮����ܾͳ���
			if(iter != stringtable_[i].begin() && distance(iter, stringtable_[i].begin()) % 47 == 0)	
			{
				assemble_buffer << static_cast<int>(*iter) << "\n	                     db ";
			}
			else
			{
				assemble_buffer << static_cast<int>(*iter) << ",";
			}
		}
		assemble_buffer << "0";
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
					<< "\n    push    ecx"
					<< "\n    push    edx";
	//// TEST ���ESP
	//assemble_buffer << "\n    mov     eax, esp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";

	assemble_buffer	<< "\n    push    ebp"
					<< "\n    mov     ebp,   esp"
					<< "\n    sub     esp,   " << 4 * (var_space + temp_space);
	//// TEST ���EBP
	//assemble_buffer << "\n    mov     eax, ebp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";
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
	//// TEST ���EBP
	//assemble_buffer << "\n    mov     eax, ebp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";
	//assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// ��ԭջ��ָ��
	assemble_buffer << "\n    mov     esp, ebp"	// ��ԭջ��ָ��
					<< "\n    pop     ebp";
	//// TEST ���ESP
	//assemble_buffer << "\n    mov     eax, esp"
	//				<< "\n    push    eax"
	//				<< "\n    push    offset _integer_format_p"
	//				<< "\n    call    printf"
	//				<< "\n    add     esp, 8";

	assemble_buffer << "\n    pop     edx"
					<< "\n    pop     ecx"
					<< "\n    pop     ebx"
					<< "\n    pop     eax";
	//assemble_buffer << "\n    pop     0";
	assemble_buffer << "\n    call    ExitProcess";	// �����ɶ�ã�
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
	//assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space)	// ��ԭջ��ָ��(����Ҳ���ԣ�������м�ջ��䶯���Ͳ����ˡ����罫��������ֵѹջ�洢ʱ�����ܻ�ѹ����ֵ������add��������ȷ����)
	assemble_buffer << "\n    mov     esp, ebp"										// ��ԭջ��ָ��
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
void AssemblyMaker::TranslateQuaternary(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
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
// ע�⣬���Դ��������������������Ҫװ����EAX��ȡ��������ֱ���ڸú�������ȡ�����ٴ洢���ڴ棬��������ֻ��һ�����ָ��
// Ȼ�������������Ѿ����м��������ʱ���Ż�������������ȡ�������������Դ��������������������
void AssemblyMaker::TranslateNeg(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assert(Quaternary::IMMEDIATE_ADDRESSING != c_iter->method1_);	// �����������Կ���
	// ��Դ��������Ŀ�Ĳ��������ʱ����ֱ�Ӷ��ڴ���в���
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_)
	{
		// ��ʱ�����������������������飬��array_offset������0
		OpGeneral(NEG, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	else
	{
		// װ��Դ��������EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
		// ȡ��
		assemble_buffer << "\n    neg     eax";
		// ��EAX����ڴ�
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// Add�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������expression���м�����������Ż���
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
// TODO �Ż���ĳ��Դ��������Ŀ�Ĳ�������ȣ�����һ��Դ��������������ʱ����ֱ�Ӷ��ڴ����ӷ�����
void AssemblyMaker::TranslateAdd(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��һ��Դ��������Ŀ�Ĳ�������ȣ��ҵڶ���Դ��������������ʱ��ֱ�Ӷ��ڴ����ӷ�����
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpGeneralImmediate(ADD, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src2_);
	}
	// �ڶ���Դ��������Ŀ�Ĳ�������ȣ��ҵ�һ��Դ��������������ʱ��ֱ�Ӷ��ڴ����ӷ�����
	else if(c_iter->method2_ == c_iter->method3_ && c_iter->src2_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpGeneralImmediate(ADD, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src1_);
	}
	// �������������һ����������������ʱ
	else 
	{
		 if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
		{
			// ���صڶ�����������EAX
			OpRegisterGeneral(MOV, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
			// ���ϵ�һ��������
			OpRegisterGeneral(ADD, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		}
		else // ����һ������������������ʱ
		{
			// ���ص�һ����������EAX
			OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
			// ���ϵڶ���������
			OpRegisterGeneral(ADD, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		}
		// ��EAX�еĽ�����浽Ŀ�Ĳ�����
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// ����
// ����Դ�������������������飨ָ���ʽ��֧�֣�
// Ҳ���������ǳ������м��������ʱ��Expression��Ż���
// TODO �Ż�����������Ŀ�Ĳ�������ȣ��Ҽ�����������ʱ����ֱ�Ӷ��ڴ�����������
void AssemblyMaker::TranslateSub(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��һ��Դ��������Ŀ�Ĳ�������ȣ��ҵڶ���Դ��������������ʱ��ֱ�Ӷ��ڴ�����������
	if(c_iter->method1_ == c_iter->method3_ && c_iter->src1_ == c_iter->dst_ && Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpGeneralImmediate(SUB, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src2_);
	}
	else
	{
		// װ�ر�������EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// ��ȥ�ڶ�����
		OpRegisterGeneral(SUB, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// �洢���
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
}

// Mul�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������term���м�����������Ż���
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
// ���ﻹ�ɽ�һ���Ż�����Ϊmulָ����Զ��ڴ���в�������ʵ����ֻ��Ҫ����һ����������EAX����mulָ�����һ�����������ڴ�ռ�ֱ�ӽ��г˷�
// ������ʵ�������Ƚ��鷳�����������ͳһ���ص�EAX��EDX�У���ȥ���˷�
void AssemblyMaker::TranslateMul(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)	// ��һ���������������������
	{
		// ���ص�һ����������EAX����Ϊ������������IMUL�Ĳ�������
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// ���ϵڶ���������
		OpGeneral(IMUL, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	else
	{
		// ���صڶ�����������EAX
		OpRegisterGeneral(MOV, EAX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// ���ϵ�һ��������
		OpGeneral(IMUL, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}

	// ��EAX�еĽ�����浽Ŀ�Ĳ�����
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// Div�����Ҳ�����ֻ�����ǣ�����������ͨ��������ʱ����������������
// �Ҳ���������ͬʱ��������������term���м�����������Ż���
// Ŀ�Ĳ�����ֻ��������ͨ��������ʱ����
// ����Ƚ���Ҫ���ǣ��������ķ�����չ����Ϊ����������EDX��EAX�ֱ��ʾ�ĸ�32λ�͵�32λ����Ҫ�÷�����չ����EDX
// ͬʱҪ�ĳ��з��ŵĳ����������Ų��������븺��ʱ���
// ���˷��е����޷��ŵĳ˷���mul, imul���ƺ�����ࡣӰ���Ӧ���ǽ���е�EDX��64λ����еĸ�32λ��
void AssemblyMaker::TranslateDiv(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ���ص�һ����������EAX
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	// �ڶ������������������������Ҫ���ص��Ĵ����вſ��Գ�
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method2_)
	{
		OpRegisterGeneral(MOV, EBX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		assemble_buffer << "\n    CDQ";				// EDX���EAX�ķ���λ��չ
		assemble_buffer << "\n    idiv     ebx";	// ���Ĵ���
	}
	else	// ���������������ֱ�ӳ�
	{
		assemble_buffer << "\n    CDQ";				// EDX���EAX�ķ���λ��չ
		OpGeneral(IDIV, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	}
	// ��EAX�еĽ�����浽Ŀ�Ĳ�����
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// ��ֵ���
// Դ����������������������ͨ��������ʱ����������Ԫ��
// Ŀ�Ĳ�������������ͨ��������ʱ����
void AssemblyMaker::TranslateAssign(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// Դ��������������ʱ����ֱ�Ӵ洢���ڴ�
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpGeneralImmediate(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level, c_iter->src1_);
	}
	else
	{
		// ��Դ������װ�ص�EAX��
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
		// ��EAX�洢��Ŀ�Ĳ�������
		OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);	// stupid bug fixed by mxf at 20:17 2/2 2016 (0 for parameter level)
	}
}


// ���鸳ֵ���
// Դ����������������������ͨ��������ʱ����
// Ŀ�Ĳ�����Ϊ����
void AssemblyMaker::TranslateArrayAssign(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// Դ��������������ʱ����ֱ�Ӵ洢���ڴ�
	if(Quaternary::IMMEDIATE_ADDRESSING == c_iter->method1_)
	{
		OpArrayImmediate(MOV, c_iter->dst_, c_iter->method2_, c_iter->offset2_, level, c_iter->src1_);
	}
	else
	{
		// ���ر�����EAX
		// ����Ĳ����������������飬�������±��Ϊ��������������㣩
		OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
		// ��EAX�����ݴ洢��������
		OpArrayRegister(MOV, c_iter->dst_, c_iter->method2_, c_iter->offset2_, level);
	}
}

// ��������ת
void AssemblyMaker::TranslateJmp(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer << "\n    jmp     " << GenerateLabelString(c_iter->dst_);
}
// ���Ҳ��������ʱ��ת
void AssemblyMaker::TranslateJe(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    je      " << GenerateLabelString(c_iter->dst_);
}
// ���Ҳ���������ʱ��ת
void AssemblyMaker::TranslateJne(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jne     " << GenerateLabelString(c_iter->dst_);
}
// ��>������ת
void AssemblyMaker::TranslateJg(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jg      " << GenerateLabelString(c_iter->dst_);
}
// ��<=������ת
void AssemblyMaker::TranslateJng(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jng     " << GenerateLabelString(c_iter->dst_);
}
// ��<������ת
void AssemblyMaker::TranslateJl(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jl      " << GenerateLabelString(c_iter->dst_);
}
// ��<=������ת
void AssemblyMaker::TranslateJnl(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method1_, c_iter->src1_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	OpRegisterGeneral(MOV, EDX, c_iter->method2_, c_iter->src2_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
	assemble_buffer << "\n    cmp     eax, edx"
					<< "\n    jnl     " << GenerateLabelString(c_iter->dst_);
}

void AssemblyMaker::TranslateLabel(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	assemble_buffer  << "\n" << GenerateLabelString(c_iter->dst_) << ":";
}

// ������ѹջ
// ��������������������ͨ���������顢��ʱ����
void AssemblyMaker::TranslateSetP(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpGeneral(PUSH, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
}

// ������Ѷ���Ҫ����display���Ŀ���
// �º�����subfunc_level��ȡֵΪ[1, level + 1]
// ��subfunc_level <= levelʱ�����Ƶ�ǰ������display����ǰsubfunc_level������
// ��subfunc_level == level + 1ʱ������ǰ������display������level����Ƹ��Ӻ������ٽ�EBP��Ϊ�Ӻ����µ�һ��display����
void AssemblyMaker::TranslateCall(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
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
void AssemblyMaker::TranslateRet(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpRegisterGeneral(MOV, EAX, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
	//LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX);
	// �ҵ������ķ������ǰ�ı��
	string exit_label = FindExitLabel(c_iter);
	assemble_buffer  << "\n    jmp     " << exit_label;
}


// ��EAX�����ݴ�������
// ���ں������÷��غ�ȡ�ú����ķ���ֵ
// �����Ŀ��һ������ʱ���������Ż�����Ҳ��������ͨ�������������������飨δ���������Ż��������������Ƿ���
void AssemblyMaker::TranslateStore(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	OpGeneralRegister(MOV, c_iter->method3_, c_iter->dst_, Quaternary::NIL_ADDRESSING, 0, para_num, var_space, level);
}

// ���뵽dst[offset]
// ע���ķ���û��֧�ֵ����飬��������������֧�֣��������չ�ķ�
// ����֧�֣���ͨ�����������������ʱ������֧�֣���Ϊ����������ʱ�����ж�����
void AssemblyMaker::TranslateRead(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��Ҫ��ȡ�ı����ĵ�ַװ��EAX
	//LoadGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level, EAX, true);
	OpRegisterGeneral(LEA, EAX, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);
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
void AssemblyMaker::TranslateWrite(const vector<Quaternary>::const_iterator &c_iter, int para_num, int var_space, int level) throw()
{
	// ��������ѹջ
	//PushGeneral(c_iter->method3_, c_iter->dst_, c_iter->offset2_, para_num, var_space, level);
	OpGeneral(PUSH, c_iter->method3_, c_iter->dst_, c_iter->method2_, c_iter->offset2_, para_num, var_space, level);	// ���߶���
	// ����ʽ�ַ���ѹջ
	if(Quaternary::STRING_ADDRESSING == c_iter->method3_)
	{
		assemble_buffer << "\n    push    offset  _string_format";
	}
	else if(TokenTableItem::CHAR == c_iter->dst_decoratetype_)
	{
		assemble_buffer << "\n    push    offset  _char_format_p";
	}
	else
	{
		assemble_buffer << "\n    push    offset  _integer_format_p";
	}
	// ����printf����
	assemble_buffer << "\n    call    printf"
					<< "\n    add     esp, 8";
}

// ��������������
void AssemblyMaker::OpGeneral(enum SINGLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index_or_value, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		OpImmediate(op, index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVar(op, index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArray(op, index_or_value, array_addr_method, array_offset, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTemp(op, index_or_value, var_space);
	}
	else if(Quaternary::STRING_ADDRESSING == addressingmethod)
	{
		assemble_buffer << "\n    " << SingleOperatorName[op] << "    offset  _String" << index_or_value;
	}
	else
	{
		assert(false);
	}
}

// �Գ����Ĳ���
void AssemblyMaker::OpImmediate(enum SINGLEOPERATOR op, int value) throw()
{
	assemble_buffer << "\n    " << SingleOperatorName[op] << "    " << value;
}

// ����ͨ�����Ĳ���
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVar(enum SINGLEOPERATOR op, int tokentable_index, int para_num, int level) throw()
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
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp + " << offset << "]";
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
		}
	}
	else	// ���εı���
	{
		// һ. ������εĲ�������
		int extern_para_num = tokentable_.GetParameterNum(tokentable_index);
		// ��. ������ε�EBP��ֵ�����ص�EBX��
		int offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, dword ptr SS:[ebp + " << offset << "]";
		// ��. �������ı���
		if(n < extern_para_num)	// ˵����������
		{
			offset = 4 * (1 + func_level + extern_para_num - n);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx + " << offset << "]";
		}
		else				    // ˵�������ֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx - " << offset << "]";
		}
	}
}

// ������Ԫ�صĲ���
// ע�⣺���鲻�����Ǻ����Ĳ������ʱ�LoadArrayҪ��һ���ж�
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	�����n��Ϊ��㺯���Ĳ��
void AssemblyMaker::OpArray(enum SINGLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	int offset = 0;
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		// �����ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebp��������ECX�С���ʱ����Ԫ�صľ���ƫ��Ϊebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2"; // ��4
			// 3. ���������ƫ�ƣ�����EAX�У�-(eax-ebp)��
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ecx]";
		}
	}
	else	// ���εı���
	{
		// һ. ������ε�EBP��ֵ�����ص�EBX��
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, dword ptr SS:[ebp + " << offset << "]";
		// ��. ����Ԫ�ص�ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebx - " << offset << "]";
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebx��������ECX�С���ʱ����Ԫ�صľ���ƫ��Ϊebx-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2"; // ��4
			// 3. ���������ƫ�ƣ�����EAX�У�-(eax-ebx)��
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ecx]";
		}
	}
}

// ����ʱ�����Ĳ���
// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTemp(enum SINGLEOPERATOR op, int index, int var_space) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << SingleOperatorName[op] << "    dword ptr SS:[ebp - " << offset << "]";
}


// ���Ĵ���EAX�����ݴ洢���ڴ���
// ���ܵ��ڴ�����Ϊ����ͨ�����������������ʱ����
// ����ȡַ��ʽ�Ĳ�ͬ�����ò�ͬ�Ĵ洢����
// �ڶ����������ܱ�ʾ���ű��е�index��Ҳ���ܱ�ʾ��ʱ������index����ֻ����Ϊindex
void AssemblyMaker::OpGeneralRegister(enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, enum REGISTER reg) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVarRegister(op, index, para_num, level, reg);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArrayRegister(op, index, array_addr_method, array_offset, level, reg);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTempRegister(op, index, var_space, reg);
	}
	else
	{
		assert(false);
	}
}

// ���������洢���ڴ���
// ���ܵ��ڴ�����Ϊ����ͨ�����������������ʱ����
// ����ȡַ��ʽ�Ĳ�ͬ�����ò�ͬ�Ĵ洢����
// �ڶ����������ܱ�ʾ���ű��е�index��Ҳ���ܱ�ʾ��ʱ������index����ֻ����Ϊindex
void AssemblyMaker::OpGeneralImmediate(enum DOUBLEOPERATOR op, Quaternary::AddressingMethod addressingmethod, int index, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level, int immediate_value) throw()
{
	if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpVarImmediate(op, index, para_num, level, immediate_value);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpArrayImmediate(op, index, array_addr_method, array_offset, level, immediate_value);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpTempImmediate(op, index, var_space, immediate_value);
	}
	else
	{
		assert(false);
	}
}

// �����ݴӼĴ���EAX�洢��������
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVarRegister(enum DOUBLEOPERATOR op, int tokentable_index, int para_num, int level, enum REGISTER reg) throw()
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp + " << offset << "], " << RegisterName[reg];
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx + " << offset << "], " << RegisterName[reg];
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx - " << offset << "], " << RegisterName[reg];
		}
	}
}

// ���������洢��������
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpVarImmediate(enum DOUBLEOPERATOR op, int tokentable_index, int para_num, int level, int immediate_value) throw()
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp + " << offset << "], " << immediate_value;
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx + " << offset << "], " << immediate_value;
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx - " << offset << "], " << immediate_value;
		}
	}
}

// ע�⣺array�������ǲ���
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpArrayRegister(enum DOUBLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level, enum REGISTER reg) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	int offset = 0;
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		// �����ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebp��������ECX�С���ʱ����Ԫ�صľ���ƫ��Ϊebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EBX�У�-(eax-ebp)��
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr SS:[ecx], " << RegisterName[reg];
		}
	}
	else	// ���εı���
	{
		// һ. ������ε�EBP��ֵ�����ص�EBX��
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. ����Ԫ�ص�ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebx - " << offset << "], " << RegisterName[reg];
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebx��������EAX�С���ʱ����Ԫ�صľ���ƫ��Ϊebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EAX�У�-(eax-ebx)��
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    SS:[ecx], " << RegisterName[reg];
		}
	}
}

// ע�⣺array�������ǲ���
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpArrayImmediate(enum DOUBLEOPERATOR op, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level, int immediate_value) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	int offset = 0;
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		// �����ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebp��������ECX�С���ʱ����Ԫ�صľ���ƫ��Ϊebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EBX�У�-(eax-ebp)��
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr dword ptr SS:[ecx], " << immediate_value;
		}
	}
	else	// ���εı���
	{
		// һ. ������ε�EBP��ֵ�����ص�EBX��
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. ����Ԫ�ص�ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebx - " << offset << "], " << immediate_value;
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebx��������EAX�С���ʱ����Ԫ�صľ���ƫ��Ϊebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EAX�У�-(eax-ebx)��
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    dword ptr SS:[ecx], " << immediate_value;
		}
	}
}

// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTempRegister(enum DOUBLEOPERATOR op, int index, int var_space, enum REGISTER reg) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "     SS:[ebp - " << offset << "], " << RegisterName[reg];
}

// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpTempImmediate(enum DOUBLEOPERATOR op, int index, int var_space, int immediate_value) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "     dword ptr SS:[ebp - " << offset << "], " << immediate_value;
}

// ˫�����������㣨��һ���������ǼĴ������ڶ����������Ǹ������͵��ڴ��������������
void AssemblyMaker::OpRegisterGeneral(enum DOUBLEOPERATOR op, enum REGISTER reg, Quaternary::AddressingMethod addressingmethod, int index_or_value, Quaternary::AddressingMethod array_addr_method, int array_offset, int para_num, int var_space, int level) throw()
{
	if(Quaternary::IMMEDIATE_ADDRESSING == addressingmethod)
	{
		OpRegisterImmediate(op, reg, index_or_value);
	}
	else if(Quaternary::VARIABLE_ADDRESSING == addressingmethod)
	{
		OpRegisterVar(op, reg, index_or_value, para_num, level);
	}
	else if(Quaternary::ARRAY_ADDRESSING == addressingmethod)
	{
		OpRegisterArray(op, reg, index_or_value, array_addr_method, array_offset, level);
	}
	else if(Quaternary::TEMPORARY_ADDRESSING == addressingmethod)
	{
		OpRegisterTemp(op, reg, index_or_value, var_space);
	}
	else if(Quaternary::STRING_ADDRESSING == addressingmethod)
	{
		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    offset  _String" << index_or_value;
	}
	else
	{
		assert(false);
	}
}

// �Գ����Ĳ���
void AssemblyMaker::OpRegisterImmediate(enum DOUBLEOPERATOR op, enum REGISTER reg, int value) throw()
{
	//TODO ͨ�������ʽ�������Ż�Ч��
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", "<< value;
}

// ����ͨ�����Ĳ���
// ��ַ���㹫ʽ��
// para#n.addr = EBP + 4 * (1 + level + para_num - n)
// var#n.addr = EBP - 4 * (n + 1)
// display#n.addr = EBP + 4 * (1 + level - n)
void AssemblyMaker::OpRegisterVar(enum DOUBLEOPERATOR op, enum REGISTER reg, int tokentable_index, int para_num, int level) throw()
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp + " << offset << "]";
		}
		else				// ˵���Ǿֲ�����
		{
			offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
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
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx + " << offset << "]";
		}
		else				    // ˵�������ֲ�����
		{
			int offset = 4 * (n + 1);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
	}
}

// ������Ԫ�صĲ���
// ע�⣺���鲻�����Ǻ����Ĳ������ʱ�LoadArrayҪ��һ���ж�
// ��ַ���㹫ʽ��
// array#n.addr = EBP - 4 * (n + 1 + array_offset)
// display#n.addr = EBP + 4 * (1 + level - n)	�����n��Ϊ��㺯���Ĳ��
void AssemblyMaker::OpRegisterArray(enum DOUBLEOPERATOR op, enum REGISTER reg, int tokentable_index, Quaternary::AddressingMethod array_addr_method, int array_offset, int level) throw()
{
	// ȷ�������Ĳ��
	int var_level = tokentable_.at(tokentable_index).level_;
	// ȷ�������Ĳ�Σ��ȱ��������1��
	int func_level = var_level - 1;
	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
	int offset = 0;
	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
	{
		// �����ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebp��������ECX�С���ʱ����Ԫ�صľ���ƫ��Ϊebp-ecx
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EBX�У�-(eax-ebp)��
			assemble_buffer << "\n    sub     ecx, ebp"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", dword ptr SS:[ecx]";
		}
	}
	else	// ���εı���
	{
		// һ. ������ε�EBP��ֵ�����ص�EBX��
		offset = 4 * (1 + level - func_level);
		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
		// ��. ����Ԫ�ص�ƫ�����ļ���
		// ��������±������������ͣ���ƫ��������array_offset
		if(Quaternary::IMMEDIATE_ADDRESSING == array_addr_method)
		{
			offset = 4 * (n + 1 + array_offset);
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
		}
		else	// ���򣬾�Ҫȡ�������±꣨ĳ����������ֵ���ڻ������м�����ƫ����
		{
			// 1. �Ȱ������±�ı�����ֵ����ECX
			OpRegisterGeneral(MOV, ECX, array_addr_method, array_offset, Quaternary::NIL_ADDRESSING, 0, 0, 0, level);
			// 2. �����ʵ�ʵ�����Ԫ�ص����ƫ�ƣ������ebx��������EAX�С���ʱ����Ԫ�صľ���ƫ��Ϊebx-eax
			assemble_buffer << "\n    add     ecx, " << (n + 1)
							<< "\n    shl     ecx, 2";	// ��4
			// 3. ���������ƫ�ƣ�����EAX�У�-(eax-ebx)��
			assemble_buffer << "\n    sub     ecx, ebx"
							<< "\n    neg     ecx";
			// 3. ִ�и�ִ�еĲ���
			assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", dword ptr SS:[ecx]";
		}
	}
}
//{
//	// ȷ�������Ĳ��
//	int var_level = tokentable_.at(tokentable_index).level_;
//	// ȷ�������Ĳ�Σ��ȱ��������1��
//	int func_level = var_level - 1;
//	int n = tokentable_.at(tokentable_index).addr_;	// ����ñ�������Ե�ַ
//	if(level == func_level)	// �ж��Ƿ�Ϊ�ֲ�����
//	{
//		int offset = 4 * (n + 1 + array_offset);
//		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
//	}
//	else	// ���εı���
//	{
//		// һ. ������ε�EBP��ֵ�����ص�EBX��
//		int offset = 4 * (1 + level - func_level);
//		assemble_buffer << "\n    mov     ebx, SS:[ebp + " << offset << "]";
//		// ��. �������ı���
//		offset = 4 * (n + 1 + array_offset);
//		assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebx - " << offset << "]";
//	}
//}

// ����ʱ�����Ĳ���
// ��ַ���㹫ʽ��
// temp#n.addr = EBP - 4 * (var_space + n + 1)
void AssemblyMaker::OpRegisterTemp(enum DOUBLEOPERATOR op, enum REGISTER reg, int index, int var_space) throw()
{
	// �����index������ʽ�е�n
	int offset = 4 * (var_space + index + 1);
	assemble_buffer << "\n    " << DoubleOperatorName[op] << "    " << RegisterName[reg] << ", SS:[ebp - " << offset << "]";
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
string AssemblyMaker::GenerateLabelString(int label_index)	
{
	std::ostringstream buffer;
	buffer << "_label" << label_index;
	return buffer.str();
}

// �Ĵ�����
const char * const AssemblyMaker::RegisterName[4] = {"eax", "ebx", "ecx", "edx"};
const char * const AssemblyMaker::SingleOperatorName[4] = {"neg", "push", "imul", "idiv"};
const char * const AssemblyMaker::DoubleOperatorName[4] = {"mov ", "add ", "sub ", "lea "};