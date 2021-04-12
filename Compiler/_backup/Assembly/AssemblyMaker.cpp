#include "AssemblyMaker.h"
#include <iostream>
#include <fstream>
using std::cout;
using std::endl;

AssemblyMaker::AssemblyMaker(const vector<Quaternary>& quaternarytable,
                             const TokenTable& tokentable,
                             const vector<string>& stringtable) throw()
    : quaternarytable_(quaternarytable)
    , tokentable_(tokentable)
    , stringtable_(stringtable)
    , assemble_buffer() {}

void AssemblyMaker::Print(std::ostream& out) const throw() { out << assemble_buffer.str(); }
bool AssemblyMaker::Print(const string& filename) const throw() {
    std::ofstream out(filename);
    if (!out.is_open()) {
        cout << "Cannot open file " << filename << endl;
        return false;
    }
    Print(out);
    return true;
}

bool AssemblyMaker::Assemble() throw() {
    assemble_buffer.str("");
    Head();
    StackSegment();
    DataSegment();
    CodeBeginSegment();
    MainFunction();
    // traverse the tokentable
    for (TokenTable::const_iterator c_iter = tokentable_.begin(); c_iter != tokentable_.end();
         ++c_iter) {
        if (TokenTableItem::PROCEDURE == c_iter->itemtype_ ||
            TokenTableItem::FUNCTION == c_iter->itemtype_) {
            OtherFunction(c_iter);
        }
        //		if(TokenTableItem::PROCEDURE == c_iter->itemtype_)
        //		{
        //			OtherProcedure(c_iter);
        ////			OtherProcedure(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
        //		}
        //		else if(TokenTableItem::FUNCTION == c_iter->itemtype_)
        //		{
        //			OtherFunction(c_iter);
        ////			OtherFunction(GetVariableSpace(c_iter + 1), GetProcFuncIterInQuaternaryTable(c_iter));
        //		}
    }
    EndStatement();
    return true;
}

void AssemblyMaker::Head() throw() {
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
void AssemblyMaker::StackSegment() throw() { assemble_buffer << "\n.STACK" << endl; }
void AssemblyMaker::DataSegment() throw() {
    assemble_buffer << "\n.DATA";
    assemble_buffer << "\n    _integer_format    db '%d ', 0";
    assemble_buffer << "\n    _char_format       db '%c ', 0";
    assemble_buffer << "\n    _str_format        db '%s ', 0";
    //	assemble_buffer << "\n    _String0		DB	'123', 0";
    for (vector<string>::const_iterator c_iter = stringtable_.begin(); c_iter != stringtable_.end();
         ++c_iter) {
    }
    assemble_buffer << endl;
}
void AssemblyMaker::CodeBeginSegment() throw() { assemble_buffer << "\n.CODE" << endl; }
void AssemblyMaker::MainFunction() throw() {
    // ����������
    assemble_buffer << "\nstart:\n";        // ��ʼλ��
    assemble_buffer << "\n_main  proc far"; // ��ת��ʽ
    // ������ͷ
    // һ. �ҳ��������оֲ���������ʱ�����Ĵ洢�ռ�
    int var_space = tokentable_.GetVariableSpace(tokentable_.begin());
    ;
    int temp_space = quaternarytable_.front().src2_;
    // ��. ����Ĵ�������������ջ
    assemble_buffer << "\n    push    eax"
                    << "\n    push    edx";
    assemble_buffer << "\n    push    ebp"
                    << "\n    mov     ebp,   esp"
                    << "\n    sub     esp,   " << 4 * (var_space + temp_space);
    assemble_buffer << '\n';
    // ��. ����Ԫʽ�����ҵ��������Ŀ�ʼ��ַ������Ԫʽ���л��
    for (vector<Quaternary>::const_iterator q_iter =
             Quaternary::GetFunctionBody(quaternarytable_.begin());
         Quaternary::END != q_iter->op_; ++q_iter) {
        // ת�������
        TranslateQuaternary(q_iter);
    }
    assemble_buffer << "\n    push    offset  _String0"
                    << "\n    push    offset  _str_format"
                    << "\n    call    printf"
                    << "\n    add     esp,   8";
    // ��. ������β����ԭ����
    assemble_buffer << '\n';
    assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space) // ��ԭջ��ָ��
                    << "\n    pop     ebp";
    assemble_buffer << "\n    pop     edx"
                    << "\n    pop     eax";
    //assemble_buffer << "\n    pop     0";
    assemble_buffer << "\n    call    ExitProcess";
    assemble_buffer << "\n_main  endp\n" << endl;
}

// ��ͨ�����Ļ�����
// c_iter�Ǹú������ڷ��ű��е�λ��
void AssemblyMaker::OtherFunction(TokenTable::const_iterator c_iter) throw() {
    // һ. �ҵ���������Ԫʽ��BEGIN���
    vector<Quaternary>::const_iterator q_iter = GetProcFuncIterInQuaternaryTable(c_iter);
    // ��. �õ�������ռ�Ŀռ䣨��λ��4bytes��
    int para_space = c_iter->value_;
    // ��. �õ��ֲ������Ŀռ䣨��λ��4bytes��
    int var_space = tokentable_.GetVariableSpace(c_iter + 1);
    // ��. �õ���ʱ�����Ŀռ䣨��λ��4bytes��
    int temp_space = q_iter->src2_;
    // ��. �������ͷ����������ջ��Ϊ�ֲ���������ʱ��������ռ䣩
    assemble_buffer << "\n_";
    assemble_buffer.width(8);
    assemble_buffer.setf(std::ios::left);
    assemble_buffer << c_iter->name_ << "  proc near";
    assemble_buffer << "\n    push    ebp"
                    << "\n    mov     ebp,   esp"
                    << "\n    sub     esp,   " << 4 * (var_space + temp_space);
    assemble_buffer << '\n';
    // ��. �ҵ�������������Ԫʽ�е����
    //     Ȼ�������Ԫʽ���ɻ����
    for (q_iter = Quaternary::GetFunctionBody(q_iter); Quaternary::END != q_iter->op_; ++q_iter) {
        TranslateQuaternary(q_iter);
    }
    // printf test
    assemble_buffer << "\n    push    offset  _String0"
                    << "\n    push    offset  _str_format"
                    << "\n    call    printf"
                    << "\n    add     esp,   8";
    // ��. �������β
    assemble_buffer << '\n';
    assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space) // ��ԭջ��ָ��
                    << "\n    pop     ebp"
                    << "\n    ret";
    assemble_buffer << "\n_";
    assemble_buffer.width(8);
    assemble_buffer.setf(std::ios::left);
    assemble_buffer << c_iter->name_ << "  endp\n";
}

void AssemblyMaker::EndStatement() throw() { assemble_buffer << "\nend start" << endl; }

// TODO ������Ԫʽ
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator& c_iter) const throw() {}
// �ú�����TokenTable��ʵ��Ϊ��Ա����
//// ���ع���/�����ľֲ�������ռ�Ŀռ䣨��λ��4bytes��
//// c_iterΪ����/�����ڷ��ű��е�λ�õ���һ��λ��
//// ֮������ôҪ������Ϊ�������ڷ��ű���û��λ�ã�ֻ���ṩ��һ��λ��
//int AssemblyMaker::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
//{
//	// ����while������ǰ����/�����Ĳ���������
//	while( tokentable_.end() != c_iter
//		&& (TokenTableItem::PARAMETER == c_iter->itemtype_
//			|| TokenTableItem::CONST == c_iter->itemtype_)
//		)
//	{
//		++c_iter;
//	}
//	// ����ù���/����û�оֲ�����
//	if(tokentable_.end() == c_iter
//		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
//		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
//	{
//		return 0;
//	}
//	// ���ڿ϶��б�����
//	// ȡ�õ�һ�������ĵ�ַ
//	int first_var_addr = c_iter->addr_;
//	// ��while������ǰ����/�����ľֲ�����
//	while( tokentable_.end() != c_iter
//		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
//		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
//	{
//		++c_iter;
//	}
//	// �ҳ����һ���������õ����ַ���ټ���ֲ������ռ䣨ĩ������ַ-�ױ�����ַ+ĩ�������ȣ�
//	const TokenTableItem &item = ((tokentable_.end() == c_iter) ? tokentable_.back() : *(c_iter - 1));
//	// ���һ�������������������ͨ���������������ֲ�ͬ�Ŀռ���㷽ʽ
//	return item.addr_ - first_var_addr + ((TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1);
//}

// ͨ��ָ����ű�Ĺ���/�����ĵ��������ҵ�������Ԫʽ���ж�Ӧ��BEGIN���ĵ�����
vector<Quaternary>::const_iterator AssemblyMaker::GetProcFuncIterInQuaternaryTable(
    TokenTable::const_iterator c_iter) const throw() {
    for (vector<Quaternary>::const_iterator iter = quaternarytable_.begin();
         iter != quaternarytable_.end(); ++iter) {
        // ��Ԫʽ��BEGIN����У�dst_��ֵΪ����/�����ڷ��ű��е��±�
        if (Quaternary::BEGIN == iter->op_ && iter->dst_ == distance(tokentable_.begin(), c_iter)) {
            return iter;
        }
    }
    return quaternarytable_.end();
}

// ��Ϊ��Quaternary����Ϊ��̬����ʵ��
//// ͨ������/��������Ԫʽ���е�BEGIN���ĵ��������ҵ������/������ĵ�һ�����ĵ�����
//// ����Ҫ�����/������BEGIN��ENDһ��Ҫ���
//vector<Quaternary>::const_iterator AssemblyMaker::GetFunctionBody(vector<Quaternary>::const_iterator begin_iter) const throw()
//{
//	++begin_iter;
//	if(Quaternary::BEGIN != begin_iter->op_)
//	{
//		return begin_iter;
//	}
//
//	int func_num = 1;	// �Ѿ�������һ��begin
//	while(func_num > 0)
//	{
//		++begin_iter;
//		if(Quaternary::BEGIN == begin_iter->op_)
//		{
//			++func_num;
//		}
//		else if(Quaternary::END == begin_iter->op_)
//		{
//			--func_num;
//		}
//	}
//	return ++begin_iter;
//}