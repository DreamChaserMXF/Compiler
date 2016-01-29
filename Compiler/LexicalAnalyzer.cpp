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

// 根据绑定的源代码文件，进行词法分析
// 分析的结果存入token_vector_中
bool LexicalAnalyzer::Parse() throw()							// 进行词法分析
{
	bool isSuccessful = true;
	//if(!srcfile_.is_open())	// 检查文件是否打开
	//{
	//	return false;
	//}
	
	// 清空token栈
	token_vector_.clear();
	// 清空常量字符串表
	string_set.clear();
	// 开始按词法状态图进行解析
	Token token;
	currentline_ = 0;
	char ch = getNextChar();	
	while(ch != '\0')
	{
		// 忽略空格
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
			// 进行词法错误恢复
			do
			{
				ch = getNextChar();
			}
			while(ch != '\0' && !isspace(ch));
			token.type_ = Token::NIL;
			continue;
		}
		if(token.type_ != Token::COMMENT)	// 若单词不是注释
		{
			token_vector_.push_back(token);	// 则将单词放入vector
		}
	}
	ResetTokenPos();	// 添加完所有单词后，重置单词迭代器的初始位置
	return isSuccessful;
}


// 将词法分析的结果输出到文件
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
// 将词法分析的结果输出到流（文件流、字符流或控制台输出流）
void LexicalAnalyzer::Print(ostream &output) const throw()		
{
	int tokenNo = 1;
	output << "TokenNumber" << '\t' << "LineNumber" << '\t' << "TokenType" << "\t    " << "TokenValue" << std::endl;
	for(vector<Token>::const_iterator iter = token_vector_.begin(); iter != token_vector_.end(); ++iter)
	{
		output << tokenNo++ << "\t\t" << iter->lineNumber_ << "\t\t";
		output.width(16);
		output << std::left << Token::sTokenTypeToString[iter->type_] << "    ";
		// 只在必要时输出
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
		// 所有Token均要输出
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

void LexicalAnalyzer::ResetTokenPos() throw()				// 重置符号位置
{
	token_iter_ = token_vector_.begin();
}
bool LexicalAnalyzer::GetNextToken(Token &token) throw()	// 获取下一符号
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

// 在最初设计时该函数没有参数，行为是按照skipSpace=false来处理的
// 考虑到+/-号之后与第一个常数之间可能有空格的情况，故加上参数skipSpace以处理该种情况
// 调用时可不写参数，默认为false
char LexicalAnalyzer::getNextChar(bool skipSpace)				// 得到下一个字符
{
	static string line;
	static string::const_iterator iter = line.end();
	
	if(skipSpace)	// 跳过空字符的读法
	{
		// 若读到了行尾，则读取新的一行，直到读到非空行或文件结束
		while(iter == line.end() || isspace(*iter))
		{
			while(iter == line.end())// 直到读到了新的非空行，或者读到文件尾，才跳出循环
			{
				if(getline(srcfile_, line))	// 读取成功
				{
					++currentline_;
					iter = line.begin();
				}
				else						// 读到文件尾
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
	else	// 不跳过空字符的读法
	{
		if(iter == line.end())
		{
			if(getline(srcfile_, line))	// 读取成功
			{
				++currentline_;
				iter = line.begin();
				return '\n';
			}
			else						// 读到文件尾
			{
				return '\0';
			}
		}
		return *(iter++);
	}
}


void LexicalAnalyzer::ParseCurrentToken(Token &token, char &ch) throw(LexException)
{
	// 先判断三种不定长字符的情况
	if(isalpha(ch))		// 保留字或标识符			
	{
		LetterHandle(token, ch);
	}
	else if(isdigit(ch))// 无符号整型常量
	{
		DigitHandle(token, ch);
	}
	else if('"' == ch)	// 字符串常量
	{
		StringHandle(token, ch);
	}
	// 一种三字符的情况
	else if('\'' == ch)	// 字符常量
	{
		CharHandle(token, ch);
	}
	// 可能为单字符或双字符的情况
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
	// 可能单字符或多字符的情况
	else if('+' == ch)
	{
		PlusHandle(token, ch);
	}
	else if('-' == ch)
	{
		MinusHandle(token, ch);
	}
	else if('/' == ch)	// 单行注释
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
	// 剩下是单字符的情况
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
		// 读入下一个符号
		ch = getNextChar(true);
	}
}

void LexicalAnalyzer::SingleLineCommentHandle(Token &token, char &ch) throw()			// 单行注释
{
	do
	{
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
	}
	while(ch != '\0' && ch != '\n');
	token.type_ = Token::COMMENT;
	ch = getNextChar();	// 这句可有可无，因为'\0'和'\n'都可以在Parse中被妥善处理。但加在这里的效率会高一些
}
void LexicalAnalyzer::BlockCommentHandle(Token &token, char &ch) throw(LexException)			// 多行注释
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
	ch = getNextChar();	// 这句一定要有，使ch变为注释后的第一个字符
}
void LexicalAnalyzer::DigitHandle(Token &token, char &ch) throw(LexException)					// 处理无符号整数常量
{
	int n = static_cast<int>(ch - '0');
	int last_n = 0;
	while(isdigit(ch = getNextChar()))	// 将数字拼接成数
	{
		last_n = n;
		n = n * 10 + static_cast<int>(ch - '0');
		if(n < last_n)	// 常数太大
		{
			throw LexException("truncation of constant value", ch, currentline_);
		}
	}
	token.type_ = Token::CONST_INTEGER;
	token.value_.integer = n;
}
void LexicalAnalyzer::LetterHandle(Token &token, char &ch) throw()					// 处理字母
{
	token.lineNumber_ = currentline_;
	token.value_.identifier.clear();
	do	// 将字母拼接成字符串
	{
		token.value_.identifier.push_back(ch);
		ch = getNextChar();
	}while(isalpha(ch) || isdigit(ch));
	// 在保留字表中查找，是否为保留字
	map<string, Token::TokenType>::const_iterator iter = Token::sReserveWordToTokenType.find(token.value_.identifier);
	if(iter != Token::sReserveWordToTokenType.end())	// 找到了，说明是保留字
	{
		token.type_ = iter->second;
	}
	else												// 没找到，说明是用户自定义的标识符
	{
		token.type_ = Token::IDENTIFIER;
	}
}
void LexicalAnalyzer::StringHandle(Token &token, char &ch) throw(LexException)					// 处理字符串常量
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
	if('"' == ch)	// 判断结束符号是不是双引号
	{
		token.type_ = Token::CONST_STRING;
		ch = getNextChar();
		// 将字符串加入常量字符串表
		string_set.insert(token.value_.identifier);
	}
	else
	{
		throw LexException("wrong string constant definition: require \" as an end sign", ch, currentline_);
	}
}
void LexicalAnalyzer::CharHandle(Token &token, char &ch) throw(LexException)						// 处理字符常量
{
	token.lineNumber_ = currentline_;
	ch = getNextChar();	// 读取单引号后面的那个字符
	if(!isalpha(ch) && !isdigit(ch))	// 判断是不是字母或者数字
	{
		throw LexException("wrong character constant definition: require letter after \"'\"", ch, currentline_);
	}
	else	// 读取结束的单引号
	{
		token.value_.character = ch;
		ch = getNextChar();
		if('\'' == ch)	// 成功读取到单引号
		{
			token.type_ = Token::CONST_CHAR;
			ch = getNextChar();
		}
		else	// 如果不是单引号
		{
			throw LexException("wrong character constant definition: require \"'\"", ch, currentline_);
		}
	}
}

void LexicalAnalyzer::ColonHandle(Token &token, char &ch) throw()					// 处理冒号
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar();// 读取冒号的下一个符号
	if('=' == ch)	// 若是等号，则与前一个冒号一起构成一个赋值符号
	{
		token.value_.identifier.push_back(ch);
		token.type_ = Token::ASSIGN;
		ch = getNextChar();
	}
	else			// 不是等号，则前面的冒号就只是单纯的冒号
	{
		token.type_ = Token::COLON;
	}
}
void LexicalAnalyzer::LessthanHandle(Token &token, char &ch) throw()						// 处理小于号
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
void LexicalAnalyzer::GreaterthanHandle(Token &token, char &ch) throw()						// 处理大于号
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

// 注意ch = getNextChar(true)一定要用true作getNextChar的参数
// 否则就只能处理+与后面常数紧邻的情况
// 而且要保证调用该函数时，token.type_存的一定是上一个token的类型
void LexicalAnalyzer::PlusHandle(Token &token, char &ch) throw(LexException)
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar(true);	// 取得下一个有效的字符(这里一定一定要用true作参数，以处理+之后有空格的情况)
	// 前面是等号或赋值号的情况（若后面是数字，则按单操作符负号来处理）
	if((token.type_ == Token::EQU || token.type_ == Token::ASSIGN) && isdigit(ch))
	{
		DigitHandle(token, ch);
	}
	else	// 按双操作符的减号来处理
	{
		token.type_ = Token::PLUS;
	}
}
// 注意ch = getNextChar(true)一定要用true作getNextChar的参数
// 否则就只能处理-与后面常数紧邻的情况
void LexicalAnalyzer::MinusHandle(Token &token, char &ch) throw(LexException)
{
	token.value_.identifier.clear();
	token.value_.identifier.push_back(ch);
	token.lineNumber_ = currentline_;
	ch = getNextChar(true);	// 取得下一个有效的字符(这里一定一定要用true作参数，以处理-之后有空格的情况)
	// 前面是等号或赋值号的情况（若后面是数字，则按单操作符负号来处理）
	if((token.type_ == Token::EQU || token.type_ == Token::ASSIGN) && isdigit(ch))
	{
		DigitHandle(token, ch);
		token.value_.integer  = -token.value_.integer;
	}
	else	// 按双操作符的减号来处理
	{
		token.type_ = Token::MINUS;
	}
}
