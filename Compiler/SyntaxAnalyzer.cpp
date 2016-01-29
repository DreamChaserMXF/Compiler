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

static string syntax_assist_buffer_;	// 注：不是线程安全的
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, int depth) throw()
{
	
	if(depth * 4 == syntax_assist_buffer_.size())
	{
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_assist_buffer_.size())
	{
		syntax_assist_buffer_.append("|");
		syntax_assist_buffer_.append(depth * 4 - syntax_assist_buffer_.size(), ' ');	// 这里不能减1
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else // depth * 4 < syntax_assist_buffer_.size()
	{
		syntax_assist_buffer_.resize(depth * 4);
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
}
// <程序> ::= <分程序>.
void SyntaxAnalyzer::Routine(int depth) throw()
{
	PrintFunctionFrame("Routine()", depth);

	SubRoutine(depth + 1);
	// 判断结束符号
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
}

// <分程序> ::= [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>
void SyntaxAnalyzer::SubRoutine(int depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

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
void SyntaxAnalyzer::ConstantPart(int depth) throw()
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
		/*while(lexical_analyzer_.GetNextToken(token_) && token_.type_ != Token::SEMICOLON)
		{ }*/
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
void SyntaxAnalyzer::constantDefination(int depth) throw()
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
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
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
	// 常量定义，插入符号表
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

// <变量说明部分> ::= var <变量定义>;{<变量定义>;}
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
void SyntaxAnalyzer::VariableDefinition(int depth) throw()
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
		arrayLength = token_.value_.integer;
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
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
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" after [*]\n";
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
	// 插入符号表
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// 修饰符类型
	if(TokenTableItem::ARRAY == itemtype_)	// 若是数组
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))	// 重定义后，仍然插入当前定义（也可不插入）
			{
				std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
				is_successful_ = false;
			}
			tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
		}
	}
	else									// 若是一般变量
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

// <过程说明部分> ::= <过程首部><分程序>;{<过程首部><分程序>;}
void SyntaxAnalyzer::ProcedurePart(int depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		int proc_index = ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		// 生成过程的END四元式
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, proc_index));
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// 分程序结束后应读入分号
	}while(Token::PROCEDURE == token_.type_);
}

// <过程首部> ::= procedure<过程标识符>'('[<形式参数表>]')';
int SyntaxAnalyzer::ProcedureHead(int depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// 过程名在符号表中的位置（下标）

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// 未找到过程名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a procedure name after \"procedure\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// 这里进行定位，是为了抵消ProcedurePart中的重定位
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// 插入过程名到符号表
	string proc_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的过程分程序的语法语义分析）
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	proc_index = tokentable_.AddProcedureItem(token_, level_++);// 过程名之后leve要+1
	// 生成过程的BEGIN四元式
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, proc_index));
	// 定位（将过程名后紧邻的位置设为局部作用域的起始点）
	tokentable_.Locate();	
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void SyntaxAnalyzer::FunctionPart(int depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		int func_index = FunctionHead(depth + 1);	// 进行函数头分析，并得到函数名在符号表中的位置
		SubRoutine(depth + 1);
		// 生成函数的END四元式
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, func_index));
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)	// 分程序结束后应读入分号
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// 读入结尾的分号
	}while(Token::FUNCTION == token_.type_);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
int SyntaxAnalyzer::FunctionHead(int depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// 函数名在符号表中的位置

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// 未找到函数名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a function name after \"function\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// 这里进行定位，是为了抵消FunctionPart中的重定位
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、BEGIN
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
	// 插入函数名到符号表
	string func_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的函数分程序的语法语义分析）
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	func_index = tokentable_.AddFunctionItem(token_, level_++);// 过程名之后leve要+1
	// 生成函数的BEGIN四元式
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, Quaternary::OPERAND_NIL, 0, Quaternary::OPERAND_NIL, 0, Quaternary::PROC_FUNC_INDEX, func_index));
	// 定位
	tokentable_.Locate();
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)	// 假设是忘记冒号
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// 分号后再读一个单词，可能会进入分程序
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
	if(token_.type_ != Token::SEMICOLON)	// 这里有可能是落了分号，所以不再继续读
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function head\n";
		is_successful_ = false;
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return func_index;
}

// <形式参数表> ::= <形式参数段>{;<形式参数段>}
// 返回形参数量
int SyntaxAnalyzer::ParameterList(int depth) throw()		// 形参表
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

// <形式参数段> ::= [var]<标识符>{,<标识符>}:<基本类型>
// 返回该形参段的形参数量
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA  && token_.type_ != Token::SEMICOLON)	// 读到结尾、分号、BEGIN
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
	// 参数名压栈准备进符号表
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::IDENTIFIER)	// 读到结尾、分号、BEGIN
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return 0;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// 参数存符号表
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
		{
			std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
			is_successful_ = false;
		}
		tokentable_.AddParameterItem(*iter, decoratetype_, level_);
	} // end 存符号表

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}

// <实在参数表> ::= <表达式>{,<表达式>}
vector<ExpressionAttribute> SyntaxAnalyzer::ArgumentList(int depth) throw()			// 实参表
{
	PrintFunctionFrame("ArgumentList()", depth);

	vector<ExpressionAttribute> attribute_buffer;
	ExpressionAttribute argument_attribute = Expression(depth + 1);
	attribute_buffer.push_back(argument_attribute);
	// 生成设置参数的四元式
	Quaternary q_addpara(Quaternary::SETP, 
		Quaternary::OPERAND_NIL, 0, 
		argument_attribute.offset_operandtype_, argument_attribute.offset_, 
		argument_attribute.operandtype_, argument_attribute.value_);
	quaternarytable_.push_back(q_addpara);
	// 回收临时变量
	if(Quaternary::TEMPORARY_OPERAND == argument_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == argument_attribute.offset_operandtype_)	// 两者不可能同时成立，故写在一起
	{
		--tempvar_index_;
	}

	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		argument_attribute = Expression(depth + 1);
		attribute_buffer.push_back(argument_attribute);
		// 生成设置参数的四元式
		q_addpara.type2_ = argument_attribute.offset_operandtype_;
		q_addpara.offset2_ = argument_attribute.offset_;
		q_addpara.type3_ = argument_attribute.operandtype_;
		q_addpara.dst_ = argument_attribute.value_;
		quaternarytable_.push_back(q_addpara);
		// 回收临时变量
		if(Quaternary::TEMPORARY_OPERAND == argument_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == argument_attribute.offset_operandtype_)	// 两者不可能同时成立，故写在一起
		{
			--tempvar_index_;
		}
	}
	return attribute_buffer;
}

// <复合语句> ::= begin <语句>{;<语句>} end
void SyntaxAnalyzer::StatementBlockPart(int depth) throw()	// 复合语句
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

// <语句> ::= <标识符>(<赋值语句>|<过程调用语句>)|<条件语句>|<情况语句>|<复合语句>|<读语句>|<写语句>|<for循环语句>|<空>
void SyntaxAnalyzer::Statement(int depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// 该token_可能是过程名，先记下，待用
	TokenTable::iterator iter;
	TokenTableItem::DecorateType l_value_type = TokenTableItem::VOID;
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		iter = tokentable_.SearchDefinition(token_);	// 查找符号表中的定义
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// 过程调用
		{
			if(iter->GetItemType() != TokenTableItem::PROCEDURE)	// 检查其属性是否为过程
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a procedure\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			// 生成过程调用四元式
			Quaternary q_procedurecall(Quaternary::PROC_CALL, Quaternary::OPERAND_NIL, 0, Quaternary::PARANUM_OPERAND, decorate_types.size(), Quaternary::PROC_FUNC_INDEX, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_procedurecall);
			ProcedureCallStatement(idToken, decorate_types, depth + 1);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(idToken, iter, depth + 1);		
		}
		else if(Token::COLON == token_.type_)	// 此分支专门为了检查赋值号写错的情况
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
	case Token::FOR:
		ForLoopStatement(depth + 1);
		break;
		// TODO 检测空语句是否合法（应该合法）
	case Token::END:	// 空语句
		break;
	default:
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error at the beginning of Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		break;
	}
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
// idToken是赋值语句之前标识符的token，iter是其在符号表中的迭代器
void SyntaxAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth)			// 赋值语句
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// 为四元式生成而定义的变量
	bool assign2array= false;	// 是否为对数组的赋值操作
	
	ExpressionAttribute offset_attribute;	// 当对数组元素赋值时，存储偏移量的属性

	if(Token::LEFT_BRACKET == token_.type_)	// 对数组元素赋值
	{
		assign2array = true;
		if(iter->GetItemType() != TokenTableItem::ARRAY)	// 检查是否为数组名
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		offset_attribute = Expression(depth + 1);	// 这里的结果一定是在temp#tempvar_index_中存储的
		// 数组下标是数组元素的特殊情况处理
		if(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_)
		{
			// 插入四元式，将数组下标值赋给一个临时变量
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = offset_attribute.operandtype_;
			q_subscript2temp.src1_ = offset_attribute.value_;
			q_subscript2temp.type2_ = offset_attribute.offset_operandtype_;
			q_subscript2temp.offset2_ = offset_attribute.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER
		&& iter->GetItemType() != TokenTableItem::FUNCTION)	// 检查是否为变量或参数或函数名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 剩下只有三种情况：变量、参数或是函数返回值
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
	// 以下三行为抛出异常做准备，放在GetNextToken前是为了准确记录赋值号的行号
	static Token errToken;
	errToken.type_ = Token::ASSIGN;
	errToken.lineNumber_ = token_.lineNumber_;

	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);
	if(TokenTableItem::CHAR == iter->GetDecorateType() && TokenTableItem::INTEGER == right_attribute.decoratetype_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert from 'int' to 'char'\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 生成四元式
	if(assign2array)	// 对数组元素赋值
	{
		// 如果right_attribute是数组元素的话，要将其赋值给临时变量
		if(Quaternary::ARRAY_OPERAND == right_attribute.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = right_attribute.operandtype_;
			q_subscript2temp.src1_ = right_attribute.value_;
			q_subscript2temp.type2_ = right_attribute.offset_operandtype_;
			q_subscript2temp.offset2_ = right_attribute.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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
		// 进行数组赋值
		Quaternary q_asg;
		q_asg.op_ = Quaternary::AASG;
		q_asg.type1_ = right_attribute.operandtype_;
		q_asg.src1_ = right_attribute.value_;
		q_asg.type2_ = offset_attribute.operandtype_;
		q_asg.offset2_ = offset_attribute.value_;
		q_asg.type3_ = Quaternary::ARRAY_OPERAND;
		q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
		quaternarytable_.push_back(q_asg);

		// 如果右操作数是临时变量，可回收
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// 如果被赋值的数组下标也是临时变量，可回收
		if(Quaternary::TEMPORARY_OPERAND == offset_attribute.operandtype_)
		{
			--tempvar_index_;
		}
	}
	else if(TokenTableItem::PARAMETER == iter->GetItemType()
			|| TokenTableItem::VARIABLE == iter->GetItemType())	// 普通变量/参数的赋值
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
		// 如果右操作数是临时变量，可回收
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// 如果右操作数是数组，且其数组下标是临时变量，可回收
		else if(Quaternary::TEMPORARY_OPERAND == offset_attribute.offset_operandtype_)
		{
			// 这里有一个程序鲁棒性的假定，即如果operandtype_不是ARRAY的话，那么operandtype_一定是OPERAND_NIL
			// 所以用下面的assert检测一下程序逻辑有无问题
			assert(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_);
			--tempvar_index_;
		}
	}
	else	// 函数返回值
	{
		Quaternary q_ret;
		q_ret.op_ = Quaternary::RET;
		q_ret.type2_ = right_attribute.offset_operandtype_;
		q_ret.offset2_ = right_attribute.offset_;
		q_ret.type3_ = right_attribute.operandtype_;
		q_ret.dst_ = right_attribute.value_;
		quaternarytable_.push_back(q_ret);
		// 如果右操作数是临时变量，可回收
		if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
		{
			--tempvar_index_;
		}
		// 如果右操作数是数组，且其数组下标是临时变量，可回收
		else if(Quaternary::TEMPORARY_OPERAND == offset_attribute.offset_operandtype_)
		{
			// 这里有一个程序鲁棒性的假定，即如果operandtype_不是ARRAY的话，那么operandtype_一定是OPERAND_NIL
			// 所以用下面的assert检测一下程序逻辑有无问题
			assert(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_);
			--tempvar_index_;
		}
	}
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
ExpressionAttribute SyntaxAnalyzer::Expression(int depth) throw()				// 表达式
{
	PrintFunctionFrame("Expression()", depth);

	Quaternary q_neg;	// 可能生成的NEG四元式
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		if(Token::MINUS == token_.type_)	// 如果是减号，就要生成一项四元式
		{
			q_neg.op_ = Quaternary::NEG;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	//TokenTableItem::DecorateType decorate_type = Term(depth + 1);
	ExpressionAttribute first_term = Term(depth + 1);
	if(Quaternary::NEG == q_neg.op_)	// 如果之前读到了一个减号
	{
		// 生成NEG的四元式
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
		// 第一次时，如果new_term是数组元素，则要将其赋值给临时变量
		if(is_first_operator && Quaternary::ARRAY_OPERAND == first_term.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = first_term.operandtype_;
			q_subscript2temp.src1_ = first_term.value_;
			q_subscript2temp.type2_ = first_term.offset_operandtype_;
			q_subscript2temp.offset2_ = first_term.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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

		// 确定四元式的操作符
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term_decoratetype = is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
		new_term = Term(depth + 1);

		// 执行类型转换
		// 只有两个类型都为CHAR，最终类型才是CHAR。否则就是INTEGER
		if(last_term_decoratetype == TokenTableItem::CHAR && new_term.decoratetype_ == TokenTableItem::CHAR)
		{
			new_term.decoratetype_ = TokenTableItem::CHAR;
		}
		else
		{
			new_term.decoratetype_ = TokenTableItem::INTEGER;
		}

		// 如果读到的new_term还是数组元素，那么仍然需要一次转换
		// 将数组元素的值赋给临时变量
		if(Quaternary::ARRAY_OPERAND == new_term.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = new_term.operandtype_;
			q_subscript2temp.src1_ = new_term.value_;
			q_subscript2temp.type2_ = new_term.offset_operandtype_;
			q_subscript2temp.offset2_ = new_term.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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

		// 确定四元式的操作数
		// src1的确定：
		// 第一次加/减时，src1就是while之前读入的那个term
		// 之后加/减时，src1就是上一个四元式的结果
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
		// src2的确定：
		// src2就是读到的新的term
		q_term.type2_ = new_term.operandtype_;
		q_term.src2_ = new_term.value_;
		// dst的确定：
		// 如果src1是临时变量，就令dst为src1
		// 否则，如果src2是临时变量，就令dst为src2
		// 否则，令dst为新的临时变量
		if(Quaternary::TEMPORARY_OPERAND == q_term.type1_)
		{
			q_term.type3_ = q_term.type1_;
			q_term.dst_ = q_term.src1_;
			// 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
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
		// 保存四元式
		quaternarytable_.push_back(q_term);
	}

	if(is_first_operator)	// 只有一项的情况
	{
		new_term = first_term;
	}
	else	// 有多项的情况，要更新new_term的属性。否则new_term除了decoratetype_外，其余都是最后一项的属性
	{
		new_term.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		new_term.value_ = q_term.dst_;
		new_term.offset_operandtype_ = Quaternary::OPERAND_NIL;
		new_term.offset_ = 0;
	}
	return new_term;
}

// <项> ::= <因子>{<乘法运算符><因子>}
ExpressionAttribute SyntaxAnalyzer::Term(int depth) throw()						// 项
{
	PrintFunctionFrame("Term()", depth);

	ExpressionAttribute first_factor = Factor(depth + 1);

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// 第一次时，如果new_factor是数组元素，则要将其赋值给临时变量
		if(is_first_operator && Quaternary::ARRAY_OPERAND == first_factor.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = first_factor.operandtype_;
			q_subscript2temp.src1_ = first_factor.value_;
			q_subscript2temp.type2_ = first_factor.offset_operandtype_;
			q_subscript2temp.offset2_ = first_factor.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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

		// 确定四元式的操作符
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor_decoratetype = is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
		new_factor = Factor(depth + 1);
		// 这里的类型转换规则很简单：只有两个类型都为CHAR，最终类型才是CHAR
		// 否则就是INTEGER
		if(last_factor_decoratetype == TokenTableItem::CHAR && new_factor.decoratetype_ == TokenTableItem::CHAR)
		{
			new_factor.decoratetype_ = TokenTableItem::CHAR;
		}
		else
		{
			new_factor.decoratetype_ = TokenTableItem::INTEGER;
		}

		// 如果读到的项还是数组元素，那么仍然需要一次转换
		// 将数组元素的值赋给临时变量
		if(Quaternary::ARRAY_OPERAND == new_factor.operandtype_)
		{
			Quaternary q_subscript2temp;
			q_subscript2temp.op_ = Quaternary::ASG;
			q_subscript2temp.type1_ = new_factor.operandtype_;
			q_subscript2temp.src1_ = new_factor.value_;
			q_subscript2temp.type2_ = new_factor.offset_operandtype_;
			q_subscript2temp.offset2_ = new_factor.offset_;
			q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
			// 目标的临时变量标号的确定
			// 如果数组下标就是临时变量，那么就用这个变量
			// 否则就新开一个临时变量
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

		// 确定四元式的操作数
		// src1的确定：
		// 第一次乘/除，src1就是while之前读入的那个factor
		// 之后加/减时，src1就是上一个四元式的结果
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
		// src2的确定：
		// src2是读到的新的factor
		q_factor.type2_ = new_factor.operandtype_;
		q_factor.src2_ = new_factor.value_;
		// dst的确定：
		// 如果src1是临时变量，就令dst为src1
		// 否则，如果src2是临时变量，就令dst为src2
		// 否则，令dst为新的临时变量
		if(Quaternary::TEMPORARY_OPERAND == q_factor.type1_)
		{
			q_factor.type3_ = q_factor.type1_;
			q_factor.dst_ = q_factor.src1_;
			// 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
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
		// 保存四元式
		quaternarytable_.push_back(q_factor);
	}
	if(is_first_operator)	// 只有一个因子
	{
		new_factor = first_factor;
	}
	else
	{
		// 更新new_factor的属性
		// 否则new_factor除decoratetype外，其余属性均保留了最后一个因子的属性
		new_factor.operandtype_ = Quaternary::TEMPORARY_OPERAND;
		new_factor.value_ = q_factor.dst_;
		new_factor.offset_operandtype_ = Quaternary::OPERAND_NIL;
		new_factor.offset_ = 0;
	}
	return new_factor;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')' 
//          | <无符号整数> 
//          | <字符>
ExpressionAttribute SyntaxAnalyzer::Factor(int depth) throw()					// 因子
{
	PrintFunctionFrame("Factor()", depth);
	ExpressionAttribute factor_attribute;	// 记录该factor因子的信息

	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);	// 寻找定义
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return factor_attribute;
		}
		factor_attribute.decoratetype_ = iter->GetDecorateType();
		iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
		Token idToken = token_;	// 记下，待用
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// 数组元素
		{
			// factor_attribute自己的类型与值
			factor_attribute.operandtype_ = Quaternary::ARRAY_OPERAND;
			factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			// 类型检查：是否为数组名
			if(iter->GetItemType() != TokenTableItem::ARRAY)	
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute offset_attribute = Expression(depth + 1);
			// 确定factor_attribute的下标
			// 这里的offset_attribute不能是数组元素，否则会构成嵌套的数组下标（即数组下标又是一个数组元素），无法翻译成四元式
			// 如果是数组，就要把offset_attribute存到临时变量中，作为当前factor的下标
			if(Quaternary::ARRAY_OPERAND == offset_attribute.operandtype_)
			{
				// 新建一个四元式，将Expression的结果保存到临时变量TEMP#tempvar_index_中
				Quaternary q_assign;
				q_assign.op_ = Quaternary::ASG;
				q_assign.type1_ = offset_attribute.operandtype_;
				q_assign.src1_ = offset_attribute.value_;
				q_assign.type2_ = offset_attribute.offset_operandtype_;
				q_assign.offset2_ = offset_attribute.offset_;
				q_assign.type3_ = Quaternary::TEMPORARY_OPERAND;
				q_assign.dst_ = tempvar_index_++;
				quaternarytable_.push_back(q_assign);
				// 将新的临时变量作为当前factor的数组下标
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
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)
		{

			// 检查是否为函数
			if(iter->GetItemType() != TokenTableItem::FUNCTION)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a function\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// 从符号表中取出函数的参数类型，待FunctionCallStatement去匹配参数
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			// 生成函数调用的四元式
			// 这里先生成调用语句，再在FunctionCallStatement中生成参数设置语句
			// 以便在转成汇编时，方便在函数调用时就留出返回值的存储空间，然后再为参数分配存储空间
			Quaternary q_functioncall(Quaternary::FUNC_CALL, Quaternary::OPERAND_NIL, 0, Quaternary::PARANUM_OPERAND, decorate_types.size(), Quaternary::PROC_FUNC_INDEX, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_functioncall);
			FunctionCallStatement(idToken, decorate_types, depth + 1);
			// Important! 
			// 约定好，函数的返回值放置在temp#tempvar_index的位置
			// 所以factor_attribute自己的类型与值都是临时变量的类型
			factor_attribute.operandtype_ = Quaternary::TEMPORARY_OPERAND;
			factor_attribute.value_ = tempvar_index_++;
			factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
			factor_attribute.offset_ = 0;
		}
		else
		{
			// 类型检查：是否为变量、常量或过程/函数的参数
			if(iter->GetItemType() != TokenTableItem::VARIABLE && iter->GetItemType() != TokenTableItem::PARAMETER && iter->GetItemType() != TokenTableItem::CONST)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  single token_ Factor should be varaible or constant\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// factor_attribute的属性
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
	else if(Token::LEFT_PAREN == token_.type_)	// 括号括起来的表达式
	{
		factor_attribute = Expression(depth + 1);	// 记录类型
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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

		factor_attribute.decoratetype_ = TokenTableItem::INTEGER;	// 记录类型
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

		factor_attribute.decoratetype_ = TokenTableItem::CHAR;	// 记录类型
		factor_attribute.operandtype_ = Quaternary::IMMEDIATE_OPERAND;
		factor_attribute.value_ = token_.value_.character;
		factor_attribute.offset_operandtype_ = Quaternary::OPERAND_NIL;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// 出错处理
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not a legal Factor\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
	}
	return factor_attribute;
}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
void SyntaxAnalyzer::IfStatement(int depth) throw()				// 条件语句
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// 先申请一个label
	int label1 = label_index_++;
	// 读取条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(label1, depth + 1);	// 在condition中设置跳转语句
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

	

	// 读取else的语句
	if(Token::ELSE == token_.type_)
	{
		// 申请第二个label
		int label2 =  label_index_++;
		// 生成无条件跳转语句
		Quaternary q_jmp(Quaternary::JMP, 
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::IMMEDIATE_OPERAND, label2);
		quaternarytable_.push_back(q_jmp);
		// 设置第一个label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::OPERAND_NIL, 0,
				Quaternary::OPERAND_NIL, 0,
				Quaternary::IMMEDIATE_OPERAND, label1);
		quaternarytable_.push_back(q_label1);
		// 读取else中的语句
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
		// 设置第二个label
		Quaternary q_label2(Quaternary::LABEL, 
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::IMMEDIATE_OPERAND, label2);
		quaternarytable_.push_back(q_label2);
	}
	else	// 如果没有else语句，就在if语句块结束的时候设置第一个label
	{
		// 设置第一个label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::OPERAND_NIL, 0,
				Quaternary::OPERAND_NIL, 0,
				Quaternary::IMMEDIATE_OPERAND, label1);
		quaternarytable_.push_back(q_label1);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>
// 由if语句中传递下来label参数，用于在处理condition时设置跳转语句
void SyntaxAnalyzer::Condition(int label, int depth) throw()				// 条件
{
	PrintFunctionFrame("Condition()", depth);

	ExpressionAttribute left_attribute = Expression(depth + 1);
	// 化简数组为临时变量
	if(Quaternary::TEMPORARY_OPERAND == left_attribute.operandtype_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.type1_ = left_attribute.operandtype_;
		q_subscript2temp.src1_ = left_attribute.value_;
		q_subscript2temp.type2_ = left_attribute.offset_operandtype_;
		q_subscript2temp.offset2_ = left_attribute.offset_;
		q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
		// 目标的临时变量标号的确定
		// 如果数组下标就是临时变量，那么就用这个变量
		// 否则就新开一个临时变量
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	Token compare_token = token_;
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);
	// 化简数组为临时变量
	if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.type1_ = right_attribute.operandtype_;
		q_subscript2temp.src1_ = right_attribute.value_;
		q_subscript2temp.type2_ = right_attribute.offset_operandtype_;
		q_subscript2temp.offset2_ = right_attribute.offset_;
		q_subscript2temp.type3_ = Quaternary::TEMPORARY_OPERAND;
		// 目标的临时变量标号的确定
		// 如果数组下标就是临时变量，那么就用这个变量
		// 否则就新开一个临时变量
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

	// 生成有条件的跳转语句
	// 不符合条件时才会跳转，所以这里的操作符与读到的token要反一下
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
	// 因为之前已经检查过了，所以正常情况下不可能有default
	default:
		assert(false);
		break;
	}
	// 操作数
	q_jmp_condition.type1_ = left_attribute.operandtype_;
	q_jmp_condition.src1_ = left_attribute.value_;
	q_jmp_condition.type2_ = right_attribute.operandtype_;
	q_jmp_condition.src2_ = right_attribute.value_;
	q_jmp_condition.type3_ = Quaternary::IMMEDIATE_OPERAND;
	q_jmp_condition.dst_ = label;
	// 保存四元式
	quaternarytable_.push_back(q_jmp_condition);
	// 回收临时变量
	if(Quaternary::TEMPORARY_OPERAND == left_attribute.operandtype_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_OPERAND == right_attribute.operandtype_)
	{
		--tempvar_index_;
	}
}

// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void SyntaxAnalyzer::CaseStatement(int depth) throw()			// 情况语句
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);
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
		CaseList(depth + 1);
	}while(Token::SEMICOLON == token_.type_);
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
void SyntaxAnalyzer::CaseList(int depth) throw()					// 情况表元素
{
	PrintFunctionFrame("CaseList()", depth);

	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号
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
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
// 可扩展出对数组的支持
void SyntaxAnalyzer::ReadStatement(int depth) throw()			// 读语句
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
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
		if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER)	// 检查是否为变量或参数或函数名
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in read call\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		// 生成READ调用的四元式
		Quaternary q_read(Quaternary::READ,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::OPERAND_NIL, 0,
			Quaternary::VARIABLE_OPERAND, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
		quaternarytable_.push_back(q_read);
		// 读取下一个单词
		lexical_analyzer_.GetNextToken(token_);
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
void SyntaxAnalyzer::WriteStatement(int depth) throw()			// 写语句
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
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		
		vector<string>::const_iterator iter = std::find(stringtable_.begin(), stringtable_.end(), token_.value_.identifier);
		// 生成WRITE调用的四元式
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
			// 生成WRITE调用的四元式
			Quaternary q_read(Quaternary::WRITE,
				Quaternary::OPERAND_NIL, 0,
				exp_attribute.offset_operandtype_, exp_attribute.offset_,
				exp_attribute.operandtype_, exp_attribute.value_);
			quaternarytable_.push_back(q_read);
			// 回收临时变量
			if(Quaternary::TEMPORARY_OPERAND == exp_attribute.operandtype_
			|| Quaternary::TEMPORARY_OPERAND == exp_attribute.offset_operandtype_)	// 两者不可能同时成立，故写在一起
			{
				--tempvar_index_;
			}
		}
	}
	else
	{
		ExpressionAttribute exp_attribute = Expression(depth + 1);
		// 生成WRITE调用的四元式
		Quaternary q_read(Quaternary::WRITE,
			Quaternary::OPERAND_NIL, 0,
			exp_attribute.offset_operandtype_, exp_attribute.offset_,
			exp_attribute.operandtype_, exp_attribute.value_);
		quaternarytable_.push_back(q_read);
		// 回收临时变量
		if(Quaternary::TEMPORARY_OPERAND == exp_attribute.operandtype_
		|| Quaternary::TEMPORARY_OPERAND == exp_attribute.offset_operandtype_)	// 两者不可能同时成立，故写在一起
		{
			--tempvar_index_;
		}
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

// <for循环语句> ::= for <标识符>  := <表达式> （downto | to） <表达式> do <语句>
void SyntaxAnalyzer::ForLoopStatement(int depth) throw()			// for循环语句
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
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
	if(iter == tokentable_.end())
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
	if(iter->GetItemType() != TokenTableItem::VARIABLE
	&& iter->GetItemType() != TokenTableItem::PARAMETER)	// 检查是否为变量或参数或函数名
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in for loop\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
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
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
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

// <过程调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::ProcedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// 过程调用语句
{
	PrintFunctionFrame("ProcedureCallStatement()", depth);

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
	lexical_analyzer_.GetNextToken(token_);
	vector<ExpressionAttribute> attributes = ArgumentList(depth + 1);
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

	// 检查过程参数与过程声明是否匹配
	if(parameter_decorate_types.size() != attributes.size())	// 检查参数数量
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// 这里的数据类型只能为CHAR或INTEGER
		assert(attributes[i].decoratetype_ != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != attributes[i].decoratetype_)
		{
			// 要求参数为CHAR，实际参数为INTEGER，则无法转换
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && attributes[i].decoratetype_ == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				is_successful_ = false;
				// 这里仅仅是检查了参数类型不匹配，实际语法成分分析还可继续，
			}
		}
	}
}

// <函数调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::FunctionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// 函数调用语句
{
	PrintFunctionFrame("FunctionCallStatement()", depth);

	assert(Token::LEFT_PAREN == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	vector<ExpressionAttribute> attributes = ArgumentList(depth + 1);
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

	// 检查函数参数与函数声明是否匹配
	if(parameter_decorate_types.size() != attributes.size())	// 检查参数数量
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
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// 这里的数据类型只能为CHAR或INTEGER
		assert(attributes[i].decoratetype_ != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != attributes[i].decoratetype_)
		{
			// 要求参数为CHAR，实际参数为INTEGER，则无法转换
			if(TokenTableItem::CHAR == parameter_decorate_types[i] && TokenTableItem::INTEGER == attributes[i].decoratetype_)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				is_successful_ = false;
				// 这里仅仅是检查了参数类型不匹配，实际语法成分分析还可继续，故不返回
			}
		}
	}
	
}