#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableException.h"
#include <assert.h>
#include <sstream>

#define SYNTAXDEBUG
#define TOKENTABLEDEBUG		// 这个标志已经无效了

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexicalAnalyzer_, TokenTable &tokenTable_, std::ostream &output_) 
	: lexicalAnalyzer(lexicalAnalyzer_), tokenTable(tokenTable_), token(), level(0), tokenBuffer(), output(output_), isSuccessful(true)
{}



bool SyntaxAnalyzer::Parse()
{
	int depth = 0;
	PrintFunctionFrame("Parse()", depth);
	lexicalAnalyzer.GetNextToken(token);
	routine(depth + 1);
	return isSuccessful;
}
static string buffer;
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, int depth)
{
	
	if(depth * 4 == buffer.size())
	{
		output << buffer << func_name << '\n';
	}
	else if(depth * 4 > (int)buffer.size())
	{
		buffer.append("|");
		buffer.append(depth * 4 - buffer.size(), ' ');	// 这里不能减1
		output << buffer << func_name << '\n';
	}
	else // depth * 4 < buffer.size()
	{
		buffer.resize(depth * 4);
		output << buffer << func_name << '\n';
	}
}
// <程序> ::= <分程序>.
void SyntaxAnalyzer::routine(int depth)
{
	PrintFunctionFrame("routine()", depth);

	subRoutine(depth + 1);
	// 判断结束符号
	if(token.type != Token::PERIOD)
	{
		std::cout << "line " << token.lineNumber << ": " << token.toString() << '\t' << "should be '.' at the end of routine\n";
		isSuccessful = false;
	}
}

// <分程序> ::= [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>
void SyntaxAnalyzer::subRoutine(int depth)
{
	PrintFunctionFrame("subRoutine()", depth);

	// 四个可选分支
	if(token.type == Token::CONST)
	{
		constantPart(depth + 1);
	}
	if(token.type == Token::VAR)
	{
		variablePart(depth + 1);
	}
	if(token.type == Token::PROCEDURE)
	{
		procedurePart(depth + 1);
	}
	if(token.type == Token::FUNCTION)
	{
		functionPart(depth + 1);
	}
	// 一个必选分支
	if(token.type == Token::BEGIN)
	{
		statementBlockPart(depth + 1);
	}
	else
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  didn't find sentence block in subroutine\n";
		return;
	}
}

// <常量说明部分> ::= const<常量定义>{,<常量定义>};
void SyntaxAnalyzer::constantPart(int depth)
{
	PrintFunctionFrame("constantPart()", depth);

	assert(Token::CONST == token.type);

	// 常量定义
	do
	{
		lexicalAnalyzer.GetNextToken(token);
		constantDefination(depth + 1);
	} while(token.type == Token::COMMA);
	if(token.type != Token::SEMICOLON)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ';' after constant definition\n";
		/*while(lexicalAnalyzer.GetNextToken(token) && token.type != Token::SEMICOLON)
		{ }*/
		isSuccessful = false;
		if(Token::VAR == token.type || Token::PROCEDURE == token.type || Token::FUNCTION == token.type || Token::BEGIN == token.type)//	这里可能是忘记加分号了，所以直接返回而不读取下一个单词
		{
			return;	
		}
		else
		{
			while(token.type != Token::NIL  && token.type != Token::SEMICOLON)	// 若是其他单词，表示常量说明部分还未结束，故要读到下一个分号
			{
				lexicalAnalyzer.GetNextToken(token);
			};
			lexicalAnalyzer.GetNextToken(token);
			return;
		}
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <常量定义> ::= <标识符>＝<常量>
void SyntaxAnalyzer::constantDefination(int depth)
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token.type != Token::IDENTIFIER)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be identifier at the beginning of constant definition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// 读到下一个逗号或分号
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// 记录token以插入符号表
	Token constIdentifier = token;
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::EQU)
	{
		std::cout << "line " << token.lineNumber << ": " << token.toString() << "  should be '=' after identifier\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// 读到下一个逗号或分号
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::CONST_INTEGER 
	&& token.type != Token::CONST_CHAR)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be constant integer or character after '='\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// 读到下一个逗号或分号
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// 常量定义，插入符号表
	if(tokenTable.SearchDefinitionInCurrentLevel(constIdentifier.value.identifier))
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
	}
	else if(Token::CONST_INTEGER == token.type)
	{
		tokenTable.AddConstItem(constIdentifier, TokenTableItem::INTEGER, token.value.integer, level);
	}
	else
	{
		tokenTable.AddConstItem(constIdentifier, TokenTableItem::CHAR, token.value.character, level);
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);
}

// <变量说明部分> ::= var <变量定义>;{<变量定义>;}
void SyntaxAnalyzer::variablePart(int depth)
{
	PrintFunctionFrame("variablePart()", depth);

	assert(Token::VAR == token.type);

	lexicalAnalyzer.GetNextToken(token);
	do
	{
		variableDefinition(depth + 1);
		if(token.type != Token::SEMICOLON)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ';' after variable definition\n";
			isSuccessful = false;
			if(Token::IDENTIFIER == token.type)	// 忘记加分号，后面还有变量定义
			{
				continue;
			}
			else
			{
				// 忘记加分号了，后面又无法继续接上变量定义，只能跳转到下一部分
				while(Token::NIL != token.type && Token::SEMICOLON != token.type && Token::PROCEDURE != token.type && Token::FUNCTION != token.type && Token::BEGIN == token.type)
				{
					lexicalAnalyzer.GetNextToken(token);
				}
				if(Token::SEMICOLON == token.type)
				{
					lexicalAnalyzer.GetNextToken(token);
				}
				return;
			}
		}
		lexicalAnalyzer.GetNextToken(token);
	}while(token.type == Token::IDENTIFIER);
}

// <变量定义> ::= <标识符>{,<标识符>}:<类型>
void SyntaxAnalyzer::variableDefinition(int depth)
{
	PrintFunctionFrame("variableDefinition()", depth);

	if(token.type != Token::IDENTIFIER)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be identifier at the beginning of variable definition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::IDENTIFIER&& token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、下一个标识符或PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(token.type != Token::IDENTIFIER)	// 若读到的不是标识符，则返回上一层处理
		{
			return;
		}
		// 读到了标识符，则继续执行
	}
	tokenBuffer.clear();
	tokenBuffer.push_back(token);
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);
	while(token.type == Token::COMMA)
	{
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::IDENTIFIER)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be identifier at the beginning of variable definition\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::IDENTIFIER && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、下一个标识符或PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			if(token.type != Token::IDENTIFIER)	// 若读到的不是标识符，则返回上一层处理
			{
				return;
			}
			// 读到了标识符，则继续执行
		}
		tokenBuffer.push_back(token);
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		lexicalAnalyzer.GetNextToken(token);
	}
	if(token.type != Token::COLON)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ':' after identifier to specify the type\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::RW_INTEGER && token.type != Token::RW_CHAR && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、类型说明符、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::RW_INTEGER == token.type || Token::RW_CHAR == token.type)	// 若读到了类型说明符
		{
			typeSpecification(depth + 1);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	typeSpecification(depth + 1);
}
// <类型> ::= [array'['<无符号整数>']'of]<基本类型>
void SyntaxAnalyzer::typeSpecification(int depth)
{
	PrintFunctionFrame("typeSpecification()", depth);

	TokenTableItem::ItemType itemType = TokenTableItem::VARIABLE;
	int arrayLength = 0;
	if(token.type == Token::ARRAY)
	{
		itemType = TokenTableItem::ARRAY;
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::LEFT_BRACKET)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be '[' after \"array\"\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;	// 数组这里的出错处理太过麻烦，故直接跳过这句，返回结果。
		}
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::CONST_INTEGER)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a constant integer after '['\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		arrayLength = token.value.integer;
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ']' to match '['\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::OF)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"of\" after [*]\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
	}

	if(token.type != Token::RW_INTEGER
		&& token.type != Token::RW_CHAR)	// 若没有类型说明
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"integer\" or \"char\" for type specification\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// 插入符号表
	TokenTableItem::DecorateType decorateType = (token.type == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// 修饰符类型
	if(TokenTableItem::ARRAY == itemType)	// 若是数组
	{
		for(vector<Token>::const_iterator iter = tokenBuffer.begin(); iter != tokenBuffer.end(); ++iter)
		{
			if(tokenTable.SearchDefinitionInCurrentLevel(iter->value.identifier))	// 重定义后，仍然插入当前定义（也可不插入）
			{
				std::cout << "line " << iter->lineNumber << ":  " << iter->toString() << "  redifinition\n";
				isSuccessful = false;
			}
			tokenTable.AddArrayItem(*iter, decorateType, arrayLength, level);
		}
	}
	else									// 若是一般变量
	{
		for(vector<Token>::const_iterator iter = tokenBuffer.begin(); iter != tokenBuffer.end(); ++iter)
		{
			if(tokenTable.SearchDefinitionInCurrentLevel(iter->value.identifier))
			{
				std::cout << "line " << iter->lineNumber << ":  " << iter->toString() << "  redifinition\n";
				isSuccessful = false;
			}
			tokenTable.AddVariableItem(*iter, decorateType, level);
		}
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);
}

// <过程说明部分> ::= <过程首部><分程序>;{<过程首部><分程序>;}
void SyntaxAnalyzer::procedurePart(int depth)
{
	PrintFunctionFrame("procedurePart()", depth);

	do
	{
		procedureHead(depth + 1);
		subRoutine(depth + 1);
/*#ifdef TOKENTABLEDEBUG
		std::cout << "temp token table before procedure relocate:\n" << tokenTable.toString() << std::endl;
#endif*/
		tokenTable.Relocate();
		--level;
		if(Token::SEMICOLON != token.type)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of procedure\n";
			isSuccessful = false;
		}
		lexicalAnalyzer.GetNextToken(token);	// 分程序结束后应读入分号
	}while(Token::PROCEDURE == token.type);
}

// <过程首部> ::= procedure<过程标识符>'('[<形式参数表>]')';
void SyntaxAnalyzer::procedureHead(int depth)
{
	PrintFunctionFrame("procedureHead()", depth);

	assert(Token::PROCEDURE == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::IDENTIFIER)	// 未找到过程名
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a procedure name after \"procedure\"\n";
		isSuccessful = false;
		tokenTable.Locate();	// 这里进行定位，是为了抵消procedurePart中的重定位
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// 插入过程名到符号表
	string proc_name = token.value.identifier;
	if(tokenTable.SearchDefinitionInCurrentLevel(token.value.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的过程分程序的语法语义分析）
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
	}
	tokenTable.AddProcedureItem(token, level++);// 过程名之后leve要+1
	tokenTable.Locate();	// 定位（将过程名后紧邻的位置设为局部作用域的起始点）
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::LEFT_PAREN)	// 没有读到左括号，视作没有参数
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(	Token::VAR == token.type
		|| Token::IDENTIFIER == token.type)
	{
		int parameterCount = parameterList(depth);
		tokenTable.SetParameterCount(proc_name, parameterCount);
	}
	if(token.type != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  need ')' to match '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::SEMICOLON)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of procedure head\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void SyntaxAnalyzer::functionPart(int depth)
{
	PrintFunctionFrame("functionPart()", depth);

	do
	{
		functionHead(depth + 1);
		subRoutine(depth + 1);
/*#ifdef TOKENTABLEDEBUG
		std::cout << "\ntemp token table before function relocate:\n" << tokenTable.toString() << std::endl;
#endif*/
		tokenTable.Relocate();
		--level;
		if(Token::SEMICOLON != token.type)	// 分程序结束后应读入分号
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of procedure\n";
			isSuccessful = false;
		}
		lexicalAnalyzer.GetNextToken(token);	// 读入结尾的分号
	}while(Token::FUNCTION == token.type);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
void SyntaxAnalyzer::functionHead(int depth)
{
	PrintFunctionFrame("functionHead()", depth);

	assert(Token::FUNCTION == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::IDENTIFIER)	// 未找到函数名
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a function name after \"function\"\n";
		isSuccessful = false;
		tokenTable.Locate();	// 这里进行定位，是为了抵消functionPart中的重定位
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// 插入函数名到符号表
	string func_name = token.value.identifier;
	if(tokenTable.SearchDefinitionInCurrentLevel(token.value.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的函数分程序的语法语义分析）
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
	}
	tokenTable.AddFunctionItem(token, level++);// 过程名之后leve要+1
	tokenTable.Locate();
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::LEFT_PAREN)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(	Token::VAR == token.type
		|| Token::IDENTIFIER == token.type)
	{
		int parameterCount = parameterList(depth + 1);
		tokenTable.SetParameterCount(func_name, parameterCount); 
	}
	if(token.type != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  need ')' to match '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::COLON)	// 假设是忘记冒号
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ':' after parameter statement\n";
		isSuccessful = false;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(	token.type != Token::RW_INTEGER
		&& token.type != Token::RW_CHAR)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"integer\" or \"char\" after ':' to specify the return type\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// 分号后再读一个单词，可能会进入分程序
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	TokenTableItem::DecorateType decorateType = (token.type == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	tokenTable.SetFunctionReturnType(func_name, decorateType);
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::SEMICOLON)	// 这里有可能是落了分号，所以不再继续读
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of function head\n";
		isSuccessful = false;
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <形式参数表> ::= <形式参数段>{;<形式参数段>}
// 返回形参数量
int SyntaxAnalyzer::parameterList(int depth)		// 形参表
{
	PrintFunctionFrame("parameterList()", depth);

	int sum = parameterTerm(depth + 1);
	while(Token::SEMICOLON == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
		sum += parameterTerm(depth + 1);
	}
	return sum;
}

// <形式参数段> ::= [var]<标识符>{,<标识符>}:<基本类型>
// 返回该形参段的形参数量
int SyntaxAnalyzer::parameterTerm(int depth)		
{
	PrintFunctionFrame("parameterTerm()", depth);

	if(Token::VAR == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
	}
	if(token.type != Token::IDENTIFIER)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be parameter name surrounded by parentheses\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA  && token.type != Token::SEMICOLON)	// 读到结尾、分号、BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::IDENTIFIER != token.type)
		{
			return 0;
		}
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// 参数名压栈准备进符号表
	tokenBuffer.clear();
	tokenBuffer.push_back(token);
	lexicalAnalyzer.GetNextToken(token);
	int sum = 1;
	while(Token::COMMA == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::IDENTIFIER)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be parameter name after ','\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::IDENTIFIER)	// 读到结尾、分号、BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			if(Token::IDENTIFIER != token.type)
			{
				return 0;
			}
		}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		tokenBuffer.push_back(token);
		++sum;
		lexicalAnalyzer.GetNextToken(token);
	}
	if(token.type != Token::COLON)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ':' to specify the type after parameter name\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return 0;
		//throw SyntaxTokenException("lost ':' after parameter name", token);
	}
	lexicalAnalyzer.GetNextToken(token);
	if(	token.type != Token::RW_INTEGER
		&& token.type != Token::RW_CHAR)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"integer\" or \"char\" after ':' to specify the parameter type\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// 读到结尾、分号、PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return 0;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// 参数存符号表
	TokenTableItem::DecorateType decorateType = (token.type == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenBuffer.begin(); iter != tokenBuffer.end(); ++iter)
	{
		if(tokenTable.SearchDefinitionInCurrentLevel(iter->value.identifier))
		{
			std::cout << "line " << iter->lineNumber << ":  " << iter->toString() << "  redifinition\n";
			isSuccessful = false;
		}
		tokenTable.AddParameterItem(*iter, decorateType, level);
	} // end 存符号表

	lexicalAnalyzer.GetNextToken(token);
	return sum;
}

// <实在参数表> ::= <表达式>{,<表达式>}
vector<TokenTableItem::DecorateType> SyntaxAnalyzer::argumentList(int depth)			// 实参表
{
	PrintFunctionFrame("argumentList()", depth);

	vector<TokenTableItem::DecorateType> decorate_type_buffer;
	decorate_type_buffer.push_back(expression(depth + 1));
	while(Token::COMMA == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
		decorate_type_buffer.push_back(expression(depth + 1));
	}
	return decorate_type_buffer;
}

// <复合语句> ::= begin <语句>{;<语句>} end
void SyntaxAnalyzer::statementBlockPart(int depth)	// 复合语句
{
	PrintFunctionFrame("statementBlockPart()", depth);

	assert(Token::BEGIN == token.type);
	do
	{
		lexicalAnalyzer.GetNextToken(token);
		statement(depth + 1);
	}while(token.type == Token::SEMICOLON);
	if(token.type != Token::END)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"end\" at the end of statement block\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::END == token.type)
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <语句> ::= <标识符>(<赋值语句>|<过程调用语句>)|<条件语句>|<情况语句>|<复合语句>|<读语句>|<写语句>|<for循环语句>|<空>
void SyntaxAnalyzer::statement(int depth)
{
	PrintFunctionFrame("statement()", depth);

	Token idToken = token;	// 该token可能是过程名，先记下，待用
	TokenTable::iterator iter;
	TokenTableItem::DecorateType l_value_type = TokenTableItem::VOID;
	switch(token.type)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		iter = tokenTable.SearchDefinition(token);	// 查找符号表中的定义
		if(iter == tokenTable.end())
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		iter->AddUsedLine(token.lineNumber);		// 在符号表中插入引用行记录
		lexicalAnalyzer.GetNextToken(token);
		if(Token::LEFT_PAREN == token.type)	// 过程调用
		{
			if(iter->GetItemType() != TokenTableItem::PROCEDURE)	// 检查是否为过程
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not declared as a procedure\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return;
			}
			vector<TokenTableItem::DecorateType> decorate_types = tokenTable.GetProcFuncParameter(iter);
			procedureCallStatement(idToken, decorate_types, depth + 1);
		}
		else if(Token::ASSIGN == token.type
			|| Token::LEFT_BRACKET == token.type)
		{
			assigningStatement(idToken, iter, depth + 1);		
		}
		else if(Token::COLON == token.type)	// 此分支专门为了检查赋值号写错的情况
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  please check the spelling of the assigning operator\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		else
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  syntax error after identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		break;
	case Token::IF:
		ifStatement(depth + 1);
		break;
	case Token::CASE:
		caseStatement(depth + 1);
		break;
	case Token::BEGIN:
		statementBlockPart(depth + 1);
		break;
	case Token::READ:
		readStatement(depth + 1);
		break;
	case Token::WRITE:
		writeStatement(depth + 1);
		break;
	case Token::FOR:
		forLoopStatement(depth + 1);
		break;
	default:
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  syntax error at the beginning of statement\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		break;
	}
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
void SyntaxAnalyzer::assigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth)			// 赋值语句
{
	PrintFunctionFrame("assigningStatement()", depth);

	if(Token::LEFT_BRACKET == token.type)	// 对数组元素赋值
	{
		if(iter->GetItemType() != TokenTableItem::ARRAY)	// 检查是否为数组名
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  subscript requires array or pointer type\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
		expression(depth + 1);
		if(token.type != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ']' to match '['\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
	}
	else
	{
		if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER
		&& iter->GetItemType() != TokenTableItem::FUNCTION)	// 检查是否为变量或参数或函数名
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
	}
	if(token.type != Token::ASSIGN)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  \":=\" doesn't occur in the assigning statement\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// 以下三行为抛出异常做准备，放在GetNextToken前是为了准确记录赋值号的行号
	static Token errToken;
	errToken.type = Token::ASSIGN;
	errToken.lineNumber = token.lineNumber;

	lexicalAnalyzer.GetNextToken(token);
	TokenTableItem::DecorateType expression_type = expression(depth + 1);
	if(TokenTableItem::CHAR == iter->GetDecorateType() && TokenTableItem::INTEGER == expression_type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert from 'int' to 'char'\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
TokenTableItem::DecorateType SyntaxAnalyzer::expression(int depth)				// 表达式
{
	PrintFunctionFrame("expression()", depth);

	if(	token.type == Token::PLUS
		|| token.type == Token::MINUS)
	{
		lexicalAnalyzer.GetNextToken(token);
	}
	TokenTableItem::DecorateType decorate_type = term(depth + 1);
	while(	token.type == Token::PLUS
		||	token.type == Token::MINUS)
	{
		lexicalAnalyzer.GetNextToken(token);
		TokenTableItem::DecorateType local_decorate_type = term(depth + 1);
		// 这里的类型转换规则很简单：只有两个类型都为CHAR，最终类型才是CHAR
		// 否则就是INTEGER
		if(decorate_type == TokenTableItem::CHAR && local_decorate_type == TokenTableItem::CHAR)
		{
			decorate_type = TokenTableItem::CHAR;
		}
		else
		{
			decorate_type = TokenTableItem::INTEGER;
		}
	}
	return decorate_type;
}

// <项> ::= <因子>{<乘法运算符><因子>}
TokenTableItem::DecorateType SyntaxAnalyzer::term(int depth)						// 项
{
	PrintFunctionFrame("term()", depth);

	TokenTableItem::DecorateType decorate_type = factor(depth + 1);
	while(	token.type == Token::MUL
		||	token.type == Token::DIV)
	{
		lexicalAnalyzer.GetNextToken(token);
		TokenTableItem::DecorateType local_decorate_type = factor(depth + 1);
		// 这里的类型转换规则很简单：只有两个类型都为CHAR，最终类型才是CHAR
		// 否则就是INTEGER
		if(decorate_type == TokenTableItem::CHAR && local_decorate_type == TokenTableItem::CHAR)
		{
			decorate_type = TokenTableItem::CHAR;
		}
		else
		{
			decorate_type = TokenTableItem::INTEGER;
		}
	}
	return decorate_type;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>]) | '('<表达式>')' | <无符号整数> | <字符>
TokenTableItem::DecorateType SyntaxAnalyzer::factor(int depth)					// 因子
{
	PrintFunctionFrame("factor()", depth);
	TokenTableItem::DecorateType decorate_type = TokenTableItem::VOID;	// 因子的类型
	if(Token::IDENTIFIER == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		//decorate_type = tokenTable.AddUsedLine(token);	// 插入到符号表
		TokenTable::iterator iter = tokenTable.SearchDefinition(token);	// 寻找定义
		if(iter == tokenTable.end())
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return decorate_type;
		}
		decorate_type = iter->GetDecorateType();
		iter->AddUsedLine(token.lineNumber);		// 在符号表中插入引用行记录
		Token idToken = token;	// 记下，待用
		lexicalAnalyzer.GetNextToken(token);
		if(Token::LEFT_BRACKET == token.type)	// 对数组元素的赋值
		{
			if(iter->GetItemType() != TokenTableItem::ARRAY)	// 检查是否为数组名
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  subscript requires array or pointer type\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
			lexicalAnalyzer.GetNextToken(token);
			expression(depth + 1);
			if(token.type != Token::RIGHT_BRACKET)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ']' to match '['\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
		}
		else if(Token::LEFT_PAREN == token.type)
		{
			// 检查是否为函数
			if(iter->GetItemType() != TokenTableItem::FUNCTION)	// 检查是否为过程
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not declared as a function\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
			// 从符号表中取出函数的参数类型，待functionCallStatement去匹配参数
			vector<TokenTableItem::DecorateType> decorate_types = tokenTable.GetProcFuncParameter(iter);
			functionCallStatement(idToken, decorate_types, depth + 1);
		}
		else
		{
			// 检查是否为变量、常量或过程/函数的参数
			if(iter->GetItemType() != TokenTableItem::VARIABLE && iter->GetItemType() != TokenTableItem::PARAMETER && iter->GetItemType() != TokenTableItem::CONST)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  single token factor should be varaible or constant\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
		}
	}
	else if(Token::LEFT_PAREN == token.type)	// 括号括起来的表达式
	{
		decorate_type = expression(depth + 1);	// 记录类型
		if(token.type != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match '('\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return decorate_type;
		}
		lexicalAnalyzer.GetNextToken(token);
	}
	else if(Token::CONST_INTEGER == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		decorate_type = TokenTableItem::INTEGER;	// 记录类型
		lexicalAnalyzer.GetNextToken(token);
	}
	else if(Token::CONST_CHAR == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		decorate_type = TokenTableItem::CHAR;	// 记录类型
		lexicalAnalyzer.GetNextToken(token);
	}
	else
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not a legal factor\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return decorate_type;
	}
	return decorate_type;
}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
void SyntaxAnalyzer::ifStatement(int depth)				// 条件语句
{
	PrintFunctionFrame("ifStatement()", depth);

	assert(Token::IF == token.type);
	lexicalAnalyzer.GetNextToken(token);
	condition(depth + 1);
	if(Token::THEN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"then\" after condition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	statement(depth + 1);
	if(Token::ELSE == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
		statement(depth + 1);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>
void SyntaxAnalyzer::condition(int depth)				// 条件
{
	PrintFunctionFrame("condition()", depth);

	expression(depth + 1);
	if(		Token::LT	!= token.type
		&&	Token::LEQ	!= token.type
		&&	Token::GT	!= token.type
		&&	Token::GEQ	!= token.type
		&&	Token::EQU	!= token.type
		&&	Token::NEQ	!= token.type
		)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be logic operator in the middle of condition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
}

// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void SyntaxAnalyzer::caseStatement(int depth)			// 情况语句
{
	PrintFunctionFrame("caseStatement()", depth);

	assert(Token::CASE == token.type);
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
	if(Token::OF != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"of\" to specify the certain case\n";
		isSuccessful = false;
		// 这里假设是忘记写of，故不返回
	}
	do
	{
		lexicalAnalyzer.GetNextToken(token);
		caseList(depth + 1);
	}while(Token::SEMICOLON == token.type);
	if(Token::END != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"end\" at the end of case statement\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::END == token.type)
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <情况表元素> ::= <情况常量表>:<语句>
void SyntaxAnalyzer::caseList(int depth)					// 情况表元素
{
	PrintFunctionFrame("caseList()", depth);

	if(Token::CONST_INTEGER != token.type
		&& Token::CONST_CHAR != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be constant integer or character after \"case\"\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);
	while(Token::COMMA == token.type)
	{
		lexicalAnalyzer.GetNextToken(token);
		if(Token::CONST_INTEGER != token.type
			&& Token::CONST_CHAR != token.type)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be constant integer or character after \"case\"\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		lexicalAnalyzer.GetNextToken(token);
	}
	if(Token::COLON != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ':' after constant to specify the certain action\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	statement(depth + 1);
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
void SyntaxAnalyzer::readStatement(int depth)			// 读语句
{
	PrintFunctionFrame("readStatement()", depth);

	assert(Token::READ == token.type);
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);

	if(Token::LEFT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be '(' to specify the arguments\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	
	do
	{
		lexicalAnalyzer.GetNextToken(token);
		if(Token::IDENTIFIER != token.type)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be variable name in the location of read argument\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		TokenTable::iterator iter = tokenTable.SearchDefinition(token);
		if(iter == tokenTable.end())
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		iter->AddUsedLine(token.lineNumber);		// 在符号表中插入引用行记录
		if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER)	// 检查是否为变量或参数或函数名
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned in read call\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
	}while(Token::COMMA == token.type);
	
	if(Token::RIGHT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match the '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <写语句> ::= write'(' (<字符串>[,<表达式>] | <表达式>) ')'
void SyntaxAnalyzer::writeStatement(int depth)			// 写语句
{
	PrintFunctionFrame("writeStatement()", depth);

	assert(Token::WRITE == token.type);

#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	lexicalAnalyzer.GetNextToken(token);
	
	if(Token::LEFT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be '(' to specify the arguments\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	
	if(Token::CONST_STRING == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		lexicalAnalyzer.GetNextToken(token);
		if(Token::COMMA == token.type)
		{
			lexicalAnalyzer.GetNextToken(token);
			expression(depth + 1);
		}
	}
	else
	{
		expression(depth + 1);
	}
	
	if(Token::RIGHT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match the '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <for循环语句> ::= for <标识符>  := <表达式> （downto | to） <表达式> do <语句>
void SyntaxAnalyzer::forLoopStatement(int depth)			// for循环语句
{
	PrintFunctionFrame("forLoopStatement()", depth);

	assert(Token::FOR == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(Token::IDENTIFIER != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be loop variable name after for\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	TokenTable::iterator iter = tokenTable.SearchDefinition(token);
	if(iter == tokenTable.end())
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	iter->AddUsedLine(token.lineNumber);		// 在符号表中插入引用行记录
	if(iter->GetItemType() != TokenTableItem::VARIABLE
	&& iter->GetItemType() != TokenTableItem::PARAMETER)	// 检查是否为变量或参数或函数名
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned in for loop\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(Token::ASSIGN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \":=\" after loop variable\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
	if(Token::DOWNTO != token.type
		&& Token::TO != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"to\" or \"downto\" after variable assigning\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
	if(Token::DO != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be do after loop head\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	statement(depth + 1);
}

// <过程调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::procedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// 过程调用语句
{
	PrintFunctionFrame("procedureCallStatement()", depth);

	if(Token::LEFT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be '(' to specify the argument\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	vector<TokenTableItem::DecorateType> argument_decorate_types = argumentList(depth + 1);
	if(Token::RIGHT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match the '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);

	// 检查过程参数与过程声明是否匹配
	if(parameter_decorate_types.size() != argument_decorate_types.size())	// 检查参数数量
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  procedure does not take " << argument_decorate_types.size() << " argument";
		if(argument_decorate_types.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// 这里的数据类型只能为CHAR或INTEGER
		assert(argument_decorate_types[i] != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != argument_decorate_types[i])
		{
			// 要求参数为CHAR，实际参数为INTEGER，则无法转换
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && argument_decorate_types[i] == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				isSuccessful = false;
				// 这里仅仅是检查了参数类型不匹配，实际语法成分分析还可继续，
			}
		}
	}
}

// <函数调用语句> ::= '('[<实在参数表>]')'
void SyntaxAnalyzer::functionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// 函数调用语句
{
	PrintFunctionFrame("functionCallStatement()", depth);

	assert(Token::LEFT_PAREN == token.type);

	lexicalAnalyzer.GetNextToken(token);
	vector<TokenTableItem::DecorateType> argument_decorate_types = argumentList(depth + 1);
	if(Token::RIGHT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match the '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);

	// 检查函数参数与函数声明是否匹配
	if(parameter_decorate_types.size() != argument_decorate_types.size())	// 检查参数数量
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  function does not take " << argument_decorate_types.size() << " argument";
		if(argument_decorate_types.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// 读到结尾或分号或END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// 这里的数据类型只能为CHAR或INTEGER
		assert(argument_decorate_types[i] != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != argument_decorate_types[i])
		{
			// 要求参数为CHAR，实际参数为INTEGER，则无法转换
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && argument_decorate_types[i] == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				isSuccessful = false;
				// 这里仅仅是检查了参数类型不匹配，实际语法成分分析还可继续，故不返回
			}
		}
	}
	
}