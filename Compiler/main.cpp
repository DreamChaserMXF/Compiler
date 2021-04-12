//#pragma warning (disable : 4996)	// freopen

#include <iostream>
#include <string>
#include <cassert>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticsAnalyzer.h"
#include "MidCodeGenerator.h"
#include "AssemblyMaker.h"

#include "Print.h"
#include <Windows.h>

using namespace std;

string kCodeFileNames[13];
string kInputFileNames[13];

int main(int argc, char* argv[]) {
    kCodeFileNames[0]  = "TestCase\\00_generaltest.cpp";
    kCodeFileNames[1]  = "TestCase\\01_write.cpp";
    kCodeFileNames[2]  = "TestCase\\02_read.cpp";
    kCodeFileNames[3]  = "TestCase\\03_arithmetic.cpp";
    kCodeFileNames[4]  = "TestCase\\04_condition.cpp";
    kCodeFileNames[5]  = "TestCase\\05_array.cpp";
    kCodeFileNames[6]  = "TestCase\\06_loop.cpp";
    kCodeFileNames[7]  = "TestCase\\07_procedurecall.cpp";
    kCodeFileNames[8]  = "TestCase\\08_functioncall_factorial.cpp";
    kCodeFileNames[9]  = "TestCase\\09_functioncall_statement_extension.cpp";
    kCodeFileNames[10] = "TestCase\\10_procedure_reference_parameter.cpp";
    kCodeFileNames[11] = "TestCase\\11_function_reference_parameter.cpp";

    kInputFileNames[0]  = "TestCase\\00_input_generaltest.txt";
    kInputFileNames[1]  = "TestCase\\01_input_write.txt";
    kInputFileNames[2]  = "TestCase\\02_input_read.txt";
    kInputFileNames[3]  = "TestCase\\03_input_arithmetic.txt";
    kInputFileNames[4]  = "TestCase\\04_input_condition.txt";
    kInputFileNames[5]  = "TestCase\\05_input_array.txt";
    kInputFileNames[6]  = "TestCase\\06_input_loop.txt";
    kInputFileNames[7]  = "TestCase\\07_input_procedurecall.txt";
    kInputFileNames[8]  = "TestCase\\08_input_functioncall_factorial.txt";
    kInputFileNames[9]  = "TestCase\\09_input_functioncall_statement_extension.txt";
    kInputFileNames[10] = "TestCase\\10_input_procedure_reference_parameter.txt";
    kInputFileNames[11] = "TestCase\\11_input_function_reference_parameter.txt";

    // 输出的文件
    const string kTokenFileName          = "TestCase\\result1_tokenlist.txt";
    const string kStringTableFileName    = "TestCase\\result2_stringtable.txt";
    const string kSyntaxFileName         = "TestCase\\result3_syntaxprocess.txt";
    const string kSemanticsFileName      = "TestCase\\result4_semanticsprocess.txt";
    const string kTokenTableFileName     = "TestCase\\result5_tokentable.txt";
    const string kMidCodeProcessFileName = "TestCase\\result6_midcodeprocess.txt";
    const string kQuaternaryFileName     = "TestCase\\result7_midcode.txt";
    const string kAssemblyCodeFileName   = "TestCase\\result_final_assembly.asm";

    //// 输入重定向
    //freopen("TestCase\\testinput.txt", "r", stdin);
    for (int i = 0; i < 12; ++i) {
        if (i == 6) continue;
        cout << "\ni = " << i << endl;
        Sleep(1000);
        // 输入重定向
        freopen(kInputFileNames[i].c_str(), "r", stdin);

        // 测试源代码
        const string kCodeFileName = kCodeFileNames[i];

        // 删除原有的文件
        remove(kTokenFileName.c_str());
        remove(kTokenTableFileName.c_str());
        remove(kStringTableFileName.c_str());
        remove(kSyntaxFileName.c_str());
        remove(kSemanticsFileName.c_str());
        remove(kQuaternaryFileName.c_str());
        remove(kAssemblyCodeFileName.c_str());

        // 分析器的状态
        bool status = false;

        // 词法分析
        LexicalAnalyzer lex_analyzer(kCodeFileName);
        if (!lex_analyzer.IsBound()) {
            cout << "Cannot open source file " << kCodeFileName << endl;
            continue;
        }
        status = lex_analyzer.Parse();                              // 进行词法分析并返回状态
        lex_analyzer.Print(kTokenFileName);                         // 输出到文件
        vector<string> stringtable = lex_analyzer.getStringTable(); // 字符串表
        PrintStringVector(stringtable, kStringTableFileName);       // 输出字符串表
        if (!status)                                                // 出错提示
        {
            cout << "词法分析出错！" << endl;
            continue;
        }

        // 语法分析
        SyntaxAnalyzer syntax_analyzer(
            lex_analyzer); // 用词法分析器、符号表、字符串表和四元式表对语法分析器进行初始化
        status = syntax_analyzer.Parse();       // 进行语法分析并返回状态
        syntax_analyzer.Print(kSyntaxFileName); // 输出语法分析过程
        if (!status)                            // 出错提示
        {
            cout << "语法分析出错！" << endl;
            continue;
        }

        // 语义分析
        // 初始化符号表和四元式表
        SemanticsAnalyzer semantics_analyzer(lex_analyzer); // 用词法分析器初始化语义分析器
        status = semantics_analyzer.Parse();                // 进行语义分析并返回状态
        semantics_analyzer.Print(kSemanticsFileName);
        TokenTable tokentable = semantics_analyzer.GetTokenTable();
        tokentable.Print(kTokenTableFileName); // 输出符号表
        if (!status)                           // 出错提示
        {
            cout << "语义分析出错！" << endl;
            continue;
        }

        //// 中间代码生成
        MidCodeGenerator midcode_generator(lex_analyzer,
                                           stringtable); // 用词法分析器和字符串表初始化中间代码生成器
        status = midcode_generator.GenerateQuaternary(); // 生成四元式并返回状态
        syntax_analyzer.Print(kMidCodeProcessFileName);  // 输出分析过程（与语法/语义分析过程相同）
        //TokenTable tokentable = midcode_generator.GetTokenTable();
        //tokentable.Print(kTokenTableFileName);										// 输出符号表
        vector<Quaternary> quaternarytable = midcode_generator.GetQuaternaryTable();
        PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryFileName); // 输出四元式
        if (!status)                                                             // 出错提示
        {
            // 这里的失败都是由于右值用于引用传参引起的
            cout << "i = " << i << endl;
            cout << "中间代码生成失败！" << endl;
            continue;
        }

        //// 目标代码生成
        //// 用四元式表、符号表、字符串表初始化汇编器
        AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
        status = assembly_maker.Assemble();
        assembly_maker.Print(kAssemblyCodeFileName);
        //出错检查
        if (!status) // 这里好像不应该有错
        {
            cout << "i = " << i << endl;
            cout << "目标代码生成失败！" << endl;
            continue;
        }

        // 不进行汇编
        //continue;

        // 汇编过程
        // 先确定不带后缀的汇编文件名
        string object_relative_path =
            kAssemblyCodeFileName.substr(0, kAssemblyCodeFileName.find_last_of('.'));
        string object_truncated_file =
            object_relative_path.substr(object_relative_path.find('\\') + 1,
                                        object_relative_path.length() -
                                            object_relative_path.find('\\') - 1);
        // 删除原有文件
        remove((object_truncated_file + ".obj").c_str());
        remove((object_truncated_file + ".exe").c_str());
        // 汇编
        string ml_command   = "masm32\\masmbin\\ml.exe /c /coff ";
        string link_command = "masm32\\masmbin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF ";
        system((ml_command + object_relative_path + ".asm").c_str());
        system((link_command + object_truncated_file + ".obj").c_str());
        // 运行
        system((object_truncated_file + ".exe").c_str());
        if (6 == i) {
            Sleep(1000); // 不知道为啥，这里就得多等一等
        }
        Sleep(1000);
    }
    return 0;
}