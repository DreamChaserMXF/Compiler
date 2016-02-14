#include "SemanticsAnalyzer.h"

#include "SemanticsAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableItem.h"
#include "TokenTableException.h"
#include <assert.h>
#include <sstream>
#include <algorithm>

//#define SEMANTICSDEBUG

SemanticsAnalyzer::SemanticsAnalyzer(LexicalAnalyzer &lexical_analyzer) 
								throw()
	: lexical_analyzer_(lexical_analyzer), tokentable_(),
	token_(), level_(1),
	is_successful_(true), semantics_process_buffer_(), semantics_format_string_(), tokenbuffer_()
{
	lexical_analyzer.ResetTokenPos();
}


bool SemanticsAnalyzer::Parse() throw()
{
	size_t depth = 0;
	semantics_process_buffer_.str("");
	semantics_process_buffer_.clear();
	semantics_format_string_.clear();
	PrintFunctionFrame("Parse()", depth);
	lexical_analyzer_.GetNextToken(token_);
	Routine(depth + 1);
	return is_successful_;
}

TokenTable SemanticsAnalyzer::GetTokenTable() const throw()
{
	return tokentable_;
}

string SemanticsAnalyzer::toString() const throw()
{
	return semantics_process_buffer_.str();
}

bool SemanticsAnalyzer::Print(const string &filename) const throw()
{
	std::ofstream output(filename);
	if(!output.is_open())
	{
		return false;
	}
	Print(output);
	output.close();
	return true;
}

void SemanticsAnalyzer::Print(std::ostream &output) const throw()
{
	output << semantics_process_buffer_.str() << std::endl;
}

static string semantics_format_string_;	// 注：不是线程安全的
void SemanticsAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
{
	if(depth * 4 == semantics_format_string_.size())
	{
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
	else if(depth * 4 > (int)semantics_format_string_.size())
	{
		semantics_format_string_.append("|");
		semantics_format_string_.append(depth * 4 - semantics_format_string_.size(), ' ');	// 这里不能减1
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
	else // depth * 4 < semantics_format_string_.size()
	{
		semantics_format_string_.resize(depth * 4);
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
}
// <程序> ::= <分程序>.
void SemanticsAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// 解析分程序
	SubRoutine(depth + 1);
	// 判断结束符号
	assert(Token::PERIOD == token_.type_);
}

// <分程序> ::= [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>
void SemanticsAnalyzer::SubRoutine(size_t depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	// 四个可选分支
	if(token_.type_ == Token::CONST)
	{
		ConstantPart(depth + 1);
	}
	if(token_.type_ == Token::VAR)
	{
		VariablePart(depth + 1);
	}
	if(token_.type_ == Token::PROCEDURE)
	{
		ProcedurePart(depth + 1);
	}
	if(token_.type_ == Token::FUNCTION)
	{
		FunctionPart(depth + 1);
	}
	// 一个必选分支
	assert(token_.type_ == Token::BEGIN);
	StatementBlockPart(depth + 1);
}

// <常量说明部分> ::= const<常量定义>{,<常量定义>};
void SemanticsAnalyzer::ConstantPart(size_t depth) throw()
{
	PrintFunctionFrame("ConstantPart()", depth);

	assert(Token::CONST == token_.type_);

	// 常量定义
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		constantDefination(depth + 1);
	} while(token_.type_ == Token::COMMA);
	// 现在的token_一定是分号
	// 读入下一个token
	lexical_analyzer_.GetNextToken(token_);
}

// <常量定义> ::= <标识符>＝<常量>
void SemanticsAnalyzer::constantDefination(size_t depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	// 现在的token_一定是标识符
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 记录token_以插入符号表
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);	// 读取等号
	lexical_analyzer_.GetNextToken(token_);	// 读取常量
	// 常量定义，插入符号表
	if(tokentable_.IsInLocalActiveScope(constIdentifier.value_.identifier))
	{
		ErrorHandle(REDEFINITION);
	}
	else if(Token::CONST_INTEGER == token_.type_)
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::INTEGER, token_.value_.integer, level_);
	}
	else
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::CHAR, token_.value_.character, level_);
	}
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <变量说明部分> ::= var <变量定义>;{<变量定义>;}
void SemanticsAnalyzer::VariablePart(size_t depth) throw()
{
	PrintFunctionFrame("VariablePart()", depth);

	assert(Token::VAR == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	do
	{
		VariableDefinition(depth + 1);
		// token_应该是分号
		// 读取下一个单词
		lexical_analyzer_.GetNextToken(token_);
	}while(token_.type_ == Token::IDENTIFIER);
}

// <变量定义> ::= <标识符>{,<标识符>}:<类型>
void SemanticsAnalyzer::VariableDefinition(size_t depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);	// 标识符入栈
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(token_.type_ == Token::COMMA)
	{
		// 读取标识符并入栈
		lexical_analyzer_.GetNextToken(token_);
		tokenbuffer_.push_back(token_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	// 读取类型说明 
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <类型> ::= [array'['<无符号整数>']'of]<基本类型>
void SemanticsAnalyzer::TypeSpecification(size_t depth) throw()
{
	PrintFunctionFrame("TypeSpecification()", depth);

	TokenTableItem::ItemType itemtype_ = TokenTableItem::VARIABLE;
	int arrayLength = 0;
	if(token_.type_ == Token::ARRAY)
	{
		itemtype_ = TokenTableItem::ARRAY;
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);	// 读取左方括号
		lexical_analyzer_.GetNextToken(token_);	// 读取无符号整数
		arrayLength = token_.value_.integer;	// 存储数组长度
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);	// 读取右方括号
		lexical_analyzer_.GetNextToken(token_);	// 读取OF
		lexical_analyzer_.GetNextToken(token_);	// 读取基本类型
	}
	// 插入符号表
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// 修饰符类型
	if(TokenTableItem::ARRAY == itemtype_)	// 若是数组
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.IsInLocalActiveScope(iter->value_.identifier))	// 重定义后，仍然插入当前定义（也可不插入）
			{
				ErrorHandle(REDEFINITION);
			}
			tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
		}
	}
	else									// 若是一般变量
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.IsInLocalActiveScope(iter->value_.identifier))
			{
				ErrorHandle(REDEFINITION);
			}
			tokentable_.AddVariableItem(*iter, decoratetype_, level_);
		}
	}
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 多读一个单词
	lexical_analyzer_.GetNextToken(token_);
}

// <过程说明部分> ::= <过程首部><分程序>;{<过程首部><分程序>;}
void SemanticsAnalyzer::ProcedurePart(size_t depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		tokentable_.Relocate();
		--level_;
		assert(token_.type_ == Token::SEMICOLON);
		lexical_analyzer_.GetNextToken(token_);	// 读入分号的下一个单词
	}while(Token::PROCEDURE == token_.type_);
}

// <过程首部> ::= procedure<过程标识符>'('[<形式参数表>]')';
void SemanticsAnalyzer::ProcedureHead(size_t depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	lexical_analyzer_.GetNextToken(token_);	// 读取过程名
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 插入过程名到符号表
	string proc_name = token_.value_.identifier;
	if(tokentable_.IsInLocalActiveScope(token_.value_.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的过程分程序的语法语义分析）
	{
		ErrorHandle(REDEFINITION);
	}
	tokentable_.AddProcedureItem(token_, level_++);// 过程名之后leve要+1
	// 定位（将过程名后紧邻的位置设为局部作用域的起始点）
	tokentable_.Locate();	
	// 继续读取单词
	lexical_analyzer_.GetNextToken(token_);	// 读取左括号
	lexical_analyzer_.GetNextToken(token_);	// 读取形式参数表
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth);
		tokentable_.SetParameterCount(proc_name, parameterCount);
	}
	// 此时的token_是右括号
	lexical_analyzer_.GetNextToken(token_);	// 读取分号
	lexical_analyzer_.GetNextToken(token_);	// 多读一个单词
}

// <函数说明部分> ::= <函数首部><分程序>;{<函数首部><分程序>;}
void SemanticsAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		FunctionHead(depth + 1);	// 进行函数头分析，并得到函数名在符号表中的位置
		SubRoutine(depth + 1);
		tokentable_.Relocate();
		--level_;
		lexical_analyzer_.GetNextToken(token_);	// 读取分号后的一个单词
	}while(Token::FUNCTION == token_.type_);
}

// <函数首部> ::= function <函数标识符>'('[<形式参数表>]')':<基本类型>;
void SemanticsAnalyzer::FunctionHead(size_t depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// 函数名在符号表中的位置

	lexical_analyzer_.GetNextToken(token_);	// 读入函数标识符
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 插入函数名到符号表
	string func_name = token_.value_.identifier;
	if(tokentable_.IsInLocalActiveScope(token_.value_.identifier))	// 重定义时，仍然插入重定义的过程定义（因为仍然插入后影响不大，而不插入的话，会影响该次的函数分程序的语法语义分析）
	{
		ErrorHandle(REDEFINITION);
	}
	tokentable_.AddFunctionItem(token_, level_++);// 添加完函数名之后leve要+1
	// 定位
	tokentable_.Locate();
	lexical_analyzer_.GetNextToken(token_);	// 读左括号
	lexical_analyzer_.GetNextToken(token_);	// 再读一个单词
	// 读形式参数表
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth + 1);
		tokentable_.SetParameterCount(func_name, parameterCount); 
	}
	// 现在的token_应该是右括号
	lexical_analyzer_.GetNextToken(token_);	// 读冒号
	lexical_analyzer_.GetNextToken(token_);	// 读返回类型
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	tokentable_.SetFunctionReturnType(func_name, decoratetype_);
	lexical_analyzer_.GetNextToken(token_);	// 读分号
	lexical_analyzer_.GetNextToken(token_);	// 多读一个单词
}

// <形式参数表> ::= <形式参数段>{;<形式参数段>}
// 返回形参数量
int SemanticsAnalyzer::ParameterList(size_t depth) throw()		// 形参表
{
	PrintFunctionFrame("ParameterList()", depth);

	int sum = ParameterTerm(depth + 1);
	while(Token::SEMICOLON == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		sum += ParameterTerm(depth + 1);
	}
	return sum;
}

// <形式参数段> ::= [var]<标识符>{,<标识符>}:<基本类型>
// 返回该形参段的形参数量
int SemanticsAnalyzer::ParameterTerm(size_t depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);
	bool isref = false;	// 是否为引用传参
	if(Token::VAR == token_.type_)
	{
		isref = true;
		lexical_analyzer_.GetNextToken(token_);
	}
	// 此时的token应为表示形参的标识符
	assert(Token::IDENTIFIER == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 参数名压栈准备进符号表
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);
	lexical_analyzer_.GetNextToken(token_);
	int sum = 1;
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		tokenbuffer_.push_back(token_);
		++sum;
		lexical_analyzer_.GetNextToken(token_);
	}
	// 此时token_应为冒号
	assert(Token::COLON == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// 读入形参类型说明
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// 参数存符号表
	TokenTableItem::DecorateType decoratetype = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.IsInLocalActiveScope(iter->value_.identifier))
		{
			ErrorHandle(REDEFINITION);
		}
		tokentable_.AddParameterItem(*iter, decoratetype, isref, level_);
	} // 存符号表完成

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}



// <复合语句> ::= begin <语句>{;<语句>} end
void SemanticsAnalyzer::StatementBlockPart(size_t depth) throw()	// 复合语句
{
	PrintFunctionFrame("StatementBlockPart()", depth);

	assert(Token::BEGIN == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}while(token_.type_ == Token::SEMICOLON);
	assert(Token::END == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// 多读一个单词
}

// <语句> ::= <标识符>(<赋值语句>|<过程调用语句>)|<条件语句>|<情况语句>|<复合语句>
// |<读语句>|<写语句>|<while循环语句>|<for循环语句>|<循环继续语句>|<循环退出语句>|<空>
void SemanticsAnalyzer::Statement(size_t depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// 该token_可能是过程名，先记下，待用
	TokenTable::iterator iter;
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		iter = tokentable_.SearchDefinition(token_);	// 查找符号表中的定义
		if(iter == tokentable_.end())
		{
			ErrorHandle(UNDEFINITION);
			//return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// 过程或函数调用
		{
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::PROCEDURE
				&& iter->itemtype_ != TokenTableItem::FUNCTION)	// 检查其属性是否为过程或函数
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as a procedure").c_str());
				//return;
			}
			vector<ExpressionAttribute> proc_func_attributes = tokentable_.GetProcFuncParameterAttributes(iter);
			
			ProcFuncCallStatement(idToken, proc_func_attributes, depth + 1);
		}
		else
		{
			assert(Token::ASSIGN == token_.type_ || Token::LEFT_BRACKET == token_.type_);
			AssigningStatement(idToken, iter, depth + 1);		
		}
		break;
	case Token::IF:
		IfStatement(depth + 1);
		break;
	case Token::CASE:
		CaseStatement(depth + 1);
		break;
	case Token::BEGIN:
		StatementBlockPart(depth + 1);
		break;
	case Token::READ:
		ReadStatement(depth + 1);
		break;
	case Token::WRITE:
		WriteStatement(depth + 1);
		break;
	case Token::WHILE:
		WhileLoopStatement(depth + 1);
		break;
	case Token::FOR:
		ForLoopStatement(depth + 1);
		break;
	case Token::CONTINUE:
		ContinueStatement(depth + 1);
		break;
	case Token::BREAK:
		BreakStatement(depth + 1);
		break;
		// 空语句
	default:
		break;
	}
}

// <赋值语句> ::= ['['<表达式>']']:=<表达式>
// idToken是赋值语句之前标识符的token，iter是其在符号表中的迭代器
void SemanticsAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth)			// 赋值语句
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// 为四元式生成而定义的变量
	bool assign2array= false;	// 是否为对数组的赋值操作
	
	if(Token::LEFT_BRACKET == token_.type_)	// 对数组元素赋值
	{
		assign2array = true;
		// 语义检查
		if(iter->itemtype_ != TokenTableItem::ARRAY)	// 检查是否为数组名
		{
			ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
			//return;
		}
		// 读入表示下标的表达式
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
		// 读入右中括号
		lexical_analyzer_.GetNextToken(token_);
	}
	// 语义检查：剩下只有三种情况：变量、参数或是函数返回值
	else if(iter->itemtype_ != TokenTableItem::VARIABLE
		&& iter->itemtype_ != TokenTableItem::PARAMETER
		&& iter->itemtype_ != TokenTableItem::FUNCTION)
	{
		ErrorHandle(WRONGTYPE, (iter->name_ + " cannot be assigned").c_str());
		//return;
	}
	// 现在的token_应为赋值号
	assert(Token::ASSIGN == token_.type_);
	// 读入赋值号右边的表达式
	lexical_analyzer_.GetNextToken(token_);
	TokenTableItem::DecorateType exp_decoratetype = Expression(depth + 1);

	// 语义检查：类型转换
	if(iter->decoratetype_ < exp_decoratetype)	// 小于表示不能从右至左的转换
	{
		std::cout << "warning: line " << idToken.lineNumber_ << ":  " << idToken.toString() 
			<< " convert from " << TokenTableItem::DecorateTypeString[exp_decoratetype] 
			<< " to " << TokenTableItem::DecorateTypeString[iter->decoratetype_] << "\n";
	}
}

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
TokenTableItem::DecorateType SemanticsAnalyzer::Expression(size_t depth) throw()				// 表达式
{
	PrintFunctionFrame("Expression()", depth);
	
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}

	TokenTableItem::DecorateType first_term = Term(depth + 1);
	TokenTableItem::DecorateType new_term;
	bool is_first_operator = true;
	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term = is_first_operator ? first_term : new_term;
		new_term = Term(depth + 1);
		// 语义分析：执行类型转换
		new_term = TokenTableItem::TypeConversionMatrix[last_term][new_term];
	}
	// 返回最终的表达式属性
	if(is_first_operator)	// 只有一项的情况
	{
		new_term = first_term;
	}
	return new_term;
}

// <项> ::= <因子>{<乘法运算符><因子>}
TokenTableItem::DecorateType SemanticsAnalyzer::Term(size_t depth) throw()						// 项
{
	PrintFunctionFrame("Term()", depth);

	TokenTableItem::DecorateType first_factor = Factor(depth + 1);
	TokenTableItem::DecorateType new_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// 读取下一项
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor = is_first_operator ? first_factor : new_factor;
		new_factor = Factor(depth + 1);
		// 语义分析：执行类型转换
		new_factor = TokenTableItem::TypeConversionMatrix[last_factor][new_factor];
	}

	// 返回项的属性
	if(is_first_operator)	// 只有一个因子
	{
		new_factor = first_factor;
	}
	return new_factor;
}

// <因子> ::= <标识符>(['['<表达式>']'] | [<函数调用语句>])
//          | '('<表达式>')' 
//          | <无符号整数> 
//          | <字符>
TokenTableItem::DecorateType SemanticsAnalyzer::Factor(size_t depth) throw()					// 因子
{
	PrintFunctionFrame("Factor()", depth);
	TokenTableItem::DecorateType factor_attribute = TokenTableItem::VOID;	// 记录该factor因子的信息

	// 语法检查：标识符的情况【变量、常变量、数组、函数调用】
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// 语义检查
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);	// 寻找定义
		if(iter == tokentable_.end())
		{
			ErrorHandle(UNDEFINITION);
			//return factor_attribute;
		}
		else
		{
			// 记录修饰类型
			factor_attribute = iter->decoratetype_;
		}
		Token idToken = token_;	// 记下，待用
		lexical_analyzer_.GetNextToken(token_);
		// 数组元素
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// 语义检查：是否为数组名
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::ARRAY)	
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
				//return factor_attribute;
			}
			// 读入作为下标的表达式
			lexical_analyzer_.GetNextToken(token_);
			TokenTableItem::DecorateType offset_attribute = Expression(depth + 1);
			// 现在token_应为右中括号
			assert(Token::RIGHT_BRACKET == token_.type_);
			// 读入右中括号的下一个单词 <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// 左括号，函数调用
		{
			// 语义检查：是否为函数
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::FUNCTION)
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as a function").c_str());
				//return factor_attribute;
			}
			// 语义：类型匹配
			// 从符号表中取出函数的参数类型，待FunctionCallStatement去匹配参数
			vector<ExpressionAttribute> parameter_attributes = tokentable_.GetProcFuncParameterAttributes(iter);
			// 过程/函数调用语句
			ProcFuncCallStatement(idToken, parameter_attributes, depth + 1);			
		}
		else	// 单独一个标识符
		{
			// 语义检查：是否为变量、常量或过程/函数的参数
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::VARIABLE 
				&& iter->itemtype_ != TokenTableItem::PARAMETER 
				&& iter->itemtype_ != TokenTableItem::CONST)
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " should be varaible or parameter or constant").c_str());
				//return factor_attribute;
			}
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// 括号括起来的表达式
	{
		// bug fixed by mxf at 0:42 1/31 2016【读表达式之前没有读取括号后的第一个单词】
		// 读表达式的第一个单词
		lexical_analyzer_.GetNextToken(token_);
		// 再读取表达式
		factor_attribute = Expression(depth + 1);	// 记录类型
		lexical_analyzer_.GetNextToken(token_);		// 读取右中括号下一个单词
	}
	else if(Token::CONST_INTEGER == token_.type_)	// 整型字面常量
	{

#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute = TokenTableItem::INTEGER;
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// 字符型字面常量
		assert(Token::CONST_CHAR == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute = TokenTableItem::CHAR;
		lexical_analyzer_.GetNextToken(token_);
	}
	return factor_attribute;
}

// <条件语句> ::= if<条件>then<语句>[else<语句>]
void SemanticsAnalyzer::IfStatement(size_t depth) throw()				// 条件语句
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);
	// 读取条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// 在condition中设置跳转语句
	assert(Token::THEN == token_.type_);
	// 读取THEN后的语句
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// 读取else的语句
	if(Token::ELSE == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>
// 由if或for语句中传递下来label参数，标识if语句块或for循环体的结束
// 用于在处理condition时设置跳转语句
void SemanticsAnalyzer::Condition(size_t depth) throw()				// 条件
{
	PrintFunctionFrame("Condition()", depth);

	Expression(depth + 1);
		
	// 生成有条件的跳转语句
	// 不符合条件时才会跳转，所以这里的操作符与读到的token要反一下
	Quaternary q_jmp_condition;
	switch(token_.type_)	// 这个单词可能是关系运算符，但也有可能是THEN（当条件中仅有一个表达式时）
	{
	case Token::LT:
	case Token::LEQ:
	case Token::GT:
	case Token::GEQ:
	case Token::EQU:
	case Token::NEQ:
	case Token::THEN:
		break;
	default:
		assert(false);
		break;
	}
	if(Token::THEN != token_.type_)	// 如果还有下一个表达式，再继续读取
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
}

// <情况语句> ::= case <表达式> of <情况表元素>{; <情况表元素>}end
void SemanticsAnalyzer::CaseStatement(size_t depth) throw()			// 情况语句
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	assert(Token::OF == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);
	assert(Token::END == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// 多读一个单词
}

// <情况表元素> ::= <情况常量表>:<语句>
// <情况常量表> ::=  <常量 | 常变量>{, <常量 | 常变量>}
void SemanticsAnalyzer::CaseElement(size_t depth) throw()					// 情况表元素
{
	PrintFunctionFrame("CaseElement()", depth);

	TokenTable::iterator iter;
	bool first_item = true;
	do
	{
		if(first_item)
		{
			first_item = false;
		}
		else
		{
			lexical_analyzer_.GetNextToken(token_);// 读取COMMA的下一个单词
		}
		iter = tokentable_.SearchDefinition(token_);
		if(Token::CONST_INTEGER != token_.type_
			&& Token::CONST_CHAR != token_.type_
			&& tokentable_.end() == iter)
		{
			ErrorHandle(WRONGTYPE, (iter->name_ + " should be constant integer or character or constant variable after \"case\"").c_str());
			//return ;
		}
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// 读取下一个单词
		lexical_analyzer_.GetNextToken(token_);
	}while(Token::COMMA == token_.type_);
	assert(Token::COLON == token_.type_);
	// 读取语句
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <读语句> ::= read'('<标识符>{,<标识符>}')'
// TODO 扩展出对数组的支持
void SemanticsAnalyzer::ReadStatement(size_t depth) throw()			// 读语句
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);	// 读左括号
	do
	{
		lexical_analyzer_.GetNextToken(token_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
		if(iter == tokentable_.end())
		{
			ErrorHandle(UNDEFINITION);
			//return;
		}
		// 读取下一个单词，判断是否为数组元素
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// 数组元素
		{
			// 类型检查
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::ARRAY)	// 检查是否为数组变量
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
				//return;
			}
			// 读入数组下标的第一个单词
			lexical_analyzer_.GetNextToken(token_);
			// 读到整个下标的表达式
			 Expression(depth + 1);
			// 读入右中括号的下一个单词
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(iter != tokentable_.end()
			&& iter->itemtype_ != TokenTableItem::VARIABLE
			&& iter->itemtype_ != TokenTableItem::PARAMETER)
		{
			ErrorHandle(WRONGTYPE, (iter->name_ + " cannot be assigned").c_str());
		}
	}while(Token::COMMA == token_.type_);
	// 读右括号的下一个单词
	lexical_analyzer_.GetNextToken(token_);
}

// <写语句> ::= write'(' (<字符串>[,<表达式>] | <表达式>) ')'
void SemanticsAnalyzer::WriteStatement(size_t depth) throw()			// 写语句
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);	// 读左括号
	
	lexical_analyzer_.GetNextToken(token_);	// 读下一个单词 
	
	if(Token::CONST_STRING == token_.type_)
	{
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);	// 读取第二个参数（表达式）
		}
	}
	else
	{
		Expression(depth + 1);
	}
	// 读取右括号的下一个单词	
	lexical_analyzer_.GetNextToken(token_);
}

// <while循环语句> ::= while <条件> do <语句>
// <while循环语句> ::= while @Label<check> <条件> @JZLabel<end> do <语句> @JMPLabel<check> @Label<end>
void SemanticsAnalyzer::WhileLoopStatement(size_t depth) throw()			// while循环语句
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// 更新循环层次
	++loop_level;
	// 读取下一个单词，并进入条件语句
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// 条件语句
	assert(Token::DO == token_.type_);
	// 读入循环体的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	// 读入循环体
	Statement(depth + 1);
	// 更新循环层次
	--loop_level;
}

// <for循环语句> ::= for <标识符> := <表达式> （downto | to） <表达式> do <语句>
// <for循环语句> ::= for <标识符> := <表达式> （downto | to）
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <表达式> @JZLABEL<end> 
// do <语句>@JMPLABEL<vary>@Label<end>
void SemanticsAnalyzer::ForLoopStatement(size_t depth) throw()			// for循环语句
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);
	// 更新循环层次
	++loop_level;

	// 读取标识符
	lexical_analyzer_.GetNextToken(token_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	TokenTable::iterator loopvar_iter = tokentable_.SearchDefinition(token_);
	if(loopvar_iter == tokentable_.end())
	{
		ErrorHandle(UNDEFINITION);
		//return;
	}
	if(loopvar_iter != tokentable_.end()
	&& loopvar_iter->itemtype_ != TokenTableItem::VARIABLE
	&& loopvar_iter->itemtype_ != TokenTableItem::PARAMETER)	// 检查是否为变量或参数
	{
		ErrorHandle(WRONGTYPE, (loopvar_iter->name_ + " cannot be assigned").c_str());
		//return;
	}
	// 读取赋值号
	lexical_analyzer_.GetNextToken(token_);
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	// 读取表达式
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	assert(Token::DO == token_.type_);
	lexical_analyzer_.GetNextToken(token_);
	// 读取循环体
	Statement(depth + 1);
	// 更新循环层次
	--loop_level;
}

void SemanticsAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	if(loop_level <= 0)
	{
		// 报错
		ErrorHandle(OUTERCONTINUE);
	}
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}
void SemanticsAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	if(loop_level <= 0)
	{
		// 报错
		ErrorHandle(OUTERBREAK);
	}
	// 读入下一个单词并返回
	lexical_analyzer_.GetNextToken(token_);
}


// <过程/函数调用语句> ::= '('[<实在参数表>]')'
void SemanticsAnalyzer::ProcFuncCallStatement(const Token proc_token, const vector<ExpressionAttribute> &parameter_attributes, size_t depth)	// 过程调用语句
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	assert(Token::LEFT_PAREN == token_.type_);

	// 语法：读入右括号或参数表的第一个单词
	lexical_analyzer_.GetNextToken(token_);
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(parameter_attributes, depth + 1);
	}
	assert(Token::RIGHT_PAREN == token_.type_);
	lexical_analyzer_.GetNextToken(token_);

	//// 语法：读入右括号或参数表的第一个单词
	//lexical_analyzer_.GetNextToken(token_);
	//if(parameter_attributes.size() == 0)	// 如果函数本身就没有参数的话，就不用读参数了
	//{
	//	if(Token::RIGHT_PAREN != token_.type_)
	//	{
	//		// 语义
	//	}
	//	lexical_analyzer_.GetNextToken(token_);	// 读函数调用完毕后的下一个单词
	//	return;
	//}
	//// 语法：读参数，并生成设置参数的四元式
	//ArgumentList(parameter_attributes, depth + 1);
	//assert(Token::RIGHT_PAREN == token_.type_);
	//lexical_analyzer_.GetNextToken(token_);

}


// <实在参数表> ::= <表达式>{,<表达式>}
void SemanticsAnalyzer::ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw()			// 实参表
{
	PrintFunctionFrame("ArgumentList()", depth);

	TokenTableItem::DecorateType argument_decoratetype = TokenTableItem::VOID;
	size_t para_count = 0;
	do
	{
		if(para_count > 0)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		argument_decoratetype = Expression(depth + 1);
		// 越界检查
		++para_count;
		if(parameter_attributes.size() < para_count)
		{
			// 实际参数过多
			ErrorHandle(ARGUMENTNUMBERNOTMATCH);
			//return;
			//std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() 
			//	<< "  procedure/function calling statement has too many arguments ";
			//is_successful_ = false;
			//while(Token::COMMA == token_.type_)
			//{
			//	lexical_analyzer_.GetNextToken(token_);
			//	argument_decoratetype = Expression(depth + 1);
			//}
		}
		// 类型匹配检查
		if(parameter_attributes.size() >= para_count
			&& parameter_attributes[para_count - 1].decoratetype_ < argument_decoratetype)
		{
			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
				<< "  cannot convert parameter " << para_count << " from " << TokenTableItem::DecorateTypeString[argument_decoratetype]
			<< " to " <<  TokenTableItem::DecorateTypeString[parameter_attributes[para_count - 1].decoratetype_] <<"\n";
			// 这里仅仅是检查了参数类型不匹配，语法分析还可继续，故不返回
		}
	}while(Token::COMMA == token_.type_);
}


void SemanticsAnalyzer::ErrorHandle(ErrorType error_type, const char *errinfo) throw()
{
	using std::cout;
	is_successful_ = false;
	cout << "semantics error(" << error_type << ")";
	int offset = lexical_analyzer_.GetLine(token_.lineNumber_).find_first_not_of(" \t\n");
	int count = lexical_analyzer_.GetLine(token_.lineNumber_).find_last_not_of(" \t\n") - offset + 1;
	string errline = offset == string::npos ? "" : lexical_analyzer_.GetLine(token_.lineNumber_).substr(offset, count);
	switch(error_type)
	{
	case REDEFINITION:	// 重定义时，不需要读完整条语句，因为不影响继续的分析
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "redefinition\n";
		break;
	case UNDEFINITION:	// 未定义时，要读完整条语句并返回，因为符号未定义，则迭代器失效，不能继续当前块的分析
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "undefinition\n";
		break;
	case ARGUMENTNUMBERNOTMATCH:// 参数数量不匹配时，要读完整条语句并返回，因为参数数量不匹配，则无法继续判断参数类型，需要直接返回
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "argument number doesn't match the declaration\n";
		break;
	case WRONGTYPE:// 参数数量不匹配时，要读完整条语句并返回，因为参数数量不匹配，则无法继续判断参数类型，需要直接返回
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << errinfo << "\n";
		break;
	case OUTERCONTINUE:	// 不影响语义分析，无须继续读完整句
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"continue\"\n";
		break;
	case OUTERBREAK:// 不影响语义分析，无须继续读完整句
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"break\"\n";
		break;
	//case REDEFINITION:	// 重定义时，不需要读完整条语句，因为不影响继续的分析
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "redefinition\n";
	//	break;
	//case UNDEFINITION:	// 未定义时，要读完整条语句并返回，因为符号未定义，则迭代器失效，不能继续当前块的分析
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "undefinition\n";
	//	break;
	//case ARGUMENTNUMBERNOTMATCH:// 参数数量不匹配时，要读完整条语句并返回，因为参数数量不匹配，则无法继续判断参数类型，需要直接返回
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "argument number doesn't match the declaration\n";
	//	break;
	//case WRONGTYPE:// 参数数量不匹配时，要读完整条语句并返回，因为参数数量不匹配，则无法继续判断参数类型，需要直接返回
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << errinfo << "\n";
	//	break;
	//case OUTERCONTINUE:	// 不影响语义分析，无须继续读完整句
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"continue\"\n";
	//	break;
	//case OUTERBREAK:// 不影响语义分析，无须继续读完整句
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"break\"\n";
	//	break;
	default:
		// 未知错误
		assert(false);
		break;
	}
}