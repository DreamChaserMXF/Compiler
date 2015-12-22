#include "Token.h"

// 构造函数
Token::Token(): type(Token::NIL), value()
{}

// 格式化函数
string Token::toTableString() const
{
	std::ostringstream buf;
	//buf << lineNumber << '\t';
	buf.width(16);
	buf << std::left << tokenTypeToString[type] << "    ";
	if(type == Token::IDENTIFIER)
	{
		buf << value.identifier;
	}
	else if(type == Token::CONST_INTEGER)
	{
		buf << value.integer;
	}
	else if(type == Token::CONST_CHAR)
	{
		buf << value.character;
	}
	else if(type == Token::CONST_STRING)
	{
		buf << value.identifier;
	}
	return buf.str();
}

// 格式化函数
string Token::toString() const
{
	std::ostringstream buf;
	buf << tokenTypeToString[type];
	if(type == Token::IDENTIFIER)
	{
		buf << "  " << value.identifier;
	}
	else if(type == Token::CONST_INTEGER)
	{
		buf << "  " << value.integer;
	}
	else if(type == Token::CONST_CHAR)
	{
		buf << "  " << value.character;
	}
	else if(type == Token::CONST_STRING)
	{
		buf << "  " << value.identifier;
	}
	return buf.str();
}

// 用于查找enum值对应的字符串
map<Token::TokenType, string> Token::tokenTypeToString = InitTokenTypeToStringMap(); // tokenTypeToString必须要在类外初始化
map<Token::TokenType, string> Token::InitTokenTypeToStringMap()
{
	map<TokenType, string> tsMap;
	tsMap.insert(map<TokenType,string>::value_type(NIL,					"NIL"					));
	tsMap.insert(map<TokenType,string>::value_type(CONST,				"CONST"					));
	tsMap.insert(map<TokenType,string>::value_type(VAR,					"VAR"					));
	tsMap.insert(map<TokenType,string>::value_type(ARRAY,				"ARRAY"					));
	tsMap.insert(map<TokenType,string>::value_type(RW_INTEGER,			"RW_INTEGER"			));
	tsMap.insert(map<TokenType,string>::value_type(RW_CHAR,				"RW_CHAR"				));
	tsMap.insert(map<TokenType,string>::value_type(PROCEDURE,			"PROCEDURE"				));
	tsMap.insert(map<TokenType,string>::value_type(FUNCTION,			"FUNCTION"				));
	tsMap.insert(map<TokenType,string>::value_type(IF,					"IF"					));
	tsMap.insert(map<TokenType,string>::value_type(THEN,				"THEN"					));
	tsMap.insert(map<TokenType,string>::value_type(ELSE,				"ELSE"					));
	tsMap.insert(map<TokenType,string>::value_type(CASE,				"CASE"					));
	tsMap.insert(map<TokenType,string>::value_type(OF,					"OF"					));
	tsMap.insert(map<TokenType,string>::value_type(FOR,					"FOR"					));
	tsMap.insert(map<TokenType,string>::value_type(DOWNTO,				"DOWNTO"				));
	tsMap.insert(map<TokenType,string>::value_type(TO,					"TO"					));
	tsMap.insert(map<TokenType,string>::value_type(DO,					"DO"					));
	tsMap.insert(map<TokenType,string>::value_type(BEGIN,				"BEGIN"					));
	tsMap.insert(map<TokenType,string>::value_type(END,					"END"					));
	tsMap.insert(map<TokenType,string>::value_type(READ,				"READ"					));
	tsMap.insert(map<TokenType,string>::value_type(WRITE,				"WRITE"					));
	tsMap.insert(map<TokenType,string>::value_type(IDENTIFIER,			"IDENTIFIER"			));
	tsMap.insert(map<TokenType,string>::value_type(CONST_INTEGER,		"CONST_INTEGER"			));
	tsMap.insert(map<TokenType,string>::value_type(CONST_CHAR,			"CONST_CHAR"			));
	tsMap.insert(map<TokenType,string>::value_type(CONST_STRING,		"CONST_STRING"			));
	tsMap.insert(map<TokenType,string>::value_type(PERIOD,				"PERIOD"				));
	tsMap.insert(map<TokenType,string>::value_type(COMMA,				"COMMA"					));
	tsMap.insert(map<TokenType,string>::value_type(SEMICOLON,			"SEMICOLON"				));
	tsMap.insert(map<TokenType,string>::value_type(SINGLE_QUOTATION,	"SINGLE_QUOTATION"		));
	tsMap.insert(map<TokenType,string>::value_type(DOUBLE_QUOTATION,	"DOUBLE_QUOTATION"		));
	tsMap.insert(map<TokenType,string>::value_type(COLON,				"COLON"					));
	tsMap.insert(map<TokenType,string>::value_type(ASSIGN,				"ASSIGN"				));
	tsMap.insert(map<TokenType,string>::value_type(LEFT_BRACKET,		"LEFT_BRACKET"			));
	tsMap.insert(map<TokenType,string>::value_type(RIGHT_BRACKET,		"RIGHT_BRACKET"			));
	tsMap.insert(map<TokenType,string>::value_type(LEFT_PAREN,			"LEFT_PAREN"			));
	tsMap.insert(map<TokenType,string>::value_type(RIGHT_PAREN,			"RIGHT_PAREN"			));
	tsMap.insert(map<TokenType,string>::value_type(PLUS,				"PLUS"					));
	tsMap.insert(map<TokenType,string>::value_type(MINUS,				"MINUS"					));
	tsMap.insert(map<TokenType,string>::value_type(MUL,					"MUL"					));
	tsMap.insert(map<TokenType,string>::value_type(DIV,					"DIV"					));
	tsMap.insert(map<TokenType,string>::value_type(LT,					"LT"					));
	tsMap.insert(map<TokenType,string>::value_type(LEQ,					"LEQ"					));
	tsMap.insert(map<TokenType,string>::value_type(GT,					"GT"					));
	tsMap.insert(map<TokenType,string>::value_type(GEQ,					"GEQ"					));
	tsMap.insert(map<TokenType,string>::value_type(EQU,					"EQU"					));
	tsMap.insert(map<TokenType,string>::value_type(NEQ,					"NEQ"					));
	return tsMap;
}
// 用于查找字符串是否有对应的保留字的enum值
map<string, Token::TokenType> Token::reserveWordToTokenType = InitReserveWordToTokenTypeMap();
map<string, Token::TokenType> Token::InitReserveWordToTokenTypeMap()
{
	map<string, TokenType> stMap;
	stMap.insert(map<string, TokenType>::value_type("const",		CONST		));	
	stMap.insert(map<string, TokenType>::value_type("var",			VAR			));	
	stMap.insert(map<string, TokenType>::value_type("array",		ARRAY		));	
	stMap.insert(map<string, TokenType>::value_type("integer",		RW_INTEGER	));	
	stMap.insert(map<string, TokenType>::value_type("char",			RW_CHAR		));	
	stMap.insert(map<string, TokenType>::value_type("procedure",	PROCEDURE	));	
	stMap.insert(map<string, TokenType>::value_type("function",		FUNCTION	));	
	stMap.insert(map<string, TokenType>::value_type("if",			IF			));	
	stMap.insert(map<string, TokenType>::value_type("then",			THEN		));	
	stMap.insert(map<string, TokenType>::value_type("else",			ELSE		));	
	stMap.insert(map<string, TokenType>::value_type("case",			CASE		));	
	stMap.insert(map<string, TokenType>::value_type("of",			OF			));	
	stMap.insert(map<string, TokenType>::value_type("for",			FOR			));	
	stMap.insert(map<string, TokenType>::value_type("downto",		DOWNTO		));	
	stMap.insert(map<string, TokenType>::value_type("to",			TO			));	
	stMap.insert(map<string, TokenType>::value_type("do",			DO			));	
	stMap.insert(map<string, TokenType>::value_type("begin",		BEGIN		));	
	stMap.insert(map<string, TokenType>::value_type("end",			END			));	
	stMap.insert(map<string, TokenType>::value_type("read",			READ		));	
	stMap.insert(map<string, TokenType>::value_type("write",		WRITE		));	
	return stMap;
}