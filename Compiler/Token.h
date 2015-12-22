#ifndef TOKEN_H
#define TOKEN_H
#include <sstream>
#include <string>
#include <map>
using std::string;
using std::map;

// 单词类
class Token
{
public:
	// 单词类的类型的枚举
	enum TokenType{
		/*Special Type*/		NIL = 0, COMMENT,
		/*Reserved Words(20)*/	CONST, VAR, ARRAY, RW_INTEGER, RW_CHAR, PROCEDURE, FUNCTION, IF, THEN, ELSE, CASE, OF, FOR, DOWNTO, TO, DO, BEGIN, END, READ, WRITE, 
		/*Identifier(1)*/		IDENTIFIER,
		/*Constant(2)*/			CONST_INTEGER, CONST_CHAR, CONST_STRING,
		/*Operator(21)*/		PERIOD, COMMA, SEMICOLON, SINGLE_QUOTATION, DOUBLE_QUOTATION, COLON, ASSIGN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_PAREN, RIGHT_PAREN, PLUS, MINUS, MUL, DIV, LT, LEQ, GT, GEQ, EQU, NEQ
	};
	// 单词类的值的共用体
	class TokenValue{
	public:
		TokenValue() : integer(0), character('\0'), identifier(){}
		int integer;
		char character;
		string identifier;
	};
	// 构造函数
	Token();
	// 格式化函数
	string toTableString() const;
	string toString() const;

	// 单词类型、值和出现的行号
	TokenType type;
	TokenValue value;
	int lineNumber;

	static map<TokenType, string> tokenTypeToString;		// 将enum<TokenType>类型映射到string(用于输出token)
	static map<string, TokenType> reserveWordToTokenType;// 将属于reserve word的string映射到TokenType(用于识别reserve word)
	static map<TokenType, string> InitTokenTypeToStringMap();		// 初始化tokenTypeToString
	static map<string, TokenType> InitReserveWordToTokenTypeMap();	// 初始化reserveWordToTokenType
};

#endif