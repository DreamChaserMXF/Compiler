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
    // 标题
    assemble_buffer << "TITLE MXF_AssemblyCode\n";

    // 指令模式（32位），寻址模式（平坦寻址），大小写模式（敏感）
    assemble_buffer << "\n.386";
    assemble_buffer << "\n.model flat, stdcall";
    assemble_buffer << "\noption casemap: none\n";
    // 包含的库
    assemble_buffer << "\nincludelib .\\masm32\\lib\\msvcrt.lib";
    assemble_buffer << "\nincludelib .\\masm32\\lib\\kernel32.lib";
    assemble_buffer << "\ninclude .\\masm32\\include\\msvcrt.inc";
    assemble_buffer << "\ninclude .\\masm32\\include\\kernel32.inc";
    // 输入输出函数调用
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
    // 主函数声明
    assemble_buffer << "\nstart:\n";        // 开始位置
    assemble_buffer << "\n_main  proc far"; // 跳转方式
    // 主函数头
    // 一. 找出主函数中局部变量与临时变量的存储空间
    int var_space = tokentable_.GetVariableSpace(tokentable_.begin());
    ;
    int temp_space = quaternarytable_.front().src2_;
    // 二. 保存寄存器并构造运行栈
    assemble_buffer << "\n    push    eax"
                    << "\n    push    edx";
    assemble_buffer << "\n    push    ebp"
                    << "\n    mov     ebp,   esp"
                    << "\n    sub     esp,   " << 4 * (var_space + temp_space);
    assemble_buffer << '\n';
    // 三. 在四元式表中找到主函数的开始地址，对四元式进行汇编
    for (vector<Quaternary>::const_iterator q_iter =
             Quaternary::GetFunctionBody(quaternarytable_.begin());
         Quaternary::END != q_iter->op_; ++q_iter) {
        // 转换汇编码
        TranslateQuaternary(q_iter);
    }
    assemble_buffer << "\n    push    offset  _String0"
                    << "\n    push    offset  _str_format"
                    << "\n    call    printf"
                    << "\n    add     esp,   8";
    // 四. 主函数尾：还原工作
    assemble_buffer << '\n';
    assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space) // 还原栈顶指针
                    << "\n    pop     ebp";
    assemble_buffer << "\n    pop     edx"
                    << "\n    pop     eax";
    //assemble_buffer << "\n    pop     0";
    assemble_buffer << "\n    call    ExitProcess";
    assemble_buffer << "\n_main  endp\n" << endl;
}

// 普通函数的汇编过程
// c_iter是该函数名在符号表中的位置
void AssemblyMaker::OtherFunction(TokenTable::const_iterator c_iter) throw() {
    // 一. 找到函数在四元式的BEGIN语句
    vector<Quaternary>::const_iterator q_iter = GetProcFuncIterInQuaternaryTable(c_iter);
    // 二. 得到参数所占的空间（单位：4bytes）
    int para_space = c_iter->value_;
    // 三. 得到局部变量的空间（单位：4bytes）
    int var_space = tokentable_.GetVariableSpace(c_iter + 1);
    // 四. 得到临时变量的空间（单位：4bytes）
    int temp_space = q_iter->src2_;
    // 五. 输出函数头，构造运行栈（为局部变量和临时变量分配空间）
    assemble_buffer << "\n_";
    assemble_buffer.width(8);
    assemble_buffer.setf(std::ios::left);
    assemble_buffer << c_iter->name_ << "  proc near";
    assemble_buffer << "\n    push    ebp"
                    << "\n    mov     ebp,   esp"
                    << "\n    sub     esp,   " << 4 * (var_space + temp_space);
    assemble_buffer << '\n';
    // 六. 找到函数主体在四元式中的语句
    //     然后逐个四元式生成汇编码
    for (q_iter = Quaternary::GetFunctionBody(q_iter); Quaternary::END != q_iter->op_; ++q_iter) {
        TranslateQuaternary(q_iter);
    }
    // printf test
    assemble_buffer << "\n    push    offset  _String0"
                    << "\n    push    offset  _str_format"
                    << "\n    call    printf"
                    << "\n    add     esp,   8";
    // 七. 输出函数尾
    assemble_buffer << '\n';
    assemble_buffer << "\n    add     esp,   " << 4 * (var_space + temp_space) // 还原栈顶指针
                    << "\n    pop     ebp"
                    << "\n    ret";
    assemble_buffer << "\n_";
    assemble_buffer.width(8);
    assemble_buffer.setf(std::ios::left);
    assemble_buffer << c_iter->name_ << "  endp\n";
}

void AssemblyMaker::EndStatement() throw() { assemble_buffer << "\nend start" << endl; }

// TODO 翻译四元式
void AssemblyMaker::TranslateQuaternary(vector<Quaternary>::const_iterator& c_iter) const throw() {}
// 该函数在TokenTable中实现为成员函数
//// 返回过程/函数的局部变量所占的空间（单位：4bytes）
//// c_iter为过程/函数在符号表中的位置的下一个位置
//// 之所以这么要求，是因为主函数在符号表中没有位置，只能提供下一个位置
//int AssemblyMaker::GetVariableSpace(TokenTable::const_iterator c_iter) const throw()
//{
//	// 先用while跳过当前过程/函数的参数及常量
//	while( tokentable_.end() != c_iter
//		&& (TokenTableItem::PARAMETER == c_iter->itemtype_
//			|| TokenTableItem::CONST == c_iter->itemtype_)
//		)
//	{
//		++c_iter;
//	}
//	// 如果该过程/函数没有局部变量
//	if(tokentable_.end() == c_iter
//		|| TokenTableItem::PROCEDURE == c_iter->itemtype_
//		|| TokenTableItem::FUNCTION == c_iter->itemtype_)
//	{
//		return 0;
//	}
//	// 现在肯定有变量了
//	// 取得第一个变量的地址
//	int first_var_addr = c_iter->addr_;
//	// 用while跳过当前过程/函数的局部变量
//	while( tokentable_.end() != c_iter
//		&& TokenTableItem::PROCEDURE != c_iter->itemtype_
//		&& TokenTableItem::FUNCTION != c_iter->itemtype_)
//	{
//		++c_iter;
//	}
//	// 找出最后一个变量，得到其地址，再计算局部变量空间（末变量地址-首变量地址+末变量长度）
//	const TokenTableItem &item = ((tokentable_.end() == c_iter) ? tokentable_.back() : *(c_iter - 1));
//	// 最后一个变量可能是数组或普通变量，所以有两种不同的空间计算方式
//	return item.addr_ - first_var_addr + ((TokenTableItem::ARRAY == item.itemtype_) ? item.value_ : 1);
//}

// 通过指向符号表的过程/函数的迭代器，找到其在四元式表中对应的BEGIN语句的迭代器
vector<Quaternary>::const_iterator AssemblyMaker::GetProcFuncIterInQuaternaryTable(
    TokenTable::const_iterator c_iter) const throw() {
    for (vector<Quaternary>::const_iterator iter = quaternarytable_.begin();
         iter != quaternarytable_.end(); ++iter) {
        // 四元式的BEGIN语句中，dst_的值为过程/函数在符号表中的下标
        if (Quaternary::BEGIN == iter->op_ && iter->dst_ == distance(tokentable_.begin(), c_iter)) {
            return iter;
        }
    }
    return quaternarytable_.end();
}

// 改为在Quaternary中作为静态函数实现
//// 通过过程/函数在四元式表中的BEGIN语句的迭代器，找到其过程/函数体的第一条语句的迭代器
//// 这里要求过程/函数的BEGIN和END一定要配对
//vector<Quaternary>::const_iterator AssemblyMaker::GetFunctionBody(vector<Quaternary>::const_iterator begin_iter) const throw()
//{
//	++begin_iter;
//	if(Quaternary::BEGIN != begin_iter->op_)
//	{
//		return begin_iter;
//	}
//
//	int func_num = 1;	// 已经读到了一个begin
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