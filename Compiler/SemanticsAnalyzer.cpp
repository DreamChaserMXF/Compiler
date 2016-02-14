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

static string semantics_format_string_;	// ע�������̰߳�ȫ��
void SemanticsAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
{
	if(depth * 4 == semantics_format_string_.size())
	{
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
	else if(depth * 4 > (int)semantics_format_string_.size())
	{
		semantics_format_string_.append("|");
		semantics_format_string_.append(depth * 4 - semantics_format_string_.size(), ' ');	// ���ﲻ�ܼ�1
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
	else // depth * 4 < semantics_format_string_.size()
	{
		semantics_format_string_.resize(depth * 4);
		semantics_process_buffer_ << semantics_format_string_ << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SemanticsAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// �����ֳ���
	SubRoutine(depth + 1);
	// �жϽ�������
	assert(Token::PERIOD == token_.type_);
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SemanticsAnalyzer::SubRoutine(size_t depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	// �ĸ���ѡ��֧
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
	// һ����ѡ��֧
	assert(token_.type_ == Token::BEGIN);
	StatementBlockPart(depth + 1);
}

// <����˵������> ::= const<��������>{,<��������>};
void SemanticsAnalyzer::ConstantPart(size_t depth) throw()
{
	PrintFunctionFrame("ConstantPart()", depth);

	assert(Token::CONST == token_.type_);

	// ��������
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		constantDefination(depth + 1);
	} while(token_.type_ == Token::COMMA);
	// ���ڵ�token_һ���Ƿֺ�
	// ������һ��token
	lexical_analyzer_.GetNextToken(token_);
}

// <��������> ::= <��ʶ��>��<����>
void SemanticsAnalyzer::constantDefination(size_t depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	// ���ڵ�token_һ���Ǳ�ʶ��
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��¼token_�Բ�����ű�
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);	// ��ȡ�Ⱥ�
	lexical_analyzer_.GetNextToken(token_);	// ��ȡ����
	// �������壬������ű�
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

// <����˵������> ::= var <��������>;{<��������>;}
void SemanticsAnalyzer::VariablePart(size_t depth) throw()
{
	PrintFunctionFrame("VariablePart()", depth);

	assert(Token::VAR == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	do
	{
		VariableDefinition(depth + 1);
		// token_Ӧ���Ƿֺ�
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}while(token_.type_ == Token::IDENTIFIER);
}

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
void SemanticsAnalyzer::VariableDefinition(size_t depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);	// ��ʶ����ջ
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(token_.type_ == Token::COMMA)
	{
		// ��ȡ��ʶ������ջ
		lexical_analyzer_.GetNextToken(token_);
		tokenbuffer_.push_back(token_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	// ��ȡ����˵�� 
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <����> ::= [array'['<�޷�������>']'of]<��������>
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
		lexical_analyzer_.GetNextToken(token_);	// ��ȡ������
		lexical_analyzer_.GetNextToken(token_);	// ��ȡ�޷�������
		arrayLength = token_.value_.integer;	// �洢���鳤��
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);	// ��ȡ�ҷ�����
		lexical_analyzer_.GetNextToken(token_);	// ��ȡOF
		lexical_analyzer_.GetNextToken(token_);	// ��ȡ��������
	}
	// ������ű�
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// ���η�����
	if(TokenTableItem::ARRAY == itemtype_)	// ��������
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.IsInLocalActiveScope(iter->value_.identifier))	// �ض������Ȼ���뵱ǰ���壨Ҳ�ɲ����룩
			{
				ErrorHandle(REDEFINITION);
			}
			tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
		}
	}
	else									// ����һ�����
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
	// ���һ������
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
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
		lexical_analyzer_.GetNextToken(token_);	// ����ֺŵ���һ������
	}while(Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
void SemanticsAnalyzer::ProcedureHead(size_t depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	lexical_analyzer_.GetNextToken(token_);	// ��ȡ������
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��������������ű�
	string proc_name = token_.value_.identifier;
	if(tokentable_.IsInLocalActiveScope(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĹ��̷ֳ�����﷨���������
	{
		ErrorHandle(REDEFINITION);
	}
	tokentable_.AddProcedureItem(token_, level_++);// ������֮��leveҪ+1
	// ��λ��������������ڵ�λ����Ϊ�ֲ����������ʼ�㣩
	tokentable_.Locate();	
	// ������ȡ����
	lexical_analyzer_.GetNextToken(token_);	// ��ȡ������
	lexical_analyzer_.GetNextToken(token_);	// ��ȡ��ʽ������
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth);
		tokentable_.SetParameterCount(proc_name, parameterCount);
	}
	// ��ʱ��token_��������
	lexical_analyzer_.GetNextToken(token_);	// ��ȡ�ֺ�
	lexical_analyzer_.GetNextToken(token_);	// ���һ������
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SemanticsAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		FunctionHead(depth + 1);	// ���к���ͷ���������õ��������ڷ��ű��е�λ��
		SubRoutine(depth + 1);
		tokentable_.Relocate();
		--level_;
		lexical_analyzer_.GetNextToken(token_);	// ��ȡ�ֺź��һ������
	}while(Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
void SemanticsAnalyzer::FunctionHead(size_t depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// �������ڷ��ű��е�λ��

	lexical_analyzer_.GetNextToken(token_);	// ���뺯����ʶ��
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ���뺯���������ű�
	string func_name = token_.value_.identifier;
	if(tokentable_.IsInLocalActiveScope(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĺ����ֳ�����﷨���������
	{
		ErrorHandle(REDEFINITION);
	}
	tokentable_.AddFunctionItem(token_, level_++);// ����꺯����֮��leveҪ+1
	// ��λ
	tokentable_.Locate();
	lexical_analyzer_.GetNextToken(token_);	// ��������
	lexical_analyzer_.GetNextToken(token_);	// �ٶ�һ������
	// ����ʽ������
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth + 1);
		tokentable_.SetParameterCount(func_name, parameterCount); 
	}
	// ���ڵ�token_Ӧ����������
	lexical_analyzer_.GetNextToken(token_);	// ��ð��
	lexical_analyzer_.GetNextToken(token_);	// ����������
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	tokentable_.SetFunctionReturnType(func_name, decoratetype_);
	lexical_analyzer_.GetNextToken(token_);	// ���ֺ�
	lexical_analyzer_.GetNextToken(token_);	// ���һ������
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
int SemanticsAnalyzer::ParameterList(size_t depth) throw()		// �βα�
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

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
int SemanticsAnalyzer::ParameterTerm(size_t depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);
	bool isref = false;	// �Ƿ�Ϊ���ô���
	if(Token::VAR == token_.type_)
	{
		isref = true;
		lexical_analyzer_.GetNextToken(token_);
	}
	// ��ʱ��tokenӦΪ��ʾ�βεı�ʶ��
	assert(Token::IDENTIFIER == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ������ѹջ׼�������ű�
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
	// ��ʱtoken_ӦΪð��
	assert(Token::COLON == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// �����β�����˵��
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��������ű�
	TokenTableItem::DecorateType decoratetype = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.IsInLocalActiveScope(iter->value_.identifier))
		{
			ErrorHandle(REDEFINITION);
		}
		tokentable_.AddParameterItem(*iter, decoratetype, isref, level_);
	} // ����ű����

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}



// <�������> ::= begin <���>{;<���>} end
void SemanticsAnalyzer::StatementBlockPart(size_t depth) throw()	// �������
{
	PrintFunctionFrame("StatementBlockPart()", depth);

	assert(Token::BEGIN == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}while(token_.type_ == Token::SEMICOLON);
	assert(Token::END == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// ���һ������
}

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>
// |<�����>|<д���>|<whileѭ�����>|<forѭ�����>|<ѭ���������>|<ѭ���˳����>|<��>
void SemanticsAnalyzer::Statement(size_t depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// ��token_�����ǹ��������ȼ��£�����
	TokenTable::iterator iter;
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		iter = tokentable_.SearchDefinition(token_);	// ���ҷ��ű��еĶ���
		if(iter == tokentable_.end())
		{
			ErrorHandle(UNDEFINITION);
			//return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// ���̻�������
		{
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::PROCEDURE
				&& iter->itemtype_ != TokenTableItem::FUNCTION)	// ����������Ƿ�Ϊ���̻���
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
		// �����
	default:
		break;
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void SemanticsAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, size_t depth)			// ��ֵ���
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// Ϊ��Ԫʽ���ɶ�����ı���
	bool assign2array= false;	// �Ƿ�Ϊ������ĸ�ֵ����
	
	if(Token::LEFT_BRACKET == token_.type_)	// ������Ԫ�ظ�ֵ
	{
		assign2array = true;
		// ������
		if(iter->itemtype_ != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ������
		{
			ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
			//return;
		}
		// �����ʾ�±�ı��ʽ
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
		// ������������
		lexical_analyzer_.GetNextToken(token_);
	}
	// �����飺ʣ��ֻ������������������������Ǻ�������ֵ
	else if(iter->itemtype_ != TokenTableItem::VARIABLE
		&& iter->itemtype_ != TokenTableItem::PARAMETER
		&& iter->itemtype_ != TokenTableItem::FUNCTION)
	{
		ErrorHandle(WRONGTYPE, (iter->name_ + " cannot be assigned").c_str());
		//return;
	}
	// ���ڵ�token_ӦΪ��ֵ��
	assert(Token::ASSIGN == token_.type_);
	// ���븳ֵ���ұߵı��ʽ
	lexical_analyzer_.GetNextToken(token_);
	TokenTableItem::DecorateType exp_decoratetype = Expression(depth + 1);

	// �����飺����ת��
	if(iter->decoratetype_ < exp_decoratetype)	// С�ڱ�ʾ���ܴ��������ת��
	{
		std::cout << "warning: line " << idToken.lineNumber_ << ":  " << idToken.toString() 
			<< " convert from " << TokenTableItem::DecorateTypeString[exp_decoratetype] 
			<< " to " << TokenTableItem::DecorateTypeString[iter->decoratetype_] << "\n";
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
TokenTableItem::DecorateType SemanticsAnalyzer::Expression(size_t depth) throw()				// ���ʽ
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
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term = is_first_operator ? first_term : new_term;
		new_term = Term(depth + 1);
		// ���������ִ������ת��
		new_term = TokenTableItem::TypeConversionMatrix[last_term][new_term];
	}
	// �������յı��ʽ����
	if(is_first_operator)	// ֻ��һ������
	{
		new_term = first_term;
	}
	return new_term;
}

// <��> ::= <����>{<�˷������><����>}
TokenTableItem::DecorateType SemanticsAnalyzer::Term(size_t depth) throw()						// ��
{
	PrintFunctionFrame("Term()", depth);

	TokenTableItem::DecorateType first_factor = Factor(depth + 1);
	TokenTableItem::DecorateType new_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor = is_first_operator ? first_factor : new_factor;
		new_factor = Factor(depth + 1);
		// ���������ִ������ת��
		new_factor = TokenTableItem::TypeConversionMatrix[last_factor][new_factor];
	}

	// �����������
	if(is_first_operator)	// ֻ��һ������
	{
		new_factor = first_factor;
	}
	return new_factor;
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
TokenTableItem::DecorateType SemanticsAnalyzer::Factor(size_t depth) throw()					// ����
{
	PrintFunctionFrame("Factor()", depth);
	TokenTableItem::DecorateType factor_attribute = TokenTableItem::VOID;	// ��¼��factor���ӵ���Ϣ

	// �﷨��飺��ʶ��������������������������顢�������á�
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// ������
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);	// Ѱ�Ҷ���
		if(iter == tokentable_.end())
		{
			ErrorHandle(UNDEFINITION);
			//return factor_attribute;
		}
		else
		{
			// ��¼��������
			factor_attribute = iter->decoratetype_;
		}
		Token idToken = token_;	// ���£�����
		lexical_analyzer_.GetNextToken(token_);
		// ����Ԫ��
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// �����飺�Ƿ�Ϊ������
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::ARRAY)	
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
				//return factor_attribute;
			}
			// ������Ϊ�±�ı��ʽ
			lexical_analyzer_.GetNextToken(token_);
			TokenTableItem::DecorateType offset_attribute = Expression(depth + 1);
			// ����token_ӦΪ��������
			assert(Token::RIGHT_BRACKET == token_.type_);
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// �����ţ���������
		{
			// �����飺�Ƿ�Ϊ����
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::FUNCTION)
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as a function").c_str());
				//return factor_attribute;
			}
			// ���壺����ƥ��
			// �ӷ��ű���ȡ�������Ĳ������ͣ���FunctionCallStatementȥƥ�����
			vector<ExpressionAttribute> parameter_attributes = tokentable_.GetProcFuncParameterAttributes(iter);
			// ����/�����������
			ProcFuncCallStatement(idToken, parameter_attributes, depth + 1);			
		}
		else	// ����һ����ʶ��
		{
			// �����飺�Ƿ�Ϊ���������������/�����Ĳ���
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
	else if(Token::LEFT_PAREN == token_.type_)	// �����������ı��ʽ
	{
		// bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
		// �����ʽ�ĵ�һ������
		lexical_analyzer_.GetNextToken(token_);
		// �ٶ�ȡ���ʽ
		factor_attribute = Expression(depth + 1);	// ��¼����
		lexical_analyzer_.GetNextToken(token_);		// ��ȡ����������һ������
	}
	else if(Token::CONST_INTEGER == token_.type_)	// �������泣��
	{

#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute = TokenTableItem::INTEGER;
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// �ַ������泣��
		assert(Token::CONST_CHAR == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute = TokenTableItem::CHAR;
		lexical_analyzer_.GetNextToken(token_);
	}
	return factor_attribute;
}

// <�������> ::= if<����>then<���>[else<���>]
void SemanticsAnalyzer::IfStatement(size_t depth) throw()				// �������
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);
	// ��ȡ�������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// ��condition��������ת���
	assert(Token::THEN == token_.type_);
	// ��ȡTHEN������
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// ��ȡelse�����
	if(Token::ELSE == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>
// ��if��for����д�������label��������ʶif�����forѭ����Ľ���
// �����ڴ���conditionʱ������ת���
void SemanticsAnalyzer::Condition(size_t depth) throw()				// ����
{
	PrintFunctionFrame("Condition()", depth);

	Expression(depth + 1);
		
	// ��������������ת���
	// ����������ʱ�Ż���ת����������Ĳ������������tokenҪ��һ��
	Quaternary q_jmp_condition;
	switch(token_.type_)	// ������ʿ����ǹ�ϵ���������Ҳ�п�����THEN���������н���һ�����ʽʱ��
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
	if(Token::THEN != token_.type_)	// ���������һ�����ʽ���ټ�����ȡ
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SemanticsAnalyzer::CaseStatement(size_t depth) throw()			// ������
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	assert(Token::OF == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);
	assert(Token::END == token_.type_);
	lexical_analyzer_.GetNextToken(token_);	// ���һ������
}

// <�����Ԫ��> ::= <���������>:<���>
// <���������> ::=  <���� | ������>{, <���� | ������>}
void SemanticsAnalyzer::CaseElement(size_t depth) throw()					// �����Ԫ��
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
			lexical_analyzer_.GetNextToken(token_);// ��ȡCOMMA����һ������
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
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}while(Token::COMMA == token_.type_);
	assert(Token::COLON == token_.type_);
	// ��ȡ���
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// TODO ��չ���������֧��
void SemanticsAnalyzer::ReadStatement(size_t depth) throw()			// �����
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);	// ��������
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
		// ��ȡ��һ�����ʣ��ж��Ƿ�Ϊ����Ԫ��
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// ����Ԫ��
		{
			// ���ͼ��
			if(iter != tokentable_.end()
				&& iter->itemtype_ != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ�������
			{
				ErrorHandle(WRONGTYPE, (iter->name_ + " is not declared as an array").c_str());
				//return;
			}
			// ���������±�ĵ�һ������
			lexical_analyzer_.GetNextToken(token_);
			// ���������±�ı��ʽ
			 Expression(depth + 1);
			// �����������ŵ���һ������
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(iter != tokentable_.end()
			&& iter->itemtype_ != TokenTableItem::VARIABLE
			&& iter->itemtype_ != TokenTableItem::PARAMETER)
		{
			ErrorHandle(WRONGTYPE, (iter->name_ + " cannot be assigned").c_str());
		}
	}while(Token::COMMA == token_.type_);
	// �������ŵ���һ������
	lexical_analyzer_.GetNextToken(token_);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void SemanticsAnalyzer::WriteStatement(size_t depth) throw()			// д���
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);	// ��������
	
	lexical_analyzer_.GetNextToken(token_);	// ����һ������ 
	
	if(Token::CONST_STRING == token_.type_)
	{
#ifdef SEMANTICSDEBUG
	semantics_process_buffer_ << semantics_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);	// ��ȡ�ڶ������������ʽ��
		}
	}
	else
	{
		Expression(depth + 1);
	}
	// ��ȡ�����ŵ���һ������	
	lexical_analyzer_.GetNextToken(token_);
}

// <whileѭ�����> ::= while <����> do <���>
// <whileѭ�����> ::= while @Label<check> <����> @JZLabel<end> do <���> @JMPLabel<check> @Label<end>
void SemanticsAnalyzer::WhileLoopStatement(size_t depth) throw()			// whileѭ�����
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// ����ѭ�����
	++loop_level;
	// ��ȡ��һ�����ʣ��������������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// �������
	assert(Token::DO == token_.type_);
	// ����ѭ����ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// ����ѭ����
	Statement(depth + 1);
	// ����ѭ�����
	--loop_level;
}

// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to�� <���ʽ> do <���>
// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to��
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <���ʽ> @JZLABEL<end> 
// do <���>@JMPLABEL<vary>@Label<end>
void SemanticsAnalyzer::ForLoopStatement(size_t depth) throw()			// forѭ�����
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);
	// ����ѭ�����
	++loop_level;

	// ��ȡ��ʶ��
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
	&& loopvar_iter->itemtype_ != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������
	{
		ErrorHandle(WRONGTYPE, (loopvar_iter->name_ + " cannot be assigned").c_str());
		//return;
	}
	// ��ȡ��ֵ��
	lexical_analyzer_.GetNextToken(token_);
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	assert(Token::DO == token_.type_);
	lexical_analyzer_.GetNextToken(token_);
	// ��ȡѭ����
	Statement(depth + 1);
	// ����ѭ�����
	--loop_level;
}

void SemanticsAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	if(loop_level <= 0)
	{
		// ����
		ErrorHandle(OUTERCONTINUE);
	}
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}
void SemanticsAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	if(loop_level <= 0)
	{
		// ����
		ErrorHandle(OUTERBREAK);
	}
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}


// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
void SemanticsAnalyzer::ProcFuncCallStatement(const Token proc_token, const vector<ExpressionAttribute> &parameter_attributes, size_t depth)	// ���̵������
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	assert(Token::LEFT_PAREN == token_.type_);

	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(parameter_attributes, depth + 1);
	}
	assert(Token::RIGHT_PAREN == token_.type_);
	lexical_analyzer_.GetNextToken(token_);

	//// �﷨�����������Ż������ĵ�һ������
	//lexical_analyzer_.GetNextToken(token_);
	//if(parameter_attributes.size() == 0)	// ������������û�в����Ļ����Ͳ��ö�������
	//{
	//	if(Token::RIGHT_PAREN != token_.type_)
	//	{
	//		// ����
	//	}
	//	lexical_analyzer_.GetNextToken(token_);	// ������������Ϻ����һ������
	//	return;
	//}
	//// �﷨�������������������ò�������Ԫʽ
	//ArgumentList(parameter_attributes, depth + 1);
	//assert(Token::RIGHT_PAREN == token_.type_);
	//lexical_analyzer_.GetNextToken(token_);

}


// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
void SemanticsAnalyzer::ArgumentList(const vector<ExpressionAttribute> &parameter_attributes, size_t depth) throw()			// ʵ�α�
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
		// Խ����
		++para_count;
		if(parameter_attributes.size() < para_count)
		{
			// ʵ�ʲ�������
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
		// ����ƥ����
		if(parameter_attributes.size() >= para_count
			&& parameter_attributes[para_count - 1].decoratetype_ < argument_decoratetype)
		{
			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
				<< "  cannot convert parameter " << para_count << " from " << TokenTableItem::DecorateTypeString[argument_decoratetype]
			<< " to " <<  TokenTableItem::DecorateTypeString[parameter_attributes[para_count - 1].decoratetype_] <<"\n";
			// ��������Ǽ���˲������Ͳ�ƥ�䣬�﷨�������ɼ������ʲ�����
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
	case REDEFINITION:	// �ض���ʱ������Ҫ����������䣬��Ϊ��Ӱ������ķ���
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "redefinition\n";
		break;
	case UNDEFINITION:	// δ����ʱ��Ҫ����������䲢���أ���Ϊ����δ���壬�������ʧЧ�����ܼ�����ǰ��ķ���
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "undefinition\n";
		break;
	case ARGUMENTNUMBERNOTMATCH:// ����������ƥ��ʱ��Ҫ����������䲢���أ���Ϊ����������ƥ�䣬���޷������жϲ������ͣ���Ҫֱ�ӷ���
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "argument number doesn't match the declaration\n";
		break;
	case WRONGTYPE:// ����������ƥ��ʱ��Ҫ����������䲢���أ���Ϊ����������ƥ�䣬���޷������жϲ������ͣ���Ҫֱ�ӷ���
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << errinfo << "\n";
		break;
	case OUTERCONTINUE:	// ��Ӱ��������������������������
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"continue\"\n";
		break;
	case OUTERBREAK:// ��Ӱ��������������������������
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"break\"\n";
		break;
	//case REDEFINITION:	// �ض���ʱ������Ҫ����������䣬��Ϊ��Ӱ������ķ���
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "redefinition\n";
	//	break;
	//case UNDEFINITION:	// δ����ʱ��Ҫ����������䲢���أ���Ϊ����δ���壬�������ʧЧ�����ܼ�����ǰ��ķ���
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "undefinition\n";
	//	break;
	//case ARGUMENTNUMBERNOTMATCH:// ����������ƥ��ʱ��Ҫ����������䲢���أ���Ϊ����������ƥ�䣬���޷������жϲ������ͣ���Ҫֱ�ӷ���
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "argument number doesn't match the declaration\n";
	//	break;
	//case WRONGTYPE:// ����������ƥ��ʱ��Ҫ����������䲢���أ���Ϊ����������ƥ�䣬���޷������жϲ������ͣ���Ҫֱ�ӷ���
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << errinfo << "\n";
	//	break;
	//case OUTERCONTINUE:	// ��Ӱ��������������������������
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"continue\"\n";
	//	break;
	//case OUTERBREAK:// ��Ӱ��������������������������
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "\terror token: " << token_.toString() << "\nerror info: " << "no loop body found around \"break\"\n";
	//	break;
	default:
		// δ֪����
		assert(false);
		break;
	}
}