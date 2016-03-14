#ifndef TOKEN_H
#define TOKEN_H
#include <sstream>
#include <string>
#include <map>
using std::string;
using std::map;

// ������
class Token
{
public:
	// ����������͵�ö��
	enum TokenType{
		/*Special Type*/		NIL = 0, COMMENT,
		/*Reserved Words(23)*/	CONST, VAR, ARRAY, RW_INTEGER, RW_CHAR, PROCEDURE, FUNCTION, IF, THEN, ELSE, CASE, OF, WHILE, FOR, DOWNTO, TO, DO, CONTINUE, BREAK, BEGIN, END, READ, WRITE, 
		/*Identifier(1)*/		IDENTIFIER,
		/*Constant(2)*/			CONST_INTEGER, CONST_CHAR, CONST_STRING,
		/*Operator(23)*/		PERIOD, COMMA, SEMICOLON, SINGLE_QUOTATION, DOUBLE_QUOTATION, COLON, ASSIGN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_PAREN, RIGHT_PAREN, PLUS, MINUS, MUL, DIV, LOGICNOT, LOGICOR, LOGICAND, LT, LEQ, GT, GEQ, EQU, NEQ
	};
	// ������
	struct TokenValue{
		int integer;
		char character;
		string identifier;
	};
	// ���캯��
	Token() throw();
	// ��ʽ������
	string toTableString() const throw();
	string toString() const throw();

	// �������͡�ֵ�ͳ��ֵ��к�
	TokenType type_;
	TokenValue value_;
	int lineNumber_;

	static map<TokenType, string> sTokenTypeToString;						// ��enum<TokenType>����ӳ�䵽string(�������token)
	static map<string, TokenType> sReserveWordToTokenType;					// ������reserve word��stringӳ�䵽TokenType(����ʶ��reserve word)
	static map<TokenType, string> InitTokenTypeToStringMap() throw();		// ��ʼ��sTokenTypeToString
	static map<string, TokenType> InitReserveWordToTokenTypeMap() throw();	// ��ʼ��sReserveWordToTokenType
};

#endif