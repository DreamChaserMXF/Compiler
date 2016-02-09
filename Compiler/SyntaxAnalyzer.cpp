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

static string syntax_format_string_;	// 注：不是线程安全的
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
{
	
	if(depth * 4 == syntax_format_string_.size())
	{
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_format_string_.size())
	{
		syntax_format_string_.append("|");
		syntax_format_string_.append(depth * 4 - syntax_format_string_.size(), ' ');	// 这里不能减1
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else // depth * 4 < syntax_format_string_.size()
	{
		syntax_format_string_.resize(depth * 4);
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
}
// <程序> ::= <分程序>.
void SyntaxAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// 解析分程序
	SubRoutine(depth + 1);
	// 判断结束符号
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
}

// <分程序> ::= [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>
void SyntaxAnalyzer::SubRoutine(size_t depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	//
	// 四个可选分支
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
	// 一个必选分支
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

// <常量说明部分> ::= const<常量定义>{,<常量定义>};
void SyntaxAnalyzer::ConstantPart(size_t depth) throw()
{
	PrintFunctionFrame("ConstantPart()", depth);

	assert(Token::CONST == token_.type_);

	// 常量定义
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		constantDefination(depth + 1);
	} while(token_.type_ == Token::COMMA);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ';' after constant definition\n";
		is_successful_ = false;
		if(Token::VAR == token_.type_ || Token::PROCEDURE == token_.type_ || Token::FUNCTION == token_.type_ || Token::BEGIN == token_.type_)//	这里可能是忘记加分号了，所以直接返回而不读取下一个单词
		{
			return;	
		}
		else
		{
			while(token_.type_ != Token::NIL  && token_.type_ != Token::SEMICOLON)	// 若是其他单词，表示常量说明部分还未结束，故要读到下一个分号
			{
				lexical_analyzer_.GetNextToken(token_);
			};
			lexical_analyzer_.GetNextToken(token_);
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <常量定义> ::= <标识符>＝<常量>
void SyntaxAnalyzer::constantDefination(size_t depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of constant definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// 读到下一个逗号或分号
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 记录token_以插入符号表
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::EQU)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << "  should be '=' after identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// 读到下一个逗号或分号
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// 读到下一个逗号或分号
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

// <变量说明部分> ::= var <变量定义>;{<变量定义>;}
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
			if(Token::IDENTIFIER == token_.type_)	// 忘记加分号，后面还有变量定义
			{
				continue;
			}
			else
			{
				// 忘记加分号了，后面又无法继续接上变量定义，只能跳转到下一部分
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

// <变量定义> ::= <标识符>{,<标识符>}:<类型>
void SyntaxAnalyzer::VariableDefinition(size_t depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of variable definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER&& token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、下一个标识符或PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(token_.type_ != Token::IDENTIFIER)	// 若读到的不是标识符，则返回上一层处理
		{
			return;
		}
		// 读到了标识符，则继续执行
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、下一个标识符或PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			if(token_.type_ != Token::IDENTIFIER)	// 若读到的不是标识符，则返回上一层处理
			{
				return;
			}
			// 读到了标识符，则继续执行
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::RW_INTEGER && token_.type_ != Token::RW_CHAR && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、类型说明符、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_)	// 若读到了类型说明符
		{
			TypeSpecification(depth + 1);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <类型> ::= [array'['<无符号整数>']'of]<基本类型>
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;	// 数组这里的出错处理太过麻烦，故直接跳过这句，返回结果。
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::CONST_INTEGER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a constant integer after '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	if(token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)	// 若没有类型说明
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" for type specification\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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

// <过程说明部分> ::= <过程首部><分程序>;{<过程首部><分程序>;}
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
		lexical_analyzer_.GetNextToken(token_);	// 分程序结束后应读入分号
	}while(Token::PROCEDURE == token_.type_);
}

// <过程首部> ::= procedure<过程标识符>'('[<形式参数表>]')';
void SyntaxAnalyzer::ProcedureHead(size_t depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// 过程名在符号表中的位置（下标）

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// 未找到过程名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a procedure name after \"procedure\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 插入过程名到符号表
	string proc_name = token_.value_.identifier;	
	// 继续读取单词
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)	// 没有读到左括号，视作没有参数
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void SyntaxAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		FunctionHead(depth + 1);	// 进行函数头分析，并得到函数名在符号表中的位置
		SubRoutine(depth + 1);
		if(Token::SEMICOLON != token_.type_)	// 分程序结束后应读入分号
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// 读入结尾的分号
	}while(Token::FUNCTION == token_.type_);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
void SyntaxAnalyzer::FunctionHead(size_t depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// 函数名在符号表中的位置

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// 未找到函数名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a function name after \"function\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、BEGIN
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
	// 插入函数名到符号表
	string func_name = token_.value_.identifier;
	// 定位
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)	// 假设是忘记冒号
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// 这里有可能是落了分号，所以不再继续读
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function head\n";
		is_successful_ = false;
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <形式参数表> ::= <形式参数段>{;<形式参数段>}
// 返回形参数量
void SyntaxAnalyzer::ParameterList(size_t depth) throw()		// 形参表
{
	PrintFunctionFrame("ParameterList()", depth);

	ParameterTerm(depth + 1);
	while(Token::SEMICOLON == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		ParameterTerm(depth + 1);
	}
}

// <形式参数段> ::= [var]<标识符>{,<标识符>}:<基本类型>
// 返回该形参段的形参数量
void SyntaxAnalyzer::ParameterTerm(size_t depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);
	bool isref = false;	// 是否为引用传参
	if(Token::VAR == token_.type_)
	{
		isref = true;
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be parameter name surrounded by parentheses\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA  && token_.type_ != Token::SEMICOLON)	// 读到结尾、分号、BEGIN
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::IDENTIFIER)	// 读到结尾、分号、BEGIN
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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

// <实在参数表> ::= <表达式>{,<表达式>}
void SyntaxAnalyzer::ArgumentList(size_t depth) throw()			// 实参表
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

// <复合语句> ::= begin <语句>{;<语句>} end
void SyntaxAnalyzer::StatementBlockPart(size_t depth) throw()	// 复合语句
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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

// <语句> ::= <标识符>(<赋值语句>|<过程调用语句>)|<条件语句>|<情况语句>|<复合语句>
// |<读语句>|<写语句>|<while循环语句>|<for循环语句>|<循环继续语句>|<循环退出语句>|<空>
void SyntaxAnalyzer::Statement(size_t depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// 该token_可能是过程名，先记下，待用
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// 过程或函数调用
		{
			ProcFuncCallStatement(depth + 1);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(depth + 1);		
		}
		else if(Token::COLON == token_.type_
			|| Token::EQU == token_.type_)	// 此分支专门为了检查赋值号写错的情况
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  please check the spelling of the assigning operator\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		else
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error after identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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
		// 检测空语句是否合法（应该合法）
	case Token::SEMICOLON:	// 空语句
	case Token::END:		// 空语句
	default:
		break;
	}
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
// idToken是赋值语句之前标识符的token，iter是其在符号表中的迭代器
void SyntaxAnalyzer::AssigningStatement(size_t depth)			// 赋值语句
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// 为四元式生成而定义的变量
	bool assign2array= false;	// 是否为对数组的赋值操作
	
	ExpressionAttribute offset_attribute;	// 当对数组元素赋值时，存储偏移量（数组下标）的属性

	if(Token::LEFT_BRACKET == token_.type_)	// 对数组元素赋值
	{
		assign2array = true;
		// 读入表示下标的表达式
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
		// 语法检查
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	// 语法检查
	if(token_.type_ != Token::ASSIGN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  \":=\" doesn't occur in the Assigning Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}

	// 读入赋值号右边的表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
void SyntaxAnalyzer::Expression(size_t depth) throw()				// 表达式
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
		// 确定四元式的操作符
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		Term(depth + 1);
	}
}

// <项> ::= <因子>{<乘法运算符><因子>}
void SyntaxAnalyzer::Term(size_t depth) throw()						// 项
{
	PrintFunctionFrame("Term()", depth);

	Factor(depth + 1);

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// 确定四元式的操作符
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// 语法分析：读取下一项
		lexical_analyzer_.GetNextToken(token_);
		Factor(depth + 1);
	}
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')' 
//          | <无符号整数> 
//          | <字符>
void SyntaxAnalyzer::Factor(size_t depth) throw()					// 因子
{
	PrintFunctionFrame("Factor()", depth);

	// 语法检查：标识符的情况【变量、常变量、数组、函数调用】
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		Token idToken = token_;	// 记下，待用
		lexical_analyzer_.GetNextToken(token_);
		// 数组元素
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// 语法：读入作为下标的表达式
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);
			// 语法检查
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return ;
			}
			// 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// 左括号，函数调用
		{
			ProcFuncCallStatement(depth + 1);			
		}
		else
		{
			// 单独的标识符
		}

	}
	else if(Token::LEFT_PAREN == token_.type_)	// 括号括起来的表达式
	{
		// bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
		// 读表达式的第一个单词
		lexical_analyzer_.GetNextToken(token_);
		// 再读取表达式
		Expression(depth + 1);	// 记录类型
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return ;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)	// 整型字面常量
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// 字符型字面常量
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// 语法：出错处理
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not a legal Factor\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
	}
	return ;
}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
void SyntaxAnalyzer::IfStatement(size_t depth) throw()				// 条件语句
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// 读取条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// 在condition中设置跳转语句
	// 语法检查
	if(Token::THEN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"then\" after Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 读取if成功后的语句
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// 读取else的语句（如果有的话）
	if(Token::ELSE == token_.type_)
	{
		// 读取else中的语句
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>
// 由if或for语句中传递下来label参数，标识if语句块或for循环体的结束
// 用于在处理condition时设置跳转语句
void SyntaxAnalyzer::Condition(size_t depth) throw()				// 条件
{
	PrintFunctionFrame("Condition()", depth);

	Expression(depth + 1);
	switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是THEN（当条件中仅有一个表达式时）
	{
	case Token::LT:
	case Token::LEQ:
	case Token::GT:
	case Token::GEQ:
	case Token::EQU:
	case Token::NEQ:
	case Token::THEN:
		break;
	// 因为之前已经检查过了，所以正常情况下不可能有default
	default:
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be logic operator in the middle of Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
		break;
	}
	if(Token::THEN != token_.type_)	// 如果还有下一个表达式，再继续读取
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
}

// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void SyntaxAnalyzer::CaseStatement(size_t depth) throw()			// 情况语句
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	if(Token::OF != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" to specify the certain case\n";
		is_successful_ = false;
		// 这里假设是忘记写of，故不返回
	}
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);

	// 检测结束标志
	if(Token::END != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"end\" at the end of case Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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

// <情况表元素> ::= <情况常量表>:<语句>
// <情况常量表> ::=  <常量 | 常变量>{, <常量 | 常变量>}
void SyntaxAnalyzer::CaseElement(size_t depth) throw()					// 情况表元素
{
	PrintFunctionFrame("CaseElement()", depth);

	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character or constant variable after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return ;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// 读取下一个单词
		lexical_analyzer_.GetNextToken(token_);
	}
	if(Token::COLON != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ':' after constant to specify the certain action\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return ;
	}
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
// TODO 扩展出对数组的支持
void SyntaxAnalyzer::ReadStatement(size_t depth) throw()			// 读语句
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// 读取下一个单词，判断是否为数组元素
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// 不是数组元素
		{
			// 读入数组下标的第一个单词
			lexical_analyzer_.GetNextToken(token_);
			// 读整个下标的表达式
			Expression(depth + 1);
			// 判断右括号
			if(Token::RIGHT_BRACKET != token_.type_)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
			}
			else
			{
				// 读入下一个单词
				lexical_analyzer_.GetNextToken(token_);
			}
		}
	}while(Token::COMMA == token_.type_);
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <写语句> ::= write'(' (<字符串>[,<表达式>] | <表达式>) ')'
void SyntaxAnalyzer::WriteStatement(size_t depth) throw()			// 写语句
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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
			Expression(depth + 1);	// 读取第二个参数（表达式）
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <while循环语句> ::= while <条件> do <语句>
// <while循环语句> ::= while @Label<check> <条件> @JZLabel<end> do <语句> @JMPLabel<check> @Label<end>
void SyntaxAnalyzer::WhileLoopStatement(size_t depth) throw()			// while循环语句
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// 读取下一个单词，并进入条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// 条件语句中会执行动作@JZLabel<end>
	// 语法检查
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"do\" before loop body\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 读入循环体的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 读入循环体
	Statement(depth + 1);
}

// <for循环语句> ::= for <标识符> := <表达式> （downto | to） <表达式> do <语句>
// <for循环语句> ::= for <标识符> := <表达式> （downto | to）
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <表达式> @JZLABEL<end> 
// do <语句>@JMPLABEL<vary>@Label<end>
void SyntaxAnalyzer::ForLoopStatement(size_t depth) throw()			// for循环语句
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);

	// 读取标识符
	lexical_analyzer_.GetNextToken(token_);
	if(Token::IDENTIFIER != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be loop variable name after for\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 读取赋值号
	lexical_analyzer_.GetNextToken(token_);
	if(Token::ASSIGN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \":=\" after loop variable\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// 检测to/downto
	if(Token::DOWNTO != token_.type_
		&& Token::TO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"to\" or \"downto\" after variable assigning\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 保存递增/减符号
	Token vary_token = token_;
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// 读取DO
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be do after loop head\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	// 读取循环体
	Statement(depth + 1);
}

void SyntaxAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}
void SyntaxAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}


// <过程/函数调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::ProcFuncCallStatement(size_t depth)	// 过程调用语句
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	// 语法检查
	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the argument\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 语法：读入右括号或参数表的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 语法：读参数
	// TODO 在语义代码中作相应修改
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(depth + 1);
		// 语法检查
		if(Token::RIGHT_PAREN != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
}
