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

// 根据绑定的源代码文件，进行词法分析
// 分析的结果存入tokenVector中
bool LexicalAnalyzer::Parse()									// 进行词法分析
{
	bool isSuccessful = true;
	if(!srcFile.is_open())	// 检查文件是否打开
	{
		return false;
	}
	
	// 清空token栈
	tokenVector.clear();
	// 清空常量字符串表
	stringSet.clear();
	// 开始按状态图进行解析
	Token token;
	currentLine = 0;
	char ch = getNextChar(true);	
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
			parseCurrentToken(token, ch);			
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
			token.type = Token::NIL;
			continue;
		}
		if(token.type != Token::COMMENT)	// 若单词不是注释
		{
			tokenVector.push_back(token);	// 则将单词放入vector
		}
	}
	ResetTokenPos();	// 添加完所有单词后，重置单词迭代器的初始位置
	return isSuccessful;
}


// 将词法分析的结果输出到文件
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
// 将词法分析的结果输出到流（文件流、字符流或控制台输出流）
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
		// 只在必要时输出
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
		// 所有Token均要输出
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

void LexicalAnalyzer::ResetTokenPos()					// 重置符号位置
{
	tokenIter = tokenVector.begin();
}
bool LexicalAnalyzer::GetNextToken(Token &token)		// 获取下一符号
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
				if(getline(srcFile, line))	// 读取成功
				{
					++currentLine;
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
			if(getline(srcFile, line))	// 读取成功
			{
				++currentLine;
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


void LexicalAnalyzer::parseCurrentToken(Token &token, char &ch)
{
	// 先判断三种不定长字符的情况
	if(isalpha(ch))		// 保留字或标识符			
	{
		letterHandle(token, ch);
	}
	else if(isdigit(ch))// 无符号整型常量
	{
		digitHandle(token, ch);
	}
	else if('"' == ch)	// 字符串常量
	{
		stringHandle(token, ch);
	}
	// 一种三字符的情况
	else if('\'' == ch)	// 字符常量
	{
		charHandle(token, ch);
	}
	// 再判断几种可能为双字符的情况
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
	else if('/' == ch)	// 单行注释
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
	// 剩下是单字符的情况
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
		// 读入下一个符号
		ch = getNextChar(true);
	}
}

void LexicalAnalyzer::singleLineCommentHandle(Token &token, char &ch)			// 单行注释
{
	do
	{
		token.value.identifier.push_back(ch);
		ch = getNextChar();
	}
	while(ch != '\0' && ch != '\n');
	token.type = Token::COMMENT;
	ch = getNextChar();	// 这句可有可无，因为'\0'和'\n'都可以在Parse中被妥善处理。但加在这里的效率会高一些
}
void LexicalAnalyzer::blockCommentHandle(Token &token, char &ch)			// 多行注释
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
	ch = getNextChar();	// 这句一定要有，使ch变为注释后的第一个字符
}
void LexicalAnalyzer::digitHandle(Token &token, char &ch)					// 处理无符号整数常量
{
	int n = static_cast<int>(ch - '0');
	int last_n = 0;
	while(isdigit(ch = getNextChar()))	// 将数字拼接成数
	{
		last_n = n;
		n = n * 10 + static_cast<int>(ch - '0');
		if(n < last_n)	// 常数太大
		{
			throw LexException("truncation of constant value", ch, currentLine);
		}
	}
	token.type = Token::CONST_INTEGER;
	token.value.integer = n;
}
void LexicalAnalyzer::letterHandle(Token &token, char &ch)					// 处理字母
{
	token.lineNumber = currentLine;
	token.value.identifier.clear();
	do	// 将字母拼接成字符串
	{
		token.value.identifier.push_back(ch);
		ch = getNextChar();
	}while(isalpha(ch) || isdigit(ch));
	// 在保留字表中查找，是否为保留字
	map<string, Token::TokenType>::const_iterator iter = Token::reserveWordToTokenType.find(token.value.identifier);
	if(iter != Token::reserveWordToTokenType.end())	// 找到了，说明是保留字
	{
		token.type = iter->second;
	}
	else												// 没找到，说明是用户自定义的标识符
	{
		token.type = Token::IDENTIFIER;
	}
}
void LexicalAnalyzer::stringHandle(Token &token, char &ch)					// 处理字符串常量
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
	if('"' == ch)	// 判断结束符号是不是双引号
	{
		token.type = Token::CONST_STRING;
		ch = getNextChar();
		// 将字符串加入常量字符串表
		stringSet.insert(token.value.identifier);
	}
	else
	{
		throw LexException("wrong string constant definition: require \" as an end sign", ch, currentLine);
	}
}
void LexicalAnalyzer::charHandle(Token &token, char &ch)						// 处理字符常量
{
	token.lineNumber = currentLine;
	ch = getNextChar();	// 读取单引号后面的那个字符
	if(!isalpha(ch) && !isdigit(ch))	// 判断是不是字母或者数字
	{
		throw LexException("wrong character constant definition: require letter after \"'\"", ch, currentLine);
	}
	else	// 读取结束的单引号
	{
		token.value.character = ch;
		ch = getNextChar();
		if('\'' == ch)	// 成功读取到单引号
		{
			token.type = Token::CONST_CHAR;
			ch = getNextChar();
		}
		else	// 如果不是单引号
		{
			throw LexException("wrong character constant definition: require \"'\"", ch, currentLine);
		}
	}
}

void LexicalAnalyzer::colonHandle(Token &token, char &ch)					// 处理冒号
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar();// 读取冒号的下一个符号
	if('=' == ch)	// 若是等号，则与前一个冒号一起构成一个赋值符号
	{
		token.value.identifier.push_back(ch);
		token.type = Token::ASSIGN;
		ch = getNextChar();
	}
	else			// 不是等号，则前面的冒号就只是单纯的冒号
	{
		token.type = Token::COLON;
	}
}
void LexicalAnalyzer::ltHandle(Token &token, char &ch)						// 处理小于号
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
void LexicalAnalyzer::gtHandle(Token &token, char &ch)						// 处理大于号
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

// 注意ch = getNextChar(true)一定要用true作getNextChar的参数
// 否则就只能处理+与后面常数紧邻的情况
// 而且要保证调用该函数时，token.type存的一定是上一个token的类型
void LexicalAnalyzer::plusHandle(Token &token, char &ch)
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar(true);	// 取得下一个有效的字符(这里一定一定要用true作参数，以处理+之后有空格的情况)
	// 前面是等号或赋值号的情况（若后面是数字，则按单操作符负号来处理）
	if((token.type == Token::EQU || token.type == Token::ASSIGN) && isdigit(ch))
	{
		digitHandle(token, ch);
	}
	else	// 按双操作符的减号来处理
	{
		token.type = Token::PLUS;
	}
}
// 注意ch = getNextChar(true)一定要用true作getNextChar的参数
// 否则就只能处理-与后面常数紧邻的情况
void LexicalAnalyzer::minusHandle(Token &token, char &ch)
{
	token.value.identifier.clear();
	token.value.identifier.push_back(ch);
	token.lineNumber = currentLine;
	ch = getNextChar(true);	// 取得下一个有效的字符(这里一定一定要用true作参数，以处理-之后有空格的情况)
	// 前面是等号或赋值号的情况（若后面是数字，则按单操作符负号来处理）
	if((token.type == Token::EQU || token.type == Token::ASSIGN) && isdigit(ch))
	{
		digitHandle(token, ch);
		token.value.integer  = -token.value.integer;
	}
	else	// 按双操作符的减号来处理
	{
		token.type = Token::MINUS;
	}
}
