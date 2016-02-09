#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include <assert.h>
#include <sstream>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer) throw()
	: lexical_analyzer_(lexical_analyzer), token_(), is_successful_(true), syntax_process_buffer_(), syntax_format_string_()
{
	lexical_analyzer.ResetTokenPos();
}


bool SyntaxAnalyzer::Parse() throw()
{
	size_t depth = 0;
	syntax_process_buffer_.str("");
	syntax_format_string_.clear();
	PrintFunctionFrame("Parse()", depth);
	lexical_analyzer_.GetNextToken(token_);
	Routine(depth + 1);
	return is_successful_;
}

string SyntaxAnalyzer::toString() const throw()
{
	return syntax_process_buffer_.str();
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
	output << syntax_process_buffer_.str() << std::endl;
}

static string syntax_format_string_;	// ע�������̰߳�ȫ��
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
{
	
	if(depth * 4 == syntax_format_string_.size())
	{
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_format_string_.size())
	{
		syntax_format_string_.append("|");
		syntax_format_string_.append(depth * 4 - syntax_format_string_.size(), ' ');	// ���ﲻ�ܼ�1
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else // depth * 4 < syntax_format_string_.size()
	{
		syntax_format_string_.resize(depth * 4);
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SyntaxAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// �����ֳ���
	SubRoutine(depth + 1);
	// �жϽ�������
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SyntaxAnalyzer::SubRoutine(size_t depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	//
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
void SyntaxAnalyzer::ConstantPart(size_t depth) throw()
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
void SyntaxAnalyzer::constantDefination(size_t depth) throw()
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
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= var <��������>;{<��������>;}
void SyntaxAnalyzer::VariablePart(size_t depth) throw()
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
void SyntaxAnalyzer::VariableDefinition(size_t depth) throw()
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
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
void SyntaxAnalyzer::TypeSpecification(size_t depth) throw()
{
	PrintFunctionFrame("TypeSpecification()", depth);

	if(token_.type_ == Token::ARRAY)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" after []\n";
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
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::ProcedurePart(size_t depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		if(Token::SEMICOLON != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �ֳ��������Ӧ����ֺ�
	}while(Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
void SyntaxAnalyzer::ProcedureHead(size_t depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// �������ڷ��ű��е�λ�ã��±꣩

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a procedure name after \"procedure\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��������������ű�
	string proc_name = token_.value_.identifier;	
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
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		ParameterList(depth);
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
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure declaration\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		FunctionHead(depth + 1);	// ���к���ͷ���������õ��������ڷ��ű��е�λ��
		SubRoutine(depth + 1);
		if(Token::SEMICOLON != token_.type_)	// �ֳ��������Ӧ����ֺ�
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �����β�ķֺ�
	}while(Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
void SyntaxAnalyzer::FunctionHead(size_t depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// �������ڷ��ű��е�λ��

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a function name after \"function\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ���뺯���������ű�
	string func_name = token_.value_.identifier;
	// ��λ
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
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		ParameterList(depth + 1);
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
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)	// ����������ð��
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ':' after Parameter Statement\n";
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
		return ;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// �����п��������˷ֺţ����Բ��ټ�����
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function head\n";
		is_successful_ = false;
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
void SyntaxAnalyzer::ParameterList(size_t depth) throw()		// �βα�
{
	PrintFunctionFrame("ParameterList()", depth);

	ParameterTerm(depth + 1);
	while(Token::SEMICOLON == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		ParameterTerm(depth + 1);
	}
}

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
void SyntaxAnalyzer::ParameterTerm(size_t depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);
	bool isref = false;	// �Ƿ�Ϊ���ô���
	if(Token::VAR == token_.type_)
	{
		isref = true;
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
			return ;
		}
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
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
				return ;
			}
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
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
		return ;
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
		return ;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
void SyntaxAnalyzer::ArgumentList(size_t depth) throw()			// ʵ�α�
{
	PrintFunctionFrame("ArgumentList()", depth);

	Expression(depth + 1);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
	return ;
}

// <�������> ::= begin <���>{;<���>} end
void SyntaxAnalyzer::StatementBlockPart(size_t depth) throw()	// �������
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

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>
// |<�����>|<д���>|<whileѭ�����>|<forѭ�����>|<ѭ���������>|<ѭ���˳����>|<��>
void SyntaxAnalyzer::Statement(size_t depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// ��token_�����ǹ��������ȼ��£�����
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// ���̻�������
		{
			ProcFuncCallStatement(depth + 1);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(depth + 1);		
		}
		else if(Token::COLON == token_.type_
			|| Token::EQU == token_.type_)	// �˷�֧ר��Ϊ�˼�鸳ֵ��д������
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
	case Token::SEMICOLON:	// �����
	case Token::END:		// �����
	default:
		break;
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void SyntaxAnalyzer::AssigningStatement(size_t depth)			// ��ֵ���
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// Ϊ��Ԫʽ���ɶ�����ı���
	bool assign2array= false;	// �Ƿ�Ϊ������ĸ�ֵ����
	
	ExpressionAttribute offset_attribute;	// ��������Ԫ�ظ�ֵʱ���洢ƫ�����������±꣩������

	if(Token::LEFT_BRACKET == token_.type_)	// ������Ԫ�ظ�ֵ
	{
		assign2array = true;
		// �����ʾ�±�ı��ʽ
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
		// �﷨���
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
	// �﷨���
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

	// ���븳ֵ���ұߵı��ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
void SyntaxAnalyzer::Expression(size_t depth) throw()				// ���ʽ
{
	PrintFunctionFrame("Expression()", depth);
	
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}

	Term(depth + 1);

	Quaternary q_term;
	ExpressionAttribute new_term;
	bool is_first_operator = true;
	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// ȷ����Ԫʽ�Ĳ�����
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		Term(depth + 1);
	}
}

// <��> ::= <����>{<�˷������><����>}
void SyntaxAnalyzer::Term(size_t depth) throw()						// ��
{
	PrintFunctionFrame("Term()", depth);

	Factor(depth + 1);

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// ȷ����Ԫʽ�Ĳ�����
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// �﷨��������ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		Factor(depth + 1);
	}
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
void SyntaxAnalyzer::Factor(size_t depth) throw()					// ����
{
	PrintFunctionFrame("Factor()", depth);

	// �﷨��飺��ʶ��������������������������顢�������á�
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		Token idToken = token_;	// ���£�����
		lexical_analyzer_.GetNextToken(token_);
		// ����Ԫ��
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// �﷨��������Ϊ�±�ı��ʽ
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);
			// �﷨���
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return ;
			}
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// �����ţ���������
		{
			ProcFuncCallStatement(depth + 1);			
		}
		else
		{
			// �����ı�ʶ��
		}

	}
	else if(Token::LEFT_PAREN == token_.type_)	// �����������ı��ʽ
	{
		// bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
		// �����ʽ�ĵ�һ������
		lexical_analyzer_.GetNextToken(token_);
		// �ٶ�ȡ���ʽ
		Expression(depth + 1);	// ��¼����
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return ;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)	// �������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// �ַ������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// �﷨��������
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not a legal Factor\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
	}
	return ;
}

// <�������> ::= if<����>then<���>[else<���>]
void SyntaxAnalyzer::IfStatement(size_t depth) throw()				// �������
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// ��ȡ�������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// ��condition��������ת���
	// �﷨���
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

	// ��ȡelse����䣨����еĻ���
	if(Token::ELSE == token_.type_)
	{
		// ��ȡelse�е����
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>
// ��if��for����д�������label��������ʶif�����forѭ����Ľ���
// �����ڴ���conditionʱ������ת���
void SyntaxAnalyzer::Condition(size_t depth) throw()				// ����
{
	PrintFunctionFrame("Condition()", depth);

	Expression(depth + 1);
	switch(token_.type_)	// ������ʿ����ǹ�ϵ���������Ҳ�п�����THEN���������н���һ�����ʽʱ��
	{
	case Token::LT:
	case Token::LEQ:
	case Token::GT:
	case Token::GEQ:
	case Token::EQU:
	case Token::NEQ:
	case Token::THEN:
		break;
	// ��Ϊ֮ǰ�Ѿ������ˣ�������������²�������default
	default:
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be logic operator in the middle of Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
		break;
	}
	if(Token::THEN != token_.type_)	// ���������һ�����ʽ���ټ�����ȡ
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SyntaxAnalyzer::CaseStatement(size_t depth) throw()			// ������
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// ��ȡ���ʽ
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
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);

	// ��������־
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
// <���������> ::=  <���� | ������>{, <���� | ������>}
void SyntaxAnalyzer::CaseElement(size_t depth) throw()					// �����Ԫ��
{
	PrintFunctionFrame("CaseElement()", depth);

	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character or constant variable after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
			return ;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// ��ȡ��һ������
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
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// TODO ��չ���������֧��
void SyntaxAnalyzer::ReadStatement(size_t depth) throw()			// �����
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// ��ȡ��һ�����ʣ��ж��Ƿ�Ϊ����Ԫ��
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// ��������Ԫ��
		{
			// ���������±�ĵ�һ������
			lexical_analyzer_.GetNextToken(token_);
			// �������±�ı��ʽ
			Expression(depth + 1);
			// �ж�������
			if(Token::RIGHT_BRACKET != token_.type_)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
			}
			else
			{
				// ������һ������
				lexical_analyzer_.GetNextToken(token_);
			}
		}
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
void SyntaxAnalyzer::WriteStatement(size_t depth) throw()			// д���
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);	// ��ȡ�ڶ������������ʽ��
		}
	}
	else
	{
		Expression(depth + 1);
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

// <whileѭ�����> ::= while <����> do <���>
// <whileѭ�����> ::= while @Label<check> <����> @JZLabel<end> do <���> @JMPLabel<check> @Label<end>
void SyntaxAnalyzer::WhileLoopStatement(size_t depth) throw()			// whileѭ�����
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// ��ȡ��һ�����ʣ��������������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// ��������л�ִ�ж���@JZLabel<end>
	// �﷨���
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"do\" before loop body\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ����ѭ����ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// ����ѭ����
	Statement(depth + 1);
}

// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to�� <���ʽ> do <���>
// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to��
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <���ʽ> @JZLABEL<end> 
// do <���>@JMPLABEL<vary>@Label<end>
void SyntaxAnalyzer::ForLoopStatement(size_t depth) throw()			// forѭ�����
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
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
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
	// �������/������
	Token vary_token = token_;
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// ��ȡDO
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

void SyntaxAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}
void SyntaxAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}


// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::ProcFuncCallStatement(size_t depth)	// ���̵������
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	// �﷨���
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
	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// �﷨��������
	// TODO ���������������Ӧ�޸�
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(depth + 1);
		// �﷨���
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
	}
	lexical_analyzer_.GetNextToken(token_);
}
