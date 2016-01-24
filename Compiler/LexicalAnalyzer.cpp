#include "LexicalAnalyzer.h"
#include "LexException.h"
#include <sstream>
#include <fstream>

using std::ostringstream;


LexicalAnalyzer::LexicalAnalyzer(const string &srcFileName) : srcFile(srcFileName), currentLine(0), tokenVector(), tokenIter(), stringSet()
{
	//if(srcFile.is_open())
	//{
	//	srcFile.close();
	//	srcFile.clear();
	//}
	//srcFile.open(srcFileName);
}

// ���ݰ󶨵�Դ�����ļ������дʷ�����
// �����Ľ������tokenVector��
bool LexicalAnalyzer::Parse()									// ���дʷ�����
{
	bool isSuccessful = true;
	if(!srcFile.is_open())	// ����ļ��Ƿ��
	{
		return false;
	}
	
	// ���tokenջ
	tokenVector.clear();
	// ��ճ����ַ�����
	stringSet.clear();
	// ��ʼ��״̬ͼ���н���
	Token token;
	currentLine = 0;
	char ch = getNextChar(true);	
	while(ch != '\0')
	{
		// ���Կո�
		if(isspace(ch))
		{
			ch = getNextChar();
			continue;
		}
		try
		{
			parseCurrentToken(token, ch);			
		}
		catch(const LexException &ex)
		{
			isSuccessful = false;
			std::cout << ex.what() << std::endl;
			// ���дʷ�����ָ�
			do
			{
				ch = getNextChar();
			}
			while(ch != '\0' && !isspace(ch));
			token.type = Token::NIL;
			continue;
		}
		if(token.type != Token::COMMENT)	// �����ʲ���ע��
		{
			tokenVector.push_back(token);	// �򽫵��ʷ���vector
		}
	}
	ResetTokenPos();	// ��������е��ʺ����õ��ʵ������ĳ�ʼλ��
	return isSuccessful;
}


// ���ʷ������Ľ��������ļ�
void LexicalAnalyzer::Print(string fileName) const			
{
	std::ofstream outFile(fileName);
	if(!outFile)
	{
		std::cout << "Cannot open file " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}
	Print(outFile);
	outFile.close();
}
// ���ʷ������Ľ������������ļ������ַ��������̨�������
void LexicalAnalyzer::Print(ostream &output) const			
{
	int tokenNo = 1;
	output << "TokenNumber" << '\t' << "LineNumber" << '\t' << "TokenType" << "\t    " << "TokenValue" << std::endl;
	for(vector<Token>::const_iterator iter = tokenVector.begin(); iter != tokenVector.end(); ++iter)
	{
		//output << tokenNo++ << "\t\t" << iter->lineNumber << "\t\t" << iter->toTableString() << std::endl;
		output << tokenNo++ << "\t\t" << iter->lineNumber << "\t\t";
		output.width(16);
		output << std::left << Token::tokenTypeToString[iter->type] << "    ";
		// ֻ�ڱ�Ҫʱ���
		//if(iter->type == Token::IDENTIFIER)
		//{
		//	output << iter->value.identifier;
		//}
		//else if(iter->type == Token::CONST_INTEGER)
		//{
		//	output << iter->value.integer;
		//}
		//else if(iter->type == Token::CONST_CHAR)
		//{
		//	output << iter->value.character;
		//}
		//else if(iter->type == Token::CONST_STRING)
		//{
		//	output << iter->value.identifier;
		//}
		// ����Token��Ҫ���
		if(iter->type == Token::CONST_INTEGER)
		{
			output << iter->value.integer;
		}
		else if(iter->type == Token::CONST_CHAR)
		{
			output << iter->value.character;
		}
		else
		{
			output << iter->value.identifier;
		}

		output << std::endl;
	}
}

void LexicalAnalyzer::ResetTokenPos()					// ���÷���λ��
{
	tokenIter = tokenVector.begin();
}
bool LexicalAnalyzer::GetNextToken(Token &token)		// ��ȡ��һ����
{
	if(tokenIter != tokenVector.end())
	{
		token = *(tokenIter++);
		return true;
	}
	else
	{
		token.type = Token::NIL;
		return false;
	}
}

vector<string> LexicalAnalyzer::getStringTable()
{
	return vector<string>(stringSet.begin(), stringSet.end());
}

// ��������ʱ�ú���û�в�������Ϊ�ǰ���skipSpace=false�������
// ���ǵ�+/-��֮�����һ������֮������пո��������ʼ��ϲ���skipSpace�Դ���������
// ����ʱ�ɲ�д������Ĭ��Ϊfalse
char LexicalAnalyzer::getNextChar(bool skipSpace)				// �õ���һ���ַ�
{
	static string line;
	static string::const_iterator iter = line.end();
	
	if(skipSpace)	// �������ַ��Ķ���
	{
		// ����������β�����ȡ�µ�һ�У�ֱ�������ǿ��л��ļ�����
		while(iter == line.end() || isspace(*iter))
		{
			while(iter == line.end())// ֱ���������µķǿ��У����߶����ļ�β��������ѭ��
			{
				if(getline(srcFile, line))	// ��ȡ�ɹ�
				{
					++currentLine;
					iter = line.begin();
				}
				else						// �����ļ�β
				{
					return '\0';
				}
			}
			if(isspace(*iter))
			{
				++iter;
			}
		}
		return *(iter++);
	}
	else	// ���������ַ��Ķ���
	{
		if(iter == line.end())
		{
			if(getline(srcFile, line))	// ��ȡ�ɹ�
			{
				++currentLine;
				iter = line.begin();
				return '\n';
			}
			else						// �����ļ�β
			{
				return '\0';
			}
		}
		return *(iter++);
	}
}


void LexicalAnalyzer::parseCurrentToken(Token &token, char &ch)
{
	// ���ж����ֲ������ַ������
	if(isalpha(ch))		// �����ֻ��ʶ��			
	{
		letterHandle(token, ch);
	}
	else if(isdigit(ch))// �޷������ͳ���
	{
		digitHandle(token, ch);
	}
	else if('"' == ch)	// �ַ�������
	{
		stringHandle(token, ch);
	}
	// һ�����ַ������
	else if('\'' == ch)	// �ַ�����
	{
		charHandle(token, ch);
	}
	// ���жϼ��ֿ���Ϊ˫�ַ������
	else if(':' == ch)
	{
		colonHandle(token, ch);
	}
	else if('<' == ch)
	{
		ltHandle(token, ch);
	}
	else if('>' == ch)
	{
		gtHandle(token, ch);
	}
	else if('+' == ch)
	{
		plusHandle(token, ch);
	}
	else if('-' == ch)
	{
		minusHandle(token, ch);
	}
	else if('/' == ch)	// ����ע��
	{
		token.value.identifier.clear();
		token.value.identifier.push_back(ch);
		ch = getNextChar();
		if('/' == ch)
		{
			singleLineCommentHandle(token, ch);
		}
		else if('*' == ch)
		{
			blockCommentHandle(token, ch);
		}
		else
		{
			throw LexException("unkown character", ch, currentLine);
		}
	}
	// ʣ���ǵ��ַ������
	else
	{
		token.value.identifier.clear();
		token.value.identifier.push_back(ch);
		switch(ch)
		{
		case '.':
			token.type = Token::PERIOD;
			break;
		case ',':
			token.type = Token::COMMA;
			break;
		case ';':
			token.type = Token::SEMICOLON;
			break;
		case '[':
			token.type = Token::LEFT_BRACKET;
			break;
		case ']':
			token.type = Token::RIGHT_BRACKET;
			break;
		case '(':
			token.type = Token::LEFT_PAREN;
			break;
		case ')':
			token.type = Token::RIGHT_PAREN;
			break;
		//case '+':
		//	token.type = Token::PLUS;
		//	break;
		//case '-':
		//	token.type = Token::MINUS;
		//	break;
		case '*':
			token.type = Token::MUL;
			break;
		case '/':
			token.type = Token::DIV;
			break;
		case '=':
			token.type = Token::EQU;
			break;
		default:
			throw LexException("unkown character", ch, currentLine);
			/*ostringstream errMsg;
			errMsg << "line " << currentLine << ": unkown character " << ch;
			throw LexExceptionDEPRECATED(errMsg.str().c_str());*/
			break;
		}
		// ������һ������
		ch = getNextChar(true);
	}
}

void LexicalAnalyzer::singleLineCommentHandle(Token &token, char &ch)			// ����ע��
{
	do
	{
		token.value.identifier.push_back(ch);
		ch = getNextChar();
	}
	while(ch != '\0' && ch != '\n');
	token.type = Token::COMMENT;
	ch = getNextChar();	// �����п��ޣ���Ϊ'\0'��'\n'��������Parse�б����ƴ��������������Ч�ʻ��һЩ
}
void LexicalAnalyzer::blockCommentHandle(Token &token, char &ch)			// ����ע��
{
	token.value.identifier.push_back(ch);
	char lastCh = '\0';
	ch = getNextChar();
	while(ch != '\0' && (lastCh != '*' || ch != '/'))
	{
		token.value.identifier.push_back(ch);
		lastCh = ch;
		ch = getNextChar();
	}
	if('\0' == ch)
	{
		throw LexException("unexpected end of file found in comment", ch, currentLine);
	}
	token.value.identifier.push_back(ch);
	token.type = Token::COMMENT;
	ch = getNextChar();	// ���һ��Ҫ�У�ʹch��Ϊע�ͺ�ĵ�һ���ַ�
}
void LexicalAnalyzer::digitHandle(Token &token, char &ch)					// �����޷�����������
{
	int n = static_cast<int>(ch - '0');
	int last_n = 0;
	while(isdigit(ch = getNextChar()))	// ������ƴ�ӳ���
	{
		last_n = n;
		n = n * 10 + static_cast<int>(ch - '0');
		if(n < last_n)	// ����̫��
		{
			throw LexException("truncation of constant value", ch, currentLine);
		}
	}
	token.type = Token::CONST_INTEGER;
	token.value.integer = n;
}
void LexicalAnalyzer::letterHandle(Token &token, char &ch)					// ������ĸ
{
	token.lineNumber = currentLine;
	token.value.identifier.clear();
	do	// ����ĸƴ�ӳ��ַ���
	{
		token.value.identifier.push_back(ch);
		ch = getNextChar();
	}while(isalpha(ch) || isdigit(ch));
	// �ڱ����ֱ��в��ң��Ƿ�Ϊ������
	map<string, Token::TokenType>::const_iterator iter = Token::reserveWordToTokenType.find(token.value.identifier);
	if(iter != Token::reserveWordToTokenType.end())	// �ҵ��ˣ�˵���Ǳ�����
	{
		token.type = iter->second;
	}
	else												// û�ҵ���˵�����û��Զ���ı�ʶ��
	{
		token.type = Token::IDENTIFIER;
	}
}
void LexicalAnalyzer::stringHandle(Token &token, char &ch)					// �����ַ�������
{
	token.lineNumber = currentLine;
	ch = getNextChar();
	token.value.identifier.clear();
	while(isprint(ch) && ch != '"')
	{
		if('\\' == ch)
		{
			ch = getNextChar();
		}
		token.value.identifier.push_back(ch);
		ch = getNextChar();
	}
	if('"' == ch)	// �жϽ��������ǲ���˫����
	{
		token.type = Token::CONST_STRING;
		ch = getNextChar();
		// ���ַ������볣���ַ�����
		stringSet.insert(token.value.identifier);
	}
	else
	{
		throw LexException("wrong string constant definition: require \" as an end sign", ch, currentLine);
	}
}
void LexicalAnalyzer::charHandle(Token &token, char &ch)						// �����ַ�����
{
	token.lineNumber = currentLine;
	ch = getNextChar();	// ��ȡ�����ź�����Ǹ��ַ�
	if(!isalpha(ch) && !isdigit(ch))	// �ж��ǲ�����ĸ��������
	{
		throw LexException("wrong character constant definition: require letter after \"'\"", ch, currentLine);
	}
	else	// ��ȡ�����ĵ�����
	{
		token.value.character = ch;
		ch = getNextChar();
		if('\'' == ch)	// �ɹ���ȡ��������
		{
			token.type = Token::CONST_CHAR;
			ch = getNextChar();
		}
		else	// ������ǵ�����
		{
			throw LexException("wrong character constant definition: require \"'\"", ch, currentLine);
		}
	}
}

void LexicalAnalyzer::colonHandle(Token &token, char &ch)					// ����ð��
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar();// ��ȡð�ŵ���һ������
	if('=' == ch)	// ���ǵȺţ�����ǰһ��ð��һ�𹹳�һ����ֵ����
	{
		token.value.identifier.push_back(ch);
		token.type = Token::ASSIGN;
		ch = getNextChar();
	}
	else			// ���ǵȺţ���ǰ���ð�ž�ֻ�ǵ�����ð��
	{
		token.type = Token::COLON;
	}
}
void LexicalAnalyzer::ltHandle(Token &token, char &ch)						// ����С�ں�
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar();
	if('=' == ch)
	{
		token.value.identifier.push_back(ch);
		token.type = Token::LEQ;
		ch = getNextChar();
	}
	else if('>' == ch)
	{
		token.value.identifier.push_back(ch);
		ch = getNextChar();
		token.type = Token::NEQ;
	}
	else
	{
		token.type = Token::LT;
	}
}
void LexicalAnalyzer::gtHandle(Token &token, char &ch)						// ������ں�
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar();
	if('=' == ch)
	{
		token.value.identifier.push_back(ch);
		token.type = Token::GEQ;
		ch = getNextChar();
	}
	else
	{
		token.type = Token::GT;
	}
}

// ע��ch = getNextChar(true)һ��Ҫ��true��getNextChar�Ĳ���
// �����ֻ�ܴ���+����泣�����ڵ����
// ����Ҫ��֤���øú���ʱ��token.type���һ������һ��token������
void LexicalAnalyzer::plusHandle(Token &token, char &ch)
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar(true);	// ȡ����һ����Ч���ַ�(����һ��һ��Ҫ��true���������Դ���+֮���пո�����)
	// ǰ���ǵȺŻ�ֵ�ŵ�����������������֣��򰴵�����������������
	if((token.type == Token::EQU || token.type == Token::ASSIGN) && isdigit(ch))
	{
		digitHandle(token, ch);
	}
	else	// ��˫�������ļ���������
	{
		token.type = Token::PLUS;
	}
}
// ע��ch = getNextChar(true)һ��Ҫ��true��getNextChar�Ĳ���
// �����ֻ�ܴ���-����泣�����ڵ����
void LexicalAnalyzer::minusHandle(Token &token, char &ch)
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar(true);	// ȡ����һ����Ч���ַ�(����һ��һ��Ҫ��true���������Դ���-֮���пո�����)
	// ǰ���ǵȺŻ�ֵ�ŵ�����������������֣��򰴵�����������������
	if((token.type == Token::EQU || token.type == Token::ASSIGN) && isdigit(ch))
	{
		digitHandle(token, ch);
		token.value.integer  = -token.value.integer;
	}
	else	// ��˫�������ļ���������
	{
		token.type = Token::MINUS;
	}
}
