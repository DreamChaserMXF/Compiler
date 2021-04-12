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

static string generating_format_string_; // ע�������̰߳�ȫ��
void MidCodeGenerator::PrintFunctionFrame(const char* func_name, size_t depth) throw() {
    if (depth * 4 == generating_format_string_.size()) {
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    } else if (depth * 4 > (int)generating_format_string_.size()) {
        generating_format_string_.append("|");
        generating_format_string_.append(depth * 4 - generating_format_string_.size(),
                                         ' '); // ���ﲻ�ܼ�1
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    } else // depth * 4 < generating_format_string_.size()
    {
        generating_format_string_.resize(depth * 4);
        generating_process_buffer_ << generating_format_string_ << func_name << '\n';
    }
}
// <����> ::= <�ֳ���>.
void MidCodeGenerator::Routine(size_t depth) throw() {
    PrintFunctionFrame("Routine()", depth);
    // ������������BEGIN
    Quaternary q_mainbegin(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::IMMEDIATE_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, -1);
    quaternarytable_.push_back(q_mainbegin);
    // �����ֳ���
    SubRoutine(depth + 1);
    // �жϽ�������
    if (token_.type_ != Token::PERIOD) {
        std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t'
                  << "should be '.' at the end of Routine\n";
        is_successful_ = false;
    }
    // ������������END
    Quaternary q_mainend(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                         0, Quaternary::IMMEDIATE_ADDRESSING, -1);
    quaternarytable_.push_back(q_mainend);
    // ���¹���/������BEGIN����е���ʱ��������
    Quaternary::UpdateTempVarSpace(quaternarytable_);
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void MidCodeGenerator::SubRoutine(size_t depth) throw() {
    PrintFunctionFrame("SubRoutine()", depth);

    //
    // �ĸ���ѡ��֧
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
    // һ����ѡ��֧
    assert(token_.type_ == Token::BEGIN);
    StatementBlockPart(depth + 1);
}

// <����˵������> ::= const<��������>{,<��������>};
void MidCodeGenerator::ConstantPart(size_t depth) throw() {
    PrintFunctionFrame("ConstantPart()", depth);

    assert(Token::CONST == token_.type_);

    // ��������
    do {
        lexical_analyzer_.GetNextToken(token_);
        constantDefination(depth + 1);
    } while (token_.type_ == Token::COMMA);
    lexical_analyzer_.GetNextToken(token_);
}

// <��������> ::= <��ʶ��>��<����>
void MidCodeGenerator::constantDefination(size_t depth) throw() {
    PrintFunctionFrame("constantDefination()", depth);

    assert(token_.type_ == Token::IDENTIFIER);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // ��¼token_�Բ�����ű�
    Token constIdentifier = token_;
    lexical_analyzer_.GetNextToken(token_);
    assert(token_.type_ == Token::EQU);
    lexical_analyzer_.GetNextToken(token_);
    assert(token_.type_ == Token::CONST_INTEGER || token_.type_ == Token::CONST_CHAR);
    // �������壬������ű�
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

// <����˵������> ::= var <��������>;{<��������>;}
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

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
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
// <����> ::= [array'['<�޷�������>']'of]<��������>
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
    // ������ű�
    TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER)
        ? TokenTableItem::INTEGER
        : TokenTableItem::CHAR;             // ���η�����
    if (TokenTableItem::ARRAY == itemtype_) // ��������
    {
        for (vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end();
             ++iter) {
            tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
        }
    } else // ����һ�����
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

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void MidCodeGenerator::ProcedurePart(size_t depth) throw() {
    PrintFunctionFrame("ProcedurePart()", depth);

    do {
        int proc_index = ProcedureHead(depth + 1);
        SubRoutine(depth + 1);
        // ���ɹ��̵�END��Ԫʽ
        quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::IMMEDIATE_ADDRESSING, proc_index));
        tokentable_.Relocate();
        --level_;
        assert(token_.type_ == Token::SEMICOLON);
        lexical_analyzer_.GetNextToken(token_); // �ֳ��������Ӧ����ֺ�
    } while (Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
int MidCodeGenerator::ProcedureHead(size_t depth) throw() {
    PrintFunctionFrame("ProcedureHead()", depth);

    assert(Token::PROCEDURE == token_.type_);

    int proc_index = -1; // �������ڷ��ű��е�λ�ã��±꣩

    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // ��������������ű�
    string proc_name = token_.value_.identifier;
    proc_index       = tokentable_.AddProcedureItem(token_, level_++); // ������֮��leveҪ+1
    // ���ɹ��̵�BEGIN��Ԫʽ
    quaternarytable_.push_back(
        Quaternary(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                   0, // ����Ĳ�����Ӧ���Ǹù���Ҫ�õ�����ʱ�������������ȹ��̷��������
                   Quaternary::IMMEDIATE_ADDRESSING, proc_index));
    // ��λ��������������ڵ�λ����Ϊ�ֲ����������ʼ�㣩
    tokentable_.Locate();
    // ������ȡ����
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

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void MidCodeGenerator::FunctionPart(size_t depth) throw() {
    PrintFunctionFrame("FunctionPart()", depth);
    do {
        int func_index = FunctionHead(depth + 1); // ���к���ͷ���������õ��������ڷ��ű��е�λ��
        SubRoutine(depth + 1);
        // ���ɺ�����END��Ԫʽ
        quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::NIL_ADDRESSING, 0,
                                              Quaternary::IMMEDIATE_ADDRESSING, func_index));
        tokentable_.Relocate();
        --level_;
        assert(Token::SEMICOLON == token_.type_); // �ֳ��������Ӧ����ֺ�
        lexical_analyzer_.GetNextToken(token_);   // �����β�ķֺ�
    } while (Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
int MidCodeGenerator::FunctionHead(size_t depth) throw() {
    PrintFunctionFrame("FunctionHead()", depth);

    assert(Token::FUNCTION == token_.type_);

    int func_index = -1; // �������ڷ��ű��е�λ��

    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // ���뺯���������ű�
    string func_name = token_.value_.identifier;
    func_index       = tokentable_.AddFunctionItem(token_, level_++); // ������֮��leveҪ+1
    // ���ɺ�����BEGIN��Ԫʽ
    quaternarytable_.push_back(
        Quaternary(Quaternary::BEGIN, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                   0, // ����Ĳ�����Ӧ���Ǹú���Ҫ�õ�����ʱ�������������Ⱥ������������
                   Quaternary::IMMEDIATE_ADDRESSING, func_index));
    // ��λ
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

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
int MidCodeGenerator::ParameterList(size_t depth) throw() // �βα�
{
    PrintFunctionFrame("ParameterList()", depth);

    int sum = ParameterTerm(depth + 1);
    while (Token::SEMICOLON == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
        sum += ParameterTerm(depth + 1);
    }
    return sum;
}

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
int MidCodeGenerator::ParameterTerm(size_t depth) throw() {
    PrintFunctionFrame("ParameterTerm()", depth);
    bool isref = false; // �Ƿ�Ϊ���ô���
    if (Token::VAR == token_.type_) {
        isref = true;
        lexical_analyzer_.GetNextToken(token_);
    }
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    // ������ѹջ׼�������ű�
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
    // ��������ű�
    TokenTableItem::DecorateType decoratetype =
        (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
    for (vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end();
         ++iter) {
        tokentable_.AddParameterItem(*iter, decoratetype, isref, level_);
    } // end ����ű�

    lexical_analyzer_.GetNextToken(token_);
    return sum;
}


// <�������> ::= begin <���>{;<���>} end
void MidCodeGenerator::StatementBlockPart(size_t depth) throw() // �������
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

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>
// |<�����>|<д���>|<whileѭ�����>|<forѭ�����>|<ѭ���������>|<ѭ���˳����>|<��>
void MidCodeGenerator::Statement(size_t depth) throw() {
    PrintFunctionFrame("Statement()", depth);

    Token idToken = token_; // ��token_�����ǹ��������ȼ��£�����
    TokenTable::iterator iter;
    switch (token_.type_) {
        case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
            generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                       << std::endl;
#endif
            iter = tokentable_.SearchDefinition(token_); // ���ҷ��ű��еĶ���
            assert(iter != tokentable_.end());
            //iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
            lexical_analyzer_.GetNextToken(token_);
            if (Token::LEFT_PAREN == token_.type_) // ���̻�������
            {
                assert(TokenTableItem::PROCEDURE == iter->itemtype_ ||
                       TokenTableItem::FUNCTION == iter->itemtype_); // ����������Ƿ�Ϊ���̻���
                vector<ExpressionAttribute> proc_func_attributes =
                    tokentable_.GetProcFuncParameterAttributes(iter);
                // ��ProcFuncCallStatement�����ò���
                ProcFuncCallStatement(idToken, proc_func_attributes, depth + 1);
                // ���ɵ�����Ԫʽ
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
            // ��������Ƿ�Ϸ���Ӧ�úϷ���
        case Token::SEMICOLON: // �����
        case Token::END:       // �����
        default:
            break;
    }
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void MidCodeGenerator::AssigningStatement(const Token& idToken,
                                          TokenTable::iterator& iter,
                                          size_t depth) // ��ֵ���
{
    PrintFunctionFrame("AssigningStatement()", depth);

    // Ϊ��Ԫʽ���ɶ�����ı���
    bool assign2array = false; // �Ƿ�Ϊ������ĸ�ֵ����

    ExpressionAttribute offset_attribute; // ��������Ԫ�ظ�ֵʱ���洢ƫ�����������±꣩������

    if (Token::LEFT_BRACKET == token_.type_) // ������Ԫ�ظ�ֵ
    {
        assign2array = true;
        // ������
        assert(TokenTableItem::ARRAY == iter->itemtype_); // ����Ƿ�Ϊ������
        // �����ʾ�±�ı��ʽ
        lexical_analyzer_.GetNextToken(token_);
        offset_attribute = Expression(depth + 1);
        // �������±���������Ԫ��
        // �������Ԫʽ���������±�ֵ����һ����ʱ����
        SimplifyArrayOperand(offset_attribute);
        // �﷨���
        assert(Token::RIGHT_BRACKET == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
    }
    // ʣ��ֻ������������������������Ǻ�������ֵ
    else if (iter->itemtype_ != TokenTableItem::VARIABLE &&
             iter->itemtype_ != TokenTableItem::PARAMETER &&
             iter->itemtype_ != TokenTableItem::FUNCTION) {
        assert(false);
    }
    // �﷨���
    assert(Token::ASSIGN == token_.type_);
    // ���븳ֵ���ұߵı��ʽ
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute right_attribute = Expression(depth + 1);

    // �����飺����ת��
    // ���ﲻ����assert����Ϊ��ֻ�ǵ����ؾ������
    //assert(iter->decoratetype_ >= right_attribute.decoratetype_);
    // �м��������
    if (assign2array) // ������Ԫ�ظ�ֵ
    {
        // ���right_attribute������Ԫ�صĻ���Ҫ���丳ֵ����ʱ����
        SimplifyArrayOperand(right_attribute);
        // �������鸳ֵ
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

        // ����Ҳ���������ʱ�������ɻ���
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        // �������ֵ�������±�Ҳ����ʱ�������ɻ���
        if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.addressingmethod_) {
            --tempvar_index_;
        }
    } else if (TokenTableItem::PARAMETER == iter->itemtype_ ||
               TokenTableItem::VARIABLE == iter->itemtype_) // ��ͨ����/�����ĸ�ֵ
    {
        // �����ֵ���ұߵı��ʽ����ʱ����������Ż�����ǰ�ĸ�ֵ��䡣
        // �����Appendix1 ��Ʊ�ע�� - chapter 4
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            quaternarytable_.back().method3_ =
                iter->isref_ ? Quaternary::REFERENCE_ADDRESSING : Quaternary::VARIABLE_ADDRESSING;
            quaternarytable_.back().dst_ =
                std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
            // �����Ҳ���������ʱ����
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
            // �����ֵ���ұߵı��ʽ�����飬���������±�����ʱ�������ɻ���
            if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_) {
                // ������һ������³���Եļٶ����������if�����ڣ����addressingmethod_����ARRAY�Ļ�����ôaddressingmethod_һ����NIL_ADDRESSING
                // �����������assert���һ�³����߼���������
                assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
                --tempvar_index_;
            }
        }
    } else // ��������ֵ
    {
        Quaternary q_ret;
        q_ret.op_      = Quaternary::RET;
        q_ret.method2_ = right_attribute.offset_addressingmethod_;
        q_ret.offset2_ = right_attribute.offset_;
        q_ret.method3_ = right_attribute.addressingmethod_;
        q_ret.dst_     = right_attribute.value_;
        quaternarytable_.push_back(q_ret);
        // �����ֵ���ұߵı��ʽ����ʱ�������ɻ���
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        // �����ֵ���ұߵı��ʽ�����飬���������±�����ʱ�������ɻ���
        else if (Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_) {
            // ������һ������³���Եļٶ��������addressingmethod_����ARRAY�Ļ�����ôaddressingmethod_һ����NIL_ADDRESSING
            // �����������assert���һ�³����߼���������
            assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
            --tempvar_index_;
        }
    }
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
ExpressionAttribute MidCodeGenerator::Expression(size_t depth) throw() // ���ʽ
{
    PrintFunctionFrame("Expression()", depth);

    Quaternary q_neg; // �������ɵ�NEG��Ԫʽ
    if (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        if (Token::MINUS == token_.type_) // ����Ǽ��ţ��Ϳ��ܻ�����һ����Ԫʽ
        {
            q_neg.op_ = Quaternary::NEG;
        }
        lexical_analyzer_.GetNextToken(token_);
    }

    ExpressionAttribute first_term = Term(depth + 1);
    //bool isref = first_term.isref_;
    if (Quaternary::NEG == q_neg.op_) // ���֮ǰ������һ������
    {
        // ����ȡ�����Ż�
        if (Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_) {
            first_term.value_ = -first_term.value_;
        } else // ����NEG����Ԫʽ
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
            // �޸�first_term������(NEG������Ӱ��decoratetype)
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
        // ��һ��ʱ�����new_term������Ԫ�أ���Ҫ���丳ֵ����ʱ����
        if (is_first_operator) {
            SimplifyArrayOperand(first_term);
        }

        // ȷ����Ԫʽ�Ĳ�����
        q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;

        // ��ȡ��һ��
        lexical_analyzer_.GetNextToken(token_);
        TokenTableItem::DecorateType last_term_decoratetype =
            is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
        new_term = Term(depth + 1);

        // ���������ִ������ת��
        new_term.decoratetype_ =
            TokenTableItem::TypeConversionMatrix[last_term_decoratetype][new_term.decoratetype_];

        // ���������new_term��������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
        // ������Ԫ�ص�ֵ������ʱ����
        SimplifyArrayOperand(new_term);

        // ȷ����Ԫʽ�Ĳ�����
        // src1��ȷ����
        // ��һ�μ�/��ʱ��src1����while֮ǰ������Ǹ�term
        // ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
        if (is_first_operator) {
            // �����������/�����Ż���ֱ���ڱ���ʱ������
            if (Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_ &&
                Quaternary::IMMEDIATE_ADDRESSING == new_term.addressingmethod_) {
                first_term.decoratetype_ = new_term.decoratetype_;
                first_term.value_        = (Quaternary::ADD == q_term.op_)
                    ? (first_term.value_ + new_term.value_)
                    : (first_term.value_ - new_term.value_);
                continue;
            }
            // ��������
            q_term.method1_   = first_term.addressingmethod_;
            q_term.src1_      = first_term.value_;
            is_first_operator = false;
        } else {
            q_term.method1_ = q_term.method3_;
            q_term.src1_    = q_term.dst_;
        }
        // src2��ȷ����
        // src2���Ƕ������µ�term
        q_term.method2_ = new_term.addressingmethod_;
        q_term.src2_    = new_term.value_;
        // dst��ȷ����
        // ���src1����ʱ����������dstΪsrc1
        // �������src2����ʱ����������dstΪsrc2
        // ������dstΪ�µ���ʱ����
        if (Quaternary::TEMPORARY_ADDRESSING == q_term.method1_) {
            q_term.method3_ = q_term.method1_;
            q_term.dst_     = q_term.src1_;
            // ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
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
        // ������Ԫʽ
        quaternarytable_.push_back(q_term);
    }


    // �������յı��ʽ����
    if (is_first_operator) // ֻ��һ������
    {
        new_term = first_term;
    } else // �ж���������Ҫ����new_term�����ԡ�����new_term����decoratetype_�⣬���඼�����һ�������
    {
        new_term.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
        new_term.value_                   = q_term.dst_;
        new_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        new_term.offset_                  = 0;
    }
    return new_term;
}

// <��> ::= <����>{<�˷������><����>}
ExpressionAttribute MidCodeGenerator::Term(size_t depth) throw() // ��
{
    PrintFunctionFrame("Term()", depth);

    ExpressionAttribute first_factor = Factor(depth + 1);

    ExpressionAttribute new_factor;
    Quaternary q_factor;
    bool is_first_operator = true;
    while (token_.type_ == Token::MUL || token_.type_ == Token::DIV) {
        // ��һ��ʱ�����first_factor������Ԫ�أ���Ҫ���丳ֵ����ʱ����
        if (is_first_operator) {
            SimplifyArrayOperand(first_factor);
        }

        // ȷ����Ԫʽ�Ĳ�����
        q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

        // �﷨��������ȡ��һ��
        lexical_analyzer_.GetNextToken(token_);
        TokenTableItem::DecorateType last_factor_decoratetype =
            is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
        new_factor = Factor(depth + 1);
        // ���������ִ������ת��
        new_factor.decoratetype_ =
            TokenTableItem::TypeConversionMatrix[last_factor_decoratetype][new_factor.decoratetype_];

        // ����������������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
        // ������Ԫ�ص�ֵ������ʱ����
        SimplifyArrayOperand(new_factor);

        // ȷ����Ԫʽ�Ĳ�����
        // src1��ȷ����
        // ��һ�γ�/����src1����while֮ǰ������Ǹ�factor
        // ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
        if (is_first_operator) {
            // �����������/�����Ż���ֱ���ڱ���ʱ������
            if (Quaternary::IMMEDIATE_ADDRESSING == first_factor.addressingmethod_ &&
                Quaternary::IMMEDIATE_ADDRESSING == new_factor.addressingmethod_) {
                first_factor.decoratetype_ = new_factor.decoratetype_;
                first_factor.value_        = (Quaternary::MUL == q_factor.op_)
                    ? (first_factor.value_ * new_factor.value_)
                    : (first_factor.value_ / new_factor.value_);
                continue;
            }
            // ��������
            q_factor.method1_ = first_factor.addressingmethod_;
            q_factor.src1_    = first_factor.value_;
            is_first_operator = false;
        } else {
            q_factor.method1_ = q_factor.method3_;
            q_factor.src1_    = q_factor.dst_;
        }
        // src2��ȷ����
        // src2�Ƕ������µ�factor
        q_factor.method2_ = new_factor.addressingmethod_;
        q_factor.src2_    = new_factor.value_;
        // dst��ȷ����
        // ���src1����ʱ����������dstΪsrc1
        // �������src2����ʱ����������dstΪsrc2
        // ������dstΪ�µ���ʱ����
        if (Quaternary::TEMPORARY_ADDRESSING == q_factor.method1_) {
            q_factor.method3_ = q_factor.method1_;
            q_factor.dst_     = q_factor.src1_;
            // ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
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
        // ������Ԫʽ
        quaternarytable_.push_back(q_factor);
    }

    // �����������
    if (is_first_operator) // ֻ��һ������
    {
        new_factor = first_factor;
    } else {
        // ����new_factor������
        // ����new_factor��decoratetype�⣬�������Ծ����������һ�����ӵ�����
        new_factor.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
        new_factor.value_                   = q_factor.dst_;
        new_factor.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
        new_factor.offset_                  = 0;
    }
    return new_factor;
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')'
//          | <�޷�������>
//          | <�ַ�>
ExpressionAttribute MidCodeGenerator::Factor(size_t depth) throw() // ����
{
    PrintFunctionFrame("Factor()", depth);
    ExpressionAttribute factor_attribute; // ��¼��factor���ӵ���Ϣ

    // �﷨��飺��ʶ��������������������������顢�������á�
    if (Token::IDENTIFIER == token_.type_) {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        // ������
        TokenTable::iterator iter = tokentable_.SearchDefinition(token_); // Ѱ�Ҷ���
        assert(iter != tokentable_.end());
        // ���壺��¼�����������������ͣ������·��ű�
        factor_attribute.decoratetype_ = iter->decoratetype_;
        //iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
        Token idToken = token_; // ���£�����
        lexical_analyzer_.GetNextToken(token_);
        // �﷨���
        if (Token::LEFT_BRACKET == token_.type_) // �������ţ�����Ԫ��
        {
            // factor_attribute�Լ���������ֵ
            factor_attribute.addressingmethod_ = Quaternary::ARRAY_ADDRESSING;
            factor_attribute.value_ =
                std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
            // �����飺�Ƿ�Ϊ������
            assert(TokenTableItem::ARRAY == iter->itemtype_);
            // �﷨��������Ϊ�±�ı��ʽ
            lexical_analyzer_.GetNextToken(token_);
            ExpressionAttribute offset_attribute = Expression(depth + 1);
            // �м��������
            // ȷ��factor_attribute���±�
            // �����offset_attribute����������Ԫ�أ�����ṹ��Ƕ�׵������±꣨�������±�����һ������Ԫ�أ����޷��������Ԫʽ
            // ��������飬��Ҫ��offset_attribute�浽��ʱ�����У���Ϊ��ǰfactor���±�
            SimplifyArrayOperand(offset_attribute);
            factor_attribute.offset_addressingmethod_ = offset_attribute.addressingmethod_;
            factor_attribute.offset_                  = offset_attribute.value_;
            // �﷨���
            assert(Token::RIGHT_BRACKET == token_.type_);
            // �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
            lexical_analyzer_.GetNextToken(token_);
        } else if (Token::LEFT_PAREN == token_.type_) // �����ţ���������
        {
            // �����飺�Ƿ�Ϊ����
            assert(TokenTableItem::FUNCTION == iter->itemtype_);
            // ���壺����ƥ��
            // �ӷ��ű���ȡ�������Ĳ������ͣ���FunctionCallStatementȥƥ�����
            vector<ExpressionAttribute> parameter_attributes =
                tokentable_.GetProcFuncParameterAttributes(iter);
            // ��Ԫʽ���Ƚ���ProcFuncCallStatement�����úò�����Ȼ�������ɺ����������
            ProcFuncCallStatement(idToken, parameter_attributes, depth + 1);
            // ���ɺ������õ���Ԫʽ
            Quaternary q_functioncall(Quaternary::FUNC_CALL, Quaternary::NIL_ADDRESSING, 0,
                                      Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                      distance(tokentable_.begin(),
                                               static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_functioncall);
            // ���������������У�ϣ���������ķ���ֵ������temp#tempvar_index��λ��
            // �����Ӻ����У��޷��ж�temp#tempvar_index��λ��
            // �����Ӻ���������ֵ�洢��EAX�У��ټ�һ��ָ���EAX��ֵ�����ʱ������
            Quaternary q_store(Quaternary::STORE, Quaternary::NIL_ADDRESSING, 0,
                               Quaternary::NIL_ADDRESSING, 0, Quaternary::TEMPORARY_ADDRESSING,
                               tempvar_index_++);
            quaternarytable_.push_back(q_store);
            // ����factor_attribute����������ʱ����
            factor_attribute.addressingmethod_        = Quaternary::TEMPORARY_ADDRESSING;
            factor_attribute.value_                   = q_store.dst_;
            factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
            factor_attribute.offset_                  = 0;
        } else // ����һ����ʶ��
        {
            // �����飺�Ƿ�Ϊ���������������/�����Ĳ���
            assert(TokenTableItem::VARIABLE == iter->itemtype_ ||
                   TokenTableItem::PARAMETER == iter->itemtype_ ||
                   TokenTableItem::CONST == iter->itemtype_);
            // factor_attribute������
            if (TokenTableItem::CONST == iter->itemtype_) // ������
            {
                // ����ֱ�ӽ�������ת�������������ͣ������Appendix1 ��Ʊ�ע��
                factor_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
                factor_attribute.value_                   = iter->value_;
                factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
                factor_attribute.offset_                  = 0;
            } else // һ�����
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
    } else if (Token::LEFT_PAREN == token_.type_) // �����������ı��ʽ
    {
        // bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
        // �����ʽ�ĵ�һ������
        lexical_analyzer_.GetNextToken(token_);
        // �ٶ�ȡ���ʽ
        factor_attribute = Expression(depth + 1); // ��¼����
        assert(Token::RIGHT_PAREN == token_.type_);
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_INTEGER == token_.type_) // �������泣��
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
    } else if (Token::CONST_CHAR == token_.type_) // �ַ������泣��
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

//// <�������> ::= if<����>then<���>[else<���>]
//void MidCodeGenerator::IfStatement(size_t depth) throw()				// �������
//{
//	PrintFunctionFrame("IfStatement()", depth);
//
//	assert(Token::IF == token_.type_);
//
//	// ������һ��label
//	int label1 = label_index_++;
//	// ��ȡ�������
//	lexical_analyzer_.GetNextToken(token_);
//	Condition(label1, depth + 1);	// ��condition��������ת���
//	assert(Token::THEN == token_.type_);
//	// ��ȡif�ɹ�������
//	lexical_analyzer_.GetNextToken(token_);
//	Statement(depth + 1);
//
//	// ��ȡelse�����
//	if(Token::ELSE == token_.type_)
//	{
//		// ����ڶ���label
//		int label2 =  label_index_++;
//		// ������������ת���
//		Quaternary q_jmp(Quaternary::JMP,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::IMMEDIATE_ADDRESSING, label2);
//		quaternarytable_.push_back(q_jmp);
//		// ���õ�һ��label
//		Quaternary q_label1(Quaternary::LABEL,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::IMMEDIATE_ADDRESSING, label1);
//		quaternarytable_.push_back(q_label1);
//		// ��ȡelse�е����
//		lexical_analyzer_.GetNextToken(token_);
//		Statement(depth + 1);
//		// ���õڶ���label
//		Quaternary q_label2(Quaternary::LABEL,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::NIL_ADDRESSING, 0,
//			Quaternary::IMMEDIATE_ADDRESSING, label2);
//		quaternarytable_.push_back(q_label2);
//	}
//	else	// ���û��else��䣬����if���������ʱ�����õ�һ��label
//	{
//		// ���õ�һ��label
//		Quaternary q_label1(Quaternary::LABEL,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::NIL_ADDRESSING, 0,
//				Quaternary::IMMEDIATE_ADDRESSING, label1);
//		quaternarytable_.push_back(q_label1);
//	}
//}

// <�������> ::= if<����>then<���>[else<���>]
// <�������> ::= if<����>@LABEL_beginthen then<���>@LABEL_endthen
// <�������> ::= if<����>@LABEL_beginthen then<���>@JMP_endelse @LABEL_endthen else<���>@LABEL_endelse
void MidCodeGenerator::IfStatement(size_t depth) throw() // �������
{
    PrintFunctionFrame("IfStatement()", depth);

    assert(Token::IF == token_.type_);
    int begin_quaternary_index = quaternarytable_.size(); // ����һ��if�����Ż�

    // ��������label����ʶthen����Ŀ�ʼ���ͽ�����
    int label_beginthen = label_index_++;
    int label_endthen   = label_index_++;

    // ��ȡ�������
    lexical_analyzer_.GetNextToken(token_);
    bool multi_jmp = Condition(label_beginthen, label_endthen, depth + 1); // ��condition��������ת���
    assert(Token::THEN == token_.type_);

    // ���ܽ��е������Ż������
    if (multi_jmp) {
        // ����ǰһ���Ƿ�Ϊ��label_beginthen����������ת��䣬���ǣ�����Ż�
        if (Quaternary::JMP == quaternarytable_.back().op_ &&
            label_beginthen == quaternarytable_.back().dst_) {
            quaternarytable_.pop_back();
        }
        // ����label_beginthen
        TryLabel(begin_quaternary_index, label_beginthen);
    }
    //else	// �Ż���label_beginthen
    //{
    //	--label_index_;	��Condition���Ѿ���������label��ţ����ﲻ��ֱ���Լ���ʵ���ϣ�����Ҫ���գ���Ϊ��������ռ��Դ��
    //}

    // ��Condition��ĳЩ������Ż�������Appendix1��Ʊ�ע 24-4����
    SpecialOptimize(begin_quaternary_index);

    // ��ȡif�ɹ�������
    lexical_analyzer_.GetNextToken(token_);
    Statement(depth + 1);

    Quaternary q_label_endthen(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                               Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                               label_endthen);
    // ��ȡelse�����
    if (Token::ELSE == token_.type_) {
        // �����label����ʶelse����Ľ�����
        int label_endelse = label_index_++;
        // ������������ת���
        Quaternary q_jmp_endelse(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                                 Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                 label_endelse);
        quaternarytable_.push_back(q_jmp_endelse);
        // ����label_endthen
        quaternarytable_.push_back(q_label_endthen);
        // ��ȡelse�е����
        lexical_analyzer_.GetNextToken(token_);
        Statement(depth + 1);
        // ����label_endelse
        Quaternary q_label_endelse(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                                   Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                                   label_endelse);
        quaternarytable_.push_back(q_label_endelse);
    } else // ���û��else��䣬����if���������ʱ�����õ�һ��label
    {
        // ����labelendthen
        quaternarytable_.push_back(q_label_endthen);
    }
}

Quaternary::OPCode MidCodeGenerator::ConvertFromToken(const Token::TokenType& token_type,
                                                      bool inverse) const throw() {
    Quaternary::OPCode ret_op = Quaternary::NIL_OP;
    if (inverse) {
        switch (token_type) // ������ʿ����ǹ�ϵ���������Ҳ�п����������Ż�THEN��OR��AND���������н���һ�����ʽʱ��
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

// <����> ::= <�������ʽ>
// ��Condition��ֻ��һ����ת��䣬�򷵻�false�����򷵻�true
bool MidCodeGenerator::Condition(int label_positive, int label_negative, size_t depth) throw() // ����
{
    PrintFunctionFrame("Condition()", depth);
    return BoolExpression(label_positive, label_negative, depth + 1);
}

// <�������ʽ> ::= <������> [<�߼���> <������>]
// <�������ʽ> ::= <������>@Label<endterm> [<�߼���> <������>@Label<endterm>] @JMP<label_negative>
// ��������������ʽ��ֻ��һ�����Ļ����ͷ���false�����򷵻�true���������ж��Ƿ�Ҫ�Ż���
bool MidCodeGenerator::BoolExpression(int label_positive,
                                      int label_negative,
                                      size_t depth) throw() // �������ʽ
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

        // BoolTerm�У���ȷ��ֱ����ת��label_positive
        // ʧ������ת��label_endterm
        // ��BoolTermֻ��һ���򵥵�BoolFactor��ʱ����ʡ��label_endterm
        int label_endterm = label_index_++;
        quaternary_index  = quaternarytable_.size();
        if (BoolTerm(label_positive, label_endterm, depth + 1)) {
            multi_jmp = true;
            // ����term������label
            TryLabel(quaternary_index, label_endterm);
        }
        //else	���else���п���
        //{
        //	--label_index_;
        //}
    } while (Token::LOGICOR == token_.type_);
    // �������Expressionֻ��һ����תָ���ʱ��תָ�������ӦΪ����ȷ������������־����
    // ����Ϊ������������������־������ʱ������ȷ��������ִ�У���Ϊ��������ŵľ�����ȷʱ�����飩
    if (!multi_jmp) {
        // ����BoolExpression��Ψһһ����ת���
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
    } else // �ж�����תָ��ʱ
    {
        //�ж�һ�£���һ������Ƿ���Label
        //����ǵĻ���˵�����һ��Term�ж�����ת��䣬������ת����һ������Label
        //�Ż�һ�£���֮ǰ��ת����������Label����ת��䣬ȫ������label_positive
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
            // ����Label����Ѿ�û���ˣ�����ɾ����
            quaternarytable_.pop_back();
        }
        // ���������䲻��Label��˵��Term��ֻ��һ����ת��䣬Ҳ���Ż��Ļ��ᣨ����Appendix1��Ʊ�ע 24--4-1,2����
        // ��������û���㹻����Ϣ���޷������Ż�
        else {
            // ȫ��term��ʧ�ܣ�����ת��label_negative
            Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_negative);
            quaternarytable_.push_back(q_jmp);
        }
    }
    return multi_jmp;
}

// <������> ::= <��������> [<�߼���><��������>]
// <������> ::= <��������>@Label<endfactor> [<�߼���><��������>@Label<endfactor>] @JMP<label_positive>
// ���ж��BoolFactor�������һ��BoolFactor����Ϊ�������ʽʱ����������true
bool MidCodeGenerator::BoolTerm(int label_positive, int label_negative, size_t depth) throw() // ������
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
                       depth + 1)) // ��BoolFactor��Ϊ�������ʽʱ������Ҫ��endfactor
        {
            multi_jmp = true;
            // ����factor������label
            TryLabel(quaternary_index, label_endfactor);
        }
        //else	// ����δʹ�õ�label
        //{
        //	--label_index_;
        //}
    } while (Token::LOGICAND == token_.type_);
    if (!multi_jmp) // ֻ��һ��BoolFactor��������ֻ��һ����������ת
    {
        // ����BoolTerm��Ψһһ����ת���
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
        // �ж�һ�£���һ������Ƿ���Label
        // ����ǵĻ���˵�����һ��Factor�ж�����ת��䣬������ת����һ������Label
        // �Ż�һ�£���֮ǰ��ת����������Label����ת��䣬ȫ������label_positive
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
            // ����Label����Ѿ�û���ˣ�����ɾ����
            quaternarytable_.pop_back();
        }
        // ���������䲻��Label��˵��Factor��ֻ��һ����ת��䣬Ҳ���Ż��Ļ��ᣨ����Appendix1��Ʊ�ע 24-4-1,2����
        // ��������û���㹻����Ϣ���޷������Ż�
        else {
            Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_positive);
            quaternarytable_.push_back(q_jmp);
        }
    }
    return multi_jmp;
}

// <��������> ::= <���ʽ>@JE<Label_negative>
// <��������> ::= <���ʽ><��ϵ�����><���ʽ>@JZ<Label_negative>
// <��������> ::= '('<�������ʽ>')'
// ����ֵ��ʾfactor���Ƿ�Ϊһ���򵥵Ĳ������ʽ
bool MidCodeGenerator::BoolFactor(int label_positive, int label_negative, size_t depth) throw() // ��������
{
    PrintFunctionFrame("BoolFactor()", depth);
    bool multi_jmp      = false;
    bool inverse_prefix = false;

    if (Token::LOGICNOT == token_.type_) {
        inverse_prefix = true;
        lexical_analyzer_.GetNextToken(token_);
    }

    // �ȿ��Ƿ������ŵ����
    if (Token::LEFT_PAREN != token_.type_ || IsExpression(depth + 1)) {
        // ��ȡ����ʽ
        ExpressionAttribute left_attribute = Expression(depth + 1);
        // ��������Ԫ��Ϊ��ʱ����
        SimplifyArrayOperand(left_attribute);
        // ��������������ת���
        // ����������ʱ�Ż���ת����������Ĳ������������tokenҪ��һ��
        Quaternary q_jmp_condition;
        switch (token_.type_) // ������ʿ����ǹ�ϵ���������Ҳ�п����������Ż�THEN��OR��AND���������н���һ�����ʽʱ��
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
            Token::LOGICAND != token_.type_) // ���������һ�����ʽ���ټ�����ȡ
        {
            lexical_analyzer_.GetNextToken(token_); // ��ȡ��ϵ���������һ������
            right_attribute = Expression(depth + 1);
            // ��������Ϊ��ʱ����
            SimplifyArrayOperand(right_attribute);
        } else {
            right_attribute.addressingmethod_        = Quaternary::IMMEDIATE_ADDRESSING;
            right_attribute.value_                   = 0;
            right_attribute.offset_addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
            right_attribute.offset_                  = 0;
            right_attribute.decoratetype_            = TokenTableItem::VOID;
        }
        // ������
        q_jmp_condition.method1_ = left_attribute.addressingmethod_;
        q_jmp_condition.src1_    = left_attribute.value_;
        q_jmp_condition.method2_ = right_attribute.addressingmethod_;
        q_jmp_condition.src2_    = right_attribute.value_;
        q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
        q_jmp_condition.dst_     = label_negative;

        // ������Ԫʽ
        quaternarytable_.push_back(q_jmp_condition);
        // ������ʱ����
        if (Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_) {
            --tempvar_index_;
        }
        if (Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_) {
            --tempvar_index_;
        }
    } else // ��һ���������ţ��Ҳ���ʶ��Ϊ���ʽ�����
    {
        lexical_analyzer_.GetNextToken(token_); // �������ŵ���һ������
        if (inverse_prefix) {
            BoolExpression(label_negative, label_positive, depth + 1);
            multi_jmp = true; // ǰ����ȡ���ķ��ţ���һ������һ���򵥵Ĳ������ʽ
        } else {
            multi_jmp = BoolExpression(label_positive, label_negative, depth + 1);
        }
        lexical_analyzer_.GetNextToken(token_); // �������ŵ���һ������
    }
    return multi_jmp;
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
bool MidCodeGenerator::IsExpression(size_t depth) throw() {
    PrintFunctionFrame("IsExpression()", depth);
    vector<Token>::const_iterator iter = lexical_analyzer_.GetTokenPosition();
    Token tmp                          = token_;
    bool result                        = ExpressionTest(depth + 1);
    token_                             = tmp;
    lexical_analyzer_.SetTokenPosition(iter);
    return result;
}


// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
bool MidCodeGenerator::ExpressionTest(size_t depth) throw() {
    PrintFunctionFrame("ExpressionTest()", depth);

    if (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        lexical_analyzer_.GetNextToken(token_);
    }
    if (!TermTest(depth + 1)) {
        return false;
    }

    while (Token::PLUS == token_.type_ || Token::MINUS == token_.type_) {
        // ��ȡ��һ��
        lexical_analyzer_.GetNextToken(token_);
        if (!TermTest(depth + 1)) {
            return false;
        }
    }
    return true;
}

// <��> ::= <����>{<�˷������><����>}
bool MidCodeGenerator::TermTest(size_t depth) throw() // ��
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

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')'
//          | <�޷�������>
//          | <�ַ�>
bool MidCodeGenerator::FactorTest(size_t depth) throw() // ����
{
    PrintFunctionFrame("FactorTest()", depth);

    // �﷨��飺��ʶ��������������������������顢�������á�
    if (Token::IDENTIFIER == token_.type_) {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
        // ����Ԫ��
        if (Token::LEFT_BRACKET == token_.type_) {
            // �﷨��������Ϊ�±�ı��ʽ
            lexical_analyzer_.GetNextToken(token_);
            if (!ExpressionTest(depth + 1)) {
                return false;
            }
            // �﷨��������false
            if (token_.type_ != Token::RIGHT_BRACKET) {
                return false;
            }
            // �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
            lexical_analyzer_.GetNextToken(token_);
        } else if (Token::LEFT_PAREN == token_.type_) // �����ţ���������
        {
            ProcFuncCallStatementTest(depth + 1);
        }
    } else if (Token::LEFT_PAREN == token_.type_) // �����������ı��ʽ
    {
        // bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
        // �����ʽ�ĵ�һ������
        lexical_analyzer_.GetNextToken(token_);
        // �ٶ�ȡ���ʽ
        if (!ExpressionTest(depth + 1)) // ��¼����
        {
            return false;
        }
        if (token_.type_ != Token::RIGHT_PAREN) {
            return false;
        }
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_INTEGER == token_.type_) // �������泣��
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
    } else if (Token::CONST_CHAR == token_.type_) // �ַ������泣��
    {
#ifdef SYNTAXDEBUG
        generating_process_buffer_ << generating_format_string_ << "  " << token_.toString()
                                   << std::endl;
#endif
        lexical_analyzer_.GetNextToken(token_);
    } else {
        // �﷨��������false
        return false;
    }
    return true;
}

// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
bool MidCodeGenerator::ProcFuncCallStatementTest(size_t depth) // ���̵������
{
    PrintFunctionFrame("ProcFuncCallStatement()", depth);
    // �﷨���
    if (Token::LEFT_PAREN != token_.type_) {
        return false;
    }
    // �﷨�����������Ż������ĵ�һ������
    lexical_analyzer_.GetNextToken(token_);
    // �﷨��������
    if (Token::RIGHT_PAREN != token_.type_) {
        if (!ArgumentListTest(depth + 1)) {
            return false;
        }
        // �﷨���
        if (Token::RIGHT_PAREN != token_.type_) {
            return false;
        }
    }
    lexical_analyzer_.GetNextToken(token_);
    return true;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
bool MidCodeGenerator::ArgumentListTest(size_t depth) throw() // ʵ�α�
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

// ��quaternarytable_���±�Ϊbegin_quaternary_index��Ԫ�ؿ�ʼ���
// �������ת��䣬Ŀ�ĵ�Ϊlabel_index�������label_index
// ���򣬲����κα�־
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

// �����Appendix1��Ʊ�ע 24-4��
void MidCodeGenerator::SpecialOptimize(size_t begin_quaternary_index) throw() {
    bool used = false;
    for (size_t i = begin_quaternary_index; i < quaternarytable_.size() - 1; ++i) {
        // ȡ�������JMPָ�������Appendix1��Ʊ�ע 24-4-1��
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
            // ����������ת������ת���
            quaternarytable_[i - 1].dst_ = quaternarytable_[i].dst_;
            // ȡ��JMPָ��
            quaternarytable_[i].op_ = Quaternary::NIL_OP;

        }
        // ��ͼȡ��Labelָ�� �����Appendix1��Ʊ�ע 24-4-1��
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
            if (!used) // ���Label�Ѿ�û����
            {
                quaternarytable_[i].op_ = Quaternary::NIL_OP;
            }
        } // end of if
    }     // end of for
}

//// <����> ::= <���ʽ><��ϵ�����><���ʽ>
//// ��if��for����д�������label��������ʶif�����forѭ����Ľ���
//// �����ڴ���conditionʱ������ת���
//void MidCodeGenerator::Condition(int endlabel, size_t depth) throw()				// ����
//{
//	PrintFunctionFrame("Condition()", depth);
//
//	ExpressionAttribute left_attribute = Expression(depth + 1);
//	// ��������Ԫ��Ϊ��ʱ����
//	SimplifyArrayOperand(left_attribute);
//
//	bool isonlyexpression = false;
//	// ��������������ת���
//	// ����������ʱ�Ż���ת����������Ĳ������������tokenҪ��һ��
//	Quaternary q_jmp_condition;
//	switch(token_.type_)	// ������ʿ����ǹ�ϵ���������Ҳ�п�����THEN���������н���һ�����ʽʱ��
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
//	if(!isonlyexpression)	// ���������һ�����ʽ���ټ�����ȡ
//	{
//		lexical_analyzer_.GetNextToken(token_);
//		right_attribute = Expression(depth + 1);
//		// ��������Ϊ��ʱ����
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
//	// ������
//	q_jmp_condition.method1_ = left_attribute.addressingmethod_;
//	q_jmp_condition.src1_ = left_attribute.value_;
//	q_jmp_condition.method2_ = right_attribute.addressingmethod_;
//	q_jmp_condition.src2_ = right_attribute.value_;
//	q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
//	q_jmp_condition.dst_ = endlabel;
//	// ������Ԫʽ
//	quaternarytable_.push_back(q_jmp_condition);
//	// ������ʱ����
//	if(Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_)
//	{
//		--tempvar_index_;
//	}
//	if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
//	{
//		--tempvar_index_;
//	}
//}


// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void MidCodeGenerator::CaseStatement(size_t depth) throw() // ������
{
    PrintFunctionFrame("CaseStatement()", depth);

    assert(Token::CASE == token_.type_);

    // ��ȡ���ʽ
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute exp_attribute = Expression(depth + 1);
    // �����ʽ��Ϊ������Ԫ��
    SimplifyArrayOperand(exp_attribute);

    assert(Token::OF == token_.type_);
    // ΪEND������һ��label
    int endlabel = label_index_++;
    // case���ʽ֮�����ת���Ĳ���λ��
    int jmp_insertion_location = quaternarytable_.size();
    // ���ɸ�JE����ת���
    Quaternary q_jmp;
    q_jmp.op_      = Quaternary::JE;
    q_jmp.method1_ = exp_attribute.addressingmethod_;
    q_jmp.src1_    = exp_attribute.value_;
    q_jmp.method2_ = Quaternary::IMMEDIATE_ADDRESSING;
    q_jmp.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
    do {
        lexical_analyzer_.GetNextToken(token_);
        // Ϊ��ǰ�����Ԫ������һ��label
        int caselabel             = label_index_++;
        vector<int> constant_list = CaseElement(caselabel, endlabel, depth + 1);
        // ����JE��ת���
        q_jmp.dst_ = caselabel;
        for (vector<int>::const_iterator c_iter = constant_list.begin();
             c_iter != constant_list.end(); ++c_iter) {
            q_jmp.src2_ = *c_iter;
            quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location++, q_jmp);
        }
    } while (Token::SEMICOLON == token_.type_);

    // ִ��������JE����ת��END��
    Quaternary q_endjmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                        0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location, q_endjmp);

    // ���������label
    Quaternary q_endlabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                          Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_endlabel);

    // ����case��Expression����ʱ����
    if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_) {
        --tempvar_index_;
    }

    // ��������־
    assert(Token::END == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <�����Ԫ��> ::= <���������>:<���>
// <���������> ::=  <���� | ������>{, <���� | ������>}
vector<int> MidCodeGenerator::CaseElement(int caselabel, int endlabel, size_t depth) throw() // �����Ԫ��
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
        // ��¼�ó���
        if (Token::CONST_INTEGER == token_.type_) {
            constant_list.push_back(token_.value_.integer);
        } else if (Token::CONST_CHAR == token_.type_) {
            constant_list.push_back(token_.value_.character);
        } else // ������
        {
            constant_list.push_back(iter->value_);
        }
        // ��ȡ��һ������
        lexical_analyzer_.GetNextToken(token_);
    } while (Token::COMMA == token_.type_);
    assert(Token::COLON == token_.type_);
    // ���������Ԫ�ص����֮ǰ������һ��label
    Quaternary q_caselabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                           caselabel);
    quaternarytable_.push_back(q_caselabel);

    lexical_analyzer_.GetNextToken(token_);
    Statement(depth + 1);

    // �����Ԫ�ص����ִ�������ת��END��
    Quaternary q_endjmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                        0, Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_endjmp);
    // ���ؼ�¼�������Ԫ�صĳ�������
    return constant_list;
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// TODO ��չ���������֧��
void MidCodeGenerator::ReadStatement(size_t depth) throw() // �����
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
        //iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼

        // ��ȡ��һ�����ʣ��ж��Ƿ�Ϊ����Ԫ��
        lexical_analyzer_.GetNextToken(token_);
        if (Token::LEFT_BRACKET != token_.type_) // ��������Ԫ��
        {
            assert(TokenTableItem::VARIABLE == iter->itemtype_ ||
                   TokenTableItem::PARAMETER == iter->itemtype_); // ����Ƿ�Ϊ���������������
            // ����READ���õ���Ԫʽ
            Quaternary q_read(Quaternary::READ, Quaternary::NIL_ADDRESSING, 0,
                              Quaternary::NIL_ADDRESSING, 0, Quaternary::VARIABLE_ADDRESSING,
                              distance(tokentable_.begin(),
                                       static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_read);
        } else // ����Ԫ��
        {
            // ���ͼ��
            assert(TokenTableItem::ARRAY == iter->itemtype_); // ����Ƿ�Ϊ�������
            // ���������±�ĵ�һ������
            lexical_analyzer_.GetNextToken(token_);
            // ���������±�ı��ʽ
            ExpressionAttribute exp_attribute = Expression(depth + 1);
            // �����黯
            SimplifyArrayOperand(exp_attribute);
            // ����READ���õ���Ԫʽ
            Quaternary q_read(Quaternary::READ, Quaternary::NIL_ADDRESSING, 0,
                              exp_attribute.addressingmethod_, exp_attribute.value_,
                              Quaternary::ARRAY_ADDRESSING,
                              distance(tokentable_.begin(),
                                       static_cast<TokenTable::const_iterator>(iter)));
            quaternarytable_.push_back(q_read);
            // �ж�������
            assert(Token::RIGHT_BRACKET == token_.type_);
            // ������һ������
            lexical_analyzer_.GetNextToken(token_);
        }
    } while (Token::COMMA == token_.type_);

    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void MidCodeGenerator::WriteStatement(size_t depth) throw() // д���
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
        // ����WRITE���õ���Ԫʽ
        Quaternary q_read(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                          Quaternary::NIL_ADDRESSING, 0, Quaternary::STRING_ADDRESSING,
                          distance(static_cast<vector<string>::const_iterator>(stringtable_.begin()),
                                   iter));
        quaternarytable_.push_back(q_read);

        lexical_analyzer_.GetNextToken(token_);
        if (Token::COMMA == token_.type_) {
            lexical_analyzer_.GetNextToken(token_);
            ExpressionAttribute exp_attribute = Expression(depth + 1); // ��ȡ�ڶ������������ʽ��
            // ����WRITE���õ���Ԫʽ
            Quaternary q_write(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                               exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
                               exp_attribute.addressingmethod_, exp_attribute.value_);
            // ��write����Ԫʽĩβһ��Ҫ��decoratetype�����ԣ��������ʱ�������Ƶ�
            q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
            quaternarytable_.push_back(q_write);
            // ������ʱ����
            if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_ ||
                Quaternary::TEMPORARY_ADDRESSING ==
                    exp_attribute.offset_addressingmethod_) // ���߲�����ͬʱ��������д��һ��
            {
                --tempvar_index_;
            }
        }
    } else {
        ExpressionAttribute exp_attribute = Expression(depth + 1);
        // ����WRITE���õ���Ԫʽ
        Quaternary q_write(Quaternary::WRITE, Quaternary::NIL_ADDRESSING, 0,
                           exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
                           exp_attribute.addressingmethod_, exp_attribute.value_);
        // ��write����Ԫʽĩβһ��Ҫ��decoratetype�����ԣ��������ʱ�������Ƶ�
        q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
        quaternarytable_.push_back(q_write);
        // ������ʱ����
        if (Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_ ||
            Quaternary::TEMPORARY_ADDRESSING ==
                exp_attribute.offset_addressingmethod_) // ���߲�����ͬʱ��������д��һ��
        {
            --tempvar_index_;
        }
    }

    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <whileѭ�����> ::= while <����> do <���>
// <whileѭ�����> ::= while @Label<check> <����> do @Label<BeginDo> <���> @JMPLabel<check> @Label<end>
void MidCodeGenerator::WhileLoopStatement(size_t depth) throw() // whileѭ�����
{
    PrintFunctionFrame("WhileLoopStatement()", depth);
    assert(Token::WHILE == token_.type_);

    // BUG! ����������ţ�һ��Ҫ����checklabel�ĺ���
    // ��������checklabel���ں��汻���õģ��ͻ���SpecialOptimize�а�checklabel�Ż���
    //size_t begin_quaternary_index = quaternarytable_.size();	// ��Ԫʽ����ʼλ��

    // �����������ǰ���label<check>�ͽ���ʱ��label<end>
    int checklabel    = label_index_++;
    int label_begindo = label_index_++;
    int label_enddo   = label_index_++;
    // ����continue_label_ջ��break_label_ջ
    continue_label_.push(checklabel);
    break_label_.push(label_enddo);
    // ����label<check>
    Quaternary q_checklabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                            Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                            checklabel);
    quaternarytable_.push_back(q_checklabel);
    // Condition��Ԫʽ����ʼλ��
    size_t begin_quaternary_index = quaternarytable_.size();
    // ��ȡ��һ�����ʣ��������������
    lexical_analyzer_.GetNextToken(token_);
    bool multi_jmp =
        Condition(label_begindo, label_enddo, depth + 1); // ��������л�ִ�ж���@JZLabel<end>
    // �﷨���
    assert(Token::DO == token_.type_);
    // ���ܽ��е������Ż������
    if (multi_jmp) {
        // ����ǰһ���Ƿ�Ϊ��label_begindo����������ת��䣬���ǣ�����Ż�
        if (Quaternary::JMP == quaternarytable_.back().op_ &&
            label_begindo == quaternarytable_.back().dst_) {
            quaternarytable_.pop_back();
        }
        // ����label_begindo
        TryLabel(begin_quaternary_index, label_begindo);
    }
    // ��Condition��ĳЩ������Ż�������Appendix1��Ʊ�ע 24-4����
    SpecialOptimize(begin_quaternary_index);

    // ����ѭ����ĵ�һ������
    lexical_analyzer_.GetNextToken(token_);
    // ����ѭ����
    Statement(depth + 1);
    // ����continue_label_ջ��break_label_ջ
    continue_label_.pop();
    break_label_.pop();
    // ѭ������������ת
    Quaternary q_jmp(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0,
                     Quaternary::IMMEDIATE_ADDRESSING, checklabel);
    quaternarytable_.push_back(q_jmp);
    // ���½�����label
    Quaternary q_label_enddo(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                             Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                             label_enddo);
    quaternarytable_.push_back(q_label_enddo);
}

// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to�� <���ʽ> do <���>
// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to��
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <���ʽ> @JZLABEL<end>
// do <���>@JMPLABEL<vary>@Label<end>
void MidCodeGenerator::ForLoopStatement(size_t depth) throw() // forѭ�����
{
    PrintFunctionFrame("ForLoopStatement()", depth);

    assert(Token::FOR == token_.type_);

    // ��ȡ��ʶ��
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::IDENTIFIER == token_.type_);
#ifdef SYNTAXDEBUG
    generating_process_buffer_ << generating_format_string_ << "  " << token_.toString() << std::endl;
#endif
    TokenTable::iterator loopvar_iter = tokentable_.SearchDefinition(token_);
    assert(loopvar_iter != tokentable_.end());
    //loopvar_iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
    assert(TokenTableItem::VARIABLE == loopvar_iter->itemtype_ ||
           TokenTableItem::PARAMETER == loopvar_iter->itemtype_); // ����Ƿ�Ϊ���������
    // ��ȡ��ֵ��
    lexical_analyzer_.GetNextToken(token_);
    assert(Token::ASSIGN == token_.type_);
    // ��ȡ���ʽ
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute init_attribute = Expression(depth + 1);

    // ���to/downto
    assert(Token::DOWNTO == token_.type_ || Token::TO == token_.type_);
    // �������/������
    Token vary_token = token_;
    // ����ѭ�������ĵ���/����label<vary>, �߽����label<check>���Լ�ĩβ��label<end>
    int varylabel  = label_index_++;
    int checklabel = label_index_++;
    int endlabel   = label_index_++;
    // ����continue_label_ջ��break_label_ջ
    continue_label_.push(varylabel);
    break_label_.push(endlabel);
    // ����for��ѭ�������ĳ�ʼ������ֵ����Ԫʽ
    Quaternary q_init(Quaternary::ASG, init_attribute.addressingmethod_, init_attribute.value_,
                      init_attribute.offset_addressingmethod_, init_attribute.offset_,
                      Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)));
    quaternarytable_.push_back(q_init);
    // ������ת���߽����JMP��Ԫʽ
    Quaternary q_jmpcheck(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                          0, Quaternary::IMMEDIATE_ADDRESSING, checklabel);
    quaternarytable_.push_back(q_jmpcheck);
    // ����ѭ����������/����label<vary>
    Quaternary q_varylabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                           Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                           varylabel);
    quaternarytable_.push_back(q_varylabel);
    // ����ѭ����������/������Ԫʽ
    Quaternary q_vary(Token::TO == vary_token.type_ ? Quaternary::ADD : Quaternary::SUB,
                      Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)),
                      Quaternary::IMMEDIATE_ADDRESSING, 1, Quaternary::VARIABLE_ADDRESSING,
                      distance(tokentable_.begin(),
                               static_cast<TokenTable::const_iterator>(loopvar_iter)));
    quaternarytable_.push_back(q_vary);
    // ����ѭ�������߽����label<check>
    Quaternary q_checklabel(Quaternary::LABEL, Quaternary::NIL_ADDRESSING, 0,
                            Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING,
                            checklabel);
    quaternarytable_.push_back(q_checklabel);

    // ��ȡ���ʽ
    lexical_analyzer_.GetNextToken(token_);
    ExpressionAttribute bound_attribute = Expression(depth + 1);

    // ������������Ԫ�ر��ʽ
    SimplifyArrayOperand(bound_attribute);
    // ���ɱ߽����JMP��Ԫʽ
    Quaternary q_check(Token::TO == vary_token.type_ ? Quaternary::JG : Quaternary::JL,
                       Quaternary::VARIABLE_ADDRESSING,
                       distance(tokentable_.begin(),
                                static_cast<TokenTable::const_iterator>(loopvar_iter)),
                       bound_attribute.addressingmethod_, bound_attribute.value_,
                       Quaternary::IMMEDIATE_ADDRESSING, endlabel);
    quaternarytable_.push_back(q_check);

    // ��������Expression����ʱ����
    if (Quaternary::TEMPORARY_ADDRESSING == init_attribute.addressingmethod_ ||
        Quaternary::TEMPORARY_ADDRESSING == init_attribute.offset_addressingmethod_) {
        --tempvar_index_;
    }
    if (Quaternary::TEMPORARY_ADDRESSING == bound_attribute.addressingmethod_) {
        --tempvar_index_;
    }

    // ��ȡDO
    assert(Token::DO == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
    // ��ȡѭ����
    Statement(depth + 1);
    // ����continue_label_ջ��break_label_ջ
    continue_label_.pop();
    break_label_.pop();
    // ��ת��ѭ����������/����label<vary>
    Quaternary q_jmpvary(Quaternary::JMP, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING,
                         0, Quaternary::IMMEDIATE_ADDRESSING, varylabel);
    quaternarytable_.push_back(q_jmpvary);
    // forѭ��������label<end>
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
    // ������һ�����ʲ�����
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
    // ������һ�����ʲ�����
    lexical_analyzer_.GetNextToken(token_);
}


// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
void MidCodeGenerator::ProcFuncCallStatement(const Token proc_token,
                                             const vector<ExpressionAttribute>& parameter_attributes,
                                             size_t depth) // ���̵������
{
    PrintFunctionFrame("ProcFuncCallStatement()", depth);
    // �﷨���
    assert(Token::LEFT_PAREN == token_.type_);
    // �﷨�����������Ż������ĵ�һ������
    lexical_analyzer_.GetNextToken(token_);
    if (parameter_attributes.size() == 0) // ������������û�в����Ļ����Ͳ��ö�������
    {
        // �﷨���
        assert(Token::RIGHT_PAREN == token_.type_);
        lexical_analyzer_.GetNextToken(token_); // ������������Ϻ����һ������
        return;
    }
    // �﷨�������������������ò�������Ԫʽ
    ArgumentList(parameter_attributes, depth + 1);
    // �﷨���
    assert(Token::RIGHT_PAREN == token_.type_);
    lexical_analyzer_.GetNextToken(token_);
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
void MidCodeGenerator::ArgumentList(const vector<ExpressionAttribute>& parameter_attributes,
                                    size_t depth) throw() // ʵ�α�
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
        // �������ò�������Ԫʽ
        // ����Ҫ�������ã��ұ��ʽΪ�����ã�����SETREFP
        // �ص����ڣ������ʽҲΪ���ã���ֻ��SETP�����ɴ����ַ
        if (Quaternary::REFERENCE_ADDRESSING == parameter_attributes[para_index].addressingmethod_ &&
            Quaternary::REFERENCE_ADDRESSING != argument_attribute.addressingmethod_) {
            q_addpara.op_ = Quaternary::SETREFP;
            // ���ô���Ҫ���������ֵ
            if (Quaternary::VARIABLE_ADDRESSING != argument_attribute.addressingmethod_ &&
                Quaternary::ARRAY_ADDRESSING != argument_attribute.addressingmethod_) {
                // ֻ����һ�����ܳ���
                std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString()
                          << "  should be left value to fit the reference parameter\n";
                is_successful_ = false;
                // ������Ǽ��ɣ����������ȡ
                // ��ʱ����Ԫʽ���������ʱ�����ĵ�ַ����ʹ���֮�����У�Ҳ�����������ʱ����
            }
        } else {
            q_addpara.op_ = Quaternary::SETP;
        }
        q_addpara.method2_ = argument_attribute.offset_addressingmethod_;
        q_addpara.offset2_ = argument_attribute.offset_;
        q_addpara.method3_ = argument_attribute.addressingmethod_;
        q_addpara.dst_     = argument_attribute.value_;
        quaternarytable_.push_back(q_addpara);
        // ������ʱ����
        if (Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_ ||
            Quaternary::TEMPORARY_ADDRESSING ==
                argument_attribute.offset_addressingmethod_) // ���߲�����ͬʱ��������д��һ��
        {
            --tempvar_index_;
        }
        ++para_index;
    } while (Token::COMMA == token_.type_);
}

// ������Ԫ�صĲ��������������������ͽ��仯��Ϊ��ʱ����
void MidCodeGenerator::SimplifyArrayOperand(ExpressionAttribute& attribute) throw() {
    if (Quaternary::ARRAY_ADDRESSING == attribute.addressingmethod_) {
        Quaternary q_subscript2temp;
        q_subscript2temp.op_      = Quaternary::ASG;
        q_subscript2temp.method1_ = attribute.addressingmethod_;
        q_subscript2temp.src1_    = attribute.value_;
        q_subscript2temp.method2_ = attribute.offset_addressingmethod_;
        q_subscript2temp.offset2_ = attribute.offset_;
        q_subscript2temp.method3_ = Quaternary::TEMPORARY_ADDRESSING;
        // Ŀ�����ʱ������ŵ�ȷ��
        // ��������±������ʱ��������ô�����������
        // ������¿�һ����ʱ����
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
