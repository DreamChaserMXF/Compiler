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

    // ������ļ�
    const string kTokenFileName          = "TestCase\\result1_tokenlist.txt";
    const string kStringTableFileName    = "TestCase\\result2_stringtable.txt";
    const string kSyntaxFileName         = "TestCase\\result3_syntaxprocess.txt";
    const string kSemanticsFileName      = "TestCase\\result4_semanticsprocess.txt";
    const string kTokenTableFileName     = "TestCase\\result5_tokentable.txt";
    const string kMidCodeProcessFileName = "TestCase\\result6_midcodeprocess.txt";
    const string kQuaternaryFileName     = "TestCase\\result7_midcode.txt";
    const string kAssemblyCodeFileName   = "TestCase\\result_final_assembly.asm";

    //// �����ض���
    //freopen("TestCase\\testinput.txt", "r", stdin);
    for (int i = 0; i < 12; ++i) {
        if (i == 6) continue;
        cout << "\ni = " << i << endl;
        Sleep(1000);
        // �����ض���
        freopen(kInputFileNames[i].c_str(), "r", stdin);

        // ����Դ����
        const string kCodeFileName = kCodeFileNames[i];

        // ɾ��ԭ�е��ļ�
        remove(kTokenFileName.c_str());
        remove(kTokenTableFileName.c_str());
        remove(kStringTableFileName.c_str());
        remove(kSyntaxFileName.c_str());
        remove(kSemanticsFileName.c_str());
        remove(kQuaternaryFileName.c_str());
        remove(kAssemblyCodeFileName.c_str());

        // ��������״̬
        bool status = false;

        // �ʷ�����
        LexicalAnalyzer lex_analyzer(kCodeFileName);
        if (!lex_analyzer.IsBound()) {
            cout << "Cannot open source file " << kCodeFileName << endl;
            continue;
        }
        status = lex_analyzer.Parse();                              // ���дʷ�����������״̬
        lex_analyzer.Print(kTokenFileName);                         // ������ļ�
        vector<string> stringtable = lex_analyzer.getStringTable(); // �ַ�����
        PrintStringVector(stringtable, kStringTableFileName);       // ����ַ�����
        if (!status)                                                // ������ʾ
        {
            cout << "�ʷ���������" << endl;
            continue;
        }

        // �﷨����
        SyntaxAnalyzer syntax_analyzer(
            lex_analyzer); // �ôʷ������������ű��ַ��������Ԫʽ����﷨���������г�ʼ��
        status = syntax_analyzer.Parse();       // �����﷨����������״̬
        syntax_analyzer.Print(kSyntaxFileName); // ����﷨��������
        if (!status)                            // ������ʾ
        {
            cout << "�﷨��������" << endl;
            continue;
        }

        // �������
        // ��ʼ�����ű����Ԫʽ��
        SemanticsAnalyzer semantics_analyzer(lex_analyzer); // �ôʷ���������ʼ�����������
        status = semantics_analyzer.Parse();                // �����������������״̬
        semantics_analyzer.Print(kSemanticsFileName);
        TokenTable tokentable = semantics_analyzer.GetTokenTable();
        tokentable.Print(kTokenTableFileName); // ������ű�
        if (!status)                           // ������ʾ
        {
            cout << "�����������" << endl;
            continue;
        }

        //// �м��������
        MidCodeGenerator midcode_generator(lex_analyzer,
                                           stringtable); // �ôʷ����������ַ������ʼ���м����������
        status = midcode_generator.GenerateQuaternary(); // ������Ԫʽ������״̬
        syntax_analyzer.Print(kMidCodeProcessFileName);  // ����������̣����﷨/�������������ͬ��
        //TokenTable tokentable = midcode_generator.GetTokenTable();
        //tokentable.Print(kTokenTableFileName);										// ������ű�
        vector<Quaternary> quaternarytable = midcode_generator.GetQuaternaryTable();
        PrintQuaternaryVector(quaternarytable, tokentable, kQuaternaryFileName); // �����Ԫʽ
        if (!status)                                                             // ������ʾ
        {
            // �����ʧ�ܶ���������ֵ�������ô��������
            cout << "i = " << i << endl;
            cout << "�м��������ʧ�ܣ�" << endl;
            continue;
        }

        //// Ŀ���������
        //// ����Ԫʽ�����ű��ַ������ʼ�������
        AssemblyMaker assembly_maker(quaternarytable, tokentable, stringtable);
        status = assembly_maker.Assemble();
        assembly_maker.Print(kAssemblyCodeFileName);
        //������
        if (!status) // �������Ӧ���д�
        {
            cout << "i = " << i << endl;
            cout << "Ŀ���������ʧ�ܣ�" << endl;
            continue;
        }

        // �����л��
        //continue;

        // ������
        // ��ȷ��������׺�Ļ���ļ���
        string object_relative_path =
            kAssemblyCodeFileName.substr(0, kAssemblyCodeFileName.find_last_of('.'));
        string object_truncated_file =
            object_relative_path.substr(object_relative_path.find('\\') + 1,
                                        object_relative_path.length() -
                                            object_relative_path.find('\\') - 1);
        // ɾ��ԭ���ļ�
        remove((object_truncated_file + ".obj").c_str());
        remove((object_truncated_file + ".exe").c_str());
        // ���
        string ml_command   = "masm32\\masmbin\\ml.exe /c /coff ";
        string link_command = "masm32\\masmbin\\link.exe /SUBSYSTEM:CONSOLE /OPT:NOREF ";
        system((ml_command + object_relative_path + ".asm").c_str());
        system((link_command + object_truncated_file + ".obj").c_str());
        // ����
        system((object_truncated_file + ".exe").c_str());
        if (6 == i) {
            Sleep(1000); // ��֪��Ϊɶ������͵ö��һ��
        }
        Sleep(1000);
    }
    return 0;
}