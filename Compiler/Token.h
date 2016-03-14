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
		/*Reserved Words(23)*/	CONST, VAR, ARRAY, RW_INTEGER, RW_CHAR, PROCEDURE, FUNCTION, IF, THEN, ELSE, CASE, OF, WHILE, FOR, DOWNTO, TO, DO, CONTINUE, BREAK, BEGIN, END, READ, WRITE, 
		/*Identifier(1)*/		IDENTIFIER,
		/*Constant(2)*/			CONST_INTEGER, CONST_CHAR, CONST_STRING,
		/*Operator(23)*/		PERIOD, COMMA, SEMICOLON, SINGLE_QUOTATION, DOUBLE_QUOTATION, COLON, ASSIGN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_PAREN, RIGHT_PAREN, PLUS, MINUS, MUL, DIV, LOGICNOT, LOGICOR, LOGICAND, LT, LEQ, GT, GEQ, EQU, NEQ
	};
	// 单词类
	struct TokenValue{
		int integer;
		char character;
		string identifier;
	};
	// 构造函数
	Token() throw();
	// 格式化函数
	string toTableString() const throw();
	string toString() const throw();

	// 单词类型、值和出现的行号
	TokenType type_;
	TokenValue value_;
	int lineNumber_;

	static map<TokenType, string> sTokenTypeToString;						// 将enum<TokenType>类型映射到string(用于输出token)
	static map<string, TokenType> sReserveWordToTokenType;					// 将属于reserve word的string映射到TokenType(用于识别reserve word)
	static map<TokenType, string> InitTokenTypeToStringMap() throw();		// 初始化sTokenTypeToString
	static map<string, TokenType> InitReserveWordToTokenTypeMap() throw();	// 初始化sReserveWordToTokenType
};

#endif