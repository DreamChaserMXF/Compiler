#include "LexicalAnalyzer.h"
#include "LexException.h"
#include <sstream>
#include <fstream>

using std::ostringstream;


LexicalAnalyzer::LexicalAnalyzer(const string &srcFileName) throw() : srcfile_(srcFileName), currentline_(0), token_vector_(), token_iter_(), string_set()
{}

bool LexicalAnalyzer::IsBound() const throw()
{
	return srcfile_.is_open();
}

// ���ݰ󶨵�Դ�����ļ������дʷ�����
// �����Ľ������token_vector_��
bool LexicalAnalyzer::Parse() throw()							// ���дʷ�����
{
	bool isSuccessful = true;
	//if(!srcfile_.is_open())	// ����ļ��Ƿ��
	//{
	//	return false;
	//}
	
	// ���tokenջ
	token_vector_.clear();
	// ��ճ����ַ�����
	string_set.clear();
	// ��ʼ���ʷ�״̬ͼ���н���
	Token token;
	currentline_ = 0;
	char ch = getNextChar();	
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
			ParseCurrentToken(token, ch);			
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
			token.type_ = Token::NIL;
			continue;
		}
		if(token.type_ != Token::COMMENT)	// �����ʲ���ע��
		{
			token_vector_.push_back(token);	// �򽫵��ʷ���vector
		}
	}
	ResetTokenPos();	// ��������е��ʺ����õ��ʵ������ĳ�ʼλ��
	return isSuccessful;
}


// ���ʷ������Ľ��������ļ�
bool LexicalAnalyzer::Print(string fileName) const throw()		
{
	std::ofstream outFile(fileName);
	if(!outFile)
	{
		std::cout << "Cannot open file " << fileName << std::endl;
		return false;
	}
	Print(outFile);
	outFile.close();
	return true;
}
// ���ʷ������Ľ������������ļ������ַ��������̨�������
void LexicalAnalyzer::Print(ostream &output) const throw()		
{
	int tokenNo = 1;
	output << "TokenNumber" << '\t' << "LineNumber" << '\t' << "TokenType" << "\t    " << "TokenValue" << std::endl;
	for(vector<Token>::const_iterator iter = token_vector_.begin(); iter != token_vector_.end(); ++iter)
	{
		output << tokenNo++ << "\t\t" << iter->lineNumber_ << "\t\t";
		output.width(16);
		output << std::left << Token::sTokenTypeToString[iter->type_] << "    ";
		// ֻ�ڱ�Ҫʱ���
		//if(iter->type_ == Token::IDENTIFIER)
		//{
		//	output << iter->value_.identifier;
		//}
		//else if(iter->type_ == Token::CONST_INTEGER)
		//{
		//	output << iter->value_.integer;
		//}
		//else if(iter->type_ == Token::CONST_CHAR)
		//{
		//	output << iter->value_.character;
		//}
		//else if(iter->type_ == Token::CONST_STRING)
		//{
		//	output << iter->value_.identifier;
		//}
		// ����Token��Ҫ���
		if(iter->type_ == Token::CONST_INTEGER)
		{
			output << iter->value_.integer;
		}
		else if(iter->type_ == Token::CONST_CHAR)
		{
			output << iter->value_.character;
		}
		else
		{
			output << iter->value_.identifier;
		}

		output << std::endl;
	}
}

void LexicalAnalyzer::ResetTokenPos() throw()				// ���÷���λ��
{
	token_iter_ = token_vector_.begin();
}
bool LexicalAnalyzer::GetNextToken(Token &token) throw()	// ��ȡ��һ����
{
	if(token_iter_ != token_vector_.end())
	{
		token = *(token_iter_++);
		return true;
	}
	else
	{
		token.type_ = Token::NIL;
		return false;
	}
}

vector<string> LexicalAnalyzer::getStringTable() const throw()
{
	return vector<string>(string_set.begin(), string_set.end());
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
				if(getline(srcfile_, line))	// ��ȡ�ɹ�
				{
					++currentline_;
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
			if(getline(srcfile_, line))	// ��ȡ�ɹ�
			{
				++currentline_;
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


void LexicalAnalyzer::ParseCurrentToken(Token &token, char &ch) throw(LexException)
{
	// ���ж����ֲ������ַ������
	if(isalpha(ch))		// �����ֻ��ʶ��			
	{
		LetterHandle(token, ch);
	}
	else if(isdigit(ch))// �޷������ͳ���
	{
		DigitHandle(token, ch);
	}
	else if('"' == ch)	// �ַ�������
	{
		StringHandle(token, ch);
	}
	// һ�����ַ������
	else if('\'' == ch)	// �ַ�����
	{
		CharHandle(token, ch);
	}
	// ����Ϊ���ַ���˫�ַ������
	else if(':' == ch)
	{
		ColonHandle(token, ch);
	}
	else if('<' == ch)
	{
		LessthanHandle(token, ch);
	}
	else if('>' == ch)
	{
		GreaterthanHandle(token, ch);
	}
	// ���ܵ��ַ�����ַ������
	else if('+' == ch)
	{
		PlusHandle(token, ch);
	}
	else if('-' == ch)
	{
		MinusHandle(token, ch);
	}
	else if('/' == ch)	// ����ע��
	{
		token.value_.identifier.clear();
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
		if('/' == ch)
		{
			SingleLineCommentHandle(token, ch);
		}
		else if('*' == ch)
		{
			BlockCommentHandle(token, ch);
		}
		else
		{
			throw LexException("unkown character", ch, currentline_);
		}
	}
	// ʣ���ǵ��ַ������
	else
	{
		token.value_.identifier.clear();
		token.value_.identifier.push_back(ch);
		switch(ch)
		{
		case '.':
			token.type_ = Token::PERIOD;
			break;
		case ',':
			token.type_ = Token::COMMA;
			break;
		case ';':
			token.type_ = Token::SEMICOLON;
			break;
		case '[':
			token.type_ = Token::LEFT_BRACKET;
			break;
		case ']':
			token.type_ = Token::RIGHT_BRACKET;
			break;
		case '(':
			token.type_ = Token::LEFT_PAREN;
			break;
		case ')':
			token.type_ = Token::RIGHT_PAREN;
			break;
		case '*':
			token.type_ = Token::MUL;
			break;
		case '/':
			token.type_ = Token::DIV;
			break;
		case '=':
			token.type_ = Token::EQU;
			break;
		default:
			throw LexException("unkown character", ch, currentline_);
			break;
		}
		// ������һ������
		ch = getNextChar(true);
	}
}

void LexicalAnalyzer::SingleLineCommentHandle(Token &token, char &ch) throw()			// ����ע��
{
	do
	{
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
	}
	while(ch != '\0' && ch != '\n');
	token.type_ = Token::COMMENT;
	ch = getNextChar();	// �����п��ޣ���Ϊ'\0'��'\n'��������Parse�б����ƴ��������������Ч�ʻ��һЩ
}
void LexicalAnalyzer::BlockCommentHandle(Token &token, char &ch) throw(LexException)			// ����ע��
{
	token.value_.identifier.push_back(ch);
	char lastCh = '\0';
	ch = getNextChar();
	while(ch != '\0' && (lastCh != '*' || ch != '/'))
	{
		token.value_.identifier.push_back(ch);
		lastCh = ch;
		ch = getNextChar();
	}
	if('\0' == ch)
	{
		throw LexException("unexpected end of file found in comment", ch, currentline_);
	}
	token.value_.identifier.push_back(ch);
	token.type_ = Token::COMMENT;
	ch = getNextChar();	// ���һ��Ҫ�У�ʹch��Ϊע�ͺ�ĵ�һ���ַ�
}
void LexicalAnalyzer::DigitHandle(Token &token, char &ch) throw(LexException)					// �����޷�����������
{
	int n = static_cast<int>(ch - '0');
	int last_n = 0;
	while(isdigit(ch = getNextChar()))	// ������ƴ�ӳ���
	{
		last_n = n;
		n = n * 10 + static_cast<int>(ch - '0');
		if(n < last_n)	// ����̫��
		{
			throw LexException("truncation of constant value", ch, currentline_);
		}
	}
	token.type_ = Token::CONST_INTEGER;
	token.value_.integer = n;
}
void LexicalAnalyzer::LetterHandle(Token &token, char &ch) throw()					// ������ĸ
{
	token.lineNumber_ = currentline_;
	token.value_.identifier.clear();
	do	// ����ĸƴ�ӳ��ַ���
	{
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
	}while(isalpha(ch) || isdigit(ch));
	// �ڱ����ֱ��в��ң��Ƿ�Ϊ������
	map<string, Token::TokenType>::const_iterator iter = Token::sReserveWordToTokenType.find(token.value_.identifier);
	if(iter != Token::sReserveWordToTokenType.end())	// �ҵ��ˣ�˵���Ǳ�����
	{
		token.type_ = iter->second;
	}
	else												// û�ҵ���˵�����û��Զ���ı�ʶ��
	{
		token.type_ = Token::IDENTIFIER;
	}
}
void LexicalAnalyzer::StringHandle(Token &token, char &ch) throw(LexException)					// �����ַ�������
{
	token.lineNumber_ = currentline_;
	ch = getNextChar();
	token.value_.identifier.clear();
	while(isprint(ch) && ch != '"')
	{
		if('\\' == ch)
		{
			ch = getNextChar();
		}
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
	}
	if('"' == ch)	// �жϽ��������ǲ���˫����
	{
		token.type_ = Token::CONST_STRING;
		ch = getNextChar();
		// ���ַ������볣���ַ�����
		string_set.insert(token.value_.identifier);
	}
	else
	{
		throw LexException("wrong string constant definition: require \" as an end sign", ch, currentline_);
	}
}
void LexicalAnalyzer::CharHandle(Token &token, char &ch) throw(LexException)						// �����ַ�����
{
	token.lineNumber_ = currentline_;
	ch = getNextChar();	// ��ȡ�����ź�����Ǹ��ַ�
	if(!isalpha(ch) && !isdigit(ch))	// �ж��ǲ�����ĸ��������
	{
		throw LexException("wrong character constant definition: require letter after \"'\"", ch, currentline_);
	}
	else	// ��ȡ�����ĵ�����
	{
		token.value_.character = ch;
		ch = getNextChar();
		if('\'' == ch)	// �ɹ���ȡ��������
		{
			token.type_ = Token::CONST_CHAR;
			ch = getNextChar();
		}
		else	// ������ǵ�����
		{
			throw LexException("wrong character constant definition: require \"'\"", ch, currentline_);
		}
	}
}

void LexicalAnalyzer::ColonHandle(Token &token, char &ch) throw()					// ����ð��
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar();// ��ȡð�ŵ���һ������
	if('=' == ch)	// ���ǵȺţ�����ǰһ��ð��һ�𹹳�һ����ֵ����
	{
		token.value_.identifier.push_back(ch);
		token.type_ = Token::ASSIGN;
		ch = getNextChar();
	}
	else			// ���ǵȺţ���ǰ���ð�ž�ֻ�ǵ�����ð��
	{
		token.type_ = Token::COLON;
	}
}
void LexicalAnalyzer::LessthanHandle(Token &token, char &ch) throw()						// ����С�ں�
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar();
	if('=' == ch)
	{
		token.value_.identifier.push_back(ch);
		token.type_ = Token::LEQ;
		ch = getNextChar();
	}
	else if('>' == ch)
	{
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
		token.type_ = Token::NEQ;
	}
	else
	{
		token.type_ = Token::LT;
	}
}
void LexicalAnalyzer::GreaterthanHandle(Token &token, char &ch) throw()						// ������ں�
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar();
	if('=' == ch)
	{
		token.value_.identifier.push_back(ch);
		token.type_ = Token::GEQ;
		ch = getNextChar();
	}
	else
	{
		token.type_ = Token::GT;
	}
}

// ע��ch = getNextChar(true)һ��Ҫ��true��getNextChar�Ĳ���
// �����ֻ�ܴ���+����泣�����ڵ����
// ����Ҫ��֤���øú���ʱ��token.type_���һ������һ��token������
void LexicalAnalyzer::PlusHandle(Token &token, char &ch) throw(LexException)
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar(true);	// ȡ����һ����Ч���ַ�(����һ��һ��Ҫ��true���������Դ���+֮���пո�����)
	// ǰ���ǵȺŻ�ֵ�ŵ�����������������֣��򰴵�����������������
	if((token.type_ == Token::EQU || token.type_ == Token::ASSIGN) && isdigit(ch))
	{
		DigitHandle(token, ch);
	}
	else	// ��˫�������ļ���������
	{
		token.type_ = Token::PLUS;
	}
}
// ע��ch = getNextChar(true)һ��Ҫ��true��getNextChar�Ĳ���
// �����ֻ�ܴ���-����泣�����ڵ����
void LexicalAnalyzer::MinusHandle(Token &token, char &ch) throw(LexException)
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar(true);	// ȡ����һ����Ч���ַ�(����һ��һ��Ҫ��true���������Դ���-֮���пո�����)
	// ǰ���ǵȺŻ�ֵ�ŵ�����������������֣��򰴵�����������������
	if((token.type_ == Token::EQU || token.type_ == Token::ASSIGN) && isdigit(ch))
	{
		DigitHandle(token, ch);
		token.value_.integer  = -token.value_.integer;
	}
	else	// ��˫�������ļ���������
	{
		token.type_ = Token::MINUS;
	}
}
