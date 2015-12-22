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
		/*Reserved Words(20)*/	CONST, VAR, ARRAY, RW_INTEGER, RW_CHAR, PROCEDURE, FUNCTION, IF, THEN, ELSE, CASE, OF, FOR, DOWNTO, TO, DO, BEGIN, END, READ, WRITE, 
		/*Identifier(1)*/		IDENTIFIER,
		/*Constant(2)*/			CONST_INTEGER, CONST_CHAR, CONST_STRING,
		/*Operator(21)*/		PERIOD, COMMA, SEMICOLON, SINGLE_QUOTATION, DOUBLE_QUOTATION, COLON, ASSIGN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_PAREN, RIGHT_PAREN, PLUS, MINUS, MUL, DIV, LT, LEQ, GT, GEQ, EQU, NEQ
	};
	// �������ֵ�Ĺ�����
	class TokenValue{
	public:
		TokenValue() : integer(0), character('\0'), identifier(){}
		int integer;
		char character;
		string identifier;
	};
	// ���캯��
	Token();
	// ��ʽ������
	string toTableString() const;
	string toString() const;

	// �������͡�ֵ�ͳ��ֵ��к�
	TokenType type;
	TokenValue value;
	int lineNumber;

	static map<TokenType, string> tokenTypeToString;		// ��enum<TokenType>����ӳ�䵽string(�������token)
	static map<string, TokenType> reserveWordToTokenType;// ������reserve word��stringӳ�䵽TokenType(����ʶ��reserve word)
	static map<TokenType, string> InitTokenTypeToStringMap();		// ��ʼ��tokenTypeToString
	static map<string, TokenType> InitReserveWordToTokenTypeMap();	// ��ʼ��reserveWordToTokenType
};

#endif