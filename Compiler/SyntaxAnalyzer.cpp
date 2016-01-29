#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableException.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <algorithm>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer, const vector<string> &stringtable, TokenTable &tokentable, vector<Quaternary> &quaternarytable)  throw()
	: lexical_analyzer_(lexical_analyzer), stringtable_(stringtable), tokentable_(tokentable), quaternarytable_(quaternarytable), 
	  token_(), level_(0), tempvar_index_(0), label_index_(0), is_successful_(true), syntax_info_buffer_(), syntax_assist_buffer_(), tokenbuffer_()
{}


bool SyntaxAnalyzer::Parse() throw()
{
	int depth = 0;
	syntax_assist_buffer_.clear();
	syntax_assist_buffer_.clear();
	PrintFunctionFrame("Parse()", depth);
	lexical_analyzer_.GetNextToken(token_);
	Routine(depth + 1);
	return is_successful_;
}

string SyntaxAnalyzer::toString() const throw()
{
	return syntax_info_buffer_.str();
}

bool SyntaxAnalyzer::Print(const string &filename) const throw()
{
	std::ofstream output(filename);
	if(!output.is_open())
	{
		return false;
	}
	Print(output);
	output.close();
	return true;
}

void SyntaxAnalyzer::Print(std::ostream &output) const throw()
{
	output << syntax_info_buffer_.str() << std::endl;
}

static string syntax_assist_buffer_;	// ע�������̰߳�ȫ��
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, int depth) throw()
{
	
	if(depth * 4 == syntax_assist_buffer_.size())
	{
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_assist_buffer_.size())
	{
		syntax_assist_buffer_.append("|");
		syntax_assist_buffer_.append(depth * 4 - syntax_assist_buffer_.size(), ' ');	// ���ﲻ�ܼ�1
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else // depth * 4 < syntax_assist_buffer_.size()
	{
		syntax_assist_buffer_.resize(depth * 4);
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SyntaxAnalyzer::Routine(int depth) throw()
{
	PrintFunctionFrame("Routine()", depth);

	SubRoutine(depth + 1);
	// �жϽ�������
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SyntaxAnalyzer::SubRoutine(int depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	// �ĸ���ѡ��֧
	if(token_.type_ == Token::CONST)
	{
		ConstantPart(depth + 1);
	}
	if(token_.type_ == Token::VAR)
	{
		VariablePart(depth + 1);
	}
	if(token_.type_ == Token::PROCEDURE)
	{
		ProcedurePart(depth + 1);
	}
	if(token_.type_ == Token::FUNCTION)
	{
		FunctionPart(depth + 1);
	}
	// һ����ѡ��֧
	if(token_.type_ == Token::BEGIN)
	{
		StatementBlockPart(depth + 1);
	}
	else
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  didn't find sentence block in subroutine\n";
		return;
	}
}

// <����˵������> ::= const<��������>{,<��������>};
void SyntaxAnalyzer::ConstantPart(int depth) throw()
{
	PrintFunctionFrame("ConstantPart()", depth);

	assert(Token::CONST == token_.type_);

	// ��������
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		constantDefination(depth + 1);
	} while(token_.type_ == Token::COMMA);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ';' after constant definition\n";
		/*while(lexical_analyzer_.GetNextToken(token_) && token_.type_ != Token::SEMICOLON)
		{ }*/
		is_successful_ = false;
		if(Token::VAR == token_.type_ || Token::PROCEDURE == token_.type_ || Token::FUNCTION == token_.type_ || Token::BEGIN == token_.type_)//	������������Ǽӷֺ��ˣ�����ֱ�ӷ��ض�����ȡ��һ������
		{
			return;	
		}
		else
		{
			while(token_.type_ != Token::NIL  && token_.type_ != Token::SEMICOLON)	// �����������ʣ���ʾ����˵�����ֻ�δ��������Ҫ������һ���ֺ�
			{
				lexical_analyzer_.GetNextToken(token_);
			};
			lexical_analyzer_.GetNextToken(token_);
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <��������> ::= <��ʶ��>��<����>
void SyntaxAnalyzer::constantDefination(int depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of constant definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��¼token_�Բ�����ű�
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::EQU)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << "  should be '=' after identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::CONST_INTEGER 
	&& token_.type_ != Token::CONST_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after '='\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �������壬������ű�
	if(tokentable_.SearchDefinitionInCurrentLevel(constIdentifier.value_.identifier))
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	else if(Token::CONST_INTEGER == token_.type_)
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::INTEGER, token_.value_.integer, level_);
	}
	else
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::CHAR, token_.value_.character, level_);
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= var <��������>;{<��������>;}
void SyntaxAnalyzer::VariablePart(int depth) throw()
{
	PrintFunctionFrame("VariablePart()", depth);

	assert(Token::VAR == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	do
	{
		VariableDefinition(depth + 1);
		if(token_.type_ != Token::SEMICOLON)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ';' after variable definition\n";
			is_successful_ = false;
			if(Token::IDENTIFIER == token_.type_)	// ���Ǽӷֺţ����滹�б�������
			{
				continue;
			}
			else
			{
				// ���Ǽӷֺ��ˣ��������޷��������ϱ������壬ֻ����ת����һ����
				while(Token::NIL != token_.type_ && Token::SEMICOLON != token_.type_ && Token::PROCEDURE != token_.type_ && Token::FUNCTION != token_.type_ && Token::BEGIN == token_.type_)
				{
					lexical_analyzer_.GetNextToken(token_);
				}
				if(Token::SEMICOLON == token_.type_)
				{
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
		}
		lexical_analyzer_.GetNextToken(token_);
	}while(token_.type_ == Token::IDENTIFIER);
}

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
void SyntaxAnalyzer::VariableDefinition(int depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of variable definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER&& token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(token_.type_ != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
		{
			return;
		}
		// �����˱�ʶ���������ִ��
	}
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(token_.type_ == Token::COMMA)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of variable definition\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			if(token_.type_ != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
			{
				return;
			}
			// �����˱�ʶ���������ִ��
		}
		tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ':' after identifier to specify the type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::RW_INTEGER && token_.type_ != Token::RW_CHAR && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β������˵�������ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_)	// ������������˵����
		{
			TypeSpecification(depth + 1);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <����> ::= [array'['<�޷�������>']'of]<��������>
void SyntaxAnalyzer::TypeSpecification(int depth) throw()
{
	PrintFunctionFrame("TypeSpecification()", depth);

	TokenTableItem::ItemType itemtype_ = TokenTableItem::VARIABLE;
	int arrayLength = 0;
	if(token_.type_ == Token::ARRAY)
	{
		itemtype_ = TokenTableItem::ARRAY;
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::LEFT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '[' after \"array\"\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;	// ��������ĳ�����̫���鷳����ֱ��������䣬���ؽ����
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::CONST_INTEGER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a constant integer after '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		arrayLength = token_.value_.integer;
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::OF)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" after [*]\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	if(token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)	// ��û������˵��
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" for type specification\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ������ű�
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// ���η�����
	if(TokenTableItem::ARRAY == itemtype_)	// ��������
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))	// �ض������Ȼ���뵱ǰ���壨Ҳ�ɲ����룩
			{
				std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
				is_successful_ = false;
			}
			tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
		}
	}
	else									// ����һ�����
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
			{
				std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
				is_successful_ = false;
			}
			tokentable_.AddVariableItem(*iter, decoratetype_, level_);
		}
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::ProcedurePart(int depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		int proc_index = ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		// ���ɹ��̵�END��Ԫʽ
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, proc_index));
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �ֳ��������Ӧ����ֺ�
	}while(Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
int SyntaxAnalyzer::ProcedureHead(int depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// �������ڷ��ű��е�λ�ã��±꣩

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a procedure name after \"procedure\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// ������ж�λ����Ϊ�˵���ProcedurePart�е��ض�λ
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��������������ű�
	string proc_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĹ��̷ֳ�����﷨���������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	proc_index = tokentable_.AddProcedureItem(token_, level_++);// ������֮��leveҪ+1
	// ���ɹ��̵�BEGIN��Ԫʽ
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, proc_index));
	// ��λ��������������ڵ�λ����Ϊ�ֲ����������ʼ�㣩
	tokentable_.Locate();	
	// ������ȡ����
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)	// û�ж��������ţ�����û�в���
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth);
		tokentable_.SetParameterCount(proc_name, parameterCount);
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  need ')' to match '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure head\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return proc_index;
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::FunctionPart(int depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		int func_index = FunctionHead(depth + 1);	// ���к���ͷ���������õ��������ڷ��ű��е�λ��
		SubRoutine(depth + 1);
		// ���ɺ�����END��Ԫʽ
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, func_index));
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)	// �ֳ��������Ӧ����ֺ�
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �����β�ķֺ�
	}while(Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
int SyntaxAnalyzer::FunctionHead(int depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// �������ڷ��ű��е�λ��

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a function name after \"function\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// ������ж�λ����Ϊ�˵���FunctionPart�е��ض�λ
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ���뺯���������ű�
	string func_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĺ����ֳ�����﷨���������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	func_index = tokentable_.AddFunctionItem(token_, level_++);// ������֮��leveҪ+1
	// ���ɺ�����BEGIN��Ԫʽ
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, func_index));
	// ��λ
	tokentable_.Locate();
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth + 1);
		tokentable_.SetParameterCount(func_name, parameterCount); 
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  need ')' to match '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)	// ����������ð��
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ':' after parameter Statement\n";
		is_successful_ = false;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" after ':' to specify the return type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	tokentable_.SetFunctionReturnType(func_name, decoratetype_);
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// �����п��������˷ֺţ����Բ��ټ�����
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function head\n";
		is_successful_ = false;
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return func_index;
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
int SyntaxAnalyzer::ParameterList(int depth) throw()		// �βα�
{
	PrintFunctionFrame("ParameterList()", depth);

	int sum = ParameterTerm(depth + 1);
	while(Token::SEMICOLON == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		sum += ParameterTerm(depth + 1);
	}
	return sum;
}

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
int SyntaxAnalyzer::ParameterTerm(int depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);

	if(Token::VAR == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be parameter name surrounded by parentheses\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA  && token_.type_ != Token::SEMICOLON)	// ������β���ֺš�BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::IDENTIFIER != token_.type_)
		{
			return 0;
		}
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ������ѹջ׼�������ű�
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);
	lexical_analyzer_.GetNextToken(token_);
	int sum = 1;
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be parameter name after ','\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::IDENTIFIER)	// ������β���ֺš�BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			if(Token::IDENTIFIER != token_.type_)
			{
				return 0;
			}
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		tokenbuffer_.push_back(token_);
		++sum;
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ':' to specify the type after parameter name\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return 0;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" after ':' to specify the parameter type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return 0;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��������ű�
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
		{
			std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
			is_successful_ = false;
		}
		tokentable_.AddParameterItem(*iter, decoratetype_, level_);
	} // end ����ű�

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
vector<ExpressionAttribute> SyntaxAnalyzer::ArgumentList(int depth) throw()			// ʵ�α�
{
	PrintFunctionFrame("ArgumentList()", depth);

	vector<ExpressionAttribute> attribute_buffer;
	ExpressionAttribute argument_attribute = Expression(depth + 1);
	attribute_buffer.push_back(argument_attribute);
	// �������ò�������Ԫʽ
	Quaternary q_addpara(Quaternary::SETP, 
		Quaternary::OPERAND_NIL, 0, 
		argument_attribute.offset_operandtype_, argument_attribute.offset_, 
		argument_attribute.operandtype_, argument_attribute.value_);
	quaternarytable_.push_back(q_addpara);
	// ������ʱ����
	if(Quaternary::TEMPORARY_OPERAND == argument_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == argument_attribute.offset_operandtype_)	// ���߲�����ͬʱ��������д��һ��
	{
		--tempvar_index_;
	}

	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		argument_attribute = Expression(depth + 1);
		attribute_buffer.push_back(argument_attribute);
		// �������ò�������Ԫʽ
		q_addpara.type2_ = argument_attribute.offset_operandtype_;
		q_addpara.offset2_ = argument_attribute.offset_;
		q_addpara.type3_ = argument_attribute.operandtype_;
		q_addpara.dst_ = argument_attribute.value_;
		quaternarytable_.push_back(q_addpara);
		// ������ʱ����
		if(Quaternary::TEMPORARY_OPERAND == argument_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == argument_attribute.offset_operandtype_)	// ���߲�����ͬʱ��������д��һ��
		{
			--tempvar_index_;
		}
	}
	return attribute_buffer;
}

// <�������> ::= begin <���>{;<���>} end
void SyntaxAnalyzer::StatementBlockPart(int depth) throw()	// �������
{
	PrintFunctionFrame("StatementBlockPart()", depth);

	assert(Token::BEGIN == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}while(token_.type_ == Token::SEMICOLON);
	if(token_.type_ != Token::END)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"end\" at the end of Statement Block\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::END == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>|<�����>|<д���>|<forѭ�����>|<��>
void SyntaxAnalyzer::Statement(int depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// ��token_�����ǹ��������ȼ��£�����
	TokenTable::iterator iter;
	TokenTableItem::DecorateType l_value_type = TokenTableItem::VOID;
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		iter = tokentable_.SearchDefinition(token_);	// ���ҷ��ű��еĶ���
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// ���̵���
		{
			if(iter->GetItemType() != TokenTableItem::PROCEDURE)	// ����������Ƿ�Ϊ����
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a procedure\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			// ���ɹ��̵�����Ԫʽ
			Quaternary q_procedurecall(Quaternary::PROC_CALL, Quaternary::OPERAND_NIL, 0, Quaternary::PARANUM_OPERAND, decorate_types.size(), Quaternary::PROC_FUNC_INDEX, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_procedurecall);
			ProcedureCallStatement(idToken, decorate_types, depth + 1);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(idToken, iter, depth + 1);		
		}
		else if(Token::COLON == token_.type_)	// �˷�֧ר��Ϊ�˼�鸳ֵ��д������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  please check the spelling of the assigning operator\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		else
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error after identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
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
	case Token::FOR:
		ForLoopStatement(depth + 1);
		break;
		// TODO ��������Ƿ�Ϸ���Ӧ�úϷ���
	case Token::END:	// �����
		break;
	default:
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error at the beginning of Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		break;
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void SyntaxAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth)			// ��ֵ���
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// Ϊ��Ԫʽ���ɶ�����ı���
	bool assign2array= false;	// �Ƿ�Ϊ������ĸ�ֵ����
	
	ExpressionAttribute offset_attribute;	// ��������Ԫ�ظ�ֵʱ���洢ƫ����������

	if(Token::LEFT_BRACKET == token_.type_)	// ������Ԫ�ظ�ֵ
	{
		assign2array = true;
		if(iter->GetItemType() != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		offset_attribute = Expression(depth + 1);	// ����Ľ��һ������temp#tempvar_index_�д洢��
		// �����±�������Ԫ�ص������������
		if(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_)
		{
			// ������Ԫʽ���������±�ֵ����һ����ʱ����
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = offset_attribute.operandtype_;
			q_subscript2temp.src1_ = offset_attribute.value_;
			q_subscript2temp.type2_ = offset_attribute.offset_operandtype_;
			q_subscript2temp.offset2_ = offset_attribute.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == offset_attribute.offset_operandtype_)
			{
				q_subscript2temp.dst_ = offset_attribute.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			offset_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			offset_attribute.value_ = q_subscript2temp.dst_;
			offset_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
			offset_attribute.offset_ = 0;
		}
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER
		&& iter->GetItemType() != TokenTableItem::FUNCTION)	// ����Ƿ�Ϊ���������������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ʣ��ֻ������������������������Ǻ�������ֵ
	if(token_.type_ != Token::ASSIGN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  \":=\" doesn't occur in the Assigning Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��������Ϊ�׳��쳣��׼��������GetNextTokenǰ��Ϊ��׼ȷ��¼��ֵ�ŵ��к�
	static Token errToken;
	errToken.type_ = Token::ASSIGN;
	errToken.lineNumber_ = token_.lineNumber_;

	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);
	if(TokenTableItem::CHAR == iter->GetDecorateType() && TokenTableItem::INTEGER == right_attribute.decoratetype_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert from 'int' to 'char'\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ������Ԫʽ
	if(assign2array)	// ������Ԫ�ظ�ֵ
	{
		// ���right_attribute������Ԫ�صĻ���Ҫ���丳ֵ����ʱ����
		if(Quaternary::ARRAY_OPERAND == right_attribute.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = right_attribute.operandtype_;
			q_subscript2temp.src1_ = right_attribute.value_;
			q_subscript2temp.type2_ = right_attribute.offset_operandtype_;
			q_subscript2temp.offset2_ = right_attribute.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == right_attribute.offset_operandtype_)
			{
				q_subscript2temp.dst_ = right_attribute.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			right_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			right_attribute.value_ = q_subscript2temp.dst_;
			right_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
			right_attribute.offset_ = 0;
		}
		// �������鸳ֵ
		Quaternary q_asg;
		q_asg.op_ = Quaternary::AASG;
		q_asg.type1_ = right_attribute.operandtype_;
		q_asg.src1_ = right_attribute.value_;
		q_asg.type2_ = offset_attribute.operandtype_;
		q_asg.offset2_ = offset_attribute.value_;
		q_asg.type3_ = Quaternary::ARRAY_OPERAND;
		q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
		quaternarytable_.push_back(q_asg);

		// ����Ҳ���������ʱ�������ɻ���
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// �������ֵ�������±�Ҳ����ʱ�������ɻ���
		if(Quaternary::TEMPORARY_OPERAND == offset_attribute.operandtype_)
		{
			--tempvar_index_;
		}
	}
	else if(TokenTableItem::PARAMETER == iter->GetItemType()
			|| TokenTableItem::VARIABLE == iter->GetItemType())	// ��ͨ����/�����ĸ�ֵ
	{
		Quaternary q_asg;
		q_asg.op_ = Quaternary::ASG;
		q_asg.type1_ = right_attribute.operandtype_;
		q_asg.src1_ = right_attribute.value_;
		q_asg.type2_ = right_attribute.offset_operandtype_;
		q_asg.offset2_ = right_attribute.offset_;
		q_asg.type3_ = Quaternary::VARIABLE_OPERAND;
		q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
		quaternarytable_.push_back(q_asg);
		// ����Ҳ���������ʱ�������ɻ���
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// ����Ҳ����������飬���������±�����ʱ�������ɻ���
		else if(Quaternary::TEMPORARY_OPERAND == offset_attribute.offset_operandtype_)
		{
			// ������һ������³���Եļٶ��������operandtype_����ARRAY�Ļ�����ôoperandtype_һ����OPERAND_NIL
			// �����������assert���һ�³����߼���������
			assert(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_);
			--tempvar_index_;
		}
	}
	else	// ��������ֵ
	{
		Quaternary q_ret;
		q_ret.op_ = Quaternary::RET;
		q_ret.type2_ = right_attribute.offset_operandtype_;
		q_ret.offset2_ = right_attribute.offset_;
		q_ret.type3_ = right_attribute.operandtype_;
		q_ret.dst_ = right_attribute.value_;
		quaternarytable_.push_back(q_ret);
		// ����Ҳ���������ʱ�������ɻ���
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// ����Ҳ����������飬���������±�����ʱ�������ɻ���
		else if(Quaternary::TEMPORARY_OPERAND == offset_attribute.offset_operandtype_)
		{
			// ������һ������³���Եļٶ��������operandtype_����ARRAY�Ļ�����ôoperandtype_һ����OPERAND_NIL
			// �����������assert���һ�³����߼���������
			assert(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_);
			--tempvar_index_;
		}
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
ExpressionAttribute SyntaxAnalyzer::Expression(int depth) throw()				// ���ʽ
{
	PrintFunctionFrame("Expression()", depth);

	Quaternary q_neg;	// �������ɵ�NEG��Ԫʽ
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		if(Token::MINUS == token_.type_)	// ����Ǽ��ţ���Ҫ����һ����Ԫʽ
		{
			q_neg.op_ = Quaternary::NEG;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	//TokenTableItem::DecorateType decorate_type = Term(depth + 1);
	ExpressionAttribute first_term = Term(depth + 1);
	if(Quaternary::NEG == q_neg.op_)	// ���֮ǰ������һ������
	{
		// ����NEG����Ԫʽ
		q_neg.type2_ = first_term.offset_operandtype_;
		q_neg.offset2_ = first_term.offset_;
		q_neg.type3_ = first_term.operandtype_;
		q_neg.dst_ = first_term.value_;
		quaternarytable_.push_back(q_neg);
	}

	Quaternary q_term;
	ExpressionAttribute new_term;
	bool is_first_operator = true;
	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// ��һ��ʱ�����new_term������Ԫ�أ���Ҫ���丳ֵ����ʱ����
		if(is_first_operator && Quaternary::ARRAY_OPERAND == first_term.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = first_term.operandtype_;
			q_subscript2temp.src1_ = first_term.value_;
			q_subscript2temp.type2_ = first_term.offset_operandtype_;
			q_subscript2temp.offset2_ = first_term.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == first_term.offset_operandtype_)
			{
				q_subscript2temp.dst_ = first_term.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			first_term.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			first_term.value_ = q_subscript2temp.dst_;
			first_term.offset_operandtype_ = Quaternary::OPERAND_NIL;
			first_term.offset_ = 0;
		}

		// ȷ����Ԫʽ�Ĳ�����
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term_decoratetype = is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
		new_term = Term(depth + 1);

		// ִ������ת��
		// ֻ���������Ͷ�ΪCHAR���������Ͳ���CHAR���������INTEGER
		if(last_term_decoratetype == TokenTableItem::CHAR && new_term.decoratetype_ == TokenTableItem::CHAR)
		{
			new_term.decoratetype_ = TokenTableItem::CHAR;
		}
		else
		{
			new_term.decoratetype_ = TokenTableItem::INTEGER;
		}

		// ���������new_term��������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
		// ������Ԫ�ص�ֵ������ʱ����
		if(Quaternary::ARRAY_OPERAND == new_term.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = new_term.operandtype_;
			q_subscript2temp.src1_ = new_term.value_;
			q_subscript2temp.type2_ = new_term.offset_operandtype_;
			q_subscript2temp.offset2_ = new_term.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == new_term.offset_operandtype_)
			{
				q_subscript2temp.dst_ = new_term.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			new_term.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			new_term.value_ = q_subscript2temp.dst_;
			new_term.offset_operandtype_ = Quaternary::OPERAND_NIL;
			new_term.offset_ = 0;
		}

		// ȷ����Ԫʽ�Ĳ�����
		// src1��ȷ����
		// ��һ�μ�/��ʱ��src1����while֮ǰ������Ǹ�term
		// ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
		if(is_first_operator)
		{
			q_term.type1_ = first_term.operandtype_;
			q_term.src1_ = first_term.value_;
			is_first_operator = false;
		}
		else
		{
			q_term.type1_ = q_term.type3_;
			q_term.src1_ = q_term.dst_;
		}
		// src2��ȷ����
		// src2���Ƕ������µ�term
		q_term.type2_ = new_term.operandtype_;
		q_term.src2_ = new_term.value_;
		// dst��ȷ����
		// ���src1����ʱ����������dstΪsrc1
		// �������src2����ʱ����������dstΪsrc2
		// ������dstΪ�µ���ʱ����
		if(Quaternary::TEMPORARY_OPERAND == q_term.type1_)
		{
			q_term.type3_ = q_term.type1_;
			q_term.dst_ = q_term.src1_;
			// ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
			if(Quaternary::TEMPORARY_OPERAND == q_term.type2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_OPERAND == q_term.type2_)
		{
			q_term.type3_ = q_term.type2_;
			q_term.dst_ = q_term.src2_;
		}
		else
		{
			q_term.type3_ = Quaternary::TEMPORARY_OPERAND;
			q_term.dst_ = tempvar_index_++;
		}
		// ������Ԫʽ
		quaternarytable_.push_back(q_term);
	}

	if(is_first_operator)	// ֻ��һ������
	{
		new_term = first_term;
	}
	else	// �ж���������Ҫ����new_term�����ԡ�����new_term����decoratetype_�⣬���඼�����һ�������
	{
		new_term.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		new_term.value_ = q_term.dst_;
		new_term.offset_operandtype_ = Quaternary::OPERAND_NIL;
		new_term.offset_ = 0;
	}
	return new_term;
}

// <��> ::= <����>{<�˷������><����>}
ExpressionAttribute SyntaxAnalyzer::Term(int depth) throw()						// ��
{
	PrintFunctionFrame("Term()", depth);

	ExpressionAttribute first_factor = Factor(depth + 1);

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// ��һ��ʱ�����new_factor������Ԫ�أ���Ҫ���丳ֵ����ʱ����
		if(is_first_operator && Quaternary::ARRAY_OPERAND == first_factor.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = first_factor.operandtype_;
			q_subscript2temp.src1_ = first_factor.value_;
			q_subscript2temp.type2_ = first_factor.offset_operandtype_;
			q_subscript2temp.offset2_ = first_factor.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == first_factor.offset_operandtype_)
			{
				q_subscript2temp.dst_ = first_factor.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			first_factor.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			first_factor.value_ = q_subscript2temp.dst_;
			first_factor.offset_operandtype_ = Quaternary::OPERAND_NIL;
			first_factor.offset_ = 0;
		}

		// ȷ����Ԫʽ�Ĳ�����
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor_decoratetype = is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
		new_factor = Factor(depth + 1);
		// ���������ת������ܼ򵥣�ֻ���������Ͷ�ΪCHAR���������Ͳ���CHAR
		// �������INTEGER
		if(last_factor_decoratetype == TokenTableItem::CHAR && new_factor.decoratetype_ == TokenTableItem::CHAR)
		{
			new_factor.decoratetype_ = TokenTableItem::CHAR;
		}
		else
		{
			new_factor.decoratetype_ = TokenTableItem::INTEGER;
		}

		// ����������������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
		// ������Ԫ�ص�ֵ������ʱ����
		if(Quaternary::ARRAY_OPERAND == new_factor.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = new_factor.operandtype_;
			q_subscript2temp.src1_ = new_factor.value_;
			q_subscript2temp.type2_ = new_factor.offset_operandtype_;
			q_subscript2temp.offset2_ = new_factor.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// Ŀ�����ʱ������ŵ�ȷ��
			// ��������±������ʱ��������ô�����������
			// ������¿�һ����ʱ����
			if(Quaternary::TEMPORARY_OPERAND == new_factor.offset_operandtype_)
			{
				q_subscript2temp.dst_ = new_factor.offset_;
			}
			else
			{
				q_subscript2temp.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_subscript2temp);
			new_factor.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			new_factor.value_ = q_subscript2temp.dst_;
			new_factor.offset_operandtype_ = Quaternary::OPERAND_NIL;
			new_factor.offset_ = 0;
		}

		// ȷ����Ԫʽ�Ĳ�����
		// src1��ȷ����
		// ��һ�γ�/����src1����while֮ǰ������Ǹ�factor
		// ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
		if(is_first_operator)
		{
			q_factor.type1_ = first_factor.operandtype_;
			q_factor.src1_ = first_factor.value_;
			is_first_operator = false;
		}
		else
		{
			q_factor.type1_ = q_factor.type3_;
			q_factor.src1_ = q_factor.dst_;
		}
		// src2��ȷ����
		// src2�Ƕ������µ�factor
		q_factor.type2_ = new_factor.operandtype_;
		q_factor.src2_ = new_factor.value_;
		// dst��ȷ����
		// ���src1����ʱ����������dstΪsrc1
		// �������src2����ʱ����������dstΪsrc2
		// ������dstΪ�µ���ʱ����
		if(Quaternary::TEMPORARY_OPERAND == q_factor.type1_)
		{
			q_factor.type3_ = q_factor.type1_;
			q_factor.dst_ = q_factor.src1_;
			// ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
			if(Quaternary::TEMPORARY_OPERAND == q_factor.type2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_OPERAND == q_factor.type2_)
		{
			q_factor.type3_ = q_factor.type2_;
			q_factor.dst_ = q_factor.src2_;
		}
		else
		{
			q_factor.type3_ = Quaternary::TEMPORARY_OPERAND;
			q_factor.dst_ = tempvar_index_++;
		}
		// ������Ԫʽ
		quaternarytable_.push_back(q_factor);
	}
	if(is_first_operator)	// ֻ��һ������
	{
		new_factor = first_factor;
	}
	else
	{
		// ����new_factor������
		// ����new_factor��decoratetype�⣬�������Ծ����������һ�����ӵ�����
		new_factor.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		new_factor.value_ = q_factor.dst_;
		new_factor.offset_operandtype_ = Quaternary::OPERAND_NIL;
		new_factor.offset_ = 0;
	}
	return new_factor;
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
ExpressionAttribute SyntaxAnalyzer::Factor(int depth) throw()					// ����
{
	PrintFunctionFrame("Factor()", depth);
	ExpressionAttribute factor_attribute;	// ��¼��factor���ӵ���Ϣ

	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);	// Ѱ�Ҷ���
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return factor_attribute;
		}
		factor_attribute.decoratetype_ = iter->GetDecorateType();
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		Token idToken = token_;	// ���£�����
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// ����Ԫ��
		{
			// factor_attribute�Լ���������ֵ
			factor_attribute.operandtype_ = Quaternary::ARRAY_OPERAND;
			factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			// ���ͼ�飺�Ƿ�Ϊ������
			if(iter->GetItemType() != TokenTableItem::ARRAY)	
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute offset_attribute = Expression(depth + 1);
			// ȷ��factor_attribute���±�
			// �����offset_attribute����������Ԫ�أ�����ṹ��Ƕ�׵������±꣨�������±�����һ������Ԫ�أ����޷��������Ԫʽ
			// ��������飬��Ҫ��offset_attribute�浽��ʱ�����У���Ϊ��ǰfactor���±�
			if(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_)
			{
				// �½�һ����Ԫʽ����Expression�Ľ�����浽��ʱ����TEMP#tempvar_index_��
				Quaternary q_assign;
				q_assign.op_ = Quaternary::ASG;
				q_assign.type1_ = offset_attribute.operandtype_;
				q_assign.src1_ = offset_attribute.value_;
				q_assign.type2_ = offset_attribute.offset_operandtype_;
				q_assign.offset2_ = offset_attribute.offset_;
				q_assign.type3_ = Quaternary::TEMPORARY_OPERAND;
				q_assign.dst_ = tempvar_index_++;
				quaternarytable_.push_back(q_assign);
				// ���µ���ʱ������Ϊ��ǰfactor�������±�
				factor_attribute.offset_operandtype_ = Quaternary::TEMPORARY_OPERAND;
				factor_attribute.offset_ = q_assign.dst_;
			}
			else
			{
				factor_attribute.offset_operandtype_ = offset_attribute.operandtype_;
				factor_attribute.offset_ = offset_attribute.value_;
			}

			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)
		{

			// ����Ƿ�Ϊ����
			if(iter->GetItemType() != TokenTableItem::FUNCTION)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a function\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// �ӷ��ű���ȡ�������Ĳ������ͣ���FunctionCallStatementȥƥ�����
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			// ���ɺ������õ���Ԫʽ
			// ���������ɵ�����䣬����FunctionCallStatement�����ɲ����������
			// �Ա���ת�ɻ��ʱ�������ں�������ʱ����������ֵ�Ĵ洢�ռ䣬Ȼ����Ϊ��������洢�ռ�
			Quaternary q_functioncall(Quaternary::FUNC_CALL, Quaternary::OPERAND_NIL, 0, Quaternary::PARANUM_OPERAND, decorate_types.size(), Quaternary::PROC_FUNC_INDEX, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_functioncall);
			FunctionCallStatement(idToken, decorate_types, depth + 1);
			// Important! 
			// Լ���ã������ķ���ֵ������temp#tempvar_index��λ��
			// ����factor_attribute�Լ���������ֵ������ʱ����������
			factor_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			factor_attribute.value_ = tempvar_index_++;
			factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
			factor_attribute.offset_ = 0;
		}
		else
		{
			// ���ͼ�飺�Ƿ�Ϊ���������������/�����Ĳ���
			if(iter->GetItemType() != TokenTableItem::VARIABLE && iter->GetItemType() != TokenTableItem::PARAMETER && iter->GetItemType() != TokenTableItem::CONST)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  single token_ Factor should be varaible or constant\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// factor_attribute������
			if(TokenTableItem::CONST == iter->GetItemType())
			{
				factor_attribute.operandtype_ = Quaternary::CONSTANT_OPERAND;
			}
			else
			{
				factor_attribute.operandtype_ = Quaternary::VARIABLE_OPERAND;
			}
			factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
			factor_attribute.offset_ = 0;
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// �����������ı��ʽ
	{
		factor_attribute = Expression(depth + 1);	// ��¼����
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return factor_attribute;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

		factor_attribute.decoratetype_ = TokenTableItem::INTEGER;	// ��¼����
		factor_attribute.operandtype_ = Quaternary::IMMEDIATE_OPERAND;
		factor_attribute.value_ = token_.value_.integer;
		factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

		factor_attribute.decoratetype_ = TokenTableItem::CHAR;	// ��¼����
		factor_attribute.operandtype_ = Quaternary::IMMEDIATE_OPERAND;
		factor_attribute.value_ = token_.value_.character;
		factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// ������
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not a legal Factor\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
	}
	return factor_attribute;
}

// <�������> ::= if<����>then<���>[else<���>]
void SyntaxAnalyzer::IfStatement(int depth) throw()				// �������
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// ������һ��label
	int label1 = label_index_++;
	// ��ȡ�������
	lexical_analyzer_.GetNextToken(token_);
	Condition(label1, depth + 1);	// ��condition��������ת���
	if(Token::THEN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"then\" after Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡif�ɹ�������
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	

	// ��ȡelse�����
	if(Token::ELSE == token_.type_)
	{
		// ����ڶ���label
		int label2 =  label_index_++;
		// ������������ת���
		Quaternary q_jmp(Quaternary::JMP, 
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::IMMEDIATE_OPERAND, label2);
		quaternarytable_.push_back(q_jmp);
		// ���õ�һ��label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::OPERAND_NIL, 0,
				Quaternary::OPERAND_NIL, 0,
				Quaternary::IMMEDIATE_OPERAND, label1);
		quaternarytable_.push_back(q_label1);
		// ��ȡelse�е����
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
		// ���õڶ���label
		Quaternary q_label2(Quaternary::LABEL, 
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::IMMEDIATE_OPERAND, label2);
		quaternarytable_.push_back(q_label2);
	}
	else	// ���û��else��䣬����if���������ʱ�����õ�һ��label
	{
		// ���õ�һ��label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::OPERAND_NIL, 0,
				Quaternary::OPERAND_NIL, 0,
				Quaternary::IMMEDIATE_OPERAND, label1);
		quaternarytable_.push_back(q_label1);
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>
// ��if����д�������label�����������ڴ���conditionʱ������ת���
void SyntaxAnalyzer::Condition(int label, int depth) throw()				// ����
{
	PrintFunctionFrame("Condition()", depth);

	ExpressionAttribute left_attribute = Expression(depth + 1);
	// ��������Ϊ��ʱ����
	if(Quaternary::TEMPORARY_OPERAND == left_attribute.operandtype_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.type1_ = left_attribute.operandtype_;
		q_subscript2temp.src1_ = left_attribute.value_;
		q_subscript2temp.type2_ = left_attribute.offset_operandtype_;
		q_subscript2temp.offset2_ = left_attribute.offset_;
		q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
		// Ŀ�����ʱ������ŵ�ȷ��
		// ��������±������ʱ��������ô�����������
		// ������¿�һ����ʱ����
		if(Quaternary::TEMPORARY_OPERAND == left_attribute.offset_operandtype_)
		{
			q_subscript2temp.dst_ = left_attribute.offset_;
		}
		else
		{
			q_subscript2temp.dst_ = tempvar_index_++;
		}
		quaternarytable_.push_back(q_subscript2temp);
		left_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		left_attribute.value_ = q_subscript2temp.dst_;
		left_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
		left_attribute.offset_ = 0;
	}
	if(		Token::LT	!= token_.type_
		&&	Token::LEQ	!= token_.type_
		&&	Token::GT	!= token_.type_
		&&	Token::GEQ	!= token_.type_
		&&	Token::EQU	!= token_.type_
		&&	Token::NEQ	!= token_.type_
		)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be logic operator in the middle of Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	Token compare_token = token_;
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);
	// ��������Ϊ��ʱ����
	if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.type1_ = right_attribute.operandtype_;
		q_subscript2temp.src1_ = right_attribute.value_;
		q_subscript2temp.type2_ = right_attribute.offset_operandtype_;
		q_subscript2temp.offset2_ = right_attribute.offset_;
		q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
		// Ŀ�����ʱ������ŵ�ȷ��
		// ��������±������ʱ��������ô�����������
		// ������¿�һ����ʱ����
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.offset_operandtype_)
		{
			q_subscript2temp.dst_ = right_attribute.offset_;
		}
		else
		{
			q_subscript2temp.dst_ = tempvar_index_++;
		}
		quaternarytable_.push_back(q_subscript2temp);
		right_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		right_attribute.value_ = q_subscript2temp.dst_;
		right_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
		right_attribute.offset_ = 0;
	}

	// ��������������ת���
	// ����������ʱ�Ż���ת����������Ĳ������������tokenҪ��һ��
	Quaternary q_jmp_condition;
	switch(compare_token.type_)
	{
	case Token::LT:
		q_jmp_condition.op_ = Quaternary::JNL;
		break;
	case Token::LEQ:
		q_jmp_condition.op_ = Quaternary::JG;
		break;
	case Token::GT:
		q_jmp_condition.op_ = Quaternary::JNG;
		break;
	case Token::GEQ:
		q_jmp_condition.op_ = Quaternary::JL;
		break;
	case Token::EQU:
		q_jmp_condition.op_ = Quaternary::JNE;
		break;
	case Token::NEQ:
		q_jmp_condition.op_ = Quaternary::JE;
		break;
	// ��Ϊ֮ǰ�Ѿ������ˣ�������������²�������default
	default:
		assert(false);
		break;
	}
	// ������
	q_jmp_condition.type1_ = left_attribute.operandtype_;
	q_jmp_condition.src1_ = left_attribute.value_;
	q_jmp_condition.type2_ = right_attribute.operandtype_;
	q_jmp_condition.src2_ = right_attribute.value_;
	q_jmp_condition.type3_ = Quaternary::IMMEDIATE_OPERAND;
	q_jmp_condition.dst_ = label;
	// ������Ԫʽ
	quaternarytable_.push_back(q_jmp_condition);
	// ������ʱ����
	if(Quaternary::TEMPORARY_OPERAND == left_attribute.operandtype_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
	{
		--tempvar_index_;
	}
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SyntaxAnalyzer::CaseStatement(int depth) throw()			// ������
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	if(Token::OF != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" to specify the certain case\n";
		is_successful_ = false;
		// �������������дof���ʲ�����
	}
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseList(depth + 1);
	}while(Token::SEMICOLON == token_.type_);
	if(Token::END != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"end\" at the end of case Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::END == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <�����Ԫ��> ::= <���������>:<���>
void SyntaxAnalyzer::CaseList(int depth) throw()					// �����Ԫ��
{
	PrintFunctionFrame("CaseList()", depth);

	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::CONST_INTEGER != token_.type_
			&& Token::CONST_CHAR != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after \"case\"\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(Token::COLON != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ':' after constant to specify the certain action\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// ����չ���������֧��
void SyntaxAnalyzer::ReadStatement(int depth) throw()			// �����
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);

	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the arguments\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::IDENTIFIER != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be variable name in the location of read argument\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in read call\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		// ����READ���õ���Ԫʽ
		Quaternary q_read(Quaternary::READ,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::VARIABLE_OPERAND, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
		quaternarytable_.push_back(q_read);
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}while(Token::COMMA == token_.type_);
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void SyntaxAnalyzer::WriteStatement(int depth) throw()			// д���
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the arguments\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::CONST_STRING == token_.type_)
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		
		vector<string>::const_iterator iter = std::find(stringtable_.begin(), stringtable_.end(), token_.value_.identifier);
		// ����WRITE���õ���Ԫʽ
		Quaternary q_read(Quaternary::WRITE,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::STRING_OPERAND, distance(static_cast<vector<string>::const_iterator>(stringtable_.begin()), iter));
		quaternarytable_.push_back(q_read);

		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute exp_attribute = Expression(depth + 1);
			// ����WRITE���õ���Ԫʽ
			Quaternary q_read(Quaternary::WRITE,
				Quaternary::OPERAND_NIL, 0,
				exp_attribute.offset_operandtype_, exp_attribute.offset_,
				exp_attribute.operandtype_, exp_attribute.value_);
			quaternarytable_.push_back(q_read);
			// ������ʱ����
			if(Quaternary::TEMPORARY_OPERAND == exp_attribute.operandtype_
			|| Quaternary::TEMPORARY_OPERAND == exp_attribute.offset_operandtype_)	// ���߲�����ͬʱ��������д��һ��
			{
				--tempvar_index_;
			}
		}
	}
	else
	{
		ExpressionAttribute exp_attribute = Expression(depth + 1);
		// ����WRITE���õ���Ԫʽ
		Quaternary q_read(Quaternary::WRITE,
			Quaternary::OPERAND_NIL, 0,
			exp_attribute.offset_operandtype_, exp_attribute.offset_,
			exp_attribute.operandtype_, exp_attribute.value_);
		quaternarytable_.push_back(q_read);
		// ������ʱ����
		if(Quaternary::TEMPORARY_OPERAND == exp_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == exp_attribute.offset_operandtype_)	// ���߲�����ͬʱ��������д��һ��
		{
			--tempvar_index_;
		}
	}
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <forѭ�����> ::= for <��ʶ��>  := <���ʽ> ��downto | to�� <���ʽ> do <���>
void SyntaxAnalyzer::ForLoopStatement(int depth) throw()			// forѭ�����
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);

	// ��ȡ��ʶ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::IDENTIFIER != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be loop variable name after for\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
	if(iter == tokentable_.end())
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
	if(iter->GetItemType() != TokenTableItem::VARIABLE
	&& iter->GetItemType() != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in for loop\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡ��ֵ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::ASSIGN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \":=\" after loop variable\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	// ���to/downto
	if(Token::DOWNTO != token_.type_
		&& Token::TO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"to\" or \"downto\" after variable assigning\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be do after loop head\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	// ��ȡѭ����
	Statement(depth + 1);
}

// <���̵������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::ProcedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// ���̵������
{
	PrintFunctionFrame("ProcedureCallStatement()", depth);

	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the argument\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	vector<ExpressionAttribute> attributes = ArgumentList(depth + 1);
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);

	// �����̲�������������Ƿ�ƥ��
	if(parameter_decorate_types.size() != attributes.size())	// ����������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  procedure does not take " << attributes.size() << " argument";
		if(attributes.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// �������������ֻ��ΪCHAR��INTEGER
		assert(attributes[i].decoratetype_ != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != attributes[i].decoratetype_)
		{
			// Ҫ�����ΪCHAR��ʵ�ʲ���ΪINTEGER�����޷�ת��
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && attributes[i].decoratetype_ == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				is_successful_ = false;
				// ��������Ǽ���˲������Ͳ�ƥ�䣬ʵ���﷨�ɷַ������ɼ�����
			}
		}
	}
}

// <�����������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::FunctionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// �����������
{
	PrintFunctionFrame("FunctionCallStatement()", depth);

	assert(Token::LEFT_PAREN == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	vector<ExpressionAttribute> attributes = ArgumentList(depth + 1);
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);

	// ��麯�������뺯�������Ƿ�ƥ��
	if(parameter_decorate_types.size() != attributes.size())	// ����������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  function does not take " << attributes.size() << " argument";
		if(attributes.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// �������������ֻ��ΪCHAR��INTEGER
		assert(attributes[i].decoratetype_ != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != attributes[i].decoratetype_)
		{
			// Ҫ�����ΪCHAR��ʵ�ʲ���ΪINTEGER�����޷�ת��
			if(TokenTableItem::CHAR == parameter_decorate_types[i] && TokenTableItem::INTEGER == attributes[i].decoratetype_)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				is_successful_ = false;
				// ��������Ǽ���˲������Ͳ�ƥ�䣬ʵ���﷨�ɷַ������ɼ������ʲ�����
			}
		}
	}
	
}