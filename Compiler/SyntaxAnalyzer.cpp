#include "SyntaxAnalyzer.h"
#include <assert.h>
#include <sstream>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer) throw()
	: lexical_analyzer_(lexical_analyzer), token_(), is_successful_(true), syntax_process_buffer_(), syntax_format_string_()
{
	
}


bool SyntaxAnalyzer::Parse() throw()
{
	size_t depth = 0;
	lexical_analyzer_.ResetTokenPos();
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
		if(Token::NIL != token_.type_)	// 程序并没有结束
		{
			ErrorHandle(WRONGENDINGTOKEN);
		}
		else	// 程序确实结束了
		{
			ErrorHandle(LACKENDINGPERIOD);
		}
		return;
	}
	// 验证PERIOD后面是否还有单词
	if(lexical_analyzer_.GetNextToken(token_))
	{
		ErrorHandle(REDUNDANTTOOKEN);
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
		ErrorHandle(NOSTATEMENTBLOCK);
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
		ErrorHandle(LACKSEMICOLON);
		//return;	// 这里暂不返回，要执行最后一条语句，读取了分号后的下一个单词后再返回
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <常量定义> ::= <标识符>＝<常量>
void SyntaxAnalyzer::constantDefination(size_t depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		ErrorHandle(LACKIDENTIFIER);
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
		ErrorHandle(LACKEQU);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::CONST_INTEGER 
	&& token_.type_ != Token::CONST_CHAR)
	{
		ErrorHandle(LACKCONSTANT);
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
			ErrorHandle(LACKSEMICOLON);
			//return;	// 暂不返回，待读取分号的下一个单词
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
		ErrorHandle(LACKIDENTIFIER);
		return;
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
			ErrorHandle(LACKIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
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
			ErrorHandle(LACKLEFTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::CONST_INTEGER)
		{
			ErrorHandle(LACKCONSTINTEGER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			ErrorHandle(LACKRIGHTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::OF)
		{
			ErrorHandle(LACKOF);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	if(token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)	// 若没有类型说明
	{
		ErrorHandle(LACKRWTYPE);
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
			ErrorHandle(LACKSEMICOLON);
			//return;	// 暂不返回，待读取分号的下一个单词
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
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 继续读取单词
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)	// 没有读到左括号，视作没有参数
	{
		ErrorHandle(LACKLEFTPAREN);
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
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)
	{
		ErrorHandle(LACKSEMICOLON);
		return;
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
			ErrorHandle(LACKSEMICOLON);
			return;
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
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		ParameterList(depth + 1);
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		ErrorHandle(LACKRWTYPE);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// 这里有可能是落了分号，所以不再继续读
	{
		ErrorHandle(LACKSEMICOLON);
		return;
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
		ErrorHandle(LACKIDENTIFIER);
		return;
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
			ErrorHandle(LACKIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		ErrorHandle(LACKRWTYPE);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);
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
		ErrorHandle(LACKSEMICOLON);	// 注：这里大部分出错原因都是缺少了分号，而不是真的忘记写end
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
		else
		{
			ErrorHandle(LACKASSIGNINGTOKEN);
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
			ErrorHandle(LACKRIGHTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	// 语法检查
	if(token_.type_ != Token::ASSIGN)
	{
		ErrorHandle(LACKASSIGNINGTOKEN);
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

	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
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

	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
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
		//Token idToken = token_;	// 记下，待用
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
				ErrorHandle(LACKRIGHTBRACKET);
				return;
			}
			// 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// 左括号，函数调用
		{
			ProcFuncCallStatement(depth + 1);			
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
			ErrorHandle(LACKRIGHTPAREN);
			return;
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
		ErrorHandle(WRONGFACTOR);
		return;
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
		ErrorHandle(LACKTHEN);
		return;
	}
	// 读取if成功后的语句
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// 读取else的语句（如果有的话）
	if(Token::ELSE == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

//// <条件> ::= <表达式><关系运算符><表达式>
//void SyntaxAnalyzer::Condition(size_t depth) throw()				// 条件
//{
//	PrintFunctionFrame("Condition()", depth);
//
//	Expression(depth + 1);
//	switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是THEN（当条件中仅有一个表达式时）
//	{
//	case Token::LT:
//	case Token::LEQ:
//	case Token::GT:
//	case Token::GEQ:
//	case Token::EQU:
//	case Token::NEQ:
//	case Token::THEN:
//		break;
//	// 因为之前已经检查过了，所以正常情况下不可能有default
//	default:
//		ErrorHandle(LACKLOGICOPERATOR);
//		return;
//		break;
//	}
//	if(Token::THEN != token_.type_)	// 如果还有下一个表达式，再继续读取
//	{
//		lexical_analyzer_.GetNextToken(token_);
//		Expression(depth + 1);
//	}
//}

// <条件> ::= <布尔表达式>
void SyntaxAnalyzer::Condition(size_t depth) throw()				// 条件
{
	PrintFunctionFrame("Condition()", depth);
	BoolExpression(depth + 1);
}

// <布尔表达式> ::= <布尔项> [ <逻辑或> <布尔项>]
void SyntaxAnalyzer::BoolExpression(size_t depth) throw()	// 布尔表达式
{
	PrintFunctionFrame("BoolExpression()", depth);
	bool isfirst = true;
	do
	{
		if(!isfirst)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		else
		{
			isfirst = false;
		}
		BoolTerm(depth + 1);
	}while(Token::LOGICOR == token_.type_);
}

// <布尔项> ::= <布尔因子> [<逻辑与><布尔因子>]
void SyntaxAnalyzer::BoolTerm(size_t depth) throw()			// 布尔项
{
	PrintFunctionFrame("BoolTerm()", depth);
	bool isfirst = true;
	do
	{
		if(!isfirst)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		else
		{
			isfirst = false;
		}
		BoolFactor(depth + 1);
	}while(Token::LOGICAND == token_.type_);
}

// <布尔因子> ::= <表达式>[<关系运算符><表达式>] | ‘(‘<布尔表达式>’)’
void SyntaxAnalyzer::BoolFactor(size_t depth) throw()		// 布尔因子
{
	PrintFunctionFrame("BoolFactor()", depth);
	// 先考虑非左括号的情况
	if(Token::LEFT_PAREN != token_.type_ || IsExpression(depth + 1))
	{
		Expression(depth + 1);
		switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是右括号或THEN或OR或AND（当条件中仅有一个表达式时）
		{
		case Token::LT:
		case Token::LEQ:
		case Token::GT:
		case Token::GEQ:
		case Token::EQU:
		case Token::NEQ:
		case Token::RIGHT_PAREN:
		case Token::THEN:
		case Token::LOGICOR:
		case Token::LOGICAND:
			break;
		default:
			ErrorHandle(LACKLOGICOPERATOR);
			return;
			break;
		}
		if(Token::RIGHT_PAREN != token_.type_
			&& Token::THEN != token_.type_
			&& Token::LOGICOR != token_.type_
			&& Token::LOGICAND != token_.type_)	// 如果还有下一个表达式，再继续读取
		{
			lexical_analyzer_.GetNextToken(token_);	// 读取关系运算符的下一个单词
			Expression(depth + 1);
		}
	}
	else	// 第一个是左括号，且不能识别为表达式的情况
	{
		lexical_analyzer_.GetNextToken(token_);	// 读左括号的下一个单词
		BoolExpression(depth + 1);
		lexical_analyzer_.GetNextToken(token_);	// 读右括号的下一个单词
	}
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
bool SyntaxAnalyzer::IsExpression(size_t depth) throw()
{
	PrintFunctionFrame("IsExpression()", depth);
	vector<Token>::const_iterator iter = lexical_analyzer_.GetTokenPosition();
	Token tmp = token_;
	bool result = ExpressionTest(depth + 1);
	token_ = tmp;
	lexical_analyzer_.SetTokenPosition(iter);
	return result;
}


// <表达式> ::= [+|-]<项>{<加法运算符><项>}
bool SyntaxAnalyzer::ExpressionTest(size_t depth) throw()
{
	PrintFunctionFrame("ExpressionTest()", depth);

	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}
	if(!TermTest(depth + 1))
	{
		return false;
	}

	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		if(!TermTest(depth + 1))
		{
			return false;
		}
	}
	return true;
}

// <项> ::= <因子>{<乘法运算符><因子>}
bool SyntaxAnalyzer::TermTest(size_t depth) throw()						// 项
{
	PrintFunctionFrame("TermTest()", depth);

	if(!FactorTest(depth + 1))
	{
		return false;
	}

	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(!FactorTest(depth + 1))
		{
			return false;
		}
	}
	return true;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')' 
//          | <无符号整数> 
//          | <字符>
bool SyntaxAnalyzer::FactorTest(size_t depth) throw()					// 因子
{
	PrintFunctionFrame("FactorTest()", depth);

	// 语法检查：标识符的情况【变量、常变量、数组、函数调用】
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		// 数组元素
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// 语法：读入作为下标的表达式
			lexical_analyzer_.GetNextToken(token_);
			if(!ExpressionTest(depth + 1))
			{
				return false;
			}
			// 语法检查
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				return false;
			}
			// 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// 左括号，函数调用
		{
			ProcFuncCallStatementTest(depth + 1);			
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// 括号括起来的表达式
	{
		// bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
		// 读表达式的第一个单词
		lexical_analyzer_.GetNextToken(token_);
		// 再读取表达式
		if(!ExpressionTest(depth + 1))	// 记录类型
		{
			return false;
		}
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			return false;
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
		return false;
	}
	return true;
}

// <过程/函数调用语句> ::= '('[<实在参数表>]')'
bool SyntaxAnalyzer::ProcFuncCallStatementTest(size_t depth)	// 过程调用语句
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	// 语法检查
	if(Token::LEFT_PAREN != token_.type_)
	{
		return false;
	}
	// 语法：读入右括号或参数表的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 语法：读参数
	if(Token::RIGHT_PAREN != token_.type_)
	{
		if(!ArgumentListTest(depth + 1))
		{
			return false;
		}
		// 语法检查
		if(Token::RIGHT_PAREN != token_.type_)
		{
			return false;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
	return true;
}

// <实在参数表> ::= <表达式>{,<表达式>}
bool SyntaxAnalyzer::ArgumentListTest(size_t depth) throw()			// 实参表
{
	PrintFunctionFrame("ArgumentList()", depth);

	if(!ExpressionTest(depth + 1))
	{
		return false;
	}
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(!ExpressionTest(depth + 1))
		{
			return false;
		}
	}
	return true;
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
		ErrorHandle(LACKOF);
		return;
	}
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);

	// 检测结束标志
	if(Token::END != token_.type_)
	{
		ErrorHandle(LACKSEMICOLON);
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
	&& Token::CONST_CHAR != token_.type_
	&& Token::IDENTIFIER != token_.type_)
	{
		ErrorHandle(LACKCONSTANTORIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_
		&& Token::IDENTIFIER != token_.type_)
		{
			ErrorHandle(LACKCONSTANTORIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// 读取下一个单词
		lexical_analyzer_.GetNextToken(token_);
	}
	if(Token::COLON != token_.type_)
	{
		ErrorHandle(LACKCASECOLON);
		return;
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
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::IDENTIFIER != token_.type_)
		{
			ErrorHandle(LACKIDENTIFIER);
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
				ErrorHandle(LACKRIGHTBRACKET);
				return;
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
		ErrorHandle(LACKLEFTBRACKET);
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
		ErrorHandle(LACKLEFTPAREN);
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
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <while循环语句> ::= while <条件> do <语句>
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
		ErrorHandle(LACKDO);
		return;
	}
	// 读入循环体的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 读入循环体
	Statement(depth + 1);
}

// <for循环语句> ::= for <标识符> := <表达式> （downto | to） <表达式> do <语句>
void SyntaxAnalyzer::ForLoopStatement(size_t depth) throw()			// for循环语句
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);

	// 读取标识符
	lexical_analyzer_.GetNextToken(token_);
	if(Token::IDENTIFIER != token_.type_)
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 读取赋值号
	lexical_analyzer_.GetNextToken(token_);
	if(Token::ASSIGN != token_.type_)
	{
		ErrorHandle(LACKASSIGNINGTOKEN);
		return;
	}
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// 检测to/downto
	if(Token::DOWNTO != token_.type_
		&& Token::TO != token_.type_)
	{
		ErrorHandle(LACKTO_DOWNTO);
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
		ErrorHandle(LACKDO);
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
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	// 语法：读入右括号或参数表的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 语法：读参数
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(depth + 1);
		// 语法检查
		if(Token::RIGHT_PAREN != token_.type_)
		{
			ErrorHandle(LACKRIGHTPAREN);
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
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



void SyntaxAnalyzer::ErrorHandle(ErrorType error_type) throw()
{
	using std::cout;
	is_successful_ = false;
	int offset = lexical_analyzer_.GetLine(token_.lineNumber_).find_first_not_of(" \t\n");
	int count = lexical_analyzer_.GetLine(token_.lineNumber_).find_last_not_of(" \t\n") - offset + 1;
	string errline = offset == string::npos ? "" : lexical_analyzer_.GetLine(token_.lineNumber_).substr(offset, count);

	cout << "syntax error(" << error_type << ")";
	//switch(error_type)
	//{
	//case LACKENDINGPERIOD:
	//	cout << "\nerror info: lack '.' at the end of program\n";
	//	break;
	//case WRONGENDINGTOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '.' at the end of program\n";
	//	break;
	//case REDUNDANTTOOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "redundant token after '.'\n";
	//	break;
	//case NOSTATEMENTBLOCK:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "didn't find statement block in subroutine\n";
	//	break;
	//case LACKSEMICOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ';' after the sentence(or do you forget ','?)\n";
	//	break;
	//case LACKIDENTIFIER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be an identifier\n";
	//	break;
	//case LACKEQU:	// 只在常量定义时出现
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '=' after identifier\n";
	//	break;
	//case LACKCONSTANT:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be constant integer or character\n";
	//	break;
	//case LACKTYPECOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the type\n";
	//	break;
	//case LACKLEFTBRACKET:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '[' to specify an array\n";
	//	break;
	//case LACKRIGHTBRACKET:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ']' to match '['\n";
	//	break;
	//case LACKCONSTINTEGER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "need a constant integer\n";
	//	break;
	//case LACKOF:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"of\" after [] to specify array type\n";
	//	break;
	//case LACKRWTYPE:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"integer\" or \"char\" for type specification\n";
	//	break;
	//case LACKLEFTPAREN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '('\n";
	//	break;
	//case LACKRIGHTPAREN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ')' to match '('\n";
	//	break;
	//case LACKASSIGNINGTOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \":=\"\n";
	//	break;
	//case WRONGFACTOR:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "illegal factor, should be a value or variable\n";
	//	break;
	//case LACKTHEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "need \"then\" after if condition\n";
	//	break;
	//case LACKLOGICOPERATOR:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be a logic operator or \"then\"\n";
	//	break;
	//case LACKCONSTANTORIDENTIFIER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be constant integer or character or identifier\n";
	//	break;
	//case LACKCASECOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the case action\n";
	//	break;
	//case LACKDO:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"do\" before loop body\n";
	//	break;
	//case LACKTO_DOWNTO:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"to\" or \"downto\" after variable assigning\n";
	//	break;
	//default:
	//	// 未知错误
	//	assert(false);
	//	break;
	//}
	switch(error_type)
	{
	case LACKENDINGPERIOD:
		cout << "\nerror info: lack '.' at the end of program\n";
		break;
	case WRONGENDINGTOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '.' at the end of program\n";
		break;
	case REDUNDANTTOOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "redundant token after '.'\n";
		break;
	case NOSTATEMENTBLOCK:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "didn't find statement block in subroutine\n";
		break;
	case LACKSEMICOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ';' after the sentence(or do you forget ','?)\n";
		break;
	case LACKIDENTIFIER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be an identifier\n";
		break;
	case LACKEQU:	// 只在常量定义时出现
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '=' after identifier\n";
		break;
	case LACKCONSTANT:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be constant integer or character\n";
		break;
	case LACKTYPECOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the type\n";
		break;
	case LACKLEFTBRACKET:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '[' to specify an array\n";
		break;
	case LACKRIGHTBRACKET:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ']' to match '['\n";
		break;
	case LACKCONSTINTEGER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "need a constant integer\n";
		break;
	case LACKOF:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"of\" after [] to specify array type\n";
		break;
	case LACKRWTYPE:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"integer\" or \"char\" for type specification\n";
		break;
	case LACKLEFTPAREN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '('\n";
		break;
	case LACKRIGHTPAREN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ')' to match '('\n";
		break;
	case LACKASSIGNINGTOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \":=\"\n";
		break;
	case WRONGFACTOR:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "illegal factor, should be a value or variable\n";
		break;
	case LACKTHEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "need \"then\" after if condition\n";
		break;
	case LACKLOGICOPERATOR:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be a logic operator or \"then\", ')' or \"||\" or \"&&\"\n";
		break;
	case LACKCONSTANTORIDENTIFIER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be constant integer or character or identifier\n";
		break;
	case LACKCASECOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the case action\n";
		break;
	case LACKDO:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"do\" before loop body\n";
		break;
	case LACKTO_DOWNTO:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"to\" or \"downto\" after variable assigning\n";
		break;
	default:
		// 未知错误
		assert(false);
		break;
	}
	while(Token::NIL != token_.type_ && Token::SEMICOLON != token_.type_)	// 读完整条语句
	{
		lexical_analyzer_.GetNextToken(token_);
	}
}