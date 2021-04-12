#include "SyntaxAnalyzer.h"
#include "MidCodeGenerator.h"
#include "TokenTableItem.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <algorithm>

#define SYNTAXDEBUG

MidCodeGenerator::MidCodeGenerator(LexicalAnalyzer& lexical_analyzer,
                                   const vector<string>& stringtable) throw()
    : lexical_analyzer_(lexical_analyzer)
    , stringtable_(stringtable)
    , tokentable_()
    , quaternarytable_()
    , token_()
    , level_(1)
    , tempvar_index_(0)
    , label_index_(0)
    , continue_label_()
    , break_label_()
    , is_successful_(true)
    , generating_process_buffer_()
    , generating_format_string_()
    , tokenbuffer_() {}


bool MidCodeGenerator::GenerateQuaternary() throw() {
    size_t depth = 0;
    generating_process_buffer_.str("");
    generating_process_buffer_.clear();
    generating_format_string_.clear();
    lexical_analyzer_.ResetTokenPos();
    PrintFunctionFrame("GenerateQuaternary()", depth);
    lexical_analyzer_.GetNextToken(token_);
    Routine(depth + 1);
    return is_successful_;
}

TokenTable MidCodeGenerator::GetTokenTable() const throw() { return tokentable_; }

vector<Quaternary> MidCodeGenerator::GetQuaternaryTable() const throw() { return quaternarytable_; }

string MidCodeGenerator::toString() const throw() { return generating_process_buffer_.str(); }

bool MidCodeGenerator::Print(const string& filename) const throw() {
    std::ofstream output(filename);
    if (!output.is_open()) {
        return false;
    }
    Print(output);
    output.close();
    return true;
}

void MidCodeGenerator::Print(std::ostream& output) const throw() {
    output << generating_process_buffer_.str() << std::endl;
}

static string generating_format_string_; // 注：不是线程安全的
void MidCodeGenerator::PrintFunctionFrame(const char* func_name, size_t depth) throw() {
    if (depth * 4 == generating_format_string_.size()) {
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    } else if (depth * 4 > (int)generating_format_string_.size()) {
        generating_format_string_.append("|");
        generating_format_string_.append(depth * 4 - generating_format_string_.size(),
                                         ' '); // 这里不能减1
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    } else // depth * 4 < generating_format_string_.size()
    {
        generating_format_string_.resize(depth * 4);
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    }
}
// <程序> ::= <分程序>.
void MidCodeGenerator::Routine(size_t depth) throw() {
    PrintFunctionFrame("Routine()", depth);
    // 插入主函数的BEGIN
    Quaternary q_mainbegin(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::IMMEDIATE_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, -1);
    quaternarytable_.push_back(q_mainbegin);
    // 解析分程序
    SubRoutine(depth + 1);
    // 判断结束符号
    if (token_.type_ != Token::PERIOD) {
        std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t'
                  << "should be '.' at the end of Routine\n";
        is_successful_ = false;
    }
    // 插入主函数的END
    Quaternary q_mainend(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                         0, Quaternary::IMMEDIATE_ADDRESSING, -1);
    quaternarytable_.push_back(q_mainend);
    // 更新过程/函数的BEGIN语句中的临时变量个数
    Quaternary::UpdateTempVarSpace(quaternarytable_);
}

// <分程序> ::= [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>
void MidCodeGenerator::SubRoutine(size_t depth) throw() {
    PrintFunctionFrame("SubRoutine()", depth);

    //
    // 四个可选分支
    if (token_.type_ == Token::CONST) {
        ConstantPart(depth + 1);
    }
    if (token_.type_ == Token::VAR) {
        VariablePart(depth + 1);
    }
    if (token_.type_ == Token::PROCEDURE) {
        ProcedurePart(depth + 1);
    }
    if (token_.type_ == Token::FUNCTION) {
        FunctionPart(depth + 1);
    }
    // 一个必选分支
    assert(token_.type_ == Token::BEGIN);
    StatementBlockPart(depth + 1);
}

// <常量说明部分> ::= const<常量定义>{,<常量定义>};
void MidCodeGenerator::ConstantPart(size_t depth) throw() {
    PrintFunctionFrame("ConstantPart()", depth);

    assert(Token::CONST == token_.type_);

    // 常量定义
    do {
        lexical_analyzer_.GetNextToken(token_);
        constantDefination(depth + 1);
    } while (token_.type_ == Token::COMMA);
    lexical_analyzer_.GetNextToken(token_);
}

// <常量定义> ::= <标识符>＝<常量>
void MidCodeGenerator::constantDefination(size_t depth) throw() {
    PrintFunctionFrame("constantDefination()", depth);

    assert(token_.type_ == Token::IDENTIFIER);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // 记录token_以插入符号表
    Token constIdentifier = token_;
    lexical_analyzer_.GetNextToken(token_);
    assert(token_.type_ == Token::EQU);
    lexical_analyzer_.GetNextToken(token_);
    assert(token_.type_ == Token::CONST_INTEGER || token_.type_ == Token::CONST_CHAR);
    // 常量定义，插入符号表
    if (Token::CONST_INTEGER == token_.type_) {
        tokentable_.AddConstItem(constIdentifier, TokenTableItem::INTEGER, token_.value_.integer,
                                 level_);
    } else {
        assert(Token::CONST_CHAR == token_.type_);
        tokentable_.AddConstItem(constIdentifier, TokenTableItem::CHAR, token_.value_.character,
                                 level_);
    }
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    lexical_analyzer_.GetNextToken(token_);
}

// <变量说明部分> ::= var <变量定义>;{<变量定义>;}
void MidCodeGenerator::VariablePart(size_t depth) throw() {
    PrintFunctionFrame("VariablePart()", depth);

    assert(Token::VAR == token_.type_);

    lexical_analyzer_.GetNextToken(token_);
    do {
        VariableDefinition(depth + 1);
        assert(token_.type_ == Token::SEMICOLON);
        lexical_analyzer_.GetNextToken(token_);
    } while (token_.type_ == Token::IDENTIFIER);
}

// <变量定义> ::= <标识符>{,<标识符>}:<类型>
void MidCodeGenerator::VariableDefinition(size_t depth) throw() {
    PrintFunctionFrame("VariableDefinition()", depth);

    assert(token_.type_ == Token::IDENTIFIER);
    tokenbuffer_.clear();
    tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    lexical_analyzer_.GetNextToken(token_);
    while (token_.type_ == Token::COMMA) {
        lexical_analyzer_.GetNextToken(token_);
        assert(token_.type_ == Token::IDENTIFIER);
        tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
    }
    assert(token_.type_ == Token::COLON);
    lexical_analyzer_.GetNextToken(token_);
    TypeSpecification(depth + 1);
}
// <类型> ::= [array'['<无符号整数>']'of]<基本类型>
void MidCodeGenerator::TypeSpecification(size_t depth) throw() {
    PrintFunctionFrame("TypeSpecification()", depth);

    TokenTableItem::ItemType itemtype_ = TokenTableItem::VARIABLE;
    int arrayLength                    = 0;
    if (token_.type_ == Token::ARRAY) {
        itemtype_ = TokenTableItem::ARRAY;
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::LEFT_BRACKET == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::CONST_INTEGER == token_.type_);
        arrayLength = token_.value_.integer;
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::RIGHT_BRACKET == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::OF == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
    }
    assert(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_);
    // 插入符号表
    TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER)
        ? TokenTableItem::INTEGER
        : TokenTableItem::CHAR;             // 修饰符类型
    if (TokenTableItem::ARRAY == itemtype_) // 若是数组
    {
        for (vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end();
             ++iter) {
            tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
        }
    } else // 若是一般变量
    {
        for (vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end();
             ++iter) {
            tokentable_.AddVariableItem(*iter, decoratetype_, level_);
        }
    }
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    lexical_analyzer_.GetNextToken(token_);
}

// <过程说明部分> ::= <过程首部><分程序>;{<过程首部><分程序>;}
void MidCodeGenerator::ProcedurePart(size_t depth) throw() {
    PrintFunctionFrame("ProcedurePart()", depth);

    do {
        int proc_index = ProcedureHead(depth + 1);
        SubRoutine(depth + 1);
        // 生成过程的END四元式
        quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::IMMEDIATE_ADDRESSING, proc_index));
        tokentable_.Relocate();
        --level_;
        assert(token_.type_ == Token::SEMICOLON);
        lexical_analyzer_.GetNextToken(token_); // 分程序结束后应读入分号
    } while (Token::PROCEDURE == token_.type_);
}

// <过程首部> ::= procedure<过程标识符>'('[<形式参数表>]')';
int MidCodeGenerator::ProcedureHead(size_t depth) throw() {
    PrintFunctionFrame("ProcedureHead()", depth);

    assert(Token::PROCEDURE == token_.type_);

    int proc_index = -1; // 过程名在符号表中的位置（下标）

    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // 插入过程名到符号表
    string proc_name = token_.value_.identifier;
    proc_index       = tokentable_.AddProcedureItem(token_, level_++); // 过程名之后leve要+1
    // 生成过程的BEGIN四元式
    quaternarytable_.push_back(
        Quaternary(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                   0, // 这里的操作数应该是该过程要用到的临时变量的数量，等过程分析完后补齐
                   Quaternary::IMMEDIATE_ADDRESSING, proc_index));
    // 定位（将过程名后紧邻的位置设为局部作用域的起始点）
    tokentable_.Locate();
    // 继续读取单词
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::LEFT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    if (Token::VAR == token_.type_ || Token::IDENTIFIER == token_.type_) {
        int parameterCount = ParameterList(depth);
        tokentable_.SetParameterCount(proc_name, parameterCount);
    }
    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::SEMICOLON == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    return proc_index;
}

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void MidCodeGenerator::FunctionPart(size_t depth) throw() {
    PrintFunctionFrame("FunctionPart()", depth);
    do {
        int func_index = FunctionHead(depth + 1); // 进行函数头分析，并得到函数名在符号表中的位置
        SubRoutine(depth + 1);
        // 生成函数的END四元式
        quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::IMMEDIATE_ADDRESSING, func_index));
        tokentable_.Relocate();
        --level_;
        assert(Token::SEMICOLON == token_.type_); // 分程序结束后应读入分号
        lexical_analyzer_.GetNextToken(token_);   // 读入结尾的分号
    } while (Token::FUNCTION == token_.type_);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
int MidCodeGenerator::FunctionHead(size_t depth) throw() {
    PrintFunctionFrame("FunctionHead()", depth);

    assert(Token::FUNCTION == token_.type_);

    int func_index = -1; // 函数名在符号表中的位置

    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // 插入函数名到符号表
    string func_name = token_.value_.identifier;
    func_index       = tokentable_.AddFunctionItem(token_, level_++); // 过程名之后leve要+1
    // 生成函数的BEGIN四元式
    quaternarytable_.push_back(
        Quaternary(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                   0, // 这里的操作数应该是该函数要用到的临时变量的数量，等函数分析完后补齐
                   Quaternary::IMMEDIATE_ADDRESSING, func_index));
    // 定位
    tokentable_.Locate();
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::LEFT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    if (Token::VAR == token_.type_ || Token::IDENTIFIER == token_.type_) {
        int parameterCount = ParameterList(depth + 1);
        tokentable_.SetParameterCount(func_name, parameterCount);
    }
    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::COLON == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    TokenTableItem::DecorateType decoratetype_ =
        (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
    tokentable_.SetFunctionReturnType(func_name, decoratetype_);
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::SEMICOLON == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    return func_index;
}

// <形式参数表> ::= <形式参数段>{;<形式参数段>}
// 返回形参数量
int MidCodeGenerator::ParameterList(size_t depth) throw() // 形参表
{
    PrintFunctionFrame("ParameterList()", depth);

    int sum = ParameterTerm(depth + 1);
    while (Token::SEMICOLON == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
        sum += ParameterTerm(depth + 1);
    }
    return sum;
}

// <形式参数段> ::= [var]<标识符>{,<标识符>}:<基本类型>
// 返回该形参段的形参数量
int MidCodeGenerator::ParameterTerm(size_t depth) throw() {
    PrintFunctionFrame("ParameterTerm()", depth);
    bool isref = false; // 是否为引用传参
    if (Token::VAR == token_.type_) {
        isref = true;
        lexical_analyzer_.GetNextToken(token_);
    }
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // 参数名压栈准备进符号表
    tokenbuffer_.clear();
    tokenbuffer_.push_back(token_);
    lexical_analyzer_.GetNextToken(token_);
    int sum = 1;
    while (Token::COMMA == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        tokenbuffer_.push_back(token_);
        ++sum;
        lexical_analyzer_.GetNextToken(token_);
    }
    assert(Token::COLON == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // 参数存符号表
    TokenTableItem::DecorateType decoratetype =
        (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
    for (vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end();
         ++iter) {
        tokentable_.AddParameterItem(*iter, decoratetype, isref, level_);
    } // end 存符号表

    lexical_analyzer_.GetNextToken(token_);
    return sum;
}


// <复合语句> ::= begin <语句>{;<语句>} end
void MidCodeGenerator::StatementBlockPart(size_t depth) throw() // 复合语句
{
    PrintFunctionFrame("StatementBlockPart()", depth);

    assert(Token::BEGIN == token_.type_);
    do {
        lexical_analyzer_.GetNextToken(token_);
        Statement(depth + 1);
    } while (token_.type_ == Token::SEMICOLON);
    assert(Token::END == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <语句> ::= <标识符>(<赋值语句>|<过程调用语句>)|<条件语句>|<情况语句>|<复合语句>
// |<读语句>|<写语句>|<while循环语句>|<for循环语句>|<循环继续语句>|<循环退出语句>|<空>
void MidCodeGenerator::Statement(size_t depth) throw() {
    PrintFunctionFrame("Statement()", depth);

    Token idToken = token_; // 该token_可能是过程名，先记下，待用
    TokenTable::iterator iter;
    switch (token_.type_) {
        case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
            generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                       << std::endl;
#endif
            iter = tokentable_.SearchDefinition(token_); // 查找符号表中的定义
            assert(iter != tokentable_.end());
            //iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
            lexical_analyzer_.GetNextToken(token_);
            if (Token::LEFT_PAREN == token_.type_) // 过程或函数调用
            {
                assert(TokenTableItem::PROCEDURE == iter->itemtype_ ||
                       TokenTableItem::FUNCTION == iter->itemtype_); // 检查其属性是否为过程或函数
                vector<ExpressionAttribute> proc_func_attributes =
                    tokentable_.GetProcFuncParameterAttributes(iter);
                // 在ProcFuncCallStatement中设置参数
                ProcFuncCallStatement(idToken, proc_func_attributes, depth + 1);
                // 生成调用四元式
                Quaternary q_procedurecall((iter->itemtype_ == TokenTableItem::PROCEDURE)
                                               ? Quaternary::PROC_CALL
                                               : Quaternary::FUNC_CALL,
                                           Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                                           0, Quaternary::IMMEDIATE_ADDRESSING,
                                           distance(tokentable_.begin(),
                                                    static_cast<TokenTable::const_iterator>(iter)));
                quaternarytable_.push_back(q_procedurecall);
            } else if (Token::ASSIGN == token_.type_ || Token::LEFT_BRACKET == token_.type_) {
                AssigningStatement(idToken, iter, depth + 1);
            } else {
                assert(false);
            }
            break;
        case Token::IF:
            IfStatement(depth + 1);
            break;
        case Token::CASE:
            CaseStatement(depth + 1);
            break;
        case Token::BEGIN:
            StatementBlockPart(depth + 1);
            break;
        case Token::READ:
            ReadStatement(depth + 1);
            break;
        case Token::WRITE:
            WriteStatement(depth + 1);
            break;
        case Token::WHILE:
            WhileLoopStatement(depth + 1);
            break;
        case Token::FOR:
            ForLoopStatement(depth + 1);
            break;
        case Token::CONTINUE:
            ContinueStatement(depth + 1);
            break;
        case Token::BREAK:
            BreakStatement(depth + 1);
            break;
            // 检测空语句是否合法（应该合法）
        case Token::SEMICOLON: // 空语句
        case Token::END:       // 空语句
        default:
            break;
    }
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
// idToken是赋值语句之前标识符的token，iter是其在符号表中的迭代器
void MidCodeGenerator::AssigningStatement(const Token& idToken,
                                          TokenTable::iterator& iter,
                                          size_t depth) // 赋值语句
{
    PrintFunctionFrame("AssigningStatement()", depth);

    // 为四元式生成而定义的变量
    bool assign2array = false; // 是否为对数组的赋值操作

    ExpressionAttribute offset_attribute; // 当对数组元素赋值时，存储偏移量（数组下标）的属性

    if (Token::LEFT_BRACKET == token_.type_) // 对数组元素赋值
    {
        assign2array = true;
        // 语义检查
        assert(TokenTableItem::ARRAY == iter->itemtype_); // 检查是否为数组名
        // 读入表示下标的表达式
        lexical_analyzer_.GetNextToken(token_);
        offset_attribute = Expression(depth + 1);
        // 若数组下标仍是数组元素
        // 则插入四元式，将数组下标值赋给一个临时变量
        SimplifyArrayOperand(offset_attribute);
        // 语法检查
        assert(Token::RIGHT_BRACKET == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
    }
    // 剩下只有三种情况：变量、参数或是函数返回值
    else if (iter->itemtype_ != TokenTableItem::VARIABLE &&
             iter->itemtype_ != TokenTableItem::PARAMETER &&
             iter->itemtype_ != TokenTableItem::FUNCTION) {
        assert(false);
    }
    // 语法检查
    assert(Token::ASSIGN == token_.type_);
    // 读入赋值号右边的表达式
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute right_attribute = Expression(depth + 1);

    // 语义检查：类型转换
    // 这里不能用assert，因为这只是单纯地警告而已
    //assert(iter->decoratetype_ >= right_attribute.decoratetype_);
    // 中间代码生成
    if (assign2array) // 对数组元素赋值
    {
        // 如果right_attribute是数组元素的话，要将其赋值给临时变量
        SimplifyArrayOperand(right_attribute);
        // 进行数组赋值
        Quaternary q_asg;
        q_asg.op_      = Quaternary::AASG;
        q_asg.method1_ = right_attribute.addressingmethod_;
        q_asg.src1_    = right_attribute.value_;
        q_asg.method2_ = offset_attribute.addressingmethod_;
        q_asg.offset2_ = offset_attribute.value_;
        q_asg.method3_ = Quaternary::ARRAY_ADDRESSING;
        q_asg.dst_ =
            std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
        quaternarytable_.push_back(q_asg);

        // 如果右操作数是临时变量，可回收
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        // 如果被赋值的数组下标也是临时变量，可回收
        if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.addressingmethod_) {
            --tempvar_index_;
        }
    } else if (TokenTableItem::PARAMETER == iter->itemtype_ ||
               TokenTableItem::VARIABLE == iter->itemtype_) // 普通变量/参数的赋值
    {
        // 如果赋值号右边的表达式是临时变量，则可优化掉当前的赋值语句。
        // 详见《Appendix1 设计备注》 - chapter 4
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            quaternarytable_.back().method3_ =
                iter->isref_ ? Quaternary::REFERENCE_ADDRESSING : Quaternary::VARIABLE_ADDRESSING;
            quaternarytable_.back().dst_ =
                std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
            // 回收右操作数的临时变量
            --tempvar_index_;
        } else {
            Quaternary q_asg;
            q_asg.op_      = Quaternary::ASG;
            q_asg.method1_ = right_attribute.addressingmethod_;
            q_asg.src1_    = right_attribute.value_;
            q_asg.method2_ = right_attribute.offset_addressingmethod_;
            q_asg.offset2_ = right_attribute.offset_;
            q_asg.method3_ =
                iter->isref_ ? Quaternary::REFERENCE_ADDRESSING : Quaternary::VARIABLE_ADDRESSING;
            q_asg.dst_ =
                std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
            quaternarytable_.push_back(q_asg);
            // 如果赋值号右边的表达式是数组，且其数组下标是临时变量，可回收
            if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_) {
                // 这里有一个程序鲁棒性的假定，即在这个if语句块内，如果addressingmethod_不是ARRAY的话，那么addressingmethod_一定是NIL_ADDRESSING
                // 所以用下面的assert检测一下程序逻辑有无问题
                assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
                --tempvar_index_;
            }
        }
    } else // 函数返回值
    {
        Quaternary q_ret;
        q_ret.op_      = Quaternary::RET;
        q_ret.method2_ = right_attribute.offset_addressingmethod_;
        q_ret.offset2_ = right_attribute.offset_;
        q_ret.method3_ = right_attribute.addressingmethod_;
        q_ret.dst_     = right_attribute.value_;
        quaternarytable_.push_back(q_ret);
        // 如果赋值号右边的表达式是临时变量，可回收
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        // 如果赋值号右边的表达式是数组，且其数组下标是临时变量，可回收
        else if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_) {
            // 这里有一个程序鲁棒性的假定，即如果addressingmethod_不是ARRAY的话，那么addressingmethod_一定是NIL_ADDRESSING
            // 所以用下面的assert检测一下程序逻辑有无问题
            assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
            --tempvar_index_;
        }
    }
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
ExpressionAttribute MidCodeGenerator::Expression(size_t depth) throw() // 表达式
{
    PrintFunctionFrame("Expression()", depth);

    Quaternary q_neg; // 可能生成的NEG四元式
    if (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        if (Token::MINUS == token_.type_) // 如果是减号，就可能会生成一项四元式
        {
            q_neg.op_ = Quaternary::NEG;
        }
        lexical_analyzer_.GetNextToken(token_);
    }

    ExpressionAttribute first_term = Term(depth + 1);
    //bool isref = first_term.isref_;
    if (Quaternary::NEG == q_neg.op_) // 如果之前读到了一个减号
    {
        // 常数取反的优化
        if (Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_) {
            first_term.value_ = -first_term.value_;
        } else // 生成NEG的四元式
        {
            q_neg.method1_ = first_term.addressingmethod_;
            q_neg.src1_    = first_term.value_;
            q_neg.method2_ = first_term.offset_addressingmethod_;
            q_neg.offset2_ = first_term.offset_;
            q_neg.method3_ = Quaternary::TEMPORARY_ADDRESSING;
            if (Quaternary::TEMPORARY_ADDRESSING == q_neg.method1_) {
                q_neg.dst_ = q_neg.src1_;
            } else {
                q_neg.dst_ = tempvar_index_++;
            }
            quaternarytable_.push_back(q_neg);
            // 修改first_term的属性(NEG操作不影响decoratetype)
            first_term.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
            first_term.value_                   = q_neg.dst_;
            first_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
            first_term.offset_                  = 0;
        }
    }

    Quaternary q_term;
    ExpressionAttribute new_term;
    bool is_first_operator = true;
    while (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        // 第一次时，如果new_term是数组元素，则要将其赋值给临时变量
        if (is_first_operator) {
            SimplifyArrayOperand(first_term);
        }

        // 确定四元式的操作符
        q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;

        // 读取下一项
        lexical_analyzer_.GetNextToken(token_);
        TokenTableItem::DecorateType last_term_decoratetype =
            is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
        new_term = Term(depth + 1);

        // 语义分析：执行类型转换
        new_term.decoratetype_ =
            TokenTableItem::TypeConversionMatrix[last_term_decoratetype][new_term.decoratetype_];

        // 如果读到的new_term还是数组元素，那么仍然需要一次转换
        // 将数组元素的值赋给临时变量
        SimplifyArrayOperand(new_term);

        // 确定四元式的操作数
        // src1的确定：
        // 第一次加/减时，src1就是while之前读入的那个term
        // 之后加/减时，src1就是上一个四元式的结果
        if (is_first_operator) {
            // 两个常数相加/减的优化：直接在编译时计算结果
            if (Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_ &&
                Quaternary::IMMEDIATE_ADDRESSING == new_term.addressingmethod_) {
                first_term.decoratetype_ = new_term.decoratetype_;
                first_term.value_        = (Quaternary::ADD == q_term.op_)
                    ? (first_term.value_ + new_term.value_)
                    : (first_term.value_ - new_term.value_);
                continue;
            }
            // 正常流程
            q_term.method1_   = first_term.addressingmethod_;
            q_term.src1_      = first_term.value_;
            is_first_operator = false;
        } else {
            q_term.method1_ = q_term.method3_;
            q_term.src1_    = q_term.dst_;
        }
        // src2的确定：
        // src2就是读到的新的term
        q_term.method2_ = new_term.addressingmethod_;
        q_term.src2_    = new_term.value_;
        // dst的确定：
        // 如果src1是临时变量，就令dst为src1
        // 否则，如果src2是临时变量，就令dst为src2
        // 否则，令dst为新的临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == q_term.method1_) {
            q_term.method3_ = q_term.method1_;
            q_term.dst_     = q_term.src1_;
            // 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
            if (Quaternary::TEMPORARY_ADDRESSING == q_term.method2_) {
                --tempvar_index_;
            }
        } else if (Quaternary::TEMPORARY_ADDRESSING == q_term.method2_) {
            q_term.method3_ = q_term.method2_;
            q_term.dst_     = q_term.src2_;
        } else {
            q_term.method3_ = Quaternary::TEMPORARY_ADDRESSING;
            q_term.dst_     = tempvar_index_++;
        }
        // 保存四元式
        quaternarytable_.push_back(q_term);
    }


    // 返回最终的表达式属性
    if (is_first_operator) // 只有一项的情况
    {
        new_term = first_term;
    } else // 有多项的情况，要更新new_term的属性。否则new_term除了decoratetype_外，其余都是最后一项的属性
    {
        new_term.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
        new_term.value_                   = q_term.dst_;
        new_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        new_term.offset_                  = 0;
    }
    return new_term;
}

// <项> ::= <因子>{<乘法运算符><因子>}
ExpressionAttribute MidCodeGenerator::Term(size_t depth) throw() // 项
{
    PrintFunctionFrame("Term()", depth);

    ExpressionAttribute first_factor = Factor(depth + 1);

    ExpressionAttribute new_factor;
    Quaternary q_factor;
    bool is_first_operator = true;
    while (token_.type_ == Token::MUL || token_.type_ == Token::DIV) {
        // 第一次时，如果first_factor是数组元素，则要将其赋值给临时变量
        if (is_first_operator) {
            SimplifyArrayOperand(first_factor);
        }

        // 确定四元式的操作符
        q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

        // 语法分析：读取下一项
        lexical_analyzer_.GetNextToken(token_);
        TokenTableItem::DecorateType last_factor_decoratetype =
            is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
        new_factor = Factor(depth + 1);
        // 语义分析：执行类型转换
        new_factor.decoratetype_ =
            TokenTableItem::TypeConversionMatrix[last_factor_decoratetype][new_factor.decoratetype_];

        // 如果读到的项还是数组元素，那么仍然需要一次转换
        // 将数组元素的值赋给临时变量
        SimplifyArrayOperand(new_factor);

        // 确定四元式的操作数
        // src1的确定：
        // 第一次乘/除，src1就是while之前读入的那个factor
        // 之后加/减时，src1就是上一个四元式的结果
        if (is_first_operator) {
            // 两个常数相乘/除的优化：直接在编译时计算结果
            if (Quaternary::IMMEDIATE_ADDRESSING == first_factor.addressingmethod_ &&
                Quaternary::IMMEDIATE_ADDRESSING == new_factor.addressingmethod_) {
                first_factor.decoratetype_ = new_factor.decoratetype_;
                first_factor.value_        = (Quaternary::MUL == q_factor.op_)
                    ? (first_factor.value_ * new_factor.value_)
                    : (first_factor.value_ / new_factor.value_);
                continue;
            }
            // 正常流程
            q_factor.method1_ = first_factor.addressingmethod_;
            q_factor.src1_    = first_factor.value_;
            is_first_operator = false;
        } else {
            q_factor.method1_ = q_factor.method3_;
            q_factor.src1_    = q_factor.dst_;
        }
        // src2的确定：
        // src2是读到的新的factor
        q_factor.method2_ = new_factor.addressingmethod_;
        q_factor.src2_    = new_factor.value_;
        // dst的确定：
        // 如果src1是临时变量，就令dst为src1
        // 否则，如果src2是临时变量，就令dst为src2
        // 否则，令dst为新的临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == q_factor.method1_) {
            q_factor.method3_ = q_factor.method1_;
            q_factor.dst_     = q_factor.src1_;
            // 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
            if (Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_) {
                --tempvar_index_;
            }
        } else if (Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_) {
            q_factor.method3_ = q_factor.method2_;
            q_factor.dst_     = q_factor.src2_;
        } else {
            q_factor.method3_ = Quaternary::TEMPORARY_ADDRESSING;
            q_factor.dst_     = tempvar_index_++;
        }
        // 保存四元式
        quaternarytable_.push_back(q_factor);
    }

    // 返回项的属性
    if (is_first_operator) // 只有一个因子
    {
        new_factor = first_factor;
    } else {
        // 更新new_factor的属性
        // 否则new_factor除decoratetype外，其余属性均保留了最后一个因子的属性
        new_factor.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
        new_factor.value_                   = q_factor.dst_;
        new_factor.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        new_factor.offset_                  = 0;
    }
    return new_factor;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')'
//          | <无符号整数>
//          | <字符>
ExpressionAttribute MidCodeGenerator::Factor(size_t depth) throw() // 因子
{
    PrintFunctionFrame("Factor()", depth);
    ExpressionAttribute factor_attribute; // 记录该factor因子的信息

    // 语法检查：标识符的情况【变量、常变量、数组、函数调用】
    if (Token::IDENTIFIER == token_.type_) {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        // 语义检查
        TokenTable::iterator iter = tokentable_.SearchDefinition(token_); // 寻找定义
        assert(iter != tokentable_.end());
        // 语义：记录修饰类型与引用类型，并更新符号表
        factor_attribute.decoratetype_ = iter->decoratetype_;
        //iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
        Token idToken = token_; // 记下，待用
        lexical_analyzer_.GetNextToken(token_);
        // 语法检查
        if (Token::LEFT_BRACKET == token_.type_) // 左中括号，数组元素
        {
            // factor_attribute自己的类型与值
            factor_attribute.addressingmethod_ = Quaternary::ARRAY_ADDRESSING;
            factor_attribute.value_ =
                std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
            // 语义检查：是否为数组名
            assert(TokenTableItem::ARRAY == iter->itemtype_);
            // 语法：读入作为下标的表达式
            lexical_analyzer_.GetNextToken(token_);
            ExpressionAttribute offset_attribute = Expression(depth + 1);
            // 中间代码生成
            // 确定factor_attribute的下标
            // 这里的offset_attribute不能是数组元素，否则会构成嵌套的数组下标（即数组下标又是一个数组元素），无法翻译成四元式
            // 如果是数组，就要把offset_attribute存到临时变量中，作为当前factor的下标
            SimplifyArrayOperand(offset_attribute);
            factor_attribute.offset_addressingmethod_ = offset_attribute.addressingmethod_;
            factor_attribute.offset_                  = offset_attribute.value_;
            // 语法检查
            assert(Token::RIGHT_BRACKET == token_.type_);
            // 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
            lexical_analyzer_.GetNextToken(token_);
        } else if (Token::LEFT_PAREN == token_.type_) // 左括号，函数调用
        {
            // 语义检查：是否为函数
            assert(TokenTableItem::FUNCTION == iter->itemtype_);
            // 语义：类型匹配
            // 从符号表中取出函数的参数类型，待FunctionCallStatement去匹配参数
            vector<ExpressionAttribute> parameter_attributes =
                tokentable_.GetProcFuncParameterAttributes(iter);
            // 四元式：先进入ProcFuncCallStatement，设置好参数，然后再生成函数调用语句
            ProcFuncCallStatement(idToken, parameter_attributes, depth + 1);
            // 生成函数调用的四元式
            Quaternary q_functioncall(Quaternary::FUNC_CALL, Quaternary::NIL_ADDRESSING, 0,
                                      Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                      distance(tokentable_.begin(),
                                               static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_functioncall);
            // 在正常处理流程中，希望将函数的返回值放置在temp#tempvar_index的位置
            // 但在子函数中，无法判定temp#tempvar_index的位置
            // 所以子函数将返回值存储在EAX中，再加一条指令，将EAX的值存进临时变量中
            Quaternary q_store(Quaternary::STORE, Quaternary::NIL_ADDRESSING, 0,
                               Quaternary::NIL_ADDRESSING, 0, Quaternary::TEMPORARY_ADDRESSING,
                               tempvar_index_++);
            quaternarytable_.push_back(q_store);
            // 所以factor_attribute的属性是临时变量
            factor_attribute.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
            factor_attribute.value_                   = q_store.dst_;
            factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
            factor_attribute.offset_                  = 0;
        } else // 单独一个标识符
        {
            // 语义检查：是否为变量、常量或过程/函数的参数
            assert(TokenTableItem::VARIABLE == iter->itemtype_ ||
                   TokenTableItem::PARAMETER == iter->itemtype_ ||
                   TokenTableItem::CONST == iter->itemtype_);
            // factor_attribute的属性
            if (TokenTableItem::CONST == iter->itemtype_) // 常变量
            {
                // 这里直接将常变量转换成立即数类型，详见《Appendix1 设计备注》
                factor_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
                factor_attribute.value_                   = iter->value_;
                factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
                factor_attribute.offset_                  = 0;
            } else // 一般变量
            {
                factor_attribute.addressingmethod_ = Quaternary::VARIABLE_ADDRESSING;
                factor_attribute.value_ =
                    std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
                factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
                factor_attribute.offset_                  = 0;
            }
        }
        if (iter->isref_) {
            factor_attribute.addressingmethod_ = Quaternary::REFERENCE_ADDRESSING;
        }
    } else if (Token::LEFT_PAREN == token_.type_) // 括号括起来的表达式
    {
        // bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
        // 读表达式的第一个单词
        lexical_analyzer_.GetNextToken(token_);
        // 再读取表达式
        factor_attribute = Expression(depth + 1); // 记录类型
        assert(Token::RIGHT_PAREN == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_INTEGER == token_.type_) // 整型字面常量
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        factor_attribute.decoratetype_            = TokenTableItem::INTEGER;
        factor_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
        factor_attribute.value_                   = token_.value_.integer;
        factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        factor_attribute.offset_                  = 0;
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_CHAR == token_.type_) // 字符型字面常量
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        factor_attribute.decoratetype_            = TokenTableItem::CHAR;
        factor_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
        factor_attribute.value_                   = token_.value_.character;
        factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        factor_attribute.offset_                  = 0;
        lexical_analyzer_.GetNextToken(token_);
    } else {
        assert(false);
    }
    return factor_attribute;
}

//// <条件语句> ::= if<条件>then<语句>[else<语句>]
//void MidCodeGenerator::IfStatement(size_t depth) throw()				// 条件语句
//{
//	PrintFunctionFrame("IfStatement()", depth);
//
//	assert(Token::IF == token_.type_);
//
//	// 先申请一个label
//	int label1 = label_index_++;
//	// 读取条件语句
//	lexical_analyzer_.GetNextToken(token_);
//	Condition(label1, depth + 1);	// 在condition中设置跳转语句
//	assert(Token::THEN == token_.type_);
//	// 读取if成功后的语句
//	lexical_analyzer_.GetNextToken(token_);
//	Statement(depth + 1);
//
//	// 读取else的语句
//	if(Token::ELSE == token_.type_)
//	{
//		// 申请第二个label
//		int label2 =  label_index_++;
//		// 生成无条件跳转语句
//		Quaternary q_jmp(Quaternary::JMP,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::IMMEDIATE_ADDRESSING, label2);
//		quaternarytable_.push_back(q_jmp);
//		// 设置第一个label
//		Quaternary q_label1(Quaternary::LABEL,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::IMMEDIATE_ADDRESSING, label1);
//		quaternarytable_.push_back(q_label1);
//		// 读取else中的语句
//		lexical_analyzer_.GetNextToken(token_);
//		Statement(depth + 1);
//		// 设置第二个label
//		Quaternary q_label2(Quaternary::LABEL,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::IMMEDIATE_ADDRESSING, label2);
//		quaternarytable_.push_back(q_label2);
//	}
//	else	// 如果没有else语句，就在if语句块结束的时候设置第一个label
//	{
//		// 设置第一个label
//		Quaternary q_label1(Quaternary::LABEL,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::IMMEDIATE_ADDRESSING, label1);
//		quaternarytable_.push_back(q_label1);
//	}
//}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
// <条件语句> ::= if<条件>@LABEL_beginthen then<语句>@LABEL_endthen
// <条件语句> ::= if<条件>@LABEL_beginthen then<语句>@JMP_endelse @LABEL_endthen else<语句>@LABEL_endelse
void MidCodeGenerator::IfStatement(size_t depth) throw() // 条件语句
{
    PrintFunctionFrame("IfStatement()", depth);

    assert(Token::IF == token_.type_);
    int begin_quaternary_index = quaternarytable_.size(); // 用于一个if语句的优化

    // 申请两个label，标识then语句块的开始处和结束处
    int label_beginthen = label_index_++;
    int label_endthen   = label_index_++;

    // 读取条件语句
    lexical_analyzer_.GetNextToken(token_);
    bool multi_jmp = Condition(label_beginthen, label_endthen, depth + 1); // 在condition中设置跳转语句
    assert(Token::THEN == token_.type_);

    // 不能进行单条件优化的情况
    if (multi_jmp) {
        // 测试前一句是否为到label_beginthen的无条件跳转语句，若是，则可优化
        if (Quaternary::JMP == quaternarytable_.back().op_ &&
            label_beginthen == quaternarytable_.back().dst_) {
            quaternarytable_.pop_back();
        }
        // 设置label_beginthen
        TryLabel(begin_quaternary_index, label_beginthen);
    }
    //else	// 优化掉label_beginthen
    //{
    //	--label_index_;	错！Condition中已经用了若干label编号，这里不能直接自减。实际上，不需要回收，因为它根本不占资源。
    //}

    // 对Condition中某些语句做优化（见【Appendix1设计备注 24-4】）
    SpecialOptimize(begin_quaternary_index);

    // 读取if成功后的语句
    lexical_analyzer_.GetNextToken(token_);
    Statement(depth + 1);

    Quaternary q_label_endthen(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                               Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                               label_endthen);
    // 读取else的语句
    if (Token::ELSE == token_.type_) {
        // 申请第label，标识else语句块的结束处
        int label_endelse = label_index_++;
        // 生成无条件跳转语句
        Quaternary q_jmp_endelse(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                                 Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                 label_endelse);
        quaternarytable_.push_back(q_jmp_endelse);
        // 设置label_endthen
        quaternarytable_.push_back(q_label_endthen);
        // 读取else中的语句
        lexical_analyzer_.GetNextToken(token_);
        Statement(depth + 1);
        // 设置label_endelse
        Quaternary q_label_endelse(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                                   Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                   label_endelse);
        quaternarytable_.push_back(q_label_endelse);
    } else // 如果没有else语句，就在if语句块结束的时候设置第一个label
    {
        // 设置labelendthen
        quaternarytable_.push_back(q_label_endthen);
    }
}

Quaternary::OPCode MidCodeGenerator::ConvertFromToken(const Token::TokenType& token_type,
                                                      bool inverse) const throw() {
    Quaternary::OPCode ret_op = Quaternary::NIL_OP;
    if (inverse) {
        switch (token_type) // 这个单词可能是关系运算符，但也有可能是右括号或THEN或OR或AND（当条件中仅有一个表达式时）
        {
            case Token::LT:
                ret_op = Quaternary::JNL;
                break;
            case Token::LEQ:
                ret_op = Quaternary::JG;
                break;
            case Token::GT:
                ret_op = Quaternary::JNG;
                break;
            case Token::GEQ:
                ret_op = Quaternary::JL;
                break;
            case Token::EQU:
                ret_op = Quaternary::JNE;
                break;
            case Token::NEQ:
                ret_op = Quaternary::JE;
                break;
            default:
                assert(Token::RIGHT_PAREN == token_.type_ || Token::THEN == token_.type_ ||
                       Token::LOGICOR == token_.type_ || Token::LOGICAND == token_.type_);
                ret_op = Quaternary::JE;
                break;
        }
    } else {
        switch (token_type) {
            case Token::LT:
                ret_op = Quaternary::JL;
                break;
            case Token::LEQ:
                ret_op = Quaternary::JNG;
                break;
            case Token::GT:
                ret_op = Quaternary::JG;
                break;
            case Token::GEQ:
                ret_op = Quaternary::JNL;
                break;
            case Token::EQU:
                ret_op = Quaternary::JE;
                break;
            case Token::NEQ:
                ret_op = Quaternary::JNE;
                break;
            default:
                break;
        }
    }
    return ret_op;
}

// <条件> ::= <布尔表达式>
// 若Condition中只有一个跳转语句，则返回false，否则返回true
bool MidCodeGenerator::Condition(int label_positive, int label_negative, size_t depth) throw() // 条件
{
    PrintFunctionFrame("Condition()", depth);
    return BoolExpression(label_positive, label_negative, depth + 1);
}

// <布尔表达式> ::= <布尔项> [<逻辑或> <布尔项>]
// <布尔表达式> ::= <布尔项>@Label<endterm> [<逻辑或> <布尔项>@Label<endterm>] @JMP<label_negative>
// 如果整个布尔表达式中只有一条语句的话，就返回false。否则返回true。（用来判断是否要优化）
bool MidCodeGenerator::BoolExpression(int label_positive,
                                      int label_negative,
                                      size_t depth) throw() // 布尔表达式
{
    PrintFunctionFrame("BoolExpression()", depth);
    bool isfirst            = true;
    bool multi_jmp          = false;
    size_t quaternary_index = 0;
    do {
        if (!isfirst) {
            multi_jmp = true;
            lexical_analyzer_.GetNextToken(token_);
        } else {
            isfirst = false;
        }

        // BoolTerm中，正确项直接跳转至label_positive
        // 失败项跳转至label_endterm
        // 当BoolTerm只有一个简单的BoolFactor项时，可省略label_endterm
        int label_endterm = label_index_++;
        quaternary_index  = quaternarytable_.size();
        if (BoolTerm(label_positive, label_endterm, depth + 1)) {
            multi_jmp = true;
            // 打下term结束的label
            TryLabel(quaternary_index, label_endterm);
        }
        //else	这个else可有可无
        //{
        //	--label_index_;
        //}
    } while (Token::LOGICOR == token_.type_);
    // 如果整个Expression只有一条跳转指令（此时跳转指令的类型应为“正确则跳至积极标志”）
    // 更新为“错误则跳至消极标志”，此时，若正确，则正常执行（因为后面紧跟着的就是正确时的语句块）
    if (!multi_jmp) {
        // 更改BoolExpression中唯一一条跳转语句
        switch (quaternarytable_.back().op_) {
            case Quaternary::JE:
                quaternarytable_.back().op_ = Quaternary::JNE;
                break;
            case Quaternary::JNE:
                quaternarytable_.back().op_ = Quaternary::JE;
                break;
            case Quaternary::JG:
                quaternarytable_.back().op_ = Quaternary::JNG;
                break;
            case Quaternary::JNG:
                quaternarytable_.back().op_ = Quaternary::JG;
                break;
            case Quaternary::JL:
                quaternarytable_.back().op_ = Quaternary::JNL;
                break;
            case Quaternary::JNL:
                quaternarytable_.back().op_ = Quaternary::JL;
                break;
            default:
                assert(false);
                break;
        }
        quaternarytable_.back().dst_ = label_negative;
    } else // 有多条跳转指令时
    {
        //判断一下，上一条语句是否是Label
        //如果是的话，说明最后一个Term有多条跳转语句，可能跳转到上一条语句的Label
        //优化一下，将之前跳转到上条语句的Label的跳转语句，全部跳至label_positive
        if (Quaternary::LABEL == quaternarytable_.back().op_) {
            int last_label = quaternarytable_.back().dst_;
            for (size_t i = quaternary_index; i < quaternarytable_.size(); ++i) {
                switch (quaternarytable_[i].op_) {
                    case Quaternary::JMP:
                    case Quaternary::JE:
                    case Quaternary::JNE:
                    case Quaternary::JG:
                    case Quaternary::JNG:
                    case Quaternary::JL:
                    case Quaternary::JNL:
                        if (quaternarytable_[i].dst_ == last_label) {
                            quaternarytable_[i].dst_ = label_negative;
                        }
                        break;
                    default:
                        break;
                }
            }
            // 上条Label语句已经没用了，可以删掉啦
            quaternarytable_.pop_back();
        }
        // 如果上条语句不是Label，说明Term中只有一条跳转语句，也有优化的机会（见【Appendix1设计备注 24--4-1,2】）
        // 但在这里没有足够的信息还无法进行优化
        else {
            // 全部term都失败，则跳转到label_negative
            Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_negative);
            quaternarytable_.push_back(q_jmp);
        }
    }
    return multi_jmp;
}

// <布尔项> ::= <布尔因子> [<逻辑与><布尔因子>]
// <布尔项> ::= <布尔因子>@Label<endfactor> [<逻辑与><布尔因子>@Label<endfactor>] @JMP<label_positive>
// 当有多个BoolFactor，或仅有一个BoolFactor但其为布尔表达式时，函数返回true
bool MidCodeGenerator::BoolTerm(int label_positive, int label_negative, size_t depth) throw() // 布尔项
{
    PrintFunctionFrame("BoolTerm()", depth);
    bool isfirst            = true;
    bool multi_jmp          = false;
    size_t quaternary_index = 0;
    do {
        if (!isfirst) {
            multi_jmp = true;
            lexical_analyzer_.GetNextToken(token_);
        } else {
            isfirst = false;
        }
        int label_endfactor = label_index_++;
        quaternary_index    = quaternarytable_.size();
        if (BoolFactor(label_endfactor, label_negative,
                       depth + 1)) // 当BoolFactor中为布尔表达式时，才需要有endfactor
        {
            multi_jmp = true;
            // 打下factor结束的label
            TryLabel(quaternary_index, label_endfactor);
        }
        //else	// 回收未使用的label
        //{
        //	--label_index_;
        //}
    } while (Token::LOGICAND == token_.type_);
    if (!multi_jmp) // 只有一个BoolFactor，且其中只有一个简单条件跳转
    {
        // 更改BoolTerm中唯一一条跳转语句
        switch (quaternarytable_.back().op_) {
            case Quaternary::JE:
                quaternarytable_.back().op_ = Quaternary::JNE;
                break;
            case Quaternary::JNE:
                quaternarytable_.back().op_ = Quaternary::JE;
                break;
            case Quaternary::JG:
                quaternarytable_.back().op_ = Quaternary::JNG;
                break;
            case Quaternary::JNG:
                quaternarytable_.back().op_ = Quaternary::JG;
                break;
            case Quaternary::JL:
                quaternarytable_.back().op_ = Quaternary::JNL;
                break;
            case Quaternary::JNL:
                quaternarytable_.back().op_ = Quaternary::JL;
                break;
            default:
                assert(false);
                break;
        }
        quaternarytable_.back().dst_ = label_positive;
    } else {
        // 判断一下，上一条语句是否是Label
        // 如果是的话，说明最后一个Factor有多条跳转语句，可能跳转到上一条语句的Label
        // 优化一下，将之前跳转到上条语句的Label的跳转语句，全部跳至label_positive
        if (Quaternary::LABEL == quaternarytable_.back().op_) {
            int last_label = quaternarytable_.back().dst_;
            for (size_t i = quaternary_index; i < quaternarytable_.size(); ++i) {
                switch (quaternarytable_[i].op_) {
                    case Quaternary::JMP:
                    case Quaternary::JE:
                    case Quaternary::JNE:
                    case Quaternary::JG:
                    case Quaternary::JNG:
                    case Quaternary::JL:
                    case Quaternary::JNL:
                        if (quaternarytable_[i].dst_ == last_label) {
                            quaternarytable_[i].dst_ = label_positive;
                        }
                        break;
                    default:
                        break;
                }
            }
            // 上条Label语句已经没用了，可以删掉啦
            quaternarytable_.pop_back();
        }
        // 如果上条语句不是Label，说明Factor中只有一条跳转语句，也有优化的机会（见【Appendix1设计备注 24-4-1,2】）
        // 但在这里没有足够的信息还无法进行优化
        else {
            Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_positive);
            quaternarytable_.push_back(q_jmp);
        }
    }
    return multi_jmp;
}

// <布尔因子> ::= <表达式>@JE<Label_negative>
// <布尔因子> ::= <表达式><关系运算符><表达式>@JZ<Label_negative>
// <布尔因子> ::= '('<布尔表达式>')'
// 返回值表示factor中是否为一个简单的布尔表达式
bool MidCodeGenerator::BoolFactor(int label_positive, int label_negative, size_t depth) throw() // 布尔因子
{
    PrintFunctionFrame("BoolFactor()", depth);
    bool multi_jmp      = false;
    bool inverse_prefix = false;

    if (Token::LOGICNOT == token_.type_) {
        inverse_prefix = true;
        lexical_analyzer_.GetNextToken(token_);
    }

    // 先考虑非左括号的情况
    if (Token::LEFT_PAREN != token_.type_ || IsExpression(depth + 1)) {
        // 读取左表达式
        ExpressionAttribute left_attribute = Expression(depth + 1);
        // 化简数组元素为临时变量
        SimplifyArrayOperand(left_attribute);
        // 生成有条件的跳转语句
        // 不符合条件时才会跳转，所以这里的操作符与读到的token要反一下
        Quaternary q_jmp_condition;
        switch (token_.type_) // 这个单词可能是关系运算符，但也有可能是右括号或THEN或OR或AND（当条件中仅有一个表达式时）
        {
            case Token::LT:
            case Token::LEQ:
            case Token::GT:
            case Token::GEQ:
            case Token::EQU:
            case Token::NEQ:
                q_jmp_condition.op_ = ConvertFromToken(token_.type_, !inverse_prefix);
                break;
            default:
                assert(Token::RIGHT_PAREN == token_.type_ || Token::THEN == token_.type_ ||
                       Token::LOGICOR == token_.type_ || Token::LOGICAND == token_.type_);
                q_jmp_condition.op_ = Quaternary::JE;
                if (inverse_prefix) {
                    q_jmp_condition.op_ = Quaternary::JNE;
                }
                break;
        }
        ExpressionAttribute right_attribute;
        if (Token::RIGHT_PAREN != token_.type_ && Token::THEN != token_.type_ &&
            Token::LOGICOR != token_.type_ &&
            Token::LOGICAND != token_.type_) // 如果还有下一个表达式，再继续读取
        {
            lexical_analyzer_.GetNextToken(token_); // 读取关系运算符的下一个单词
            right_attribute = Expression(depth + 1);
            // 化简数组为临时变量
            SimplifyArrayOperand(right_attribute);
        } else {
            right_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
            right_attribute.value_                   = 0;
            right_attribute.offset_addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
            right_attribute.offset_                  = 0;
            right_attribute.decoratetype_            = TokenTableItem::VOID;
        }
        // 操作数
        q_jmp_condition.method1_ = left_attribute.addressingmethod_;
        q_jmp_condition.src1_    = left_attribute.value_;
        q_jmp_condition.method2_ = right_attribute.addressingmethod_;
        q_jmp_condition.src2_    = right_attribute.value_;
        q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
        q_jmp_condition.dst_     = label_negative;

        // 保存四元式
        quaternarytable_.push_back(q_jmp_condition);
        // 回收临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
    } else // 第一个是左括号，且不能识别为表达式的情况
    {
        lexical_analyzer_.GetNextToken(token_); // 读左括号的下一个单词
        if (inverse_prefix) {
            BoolExpression(label_negative, label_positive, depth + 1);
            multi_jmp = true; // 前面有取反的符号，则一定不是一个简单的布尔表达式
        } else {
            multi_jmp = BoolExpression(label_positive, label_negative, depth + 1);
        }
        lexical_analyzer_.GetNextToken(token_); // 读右括号的下一个单词
    }
    return multi_jmp;
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
bool MidCodeGenerator::IsExpression(size_t depth) throw() {
    PrintFunctionFrame("IsExpression()", depth);
    vector<Token>::const_iterator iter = lexical_analyzer_.GetTokenPosition();
    Token tmp                          = token_;
    bool result                        = ExpressionTest(depth + 1);
    token_                             = tmp;
    lexical_analyzer_.SetTokenPosition(iter);
    return result;
}


// <表达式> ::= [+|-]<项>{<加法运算符><项>}
bool MidCodeGenerator::ExpressionTest(size_t depth) throw() {
    PrintFunctionFrame("ExpressionTest()", depth);

    if (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
    }
    if (!TermTest(depth + 1)) {
        return false;
    }

    while (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        // 读取下一项
        lexical_analyzer_.GetNextToken(token_);
        if (!TermTest(depth + 1)) {
            return false;
        }
    }
    return true;
}

// <项> ::= <因子>{<乘法运算符><因子>}
bool MidCodeGenerator::TermTest(size_t depth) throw() // 项
{
    PrintFunctionFrame("TermTest()", depth);

    if (!FactorTest(depth + 1)) {
        return false;
    }

    while (token_.type_ == Token::MUL || token_.type_ == Token::DIV) {
        lexical_analyzer_.GetNextToken(token_);
        if (!FactorTest(depth + 1)) {
            return false;
        }
    }
    return true;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')'
//          | <无符号整数>
//          | <字符>
bool MidCodeGenerator::FactorTest(size_t depth) throw() // 因子
{
    PrintFunctionFrame("FactorTest()", depth);

    // 语法检查：标识符的情况【变量、常变量、数组、函数调用】
    if (Token::IDENTIFIER == token_.type_) {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
        // 数组元素
        if (Token::LEFT_BRACKET == token_.type_) {
            // 语法：读入作为下标的表达式
            lexical_analyzer_.GetNextToken(token_);
            if (!ExpressionTest(depth + 1)) {
                return false;
            }
            // 语法出错，返回false
            if (token_.type_ != Token::RIGHT_BRACKET) {
                return false;
            }
            // 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
            lexical_analyzer_.GetNextToken(token_);
        } else if (Token::LEFT_PAREN == token_.type_) // 左括号，函数调用
        {
            ProcFuncCallStatementTest(depth + 1);
        }
    } else if (Token::LEFT_PAREN == token_.type_) // 括号括起来的表达式
    {
        // bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
        // 读表达式的第一个单词
        lexical_analyzer_.GetNextToken(token_);
        // 再读取表达式
        if (!ExpressionTest(depth + 1)) // 记录类型
        {
            return false;
        }
        if (token_.type_ != Token::RIGHT_PAREN) {
            return false;
        }
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_INTEGER == token_.type_) // 整型字面常量
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_CHAR == token_.type_) // 字符型字面常量
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
    } else {
        // 语法出错，返回false
        return false;
    }
    return true;
}

// <过程/函数调用语句> ::= '('[<实在参数表>]')'
bool MidCodeGenerator::ProcFuncCallStatementTest(size_t depth) // 过程调用语句
{
    PrintFunctionFrame("ProcFuncCallStatement()", depth);
    // 语法检查
    if (Token::LEFT_PAREN != token_.type_) {
        return false;
    }
    // 语法：读入右括号或参数表的第一个单词
    lexical_analyzer_.GetNextToken(token_);
    // 语法：读参数
    if (Token::RIGHT_PAREN != token_.type_) {
        if (!ArgumentListTest(depth + 1)) {
            return false;
        }
        // 语法检查
        if (Token::RIGHT_PAREN != token_.type_) {
            return false;
        }
    }
    lexical_analyzer_.GetNextToken(token_);
    return true;
}

// <实在参数表> ::= <表达式>{,<表达式>}
bool MidCodeGenerator::ArgumentListTest(size_t depth) throw() // 实参表
{
    PrintFunctionFrame("ArgumentList()", depth);

    if (!ExpressionTest(depth + 1)) {
        return false;
    }
    while (Token::COMMA == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
        if (!ExpressionTest(depth + 1)) {
            return false;
        }
    }
    return true;
}

// 对quaternarytable_从下标为begin_quaternary_index的元素开始检查
// 如果有跳转语句，目的地为label_index，则打下label_index
// 否则，不做任何标志
void MidCodeGenerator::TryLabel(size_t begin_quaternary_index, int label_index) throw() {
    Quaternary q_label(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                       0, Quaternary::IMMEDIATE_ADDRESSING, label_index);
    for (size_t i = begin_quaternary_index; i < quaternarytable_.size(); ++i) {
        switch (quaternarytable_[i].op_) {
            case Quaternary::JMP:
            case Quaternary::JE:
            case Quaternary::JNE:
            case Quaternary::JG:
            case Quaternary::JNG:
            case Quaternary::JL:
            case Quaternary::JNL:
                if (quaternarytable_[i].dst_ == label_index) {
                    quaternarytable_.push_back(q_label);
                    return;
                }
            default:
                break;
        }
    }
}

static bool IsConditionJmp(Quaternary::OPCode op) {
    switch (op) {
        case Quaternary::JE:
        case Quaternary::JNE:
        case Quaternary::JG:
        case Quaternary::JNG:
        case Quaternary::JL:
        case Quaternary::JNL:
            return true;
        default:
            return false;
    }
}

// 详见【Appendix1设计备注 24-4】
void MidCodeGenerator::SpecialOptimize(size_t begin_quaternary_index) throw() {
    bool used = false;
    for (size_t i = begin_quaternary_index; i < quaternarytable_.size() - 1; ++i) {
        // 取消特殊的JMP指令详见【Appendix1设计备注 24-4-1】
        if (Quaternary::JMP == quaternarytable_[i].op_ &&
            Quaternary::LABEL == quaternarytable_[i + 1].op_ &&
            quaternarytable_[i - 1].dst_ == quaternarytable_[i + 1].dst_ &&
            IsConditionJmp(quaternarytable_[i - 1].op_)) {
            switch (quaternarytable_[i - 1].op_) {
                case Quaternary::JE:
                    quaternarytable_[i - 1].op_ = Quaternary::JNE;
                    break;
                case Quaternary::JNE:
                    quaternarytable_[i - 1].op_ = Quaternary::JE;
                    break;
                case Quaternary::JG:
                    quaternarytable_[i - 1].op_ = Quaternary::JNG;
                    break;
                case Quaternary::JNG:
                    quaternarytable_[i - 1].op_ = Quaternary::JG;
                    break;
                case Quaternary::JL:
                    quaternarytable_[i - 1].op_ = Quaternary::JNL;
                    break;
                case Quaternary::JNL:
                    quaternarytable_[i - 1].op_ = Quaternary::JL;
                    break;
                default:
                    assert(false);
                    break;
            } // end of switch
            // 更改条件跳转语句的跳转标号
            quaternarytable_[i - 1].dst_ = quaternarytable_[i].dst_;
            // 取消JMP指令
            quaternarytable_[i].op_ = Quaternary::NIL_OP;

        }
        // 试图取消Label指令 详见【Appendix1设计备注 24-4-1】
        else if (Quaternary::LABEL == quaternarytable_[i].op_) {
            used = false;
            for (size_t j = begin_quaternary_index; j < i; ++j) {
                switch (quaternarytable_[j].op_) {
                    case Quaternary::JMP:
                    case Quaternary::JE:
                    case Quaternary::JNE:
                    case Quaternary::JG:
                    case Quaternary::JNG:
                    case Quaternary::JL:
                    case Quaternary::JNL:
                        if (quaternarytable_[i].dst_ == quaternarytable_[j].dst_) {
                            used = true;
                        }
                        break;
                    default:
                        break;
                }
                if (used) {
                    break;
                }
            }
            if (!used) // 这个Label已经没用啦
            {
                quaternarytable_[i].op_ = Quaternary::NIL_OP;
            }
        } // end of if
    }     // end of for
}

//// <条件> ::= <表达式><关系运算符><表达式>
//// 由if或for语句中传递下来label参数，标识if语句块或for循环体的结束
//// 用于在处理condition时设置跳转语句
//void MidCodeGenerator::Condition(int endlabel, size_t depth) throw()				// 条件
//{
//	PrintFunctionFrame("Condition()", depth);
//
//	ExpressionAttribute left_attribute = Expression(depth + 1);
//	// 化简数组元素为临时变量
//	SimplifyArrayOperand(left_attribute);
//
//	bool isonlyexpression = false;
//	// 生成有条件的跳转语句
//	// 不符合条件时才会跳转，所以这里的操作符与读到的token要反一下
//	Quaternary q_jmp_condition;
//	switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是THEN（当条件中仅有一个表达式时）
//	{
//	case Token::LT:
//		q_jmp_condition.op_ = Quaternary::JNL;
//		break;
//	case Token::LEQ:
//		q_jmp_condition.op_ = Quaternary::JG;
//		break;
//	case Token::GT:
//		q_jmp_condition.op_ = Quaternary::JNG;
//		break;
//	case Token::GEQ:
//		q_jmp_condition.op_ = Quaternary::JL;
//		break;
//	case Token::EQU:
//		q_jmp_condition.op_ = Quaternary::JNE;
//		break;
//	case Token::NEQ:
//		q_jmp_condition.op_ = Quaternary::JE;
//		break;
//	case Token::THEN:
//		q_jmp_condition.op_ = Quaternary::JE;
//		isonlyexpression = true;
//		break;
//	default:
//		assert(false);
//		break;
//	}
//	ExpressionAttribute right_attribute;
//	if(!isonlyexpression)	// 如果还有下一个表达式，再继续读取
//	{
//		lexical_analyzer_.GetNextToken(token_);
//		right_attribute = Expression(depth + 1);
//		// 化简数组为临时变量
//		SimplifyArrayOperand(right_attribute);
//	}
//	else
//	{
//		right_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
//		right_attribute.value_ = 0;
//		right_attribute.offset_addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
//		right_attribute.offset_ = 0;
//		right_attribute.decoratetype_ = TokenTableItem::VOID;
//	}
//	// 操作数
//	q_jmp_condition.method1_ = left_attribute.addressingmethod_;
//	q_jmp_condition.src1_ = left_attribute.value_;
//	q_jmp_condition.method2_ = right_attribute.addressingmethod_;
//	q_jmp_condition.src2_ = right_attribute.value_;
//	q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
//	q_jmp_condition.dst_ = endlabel;
//	// 保存四元式
//	quaternarytable_.push_back(q_jmp_condition);
//	// 回收临时变量
//	if(Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_)
//	{
//		--tempvar_index_;
//	}
//	if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
//	{
//		--tempvar_index_;
//	}
//}


// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void MidCodeGenerator::CaseStatement(size_t depth) throw() // 情况语句
{
    PrintFunctionFrame("CaseStatement()", depth);

    assert(Token::CASE == token_.type_);

    // 读取表达式
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute exp_attribute = Expression(depth + 1);
    // 将表达式化为非数组元素
    SimplifyArrayOperand(exp_attribute);

    assert(Token::OF == token_.type_);
    // 为END处申请一个label
    int endlabel = label_index_++;
    // case表达式之后的跳转语句的插入位置
    int jmp_insertion_location = quaternarytable_.size();
    // 若干个JE的跳转语句
    Quaternary q_jmp;
    q_jmp.op_      = Quaternary::JE;
    q_jmp.method1_ = exp_attribute.addressingmethod_;
    q_jmp.src1_    = exp_attribute.value_;
    q_jmp.method2_ = Quaternary::IMMEDIATE_ADDRESSING;
    q_jmp.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
    do {
        lexical_analyzer_.GetNextToken(token_);
        // 为当前情况表元素申请一个label
        int caselabel             = label_index_++;
        vector<int> constant_list = CaseElement(caselabel, endlabel, depth + 1);
        // 插入JE跳转语句
        q_jmp.dst_ = caselabel;
        for (vector<int>::const_iterator c_iter = constant_list.begin();
             c_iter != constant_list.end(); ++c_iter) {
            q_jmp.src2_ = *c_iter;
            quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location++, q_jmp);
        }
    } while (Token::SEMICOLON == token_.type_);

    // 执行完所有JE后，跳转到END处
    Quaternary q_endjmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                        0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location, q_endjmp);

    // 插入结束的label
    Quaternary q_endlabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                          Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_endlabel);

    // 回收case后Expression的临时变量
    if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_) {
        --tempvar_index_;
    }

    // 检测结束标志
    assert(Token::END == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <情况表元素> ::= <情况常量表>:<语句>
// <情况常量表> ::=  <常量 | 常变量>{, <常量 | 常变量>}
vector<int> MidCodeGenerator::CaseElement(int caselabel, int endlabel, size_t depth) throw() // 情况表元素
{
    PrintFunctionFrame("CaseElement()", depth);

    vector<int> constant_list;
    TokenTable::iterator iter;
    bool first_item = true;
    do {
        if (!first_item) {
            lexical_analyzer_.GetNextToken(token_);
        } else {
            first_item = false;
        }
        iter = tokentable_.SearchDefinition(token_);
        assert(Token::CONST_INTEGER == token_.type_ || Token::CONST_CHAR == token_.type_ ||
               tokentable_.end() != iter);
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        // 记录该常量
        if (Token::CONST_INTEGER == token_.type_) {
            constant_list.push_back(token_.value_.integer);
        } else if (Token::CONST_CHAR == token_.type_) {
            constant_list.push_back(token_.value_.character);
        } else // 常变量
        {
            constant_list.push_back(iter->value_);
        }
        // 读取下一个单词
        lexical_analyzer_.GetNextToken(token_);
    } while (Token::COMMA == token_.type_);
    assert(Token::COLON == token_.type_);
    // 读入情况表元素的语句之前，打下一个label
    Quaternary q_caselabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                           caselabel);
    quaternarytable_.push_back(q_caselabel);

    lexical_analyzer_.GetNextToken(token_);
    Statement(depth + 1);

    // 情况表元素的语句执行完后，跳转到END处
    Quaternary q_endjmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                        0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_endjmp);
    // 返回记录的情况表元素的常量数组
    return constant_list;
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
// TODO 扩展出对数组的支持
void MidCodeGenerator::ReadStatement(size_t depth) throw() // 读语句
{
    PrintFunctionFrame("ReadStatement()", depth);

    assert(Token::READ == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::LEFT_PAREN == token_.type_);

    do {
        lexical_analyzer_.GetNextToken(token_);
        assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
        assert(iter != tokentable_.end());
        //iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录

        // 读取下一个单词，判断是否为数组元素
        lexical_analyzer_.GetNextToken(token_);
        if (Token::LEFT_BRACKET != token_.type_) // 不是数组元素
        {
            assert(TokenTableItem::VARIABLE == iter->itemtype_ ||
                   TokenTableItem::PARAMETER == iter->itemtype_); // 检查是否为变量或参数或函数名
            // 生成READ调用的四元式
            Quaternary q_read(Quaternary::READ, Quaternary::NIL_ADDRESSING, 0,
                              Quaternary::NIL_ADDRESSING, 0, Quaternary::VARIABLE_ADDRESSING,
                              distance(tokentable_.begin(),
                                       static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_read);
        } else // 数组元素
        {
            // 类型检查
            assert(TokenTableItem::ARRAY == iter->itemtype_); // 检查是否为数组变量
            // 读入数组下标的第一个单词
            lexical_analyzer_.GetNextToken(token_);
            // 读到整个下标的表达式
            ExpressionAttribute exp_attribute = Expression(depth + 1);
            // 非数组化
            SimplifyArrayOperand(exp_attribute);
            // 生成READ调用的四元式
            Quaternary q_read(Quaternary::READ, Quaternary::NIL_ADDRESSING, 0,
                              exp_attribute.addressingmethod_, exp_attribute.value_,
                              Quaternary::ARRAY_ADDRESSING,
                              distance(tokentable_.begin(),
                                       static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_read);
            // 判断右括号
            assert(Token::RIGHT_BRACKET == token_.type_);
            // 读入下一个单词
            lexical_analyzer_.GetNextToken(token_);
        }
    } while (Token::COMMA == token_.type_);

    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <写语句> ::= write'(' (<字符串>[,<表达式>] | <表达式>) ')'
void MidCodeGenerator::WriteStatement(size_t depth) throw() // 写语句
{
    PrintFunctionFrame("WriteStatement()", depth);

    assert(Token::WRITE == token_.type_);

#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    lexical_analyzer_.GetNextToken(token_);

    assert(Token::LEFT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);

    if (Token::CONST_STRING == token_.type_) {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        vector<string>::const_iterator iter =
            std::find(stringtable_.begin(), stringtable_.end(), token_.value_.identifier);
        // 生成WRITE调用的四元式
        Quaternary q_read(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                          Quaternary::NIL_ADDRESSING, 0, Quaternary::STRING_ADDRESSING,
                          distance(static_cast<vector<string>::const_iterator>(stringtable_.begin()),
                                   iter));
        quaternarytable_.push_back(q_read);

        lexical_analyzer_.GetNextToken(token_);
        if (Token::COMMA == token_.type_) {
            lexical_analyzer_.GetNextToken(token_);
            ExpressionAttribute exp_attribute = Expression(depth + 1); // 读取第二个参数（表达式）
            // 生成WRITE调用的四元式
            Quaternary q_write(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                               exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
                               exp_attribute.addressingmethod_, exp_attribute.value_);
            // 在write的四元式末尾一定要有decoratetype的属性，用作输出时的类型推导
            q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
            quaternarytable_.push_back(q_write);
            // 回收临时变量
            if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_ ||
                Quaternary::TEMPORARY_ADDRESSING ==
                    exp_attribute.offset_addressingmethod_) // 两者不可能同时成立，故写在一起
            {
                --tempvar_index_;
            }
        }
    } else {
        ExpressionAttribute exp_attribute = Expression(depth + 1);
        // 生成WRITE调用的四元式
        Quaternary q_write(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                           exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
                           exp_attribute.addressingmethod_, exp_attribute.value_);
        // 在write的四元式末尾一定要有decoratetype的属性，用作输出时的类型推导
        q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
        quaternarytable_.push_back(q_write);
        // 回收临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_ ||
            Quaternary::TEMPORARY_ADDRESSING ==
                exp_attribute.offset_addressingmethod_) // 两者不可能同时成立，故写在一起
        {
            --tempvar_index_;
        }
    }

    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <while循环语句> ::= while <条件> do <语句>
// <while循环语句> ::= while @Label<check> <条件> do @Label<BeginDo> <语句> @JMPLabel<check> @Label<end>
void MidCodeGenerator::WhileLoopStatement(size_t depth) throw() // while循环语句
{
    PrintFunctionFrame("WhileLoopStatement()", depth);
    assert(Token::WHILE == token_.type_);

    // BUG! 不能在这里放，一定要放在checklabel的后面
    // 否则由于checklabel是在后面被引用的，就会在SpecialOptimize中把checklabel优化掉
    //size_t begin_quaternary_index = quaternarytable_.size();	// 四元式的起始位置

    // 申请条件语句前面的label<check>和结束时的label<end>
    int checklabel    = label_index_++;
    int label_begindo = label_index_++;
    int label_enddo   = label_index_++;
    // 更新continue_label_栈与break_label_栈
    continue_label_.push(checklabel);
    break_label_.push(label_enddo);
    // 放下label<check>
    Quaternary q_checklabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                            Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                            checklabel);
    quaternarytable_.push_back(q_checklabel);
    // Condition四元式的起始位置
    size_t begin_quaternary_index = quaternarytable_.size();
    // 读取下一个单词，并进入条件语句
    lexical_analyzer_.GetNextToken(token_);
    bool multi_jmp =
        Condition(label_begindo, label_enddo, depth + 1); // 条件语句中会执行动作@JZLabel<end>
    // 语法检查
    assert(Token::DO == token_.type_);
    // 不能进行单条件优化的情况
    if (multi_jmp) {
        // 测试前一句是否为到label_begindo的无条件跳转语句，若是，则可优化
        if (Quaternary::JMP == quaternarytable_.back().op_ &&
            label_begindo == quaternarytable_.back().dst_) {
            quaternarytable_.pop_back();
        }
        // 设置label_begindo
        TryLabel(begin_quaternary_index, label_begindo);
    }
    // 对Condition中某些语句做优化（见【Appendix1设计备注 24-4】）
    SpecialOptimize(begin_quaternary_index);

    // 读入循环体的第一个单词
    lexical_analyzer_.GetNextToken(token_);
    // 读入循环体
    Statement(depth + 1);
    // 更新continue_label_栈与break_label_栈
    continue_label_.pop();
    break_label_.pop();
    // 循环体结束后的跳转
    Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0,
                     Quaternary::IMMEDIATE_ADDRESSING, checklabel);
    quaternarytable_.push_back(q_jmp);
    // 放下结束的label
    Quaternary q_label_enddo(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_enddo);
    quaternarytable_.push_back(q_label_enddo);
}

// <for循环语句> ::= for <标识符> := <表达式> （downto | to） <表达式> do <语句>
// <for循环语句> ::= for <标识符> := <表达式> （downto | to）
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <表达式> @JZLABEL<end>
// do <语句>@JMPLABEL<vary>@Label<end>
void MidCodeGenerator::ForLoopStatement(size_t depth) throw() // for循环语句
{
    PrintFunctionFrame("ForLoopStatement()", depth);

    assert(Token::FOR == token_.type_);

    // 读取标识符
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    TokenTable::iterator loopvar_iter = tokentable_.SearchDefinition(token_);
    assert(loopvar_iter != tokentable_.end());
    //loopvar_iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
    assert(TokenTableItem::VARIABLE == loopvar_iter->itemtype_ ||
           TokenTableItem::PARAMETER == loopvar_iter->itemtype_); // 检查是否为变量或参数
    // 读取赋值号
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::ASSIGN == token_.type_);
    // 读取表达式
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute init_attribute = Expression(depth + 1);

    // 检测to/downto
    assert(Token::DOWNTO == token_.type_ || Token::TO == token_.type_);
    // 保存递增/减符号
    Token vary_token = token_;
    // 申请循环变量的递增/减的label<vary>, 边界检查的label<check>，以及末尾的label<end>
    int varylabel  = label_index_++;
    int checklabel = label_index_++;
    int endlabel   = label_index_++;
    // 更新continue_label_栈与break_label_栈
    continue_label_.push(varylabel);
    break_label_.push(endlabel);
    // 生成for的循环变量的初始化（赋值）四元式
    Quaternary q_init(Quaternary::ASG, init_attribute.addressingmethod_, init_attribute.value_,
                      init_attribute.offset_addressingmethod_, init_attribute.offset_,
                      Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)));
    quaternarytable_.push_back(q_init);
    // 生成跳转到边界检查的JMP四元式
    Quaternary q_jmpcheck(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                          0, Quaternary::IMMEDIATE_ADDRESSING, checklabel);
    quaternarytable_.push_back(q_jmpcheck);
    // 生成循环变量递增/减的label<vary>
    Quaternary q_varylabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                           varylabel);
    quaternarytable_.push_back(q_varylabel);
    // 生成循环变量递增/减的四元式
    Quaternary q_vary(Token::TO == vary_token.type_ ? Quaternary::ADD : Quaternary::SUB,
                      Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)),
                      Quaternary::IMMEDIATE_ADDRESSING, 1, Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)));
    quaternarytable_.push_back(q_vary);
    // 生成循环变量边界检查的label<check>
    Quaternary q_checklabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                            Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                            checklabel);
    quaternarytable_.push_back(q_checklabel);

    // 读取表达式
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute bound_attribute = Expression(depth + 1);

    // 化简至非数组元素表达式
    SimplifyArrayOperand(bound_attribute);
    // 生成边界检查的JMP四元式
    Quaternary q_check(Token::TO == vary_token.type_ ? Quaternary::JG : Quaternary::JL,
                       Quaternary::VARIABLE_ADDRESSING,
                       distance(tokentable_.begin(),
                                static_cast<TokenTable::const_iterator>(loopvar_iter)),
                       bound_attribute.addressingmethod_, bound_attribute.value_,
                       Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_check);

    // 回收两个Expression的临时变量
    if (Quaternary::TEMPORARY_ADDRESSING == init_attribute.addressingmethod_ ||
        Quaternary::TEMPORARY_ADDRESSING == init_attribute.offset_addressingmethod_) {
        --tempvar_index_;
    }
    if (Quaternary::TEMPORARY_ADDRESSING == bound_attribute.addressingmethod_) {
        --tempvar_index_;
    }

    // 读取DO
    assert(Token::DO == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    // 读取循环体
    Statement(depth + 1);
    // 更新continue_label_栈与break_label_栈
    continue_label_.pop();
    break_label_.pop();
    // 跳转回循环变量递增/减的label<vary>
    Quaternary q_jmpvary(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                         0, Quaternary::IMMEDIATE_ADDRESSING, varylabel);
    quaternarytable_.push_back(q_jmpvary);
    // for循环结束的label<end>
    Quaternary q_endlabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                          Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_endlabel);
}

void MidCodeGenerator::ContinueStatement(size_t depth) throw() // continue
{
    PrintFunctionFrame("ContinueStatement()", depth);
    assert(Token::CONTINUE == token_.type_);
    assert(continue_label_.size() > 0);
    Quaternary q_continue(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                          0, Quaternary::IMMEDIATE_ADDRESSING, continue_label_.top());
    quaternarytable_.push_back(q_continue);
    // 读入下一个单词并返回
    lexical_analyzer_.GetNextToken(token_);
}
void MidCodeGenerator::BreakStatement(size_t depth) throw() // break
{
    PrintFunctionFrame("BreakStatement()", depth);
    assert(Token::BREAK == token_.type_);
    assert(break_label_.size() > 0);
    Quaternary q_break(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                       0, Quaternary::IMMEDIATE_ADDRESSING, break_label_.top());
    quaternarytable_.push_back(q_break);
    // 读入下一个单词并返回
    lexical_analyzer_.GetNextToken(token_);
}


// <过程/函数调用语句> ::= '('[<实在参数表>]')'
void MidCodeGenerator::ProcFuncCallStatement(const Token proc_token,
                                             const vector<ExpressionAttribute>& parameter_attributes,
                                             size_t depth) // 过程调用语句
{
    PrintFunctionFrame("ProcFuncCallStatement()", depth);
    // 语法检查
    assert(Token::LEFT_PAREN == token_.type_);
    // 语法：读入右括号或参数表的第一个单词
    lexical_analyzer_.GetNextToken(token_);
    if (parameter_attributes.size() == 0) // 如果函数本身就没有参数的话，就不用读参数了
    {
        // 语法检查
        assert(Token::RIGHT_PAREN == token_.type_);
        lexical_analyzer_.GetNextToken(token_); // 读函数调用完毕后的下一个单词
        return;
    }
    // 语法：读参数，并生成设置参数的四元式
    ArgumentList(parameter_attributes, depth + 1);
    // 语法检查
    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <实在参数表> ::= <表达式>{,<表达式>}
void MidCodeGenerator::ArgumentList(const vector<ExpressionAttribute>& parameter_attributes,
                                    size_t depth) throw() // 实参表
{
    PrintFunctionFrame("ArgumentList()", depth);

    ExpressionAttribute argument_attribute;
    Quaternary q_addpara;
    q_addpara.method1_ = Quaternary::NIL_ADDRESSING;
    q_addpara.src1_    = 0;
    size_t para_index  = 0;
    do {
        if (para_index > 0) {
            lexical_analyzer_.GetNextToken(token_);
        }
        argument_attribute = Expression(depth + 1);
        //attribute_buffer.push_back(argument_attribute);
        // 生成设置参数的四元式
        // 参数要求是引用，且表达式为非引用，才用SETREFP
        // 重点在于，若表达式也为引用，则只用SETP，即可传入地址
        if (Quaternary::REFERENCE_ADDRESSING == parameter_attributes[para_index].addressingmethod_ &&
            Quaternary::REFERENCE_ADDRESSING != argument_attribute.addressingmethod_) {
            q_addpara.op_ = Quaternary::SETREFP;
            // 引用传参要求参数是左值
            if (Quaternary::VARIABLE_ADDRESSING != argument_attribute.addressingmethod_ &&
                Quaternary::ARRAY_ADDRESSING != argument_attribute.addressingmethod_) {
                // 只有这一处可能出错
                std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString()
                          << "  should be left value to fit the reference parameter\n";
                is_successful_ = false;
                // 出错后标记即可，不需继续读取
                // 此时的四元式传入的是临时变量的地址，即使汇编之后运行，也不会出现运行时错误
            }
        } else {
            q_addpara.op_ = Quaternary::SETP;
        }
        q_addpara.method2_ = argument_attribute.offset_addressingmethod_;
        q_addpara.offset2_ = argument_attribute.offset_;
        q_addpara.method3_ = argument_attribute.addressingmethod_;
        q_addpara.dst_     = argument_attribute.value_;
        quaternarytable_.push_back(q_addpara);
        // 回收临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_ ||
            Quaternary::TEMPORARY_ADDRESSING ==
                argument_attribute.offset_addressingmethod_) // 两者不可能同时成立，故写在一起
        {
            --tempvar_index_;
        }
        ++para_index;
    } while (Token::COMMA == token_.type_);
}

// 给定的元素的操作数如果是数组变量，就将其化简为临时变量
void MidCodeGenerator::SimplifyArrayOperand(ExpressionAttribute& attribute) throw() {
    if (Quaternary::ARRAY_ADDRESSING == attribute.addressingmethod_) {
        Quaternary q_subscript2temp;
        q_subscript2temp.op_      = Quaternary::ASG;
        q_subscript2temp.method1_ = attribute.addressingmethod_;
        q_subscript2temp.src1_    = attribute.value_;
        q_subscript2temp.method2_ = attribute.offset_addressingmethod_;
        q_subscript2temp.offset2_ = attribute.offset_;
        q_subscript2temp.method3_ = Quaternary::TEMPORARY_ADDRESSING;
        // 目标的临时变量标号的确定
        // 如果数组下标就是临时变量，那么就用这个变量
        // 否则就新开一个临时变量
        if (Quaternary::TEMPORARY_ADDRESSING == attribute.offset_addressingmethod_) {
            q_subscript2temp.dst_ = attribute.offset_;
        } else {
            q_subscript2temp.dst_ = tempvar_index_++;
        }
        quaternarytable_.push_back(q_subscript2temp);
        attribute.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
        attribute.value_                   = q_subscript2temp.dst_;
        attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        attribute.offset_                  = 0;
    }
}
