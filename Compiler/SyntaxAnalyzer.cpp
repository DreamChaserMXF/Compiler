#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableException.h"
#include <assert.h>
#include <sstream>

#define SYNTAXDEBUG
#define TOKENTABLEDEBUG		// �����־�Ѿ���Ч��

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
		buffer.append(depth * 4 - buffer.size(), ' ');	// ���ﲻ�ܼ�1
		output << buffer << func_name << '\n';
	}
	else // depth * 4 < buffer.size()
	{
		buffer.resize(depth * 4);
		output << buffer << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SyntaxAnalyzer::routine(int depth)
{
	PrintFunctionFrame("routine()", depth);

	subRoutine(depth + 1);
	// �жϽ�������
	if(token.type != Token::PERIOD)
	{
		std::cout << "line " << token.lineNumber << ": " << token.toString() << '\t' << "should be '.' at the end of routine\n";
		isSuccessful = false;
	}
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SyntaxAnalyzer::subRoutine(int depth)
{
	PrintFunctionFrame("subRoutine()", depth);

	// �ĸ���ѡ��֧
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
	// һ����ѡ��֧
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

// <����˵������> ::= const<��������>{,<��������>};
void SyntaxAnalyzer::constantPart(int depth)
{
	PrintFunctionFrame("constantPart()", depth);

	assert(Token::CONST == token.type);

	// ��������
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
		if(Token::VAR == token.type || Token::PROCEDURE == token.type || Token::FUNCTION == token.type || Token::BEGIN == token.type)//	������������Ǽӷֺ��ˣ�����ֱ�ӷ��ض�����ȡ��һ������
		{
			return;	
		}
		else
		{
			while(token.type != Token::NIL  && token.type != Token::SEMICOLON)	// �����������ʣ���ʾ����˵�����ֻ�δ��������Ҫ������һ���ֺ�
			{
				lexicalAnalyzer.GetNextToken(token);
			};
			lexicalAnalyzer.GetNextToken(token);
			return;
		}
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <��������> ::= <��ʶ��>��<����>
void SyntaxAnalyzer::constantDefination(int depth)
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token.type != Token::IDENTIFIER)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be identifier at the beginning of constant definition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// ��¼token�Բ�����ű�
	Token constIdentifier = token;
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::EQU)
	{
		std::cout << "line " << token.lineNumber << ": " << token.toString() << "  should be '=' after identifier\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
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
		while(token.type != Token::NIL && token.type != Token::COMMA && token.type != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// �������壬������ű�
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

// <����˵������> ::= var <��������>;{<��������>;}
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
			if(Token::IDENTIFIER == token.type)	// ���Ǽӷֺţ����滹�б�������
			{
				continue;
			}
			else
			{
				// ���Ǽӷֺ��ˣ��������޷��������ϱ������壬ֻ����ת����һ����
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

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
void SyntaxAnalyzer::variableDefinition(int depth)
{
	PrintFunctionFrame("variableDefinition()", depth);

	if(token.type != Token::IDENTIFIER)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be identifier at the beginning of variable definition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::IDENTIFIER&& token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(token.type != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
		{
			return;
		}
		// �����˱�ʶ���������ִ��
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::IDENTIFIER && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			if(token.type != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
			{
				return;
			}
			// �����˱�ʶ���������ִ��
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
		while(token.type != Token::NIL && token.type != Token::RW_INTEGER && token.type != Token::RW_CHAR && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β������˵�������ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::RW_INTEGER == token.type || Token::RW_CHAR == token.type)	// ������������˵����
		{
			typeSpecification(depth + 1);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	typeSpecification(depth + 1);
}
// <����> ::= [array'['<�޷�������>']'of]<��������>
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;	// ��������ĳ�����̫���鷳����ֱ��������䣬���ؽ����
		}
		lexicalAnalyzer.GetNextToken(token);
		if(token.type != Token::CONST_INTEGER)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a constant integer after '['\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		lexicalAnalyzer.GetNextToken(token);
	}

	if(token.type != Token::RW_INTEGER
		&& token.type != Token::RW_CHAR)	// ��û������˵��
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"integer\" or \"char\" for type specification\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// ������ű�
	TokenTableItem::DecorateType decorateType = (token.type == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// ���η�����
	if(TokenTableItem::ARRAY == itemType)	// ��������
	{
		for(vector<Token>::const_iterator iter = tokenBuffer.begin(); iter != tokenBuffer.end(); ++iter)
		{
			if(tokenTable.SearchDefinitionInCurrentLevel(iter->value.identifier))	// �ض������Ȼ���뵱ǰ���壨Ҳ�ɲ����룩
			{
				std::cout << "line " << iter->lineNumber << ":  " << iter->toString() << "  redifinition\n";
				isSuccessful = false;
			}
			tokenTable.AddArrayItem(*iter, decorateType, arrayLength, level);
		}
	}
	else									// ����һ�����
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

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
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
		lexicalAnalyzer.GetNextToken(token);	// �ֳ��������Ӧ����ֺ�
	}while(Token::PROCEDURE == token.type);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
void SyntaxAnalyzer::procedureHead(int depth)
{
	PrintFunctionFrame("procedureHead()", depth);

	assert(Token::PROCEDURE == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a procedure name after \"procedure\"\n";
		isSuccessful = false;
		tokenTable.Locate();	// ������ж�λ����Ϊ�˵���procedurePart�е��ض�λ
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// ��������������ű�
	string proc_name = token.value.identifier;
	if(tokenTable.SearchDefinitionInCurrentLevel(token.value.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĹ��̷ֳ�����﷨���������
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
	}
	tokenTable.AddProcedureItem(token, level++);// ������֮��leveҪ+1
	tokenTable.Locate();	// ��λ��������������ڵ�λ����Ϊ�ֲ����������ʼ�㣩
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::LEFT_PAREN)	// û�ж��������ţ�����û�в���
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
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

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
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
		if(Token::SEMICOLON != token.type)	// �ֳ��������Ӧ����ֺ�
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of procedure\n";
			isSuccessful = false;
		}
		lexicalAnalyzer.GetNextToken(token);	// �����β�ķֺ�
	}while(Token::FUNCTION == token.type);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
void SyntaxAnalyzer::functionHead(int depth)
{
	PrintFunctionFrame("functionHead()", depth);

	assert(Token::FUNCTION == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be a function name after \"function\"\n";
		isSuccessful = false;
		tokenTable.Locate();	// ������ж�λ����Ϊ�˵���functionPart�е��ض�λ
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�BEGIN
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
	// ���뺯���������ű�
	string func_name = token.value.identifier;
	if(tokenTable.SearchDefinitionInCurrentLevel(token.value.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĺ����ֳ�����﷨���������
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
	}
	tokenTable.AddFunctionItem(token, level++);// ������֮��leveҪ+1
	tokenTable.Locate();
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::LEFT_PAREN)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  redifinition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	if(token.type != Token::COLON)	// ����������ð��
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		if(Token::SEMICOLON == token.type)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
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
	if(token.type != Token::SEMICOLON)	// �����п��������˷ֺţ����Բ��ټ�����
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  lost ';' at the end of function head\n";
		isSuccessful = false;
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
int SyntaxAnalyzer::parameterList(int depth)		// �βα�
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

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
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
		while(token.type != Token::NIL && token.type != Token::COMMA  && token.type != Token::SEMICOLON)	// ������β���ֺš�BEGIN
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
	// ������ѹջ׼�������ű�
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON  && token.type != Token::IDENTIFIER)	// ������β���ֺš�BEGIN
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::PROCEDURE && token.type != Token::FUNCTION && token.type != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return 0;
	}
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
	// ��������ű�
	TokenTableItem::DecorateType decorateType = (token.type == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenBuffer.begin(); iter != tokenBuffer.end(); ++iter)
	{
		if(tokenTable.SearchDefinitionInCurrentLevel(iter->value.identifier))
		{
			std::cout << "line " << iter->lineNumber << ":  " << iter->toString() << "  redifinition\n";
			isSuccessful = false;
		}
		tokenTable.AddParameterItem(*iter, decorateType, level);
	} // end ����ű�

	lexicalAnalyzer.GetNextToken(token);
	return sum;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
vector<TokenTableItem::DecorateType> SyntaxAnalyzer::argumentList(int depth)			// ʵ�α�
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

// <�������> ::= begin <���>{;<���>} end
void SyntaxAnalyzer::statementBlockPart(int depth)	// �������
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>|<�����>|<д���>|<forѭ�����>|<��>
void SyntaxAnalyzer::statement(int depth)
{
	PrintFunctionFrame("statement()", depth);

	Token idToken = token;	// ��token�����ǹ��������ȼ��£�����
	TokenTable::iterator iter;
	TokenTableItem::DecorateType l_value_type = TokenTableItem::VOID;
	switch(token.type)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		iter = tokenTable.SearchDefinition(token);	// ���ҷ��ű��еĶ���
		if(iter == tokenTable.end())
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		iter->AddUsedLine(token.lineNumber);		// �ڷ��ű��в��������м�¼
		lexicalAnalyzer.GetNextToken(token);
		if(Token::LEFT_PAREN == token.type)	// ���̵���
		{
			if(iter->GetItemType() != TokenTableItem::PROCEDURE)	// ����Ƿ�Ϊ����
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not declared as a procedure\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		else if(Token::COLON == token.type)	// �˷�֧ר��Ϊ�˼�鸳ֵ��д������
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  please check the spelling of the assigning operator\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		else
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  syntax error after identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		break;
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
void SyntaxAnalyzer::assigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth)			// ��ֵ���
{
	PrintFunctionFrame("assigningStatement()", depth);

	if(Token::LEFT_BRACKET == token.type)	// ������Ԫ�ظ�ֵ
	{
		if(iter->GetItemType() != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ������
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  subscript requires array or pointer type\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		&& iter->GetItemType() != TokenTableItem::FUNCTION)	// ����Ƿ�Ϊ���������������
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	// ��������Ϊ�׳��쳣��׼��������GetNextTokenǰ��Ϊ��׼ȷ��¼��ֵ�ŵ��к�
	static Token errToken;
	errToken.type = Token::ASSIGN;
	errToken.lineNumber = token.lineNumber;

	lexicalAnalyzer.GetNextToken(token);
	TokenTableItem::DecorateType expression_type = expression(depth + 1);
	if(TokenTableItem::CHAR == iter->GetDecorateType() && TokenTableItem::INTEGER == expression_type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert from 'int' to 'char'\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
TokenTableItem::DecorateType SyntaxAnalyzer::expression(int depth)				// ���ʽ
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
		// ���������ת������ܼ򵥣�ֻ���������Ͷ�ΪCHAR���������Ͳ���CHAR
		// �������INTEGER
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

// <��> ::= <����>{<�˷������><����>}
TokenTableItem::DecorateType SyntaxAnalyzer::term(int depth)						// ��
{
	PrintFunctionFrame("term()", depth);

	TokenTableItem::DecorateType decorate_type = factor(depth + 1);
	while(	token.type == Token::MUL
		||	token.type == Token::DIV)
	{
		lexicalAnalyzer.GetNextToken(token);
		TokenTableItem::DecorateType local_decorate_type = factor(depth + 1);
		// ���������ת������ܼ򵥣�ֻ���������Ͷ�ΪCHAR���������Ͳ���CHAR
		// �������INTEGER
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

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>]) | '('<���ʽ>')' | <�޷�������> | <�ַ�>
TokenTableItem::DecorateType SyntaxAnalyzer::factor(int depth)					// ����
{
	PrintFunctionFrame("factor()", depth);
	TokenTableItem::DecorateType decorate_type = TokenTableItem::VOID;	// ���ӵ�����
	if(Token::IDENTIFIER == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		//decorate_type = tokenTable.AddUsedLine(token);	// ���뵽���ű�
		TokenTable::iterator iter = tokenTable.SearchDefinition(token);	// Ѱ�Ҷ���
		if(iter == tokenTable.end())
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  undeclared identifier\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return decorate_type;
		}
		decorate_type = iter->GetDecorateType();
		iter->AddUsedLine(token.lineNumber);		// �ڷ��ű��в��������м�¼
		Token idToken = token;	// ���£�����
		lexicalAnalyzer.GetNextToken(token);
		if(Token::LEFT_BRACKET == token.type)	// ������Ԫ�صĸ�ֵ
		{
			if(iter->GetItemType() != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ������
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  subscript requires array or pointer type\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
		}
		else if(Token::LEFT_PAREN == token.type)
		{
			// ����Ƿ�Ϊ����
			if(iter->GetItemType() != TokenTableItem::FUNCTION)	// ����Ƿ�Ϊ����
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not declared as a function\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
			// �ӷ��ű���ȡ�������Ĳ������ͣ���functionCallStatementȥƥ�����
			vector<TokenTableItem::DecorateType> decorate_types = tokenTable.GetProcFuncParameter(iter);
			functionCallStatement(idToken, decorate_types, depth + 1);
		}
		else
		{
			// ����Ƿ�Ϊ���������������/�����Ĳ���
			if(iter->GetItemType() != TokenTableItem::VARIABLE && iter->GetItemType() != TokenTableItem::PARAMETER && iter->GetItemType() != TokenTableItem::CONST)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  single token factor should be varaible or constant\n";
				isSuccessful = false;
				while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
				{ 
					lexicalAnalyzer.GetNextToken(token);
				}
				return decorate_type;
			}
		}
	}
	else if(Token::LEFT_PAREN == token.type)	// �����������ı��ʽ
	{
		decorate_type = expression(depth + 1);	// ��¼����
		if(token.type != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match '('\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		decorate_type = TokenTableItem::INTEGER;	// ��¼����
		lexicalAnalyzer.GetNextToken(token);
	}
	else if(Token::CONST_CHAR == token.type)
	{
#ifdef SYNTAXDEBUG
	output << buffer << "  " << token.toString() << std::endl;
#endif
		decorate_type = TokenTableItem::CHAR;	// ��¼����
		lexicalAnalyzer.GetNextToken(token);
	}
	else
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  not a legal factor\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return decorate_type;
	}
	return decorate_type;
}

// <�������> ::= if<����>then<���>[else<���>]
void SyntaxAnalyzer::ifStatement(int depth)				// �������
{
	PrintFunctionFrame("ifStatement()", depth);

	assert(Token::IF == token.type);
	lexicalAnalyzer.GetNextToken(token);
	condition(depth + 1);
	if(Token::THEN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"then\" after condition\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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

// <����> ::= <���ʽ><��ϵ�����><���ʽ>
void SyntaxAnalyzer::condition(int depth)				// ����
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SyntaxAnalyzer::caseStatement(int depth)			// ������
{
	PrintFunctionFrame("caseStatement()", depth);

	assert(Token::CASE == token.type);
	lexicalAnalyzer.GetNextToken(token);
	expression(depth + 1);
	if(Token::OF != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be \"of\" to specify the certain case\n";
		isSuccessful = false;
		// �������������дof���ʲ�����
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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

// <�����Ԫ��> ::= <���������>:<���>
void SyntaxAnalyzer::caseList(int depth)					// �����Ԫ��
{
	PrintFunctionFrame("caseList()", depth);

	if(Token::CONST_INTEGER != token.type
		&& Token::CONST_CHAR != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be constant integer or character after \"case\"\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺ�
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	statement(depth + 1);
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
void SyntaxAnalyzer::readStatement(int depth)			// �����
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
			{ 
				lexicalAnalyzer.GetNextToken(token);
			}
			return;
		}
		iter->AddUsedLine(token.lineNumber);		// �ڷ��ű��в��������м�¼
		if(iter->GetItemType() != TokenTableItem::VARIABLE
		&& iter->GetItemType() != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������������
		{
			std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned in read call\n";
			isSuccessful = false;
			while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void SyntaxAnalyzer::writeStatement(int depth)			// д���
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
}

// <forѭ�����> ::= for <��ʶ��>  := <���ʽ> ��downto | to�� <���ʽ> do <���>
void SyntaxAnalyzer::forLoopStatement(int depth)			// forѭ�����
{
	PrintFunctionFrame("forLoopStatement()", depth);

	assert(Token::FOR == token.type);

	lexicalAnalyzer.GetNextToken(token);
	if(Token::IDENTIFIER != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be loop variable name after for\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	iter->AddUsedLine(token.lineNumber);		// �ڷ��ű��в��������м�¼
	if(iter->GetItemType() != TokenTableItem::VARIABLE
	&& iter->GetItemType() != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������������
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot be assigned in for loop\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);
	statement(depth + 1);
}

// <���̵������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::procedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// ���̵������
{
	PrintFunctionFrame("procedureCallStatement()", depth);

	if(Token::LEFT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be '(' to specify the argument\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);

	// �����̲�������������Ƿ�ƥ��
	if(parameter_decorate_types.size() != argument_decorate_types.size())	// ����������
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// �������������ֻ��ΪCHAR��INTEGER
		assert(argument_decorate_types[i] != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != argument_decorate_types[i])
		{
			// Ҫ�����ΪCHAR��ʵ�ʲ���ΪINTEGER�����޷�ת��
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && argument_decorate_types[i] == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				isSuccessful = false;
				// ��������Ǽ���˲������Ͳ�ƥ�䣬ʵ���﷨�ɷַ������ɼ�����
			}
		}
	}
}

// <�����������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::functionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// �����������
{
	PrintFunctionFrame("functionCallStatement()", depth);

	assert(Token::LEFT_PAREN == token.type);

	lexicalAnalyzer.GetNextToken(token);
	vector<TokenTableItem::DecorateType> argument_decorate_types = argumentList(depth + 1);
	if(Token::RIGHT_PAREN != token.type)
	{
		std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  should be ')' to match the '('\n";
		isSuccessful = false;
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	lexicalAnalyzer.GetNextToken(token);

	// ��麯�������뺯�������Ƿ�ƥ��
	if(parameter_decorate_types.size() != argument_decorate_types.size())	// ����������
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
		while(token.type != Token::NIL && token.type != Token::SEMICOLON && token.type != Token::END)	// ������β��ֺŻ�END
		{ 
			lexicalAnalyzer.GetNextToken(token);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// �������������ֻ��ΪCHAR��INTEGER
		assert(argument_decorate_types[i] != TokenTableItem::VOID);
		if(parameter_decorate_types[i] != argument_decorate_types[i])
		{
			// Ҫ�����ΪCHAR��ʵ�ʲ���ΪINTEGER�����޷�ת��
			if(parameter_decorate_types[i] == TokenTableItem::CHAR && argument_decorate_types[i] == TokenTableItem::INTEGER)
			{
				std::cout << "line " << token.lineNumber << ":  " << token.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
				isSuccessful = false;
				// ��������Ǽ���˲������Ͳ�ƥ�䣬ʵ���﷨�ɷַ������ɼ������ʲ�����
			}
		}
	}
	
}