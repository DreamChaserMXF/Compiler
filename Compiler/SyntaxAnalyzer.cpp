#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableItem.h"
#include "TokenTableException.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <algorithm>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer, 
								const vector<string> &stringtable, 
								TokenTable &tokentable, 
								vector<Quaternary> &quaternarytable) 
								throw()
	: lexical_analyzer_(lexical_analyzer), stringtable_(stringtable), tokentable_(tokentable), quaternarytable_(quaternarytable), 
	token_(), level_(1), tempvar_index_(0), label_index_(0), continue_label_(), break_label_(), 
	is_successful_(true), syntax_info_buffer_(), syntax_assist_buffer_(), tokenbuffer_()
{
	//// 插入主函数的BEGIN
	//Quaternary q_mainbegin(
	//	Quaternary::BEGIN,
	//	Quaternary::NIL_ADDRESSING, 0,
	//	Quaternary::IMMEDIATE_ADDRESSING, 0,
	//	Quaternary::IMMEDIATE_ADDRESSING, -1
	//	);
	//quaternarytable_.push_back(q_mainbegin);
}


bool SyntaxAnalyzer::Parse() throw()
{
	size_t depth = 0;
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
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
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
void SyntaxAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// 插入主函数的BEGIN
	Quaternary q_mainbegin(
		Quaternary::BEGIN,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, -1
	);
	quaternarytable_.push_back(q_mainbegin);
	// 解析分程序
	SubRoutine(depth + 1);
	// 判断结束符号
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
	// 插入主函数的END
	Quaternary q_mainend(
		Quaternary::END,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, -1
	);
	quaternarytable_.push_back(q_mainend);
	// 更新过程/函数的BEGIN语句中的临时变量个数
	Quaternary::UpdateTempVarSpace(quaternarytable_);
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
void SyntaxAnalyzer::TypeSpecification(size_t depth) throw()
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
void SyntaxAnalyzer::ProcedurePart(size_t depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		int proc_index = ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		// 生成过程的END四元式
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, proc_index));
		// TODO 修改上一个BEGIN四元式中的临时变量数量
//		SetTempVarCount(proc_index, max_local_temp_count_);
//		max_local_temp_count_ = 0;	// 初始化函数的临时变量最大数量
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
int SyntaxAnalyzer::ProcedureHead(size_t depth) throw()
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
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, 
		Quaternary::NIL_ADDRESSING, 0, 
		Quaternary::IMMEDIATE_ADDRESSING, 0,	// 这里的操作数应该是该过程要用到的临时变量的数量，等过程分析完后补齐
		Quaternary::IMMEDIATE_ADDRESSING, proc_index));
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
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return proc_index;
}

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void SyntaxAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		int func_index = FunctionHead(depth + 1);	// 进行函数头分析，并得到函数名在符号表中的位置
		SubRoutine(depth + 1);
		// 生成函数的END四元式
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, func_index));
		// TODO 修改上一个BEGIN四元式中的临时变量数量
//		SetTempVarCount(func_index, max_local_temp_count_);
//		max_local_temp_count_ = 0;	// 初始化函数的临时变量最大下标
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)	// 分程序结束后应读入分号
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// 读入结尾的分号
	}while(Token::FUNCTION == token_.type_);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
int SyntaxAnalyzer::FunctionHead(size_t depth) throw()
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
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, 
		Quaternary::NIL_ADDRESSING, 0, 
		Quaternary::IMMEDIATE_ADDRESSING, 0, // 这里的操作数应该是该函数要用到的临时变量的数量，等函数分析完后补齐
		Quaternary::IMMEDIATE_ADDRESSING, func_index));
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
int SyntaxAnalyzer::ParameterList(size_t depth) throw()		// 形参表
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
int SyntaxAnalyzer::ParameterTerm(size_t depth) throw()		
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
	TokenTableItem::DecorateType decoratetype = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
		{
			std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
			is_successful_ = false;
		}
		tokentable_.AddParameterItem(*iter, decoratetype, isref, level_);
	} // end 存符号表

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}

// <实在参数表> ::= <表达式>{,<表达式>}
vector<ExpressionAttribute> SyntaxAnalyzer::ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw()			// 实参表
{
	PrintFunctionFrame("ArgumentList()", depth);

	vector<ExpressionAttribute> attribute_buffer;
	ExpressionAttribute argument_attribute = Expression(depth + 1);
	attribute_buffer.push_back(argument_attribute);
	// 生成设置参数的四元式
	Quaternary q_addpara(Quaternary::SETP,
		Quaternary::NIL_ADDRESSING, 0, 
		argument_attribute.offset_addressingmethod_, argument_attribute.offset_, 
		argument_attribute.addressingmethod_, argument_attribute.value_);
	// 参数的传递方式
	if(Quaternary::REFERENCE_ADDRESSING == parameter_attributes[0].addressingmethod_
		&& Quaternary::REFERENCE_ADDRESSING != argument_attribute.addressingmethod_)
	{
		q_addpara.op_ = Quaternary::SETREFP;
		// 引用传参要求参数是左值
		if(Quaternary::VARIABLE_ADDRESSING != argument_attribute.addressingmethod_
			&& Quaternary::ARRAY_ADDRESSING != argument_attribute.addressingmethod_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be left value to fit the reference parameter\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::RIGHT_PAREN && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return attribute_buffer;
		}
	}
	quaternarytable_.push_back(q_addpara);
	// 回收临时变量
	if(Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == argument_attribute.offset_addressingmethod_)	// 两者不可能同时成立，故写在一起
	{
		--tempvar_index_;
	}
	size_t para_index = 0;
	while(Token::COMMA == token_.type_)
	{
		++para_index;
		lexical_analyzer_.GetNextToken(token_);
		argument_attribute = Expression(depth + 1);
		attribute_buffer.push_back(argument_attribute);
		// 越界检查
		if(parameter_attributes.size() <= para_index)
		{
			// 实际参数过多
			continue;
		}
		// 生成设置参数的四元式
		// 参数要求是引用，且表达式为非引用，才用SETREFP
		// 重点在于，若表达式也为引用，则只用SETP，即可传入地址
		if(Quaternary::REFERENCE_ADDRESSING == parameter_attributes[para_index].addressingmethod_
		&& Quaternary::REFERENCE_ADDRESSING != argument_attribute.addressingmethod_)
		{
			q_addpara.op_ = Quaternary::SETREFP;
		}
		else
		{
			q_addpara.op_ = Quaternary::SETP;
		}
		q_addpara.method2_ = argument_attribute.offset_addressingmethod_;
		q_addpara.offset2_ = argument_attribute.offset_;
		q_addpara.method3_ = argument_attribute.addressingmethod_;
		q_addpara.dst_ = argument_attribute.value_;
		quaternarytable_.push_back(q_addpara);
		// 回收临时变量
		if(Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == argument_attribute.offset_addressingmethod_)	// 两者不可能同时成立，故写在一起
		{
			--tempvar_index_;
		}
	}
	return attribute_buffer;
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
	TokenTable::iterator iter;
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
		if(Token::LEFT_PAREN == token_.type_)	// 过程或函数调用
		{
			if(iter->itemtype_ != TokenTableItem::PROCEDURE
				&& iter->itemtype_ != TokenTableItem::FUNCTION)	// 检查其属性是否为过程或函数
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a procedure\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
			vector<ExpressionAttribute> proc_func_attributes = tokentable_.GetProcFuncParameterAttributes(iter);
			
			// 在ProcedureCallStatement中设置参数
			//ProcedureCallStatement(idToken, decorate_types, depth + 1);
			// 在ProcFuncCallStatement中设置参数
			ProcFuncCallStatement(idToken, proc_func_attributes, depth + 1);
			// 生成调用四元式
			Quaternary q_procedurecall((iter->itemtype_ == TokenTableItem::PROCEDURE) ? Quaternary::PROC_CALL : Quaternary::FUNC_CALL, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::IMMEDIATE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_procedurecall);
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
	//default:
	//	std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error at the beginning of Statement\n";
	//	is_successful_ = false;
	//	while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
	//	{ 
	//		lexical_analyzer_.GetNextToken(token_);
	//	}
	//	break;
	}
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
// idToken是赋值语句之前标识符的token，iter是其在符号表中的迭代器
void SyntaxAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth)			// 赋值语句
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// 为四元式生成而定义的变量
	bool assign2array= false;	// 是否为对数组的赋值操作
	
	ExpressionAttribute offset_attribute;	// 当对数组元素赋值时，存储偏移量（数组下标）的属性

	if(Token::LEFT_BRACKET == token_.type_)	// 对数组元素赋值
	{
		assign2array = true;
		// 语义检查
		if(iter->itemtype_ != TokenTableItem::ARRAY)	// 检查是否为数组名
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		// 读入表示下标的表达式
		lexical_analyzer_.GetNextToken(token_);
		offset_attribute = Expression(depth + 1);
		// 若数组下标仍是数组元素
		// 则插入四元式，将数组下标值赋给一个临时变量
		SimplifyArrayOperand(offset_attribute);
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
	// 语义检查：剩下只有三种情况：变量、参数或是函数返回值
	else if(iter->itemtype_ != TokenTableItem::VARIABLE
		&& iter->itemtype_ != TokenTableItem::PARAMETER
		&& iter->itemtype_ != TokenTableItem::FUNCTION)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
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
	// 以下三行为抛出异常做准备，放在GetNextToken前是为了准确记录赋值号的行号
	static Token errToken;
	errToken.type_ = Token::ASSIGN;
	errToken.lineNumber_ = token_.lineNumber_;

	// 读入赋值号右边的表达式
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);

	// 语义检查：类型转换
	if(iter->decoratetype_ < right_attribute.decoratetype_)	// 小于表示不能从右至左的转换
	{
		std::cout << "warning: line " << idToken.lineNumber_ << ":  " << idToken.toString() 
			<< " convert from " << TokenTableItem::DecorateTypeString[right_attribute.decoratetype_] 
			<< " to " << TokenTableItem::DecorateTypeString[iter->decoratetype_] << "\n";
	}

	// 中间代码生成
	if(assign2array)	// 对数组元素赋值
	{
		// 如果right_attribute是数组元素的话，要将其赋值给临时变量
		SimplifyArrayOperand(right_attribute);
		// 进行数组赋值
		Quaternary q_asg;
		q_asg.op_ = Quaternary::AASG;
		q_asg.method1_ = right_attribute.addressingmethod_;
		q_asg.src1_ = right_attribute.value_;
		q_asg.method2_ = offset_attribute.addressingmethod_;
		q_asg.offset2_ = offset_attribute.value_;
		q_asg.method3_ = Quaternary::ARRAY_ADDRESSING;
		q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
		quaternarytable_.push_back(q_asg);

		// 如果右操作数是临时变量，可回收
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
		// 如果被赋值的数组下标也是临时变量，可回收
		if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
	}
	else if(TokenTableItem::PARAMETER == iter->itemtype_
			|| TokenTableItem::VARIABLE == iter->itemtype_)	// 普通变量/参数的赋值
	{
		// 如果赋值号右边的表达式是临时变量，则可优化掉当前的赋值语句。
		// 详见《Appendix1 设计备注》 - chapter 4
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			quaternarytable_.back().method3_ = iter->isref_ ? Quaternary::REFERENCE_ADDRESSING : Quaternary::VARIABLE_ADDRESSING;
			quaternarytable_.back().dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			// 回收右操作数的临时变量
			--tempvar_index_;
		}
		else
		{
			Quaternary q_asg;
			q_asg.op_ = Quaternary::ASG;
			q_asg.method1_ = right_attribute.addressingmethod_;
			q_asg.src1_ = right_attribute.value_;
			q_asg.method2_ = right_attribute.offset_addressingmethod_;
			q_asg.offset2_ = right_attribute.offset_;
			q_asg.method3_ = iter->isref_ ? Quaternary::REFERENCE_ADDRESSING : Quaternary::VARIABLE_ADDRESSING;
			q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			quaternarytable_.push_back(q_asg);
			// 如果赋值号右边的表达式是数组，且其数组下标是临时变量，可回收
			if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_)
			{
				// 这里有一个程序鲁棒性的假定，即在这个if语句块内，如果addressingmethod_不是ARRAY的话，那么addressingmethod_一定是NIL_ADDRESSING
				// 所以用下面的assert检测一下程序逻辑有无问题
				assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
				--tempvar_index_;
			}
		}
	}
	else	// 函数返回值
	{
		Quaternary q_ret;
		q_ret.op_ = Quaternary::RET;
		q_ret.method2_ = right_attribute.offset_addressingmethod_;
		q_ret.offset2_ = right_attribute.offset_;
		q_ret.method3_ = right_attribute.addressingmethod_;
		q_ret.dst_ = right_attribute.value_;
		quaternarytable_.push_back(q_ret);
		// 如果赋值号右边的表达式是临时变量，可回收
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
		// 如果赋值号右边的表达式是数组，且其数组下标是临时变量，可回收
		else if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_)
		{
			// 这里有一个程序鲁棒性的假定，即如果addressingmethod_不是ARRAY的话，那么addressingmethod_一定是NIL_ADDRESSING
			// 所以用下面的assert检测一下程序逻辑有无问题
			assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
			--tempvar_index_;
		}
	}
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
ExpressionAttribute SyntaxAnalyzer::Expression(size_t depth) throw()				// 表达式
{
	PrintFunctionFrame("Expression()", depth);
	
	Quaternary q_neg;	// 可能生成的NEG四元式
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		if(Token::MINUS == token_.type_)	// 如果是减号，就可能会生成一项四元式
		{
			q_neg.op_ = Quaternary::NEG;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	ExpressionAttribute first_term = Term(depth + 1);
	//bool isref = first_term.isref_;
	if(Quaternary::NEG == q_neg.op_)	// 如果之前读到了一个减号
	{
		//// 只要有过操作，即变为非引用
		//isref = false;
		// 常数取反的优化
		if(Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_)
		{
			first_term.value_ = -first_term.value_;
		}
		else	// 生成NEG的四元式
		{
			q_neg.method1_ = first_term.addressingmethod_;
			q_neg.src1_ = first_term.value_;
			q_neg.method2_ = first_term.offset_addressingmethod_;
			q_neg.offset2_ = first_term.offset_;
			q_neg.method3_ = Quaternary::TEMPORARY_ADDRESSING;
			if(Quaternary::TEMPORARY_ADDRESSING == q_neg.method1_)
			{
				q_neg.dst_ = q_neg.src1_;
			}
			else
			{
				q_neg.dst_ = tempvar_index_++;
			}
			quaternarytable_.push_back(q_neg);
			// 修改first_term的属性(NEG操作不影响decoratetype)
			first_term.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
			first_term.value_ = q_neg.dst_;
			first_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
			first_term.offset_ = 0;
		}
	}

	Quaternary q_term;
	ExpressionAttribute new_term;
	bool is_first_operator = true;
	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		//// 只要有过操作，即变为非引用
		//isref = false;

		// 第一次时，如果new_term是数组元素，则要将其赋值给临时变量
		if(is_first_operator)
		{
			SimplifyArrayOperand(first_term);
		}

		// 确定四元式的操作符
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term_decoratetype = is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
		new_term = Term(depth + 1);

		// 语义分析：执行类型转换
		new_term.decoratetype_ = TokenTableItem::TypeConversionMatrix[last_term_decoratetype][new_term.decoratetype_];

		// 如果读到的new_term还是数组元素，那么仍然需要一次转换
		// 将数组元素的值赋给临时变量
		SimplifyArrayOperand(new_term);

		// 确定四元式的操作数
		// src1的确定：
		// 第一次加/减时，src1就是while之前读入的那个term
		// 之后加/减时，src1就是上一个四元式的结果
		if(is_first_operator)
		{
			// 两个常数相加/减的优化：直接在编译时计算结果
			if(Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_
				&& Quaternary::IMMEDIATE_ADDRESSING == new_term.addressingmethod_)
			{
				first_term.decoratetype_ = new_term.decoratetype_;
				first_term.value_ = (Quaternary::ADD == q_term.op_) ? 
						(first_term.value_ + new_term.value_) : (first_term.value_ - new_term.value_);
				continue;
			}
			// 正常流程
			q_term.method1_ = first_term.addressingmethod_;
			q_term.src1_ = first_term.value_;
			is_first_operator = false;
		}
		else
		{
			q_term.method1_ = q_term.method3_;
			q_term.src1_ = q_term.dst_;
		}
		// src2的确定：
		// src2就是读到的新的term
		q_term.method2_ = new_term.addressingmethod_;
		q_term.src2_ = new_term.value_;
		// dst的确定：
		// 如果src1是临时变量，就令dst为src1
		// 否则，如果src2是临时变量，就令dst为src2
		// 否则，令dst为新的临时变量
		if(Quaternary::TEMPORARY_ADDRESSING == q_term.method1_)
		{
			q_term.method3_ = q_term.method1_;
			q_term.dst_ = q_term.src1_;
			// 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
			if(Quaternary::TEMPORARY_ADDRESSING == q_term.method2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_ADDRESSING == q_term.method2_)
		{
			q_term.method3_ = q_term.method2_;
			q_term.dst_ = q_term.src2_;
		}
		else
		{
			q_term.method3_ = Quaternary::TEMPORARY_ADDRESSING;
			q_term.dst_ = tempvar_index_++;
		}
		// 保存四元式
		quaternarytable_.push_back(q_term);
	}

	

	// 返回最终的表达式属性
	if(is_first_operator)	// 只有一项的情况
	{
		new_term = first_term;
	}
	else	// 有多项的情况，要更新new_term的属性。否则new_term除了decoratetype_外，其余都是最后一项的属性
	{
		new_term.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		new_term.value_ = q_term.dst_;
		new_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		new_term.offset_ = 0;
	}
	//// 引用的属性
	//new_term.isref_ = isref;
	return new_term;
}

// <项> ::= <因子>{<乘法运算符><因子>}
ExpressionAttribute SyntaxAnalyzer::Term(size_t depth) throw()						// 项
{
	PrintFunctionFrame("Term()", depth);

	ExpressionAttribute first_factor = Factor(depth + 1);
	//bool isref = first_factor.isref_;

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		//// 只要有过操作，即变为非引用
		//isref = false;

		// 第一次时，如果first_factor是数组元素，则要将其赋值给临时变量
		if(is_first_operator)
		{
			SimplifyArrayOperand(first_factor);
		}

		// 确定四元式的操作符
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// 语法分析：读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor_decoratetype = is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
		new_factor = Factor(depth + 1);
		// 语义分析：执行类型转换
		new_factor.decoratetype_ = TokenTableItem::TypeConversionMatrix[last_factor_decoratetype][new_factor.decoratetype_];

		// 如果读到的项还是数组元素，那么仍然需要一次转换
		// 将数组元素的值赋给临时变量
		SimplifyArrayOperand(new_factor);

		// 确定四元式的操作数
		// src1的确定：
		// 第一次乘/除，src1就是while之前读入的那个factor
		// 之后加/减时，src1就是上一个四元式的结果
		if(is_first_operator)
		{
			// 两个常数相乘/除的优化：直接在编译时计算结果
			if(Quaternary::IMMEDIATE_ADDRESSING == first_factor.addressingmethod_
				&& Quaternary::IMMEDIATE_ADDRESSING == new_factor.addressingmethod_)
			{
				first_factor.decoratetype_ = new_factor.decoratetype_;
				first_factor.value_ = (Quaternary::MUL == q_factor.op_) ? 
						(first_factor.value_ * new_factor.value_) : (first_factor.value_ / new_factor.value_);
				continue;
			}
			// 正常流程
			q_factor.method1_ = first_factor.addressingmethod_;
			q_factor.src1_ = first_factor.value_;
			is_first_operator = false;
		}
		else
		{
			q_factor.method1_ = q_factor.method3_;
			q_factor.src1_ = q_factor.dst_;
		}
		// src2的确定：
		// src2是读到的新的factor
		q_factor.method2_ = new_factor.addressingmethod_;
		q_factor.src2_ = new_factor.value_;
		// dst的确定：
		// 如果src1是临时变量，就令dst为src1
		// 否则，如果src2是临时变量，就令dst为src2
		// 否则，令dst为新的临时变量
		if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method1_)
		{
			q_factor.method3_ = q_factor.method1_;
			q_factor.dst_ = q_factor.src1_;
			// 此时，如果src2也是临时变量，那么就可以在执行完这个四元式后，把这个临时变量的标号回收
			if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_)
		{
			q_factor.method3_ = q_factor.method2_;
			q_factor.dst_ = q_factor.src2_;
		}
		else
		{
			q_factor.method3_ = Quaternary::TEMPORARY_ADDRESSING;
			q_factor.dst_ = tempvar_index_++;
		}
		// 保存四元式
		quaternarytable_.push_back(q_factor);
	}

	// 返回项的属性
	if(is_first_operator)	// 只有一个因子
	{
		new_factor = first_factor;
	}
	else
	{
		// 更新new_factor的属性
		// 否则new_factor除decoratetype外，其余属性均保留了最后一个因子的属性
		new_factor.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		new_factor.value_ = q_factor.dst_;
		new_factor.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		new_factor.offset_ = 0;
	}
	// 引用的属性
	//new_factor.isref_ = isref;
	return new_factor;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')' 
//          | <无符号整数> 
//          | <字符>
ExpressionAttribute SyntaxAnalyzer::Factor(size_t depth) throw()					// 因子
{
	PrintFunctionFrame("Factor()", depth);
	ExpressionAttribute factor_attribute;	// 记录该factor因子的信息

	// 语法检查：标识符的情况【变量、常变量、数组、函数调用】
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		// 语义检查
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
		// 语义：记录修饰类型与引用类型，并更新符号表
		factor_attribute.decoratetype_ = iter->decoratetype_;		
		iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
		Token idToken = token_;	// 记下，待用
		lexical_analyzer_.GetNextToken(token_);
		// 语法检查
		if(Token::LEFT_BRACKET == token_.type_)	// 左中括号，数组元素
		{
			// factor_attribute自己的类型与值
			factor_attribute.addressingmethod_ = Quaternary::ARRAY_ADDRESSING;
			factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			// 语义检查：是否为数组名
			if(iter->itemtype_ != TokenTableItem::ARRAY)	
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// 语法：读入作为下标的表达式
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute offset_attribute = Expression(depth + 1);
			// 中间代码生成
			// 确定factor_attribute的下标
			// 这里的offset_attribute不能是数组元素，否则会构成嵌套的数组下标（即数组下标又是一个数组元素），无法翻译成四元式
			// 如果是数组，就要把offset_attribute存到临时变量中，作为当前factor的下标
			SimplifyArrayOperand(offset_attribute);
			factor_attribute.offset_addressingmethod_ = offset_attribute.addressingmethod_;
			factor_attribute.offset_ = offset_attribute.value_;
			// 语法检查
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
		else if(Token::LEFT_PAREN == token_.type_)	// 左括号，函数调用
		{
			// 语义检查：是否为函数
			if(iter->itemtype_ != TokenTableItem::FUNCTION)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a function\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// 语义：类型匹配
			// 从符号表中取出函数的参数类型，待FunctionCallStatement去匹配参数
			vector<ExpressionAttribute> parameter_attributes = tokentable_.GetProcFuncParameterAttributes(iter);
			// 四元式：先进入FunctionCallStatement，设置好参数，然后再生成函数调用语句
			//FunctionCallStatement(idToken, decorate_types, depth + 1);
			// 四元式：先进入ProcFuncCallStatement，设置好参数，然后再生成函数调用语句
			ProcFuncCallStatement(idToken, parameter_attributes, depth + 1);			
			// 生成函数调用的四元式
			Quaternary q_functioncall(Quaternary::FUNC_CALL, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_functioncall);
			// 在正常处理流程中，希望将函数的返回值放置在temp#tempvar_index的位置
			// 但在子函数中，无法判定temp#tempvar_index的位置
			// 所以子函数将返回值存储在EAX中，再加一条指令，将EAX的值存进临时变量中
			Quaternary q_store(Quaternary::STORE, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::TEMPORARY_ADDRESSING, tempvar_index_++);
			quaternarytable_.push_back(q_store);
			// 所以factor_attribute的属性是临时变量
			factor_attribute.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
			factor_attribute.value_ = q_store.dst_;
			factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
			factor_attribute.offset_ = 0;
		}
		else	// 单独一个标识符
		{
			// 语义检查：是否为变量、常量或过程/函数的参数
			if(iter->itemtype_ != TokenTableItem::VARIABLE && iter->itemtype_ != TokenTableItem::PARAMETER && iter->itemtype_ != TokenTableItem::CONST)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  single token Factor should be varaible or constant\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// factor_attribute的属性
			if(TokenTableItem::CONST == iter->itemtype_)	// 常变量
			{
				// 这里直接将常变量转换成立即数类型，详见《Appendix1 设计备注》
				factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
				factor_attribute.value_ = iter->value_;
				factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
				factor_attribute.offset_ = 0;
			}
			else	// 一般变量
			{
				factor_attribute.addressingmethod_ = Quaternary::VARIABLE_ADDRESSING;
				factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
				factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
				factor_attribute.offset_ = 0;
			}
		}
		if(iter->isref_)
		{
			factor_attribute.addressingmethod_ = Quaternary::REFERENCE_ADDRESSING;
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// 括号括起来的表达式
	{
		// bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
		// 读表达式的第一个单词
		lexical_analyzer_.GetNextToken(token_);
		// 再读取表达式
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
	else if(Token::CONST_INTEGER == token_.type_)	// 整型字面常量
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute.decoratetype_ = TokenTableItem::INTEGER;
		factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		factor_attribute.value_ = token_.value_.integer;
		factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// 字符型字面常量
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute.decoratetype_ = TokenTableItem::CHAR;
		factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		factor_attribute.value_ = token_.value_.character;
		factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		factor_attribute.offset_ = 0;
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
	return factor_attribute;
}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
void SyntaxAnalyzer::IfStatement(size_t depth) throw()				// 条件语句
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
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, label2);
		quaternarytable_.push_back(q_jmp);
		// 设置第一个label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, label1);
		quaternarytable_.push_back(q_label1);
		// 读取else中的语句
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
		// 设置第二个label
		Quaternary q_label2(Quaternary::LABEL, 
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, label2);
		quaternarytable_.push_back(q_label2);
	}
	else	// 如果没有else语句，就在if语句块结束的时候设置第一个label
	{
		// 设置第一个label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, label1);
		quaternarytable_.push_back(q_label1);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>
// 由if或for语句中传递下来label参数，标识if语句块或for循环体的结束
// 用于在处理condition时设置跳转语句
void SyntaxAnalyzer::Condition(int endlabel, size_t depth) throw()				// 条件
{
	PrintFunctionFrame("Condition()", depth);

	ExpressionAttribute left_attribute = Expression(depth + 1);
	// 化简数组元素为临时变量
	SimplifyArrayOperand(left_attribute);
		
	bool isonlyexpression = false;
	// 生成有条件的跳转语句
	// 不符合条件时才会跳转，所以这里的操作符与读到的token要反一下
	Quaternary q_jmp_condition;
	switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是THEN（当条件中仅有一个表达式时）
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
	case Token::THEN:
		q_jmp_condition.op_ = Quaternary::JE;
		isonlyexpression = true;
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
	ExpressionAttribute right_attribute;
	if(!isonlyexpression)	// 如果还有下一个表达式，再继续读取
	{
		lexical_analyzer_.GetNextToken(token_);
		right_attribute = Expression(depth + 1);
		// 化简数组为临时变量
		SimplifyArrayOperand(right_attribute);
	}
	else
	{
		right_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		right_attribute.value_ = 0;
		right_attribute.offset_addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		right_attribute.offset_ = 0;
		right_attribute.decoratetype_ = TokenTableItem::VOID;
	}
	// 操作数
	q_jmp_condition.method1_ = left_attribute.addressingmethod_;
	q_jmp_condition.src1_ = left_attribute.value_;
	q_jmp_condition.method2_ = right_attribute.addressingmethod_;
	q_jmp_condition.src2_ = right_attribute.value_;
	q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
	q_jmp_condition.dst_ = endlabel;
	// 保存四元式
	quaternarytable_.push_back(q_jmp_condition);
	// 回收临时变量
	if(Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}
}

// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void SyntaxAnalyzer::CaseStatement(size_t depth) throw()			// 情况语句
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute exp_attribute = Expression(depth + 1);
	// 将表达式化为非数组元素
	SimplifyArrayOperand(exp_attribute);

	if(Token::OF != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" to specify the certain case\n";
		is_successful_ = false;
		// 这里假设是忘记写of，故不返回
	}
	// 为END处申请一个label
	int endlabel = label_index_++;
	// case表达式之后的跳转语句的插入位置
	int jmp_insertion_location = quaternarytable_.size();
	// 若干个JE的跳转语句
	Quaternary q_jmp;
	q_jmp.op_ = Quaternary::JE;
	q_jmp.method1_ = exp_attribute.addressingmethod_;
	q_jmp.src1_ = exp_attribute.value_;
	q_jmp.method2_ = Quaternary::IMMEDIATE_ADDRESSING;
	q_jmp.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		// 为当前情况表元素申请一个label
		int caselabel = label_index_++;
		vector<int> constant_list = CaseElement(caselabel, endlabel, depth + 1);
		// 插入JE跳转语句
		q_jmp.dst_ = caselabel;
		for(vector<int>::const_iterator c_iter = constant_list.begin();
			c_iter != constant_list.end(); ++c_iter)
		{
			q_jmp.src2_ = *c_iter;
			quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location++, q_jmp);
		}
	}while(Token::SEMICOLON == token_.type_);

	// 执行完所有JE后，跳转到END处
	Quaternary q_endjmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location, q_endjmp);
	
	// 插入结束的label
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);

	// 回收case后Expression的临时变量
	if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}

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
vector<int> SyntaxAnalyzer::CaseElement(int caselabel, int endlabel, size_t depth) throw()					// 情况表元素
{
	PrintFunctionFrame("CaseElement()", depth);

	vector<int> constant_list;

	TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_
		&& tokentable_.end() == iter)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character or constant variable after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return constant_list;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

	// 记录该常量
	if(Token::CONST_INTEGER == token_.type_)
	{
		constant_list.push_back(token_.value_.integer);
	}
	else if(Token::CONST_CHAR == token_.type_)
	{	
		constant_list.push_back(token_.value_.character);
	}
	else	// 常变量
	{
		constant_list.push_back(iter->value_);		
	}

	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		iter = tokentable_.SearchDefinition(token_);
		if(Token::CONST_INTEGER != token_.type_
			&& Token::CONST_CHAR != token_.type_
			&& tokentable_.end() == iter)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after \"case\"\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return constant_list;
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		// 记录该常量
		if(Token::CONST_INTEGER == token_.type_)
		{
			constant_list.push_back(token_.value_.integer);
		}
		else if(Token::CONST_CHAR == token_.type_)
		{	
			constant_list.push_back(token_.value_.character);
		}
		else	// 常变量
		{
			constant_list.push_back(iter->value_);		
		}
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
		return constant_list;
	}
	// 读入情况表元素的语句之前，打下一个label
	Quaternary q_caselabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, caselabel);
	quaternarytable_.push_back(q_caselabel);

	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// 情况表元素的语句执行完后，跳转到END处
	Quaternary q_endjmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endjmp);
	// 返回记录的情况表元素的常量数组
	return constant_list;
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
// TODO 扩展出对数组的支持
void SyntaxAnalyzer::ReadStatement(size_t depth) throw()			// 读语句
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

		// 读取下一个单词，判断是否为数组元素
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET != token_.type_)	// 不是数组元素
		{
			if(iter->itemtype_ != TokenTableItem::VARIABLE
			&& iter->itemtype_ != TokenTableItem::PARAMETER)	// 检查是否为变量或参数或函数名
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
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_read);
		}
		else	// 数组元素
		{
			// 类型检查
			if(iter->itemtype_ != TokenTableItem::ARRAY)	// 检查是否为数组变量
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << iter->name_ << "  is not an array\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
			// 读入数组下标的第一个单词
			lexical_analyzer_.GetNextToken(token_);
			// 读到整个下标的表达式
			ExpressionAttribute exp_attribute = Expression(depth + 1);
			// 非数组化
			SimplifyArrayOperand(exp_attribute);
			// 生成READ调用的四元式
			Quaternary q_read(Quaternary::READ,
				Quaternary::NIL_ADDRESSING, 0,
				exp_attribute.addressingmethod_, exp_attribute.value_,
				Quaternary::ARRAY_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_read);
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
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::STRING_ADDRESSING, distance(static_cast<vector<string>::const_iterator>(stringtable_.begin()), iter));
		quaternarytable_.push_back(q_read);

		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute exp_attribute = Expression(depth + 1);	// 读取第二个参数（表达式）
			// 生成WRITE调用的四元式
			Quaternary q_write(Quaternary::WRITE,
				Quaternary::NIL_ADDRESSING, 0,
				exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
				exp_attribute.addressingmethod_, exp_attribute.value_);
			//// Imp！如果整个表达式就只是一个立即数，那么在最后给它存储一个decoratetype的属性，用作输出时的类型推导
			/*if(Quaternary::IMMEDIATE_ADDRESSING == exp_attribute.addressingmethod_)
			{
				q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
			}*/
			// 更正：在write的四元式末尾一定要有decoratetype的属性，用作输出时的类型推导
			q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
			quaternarytable_.push_back(q_write);
			// 回收临时变量
			if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_
			|| Quaternary::TEMPORARY_ADDRESSING == exp_attribute.offset_addressingmethod_)	// 两者不可能同时成立，故写在一起
			{
				--tempvar_index_;
			}
		}
	}
	else
	{
		ExpressionAttribute exp_attribute = Expression(depth + 1);
		// 生成WRITE调用的四元式
		Quaternary q_write(Quaternary::WRITE,
			Quaternary::NIL_ADDRESSING, 0,
			exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
			exp_attribute.addressingmethod_, exp_attribute.value_);
		//// Imp！如果整个表达式就只是一个立即数，那么在最后给它存储一个decoratetype的属性，用作输出时的类型推导
		/*if(Quaternary::IMMEDIATE_ADDRESSING == exp_attribute.addressingmethod_)
		{
			q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
		}*/
		// 更正：在write的四元式末尾一定要有decoratetype的属性，用作输出时的类型推导
		q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
		quaternarytable_.push_back(q_write);
		// 回收临时变量
		if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == exp_attribute.offset_addressingmethod_)	// 两者不可能同时成立，故写在一起
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

// <while循环语句> ::= while <条件> do <语句>
// <while循环语句> ::= while @Label<check> <条件> @JZLabel<end> do <语句> @JMPLabel<check> @Label<end>
void SyntaxAnalyzer::WhileLoopStatement(size_t depth) throw()			// while循环语句
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// 申请条件语句前面的label<check>和结束时的label<end>
	int checklabel = label_index_++;
	int endlabel = label_index_++;
	// 更新continue_label_栈与break_label_栈
	continue_label_.push(checklabel);
	break_label_.push(endlabel);
	// 放下label<check>
	Quaternary q_checklabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_checklabel);
	
	// 读取下一个单词，并进入条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(endlabel, depth + 1);	// 条件语句中会执行动作@JZLabel<end>
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
	// 更新continue_label_栈与break_label_栈
	continue_label_.pop();
	break_label_.pop();
	// 循环体结束后的跳转
	Quaternary q_jmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_jmp);
	// 放下结束的label
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);
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
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTable::iterator loopvar_iter = tokentable_.SearchDefinition(token_);
	if(loopvar_iter == tokentable_.end())
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	loopvar_iter->AddUsedLine(token_.lineNumber_);		// 在符号表中插入引用行记录
	if(loopvar_iter->itemtype_ != TokenTableItem::VARIABLE
	&& loopvar_iter->itemtype_ != TokenTableItem::PARAMETER)	// 检查是否为变量或参数
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
	ExpressionAttribute init_attribute = Expression(depth + 1);

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
	// 申请循环变量的递增/减的label<vary>, 边界检查的label<check>，以及末尾的label<end>
	int varylabel = label_index_++;
	int checklabel = label_index_++;
	int endlabel = label_index_++;
	// 更新continue_label_栈与break_label_栈
	continue_label_.push(varylabel);
	break_label_.push(endlabel);
	// 生成for的循环变量的初始化（赋值）四元式
	Quaternary q_init(Quaternary::ASG,
		init_attribute.addressingmethod_, init_attribute.value_,
		init_attribute.offset_addressingmethod_, init_attribute.offset_,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)));
	quaternarytable_.push_back(q_init);
	// 生成跳转到边界检查的JMP四元式
	Quaternary q_jmpcheck(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_jmpcheck);
	// 生成循环变量递增/减的label<vary>
	Quaternary q_varylabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, varylabel);
	quaternarytable_.push_back(q_varylabel);
	// 生成循环变量递增/减的四元式
	Quaternary q_vary(Token::TO == vary_token.type_ ? Quaternary::ADD : Quaternary::SUB,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)),
		Quaternary::IMMEDIATE_ADDRESSING, 1,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)));
	quaternarytable_.push_back(q_vary);
	// 生成循环变量边界检查的label<check>
	Quaternary q_checklabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_checklabel);

	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute bound_attribute = Expression(depth + 1);

	// 化简至非数组元素表达式
	SimplifyArrayOperand(bound_attribute);
	// 生成边界检查的JMP四元式
	Quaternary q_check(Token::TO == vary_token.type_ ? Quaternary::JG : Quaternary::JL,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)),
		bound_attribute.addressingmethod_, bound_attribute.value_,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_check);

	// 回收两个Expression的临时变量
	if(	Quaternary::TEMPORARY_ADDRESSING == init_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == init_attribute.offset_addressingmethod_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_ADDRESSING == bound_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}

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
	// 更新continue_label_栈与break_label_栈
	continue_label_.pop();
	break_label_.pop();
	// 跳转回循环变量递增/减的label<vary>
	Quaternary q_jmpvary(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, varylabel);
	quaternarytable_.push_back(q_jmpvary);
	// for循环结束的label<end>
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);
}

void SyntaxAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	if(continue_label_.size() != 0)
	{
		Quaternary q_continue(Quaternary::JMP,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, continue_label_.top());
		quaternarytable_.push_back(q_continue);
	}
	else
	{
		// 报错
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  no loop body found around \"continue\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}
void SyntaxAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	if(break_label_.size() != 0)
	{
		Quaternary q_break(Quaternary::JMP,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, break_label_.top());
		quaternarytable_.push_back(q_break);
	}
	else
	{
		// 报错
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  no loop body found around \"break\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}


//// <过程调用语句> ::= '('[<实在参数表>]')'
//void SyntaxAnalyzer::ProcedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, size_t depth)	// 过程调用语句
//{
//	PrintFunctionFrame("ProcedureCallStatement()", depth);
//	// 语法检查
//	if(Token::LEFT_PAREN != token_.type_)
//	{
//		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the argument\n";
//		is_successful_ = false;
//		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//		{ 
//			lexical_analyzer_.GetNextToken(token_);
//		}
//		return;
//	}
//	// 语法：读入右括号或参数表的第一个单词
//	lexical_analyzer_.GetNextToken(token_);
//	if(parameter_decorate_types.size() == 0)	// 如果函数本身就没有参数的话，就不用读参数了
//	{
//		// 语法检查
//		if(Token::RIGHT_PAREN != token_.type_)
//		{
//			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
//			is_successful_ = false;
//			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//			{ 
//				lexical_analyzer_.GetNextToken(token_);
//			}
//			return;
//		}
//		lexical_analyzer_.GetNextToken(token_);	// 读函数调用完毕后的下一个单词
//		return;
//	}
//	// 语法：读参数，并生成设置参数的四元式
//	vector<TokenTableItem::DecorateType> arg_decoratetypes = ArgumentList(depth + 1);
//	// 语法检查
//	if(Token::RIGHT_PAREN != token_.type_)
//	{
//		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
//		is_successful_ = false;
//		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//		{ 
//			lexical_analyzer_.GetNextToken(token_);
//		}
//		return;
//	}
//	lexical_analyzer_.GetNextToken(token_);
//
//	// 语义检查：过程参数与过程声明是否匹配
//	if(parameter_decorate_types.size() != arg_decoratetypes.size())	// 检查参数数量
//	{
//		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  procedure does not take " << arg_decoratetypes.size() << " argument";
//		if(arg_decoratetypes.size() > 1)
//		{
//			std::cout << "s\n";
//		}
//		else
//		{
//			std::cout << "\n";
//		}
//		is_successful_ = false;
//		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//		{ 
//			lexical_analyzer_.GetNextToken(token_);
//		}
//		return;
//	}
//	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
//	{
//		// 左小于右小于表示不能从右操作数类型转到左操作数类型
//		if(parameter_decorate_types[i] < arg_decoratetypes[i])
//		{
//			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
//				<< "  cannot convert parameter " << i + 1 << " from " << TokenTableItem::DecorateTypeString[arg_decoratetypes[i]]
//				<< " to " <<  TokenTableItem::DecorateTypeString[parameter_decorate_types[i]] <<"\n";
//			// 这里仅仅是检查了参数类型不匹配，语法分析还可继续，故不返回
//		}
//		//if(parameter_decorate_types[i] == TokenTableItem::CHAR && arg_decoratetypes[i] == TokenTableItem::INTEGER)
//		//{
//		//	std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
//		//	is_successful_ = false;
//		//	// 这里仅仅是检查了参数类型不匹配，实际语法成分分析还可继续，
//		//}
//	}
//}
//
//// <函数调用语句> ::= '('[<实在参数表>]')'
//void SyntaxAnalyzer::FunctionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, size_t depth)	// 函数调用语句
//{
//	PrintFunctionFrame("FunctionCallStatement()", depth);
//
//	assert(Token::LEFT_PAREN == token_.type_);
//
//	// 语法：读入右括号或参数表的第一个单词
//	lexical_analyzer_.GetNextToken(token_);
//	if(parameter_decorate_types.size() == 0)	// 如果函数本身就没有参数的话，就不用读参数了
//	{
//		// 语法检查
//		if(Token::RIGHT_PAREN != token_.type_)
//		{
//			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
//			is_successful_ = false;
//			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//			{ 
//				lexical_analyzer_.GetNextToken(token_);
//			}
//			return;
//		}
//		lexical_analyzer_.GetNextToken(token_);	// 读函数调用完毕后的下一个单词
//		return;
//	}
//	// 语法：读参数
//	vector<TokenTableItem::DecorateType> arg_decoratetypes = ArgumentList(depth + 1);
//	// 语法检查
//	if(Token::RIGHT_PAREN != token_.type_)
//	{
//		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
//		is_successful_ = false;
//		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//		{ 
//			lexical_analyzer_.GetNextToken(token_);
//		}
//		return;
//	}
//	lexical_analyzer_.GetNextToken(token_);
//
//	// 语义检查：函数参数与函数声明是否匹配
//	if(parameter_decorate_types.size() != arg_decoratetypes.size())	// 检查参数数量
//	{
//		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  function does not take " << arg_decoratetypes.size() << " argument";
//		if(arg_decoratetypes.size() > 1)
//		{
//			std::cout << "s\n";
//		}
//		else
//		{
//			std::cout << "\n";
//		}
//		is_successful_ = false;
//		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// 读到结尾或分号或END
//		{ 
//			lexical_analyzer_.GetNextToken(token_);
//		}
//		return;
//	}
//	// 类型是否匹配
//	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
//	{
//		// 左小于右小于表示不能从右操作数类型转到左操作数类型
//		if(parameter_decorate_types[i] < arg_decoratetypes[i])
//		{
//			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
//				<< " convert parameter " << i + 1 << " from " << TokenTableItem::DecorateTypeString[parameter_decorate_types[i]]
//				<< " to " <<  TokenTableItem::DecorateTypeString[arg_decoratetypes[i]] <<"\n";
//			// 这里仅仅是检查了参数类型不匹配，语法分析还可继续，故不返回
//		}
//	}
//	
//}

// <过程/函数调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::ProcFuncCallStatement(const Token proc_token, const vector<ExpressionAttribute> &parameter_attributes, size_t depth)	// 过程调用语句
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
	if(parameter_attributes.size() == 0)	// 如果函数本身就没有参数的话，就不用读参数了
	{
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
		lexical_analyzer_.GetNextToken(token_);	// 读函数调用完毕后的下一个单词
		return;
	}
	// 语法：读参数，并生成设置参数的四元式
	vector<ExpressionAttribute> exp_attributes = ArgumentList(parameter_attributes, depth + 1);
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
	lexical_analyzer_.GetNextToken(token_);

	// 语义检查：过程参数与过程声明是否匹配
	if(parameter_attributes.size() != exp_attributes.size())	// 检查参数数量
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  procedure/function does not take " << exp_attributes.size() << " argument";
		if(exp_attributes.size() > 1)
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
	for(int i = 0; i < static_cast<int>(exp_attributes.size()); ++i)
	{
		// 左小于右小于表示不能从右操作数类型转到左操作数类型
		if(parameter_attributes[i].decoratetype_ < exp_attributes[i].decoratetype_)
		{
			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
				<< "  cannot convert parameter " << i + 1 << " from " << TokenTableItem::DecorateTypeString[exp_attributes[i].decoratetype_]
			<< " to " <<  TokenTableItem::DecorateTypeString[parameter_attributes[i].decoratetype_] <<"\n";
			// 这里仅仅是检查了参数类型不匹配，语法分析还可继续，故不返回
		}
	}
}

// 给定的元素的操作数如果是数组变量，就将其化简为临时变量
void SyntaxAnalyzer::SimplifyArrayOperand(ExpressionAttribute &attribute) throw()
{
	if(Quaternary::ARRAY_ADDRESSING == attribute.addressingmethod_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.method1_ = attribute.addressingmethod_;
		q_subscript2temp.src1_ = attribute.value_;
		q_subscript2temp.method2_ = attribute.offset_addressingmethod_;
		q_subscript2temp.offset2_ = attribute.offset_;
		q_subscript2temp.method3_ = Quaternary::TEMPORARY_ADDRESSING;
		// 目标的临时变量标号的确定
		// 如果数组下标就是临时变量，那么就用这个变量
		// 否则就新开一个临时变量
		if(Quaternary::TEMPORARY_ADDRESSING == attribute.offset_addressingmethod_)
		{
			q_subscript2temp.dst_ = attribute.offset_;
		}
		else
		{
			q_subscript2temp.dst_ = tempvar_index_++;
			//// 更新最大临时变量个数
			//if(tempvar_index_ > max_local_temp_count_)
			//{
			//	max_local_temp_count_ = tempvar_index_;
			//}
		}
		quaternarytable_.push_back(q_subscript2temp);
		attribute.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		attribute.value_ = q_subscript2temp.dst_;
		attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		attribute.offset_ = 0;
	}
}

//// 修改在符号表的位置proc_func_index处的过程/函数对应的四元式的BEGIN语句，设置其临时变量的数量
//void SyntaxAnalyzer::SetTempVarCount(int proc_func_index, int max_tempvar_count) throw()
//{
//	for(vector<Quaternary>::reverse_iterator r_iter = quaternarytable_.rbegin();
//		r_iter != quaternarytable_.rend(); ++r_iter)
//	{
//		if(Quaternary::BEGIN == r_iter->op_
//			&& proc_func_index == r_iter->dst_)
//		{
//			r_iter->src2_ = max_tempvar_count;
//			return;
//		}
//	}
//}