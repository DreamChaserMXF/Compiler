#include "SyntaxAnalyzer.h"
#include "SyntaxException.h"
#include "TokenTableItem.h"
#include "TokenTableException.h"
#include "Quaternary.h"
#include "ExpressionAttribute.h"
#include <assert.h>
#include <sstream>
#include <algorithm>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer, 
								const vector<string> &stringtable, 
								TokenTable &tokentable, 
								vector<Quaternary> &quaternarytable) 
								throw()
	: lexical_analyzer_(lexical_analyzer), stringtable_(stringtable), tokentable_(tokentable), quaternarytable_(quaternarytable), 
	token_(), level_(1), tempvar_index_(0), label_index_(0), continue_label_(), break_label_(), 
	is_successful_(true), syntax_info_buffer_(), syntax_assist_buffer_(), tokenbuffer_()
{
	//// ������������BEGIN
	//Quaternary q_mainbegin(
	//	Quaternary::BEGIN,
	//	Quaternary::NIL_ADDRESSING, 0,
	//	Quaternary::IMMEDIATE_ADDRESSING, 0,
	//	Quaternary::IMMEDIATE_ADDRESSING, -1
	//	);
	//quaternarytable_.push_back(q_mainbegin);
}


bool SyntaxAnalyzer::Parse() throw()
{
	int depth = 0;
	syntax_assist_buffer_.clear();
	syntax_assist_buffer_.clear();
	PrintFunctionFrame("Parse()", depth);
	lexical_analyzer_.GetNextToken(token_);
	Routine(depth + 1);
	return is_successful_;
}

string SyntaxAnalyzer::toString() const throw()
{
	return syntax_info_buffer_.str();
}

bool SyntaxAnalyzer::Print(const string &filename) const throw()
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

void SyntaxAnalyzer::Print(std::ostream &output) const throw()
{
	output << syntax_info_buffer_.str() << std::endl;
}

static string syntax_assist_buffer_;	// ע�������̰߳�ȫ��
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, int depth) throw()
{
	
	if(depth * 4 == syntax_assist_buffer_.size())
	{
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_assist_buffer_.size())
	{
		syntax_assist_buffer_.append("|");
		syntax_assist_buffer_.append(depth * 4 - syntax_assist_buffer_.size(), ' ');	// ���ﲻ�ܼ�1
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
	else // depth * 4 < syntax_assist_buffer_.size()
	{
		syntax_assist_buffer_.resize(depth * 4);
		syntax_info_buffer_ << syntax_assist_buffer_ << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SyntaxAnalyzer::Routine(int depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// ������������BEGIN
	Quaternary q_mainbegin(
		Quaternary::BEGIN,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, -1
	);
	quaternarytable_.push_back(q_mainbegin);
	// �����ֳ���
	SubRoutine(depth + 1);
	// �жϽ�������
	if(token_.type_ != Token::PERIOD)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << '\t' << "should be '.' at the end of Routine\n";
		is_successful_ = false;
	}
	// ������������END
	Quaternary q_mainend(
		Quaternary::END,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, -1
	);
	quaternarytable_.push_back(q_mainend);
	// ���¹���/������BEGIN����е���ʱ��������
	Quaternary::UpdateTempVarSpace(quaternarytable_);
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SyntaxAnalyzer::SubRoutine(int depth) throw()
{
	PrintFunctionFrame("SubRoutine()", depth);

	//
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
	if(token_.type_ == Token::BEGIN)
	{
		StatementBlockPart(depth + 1);
	}
	else
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  didn't find sentence block in subroutine\n";
		return;
	}
}

// <����˵������> ::= const<��������>{,<��������>};
void SyntaxAnalyzer::ConstantPart(int depth) throw()
{
	PrintFunctionFrame("ConstantPart()", depth);

	assert(Token::CONST == token_.type_);

	// ��������
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		constantDefination(depth + 1);
	} while(token_.type_ == Token::COMMA);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ';' after constant definition\n";
		/*while(lexical_analyzer_.GetNextToken(token_) && token_.type_ != Token::SEMICOLON)
		{ }*/
		is_successful_ = false;
		if(Token::VAR == token_.type_ || Token::PROCEDURE == token_.type_ || Token::FUNCTION == token_.type_ || Token::BEGIN == token_.type_)//	������������Ǽӷֺ��ˣ�����ֱ�ӷ��ض�����ȡ��һ������
		{
			return;	
		}
		else
		{
			while(token_.type_ != Token::NIL  && token_.type_ != Token::SEMICOLON)	// �����������ʣ���ʾ����˵�����ֻ�δ��������Ҫ������һ���ֺ�
			{
				lexical_analyzer_.GetNextToken(token_);
			};
			lexical_analyzer_.GetNextToken(token_);
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <��������> ::= <��ʶ��>��<����>
void SyntaxAnalyzer::constantDefination(int depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of constant definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��¼token_�Բ�����ű�
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::EQU)
	{
		std::cout << "line " << token_.lineNumber_ << ": " << token_.toString() << "  should be '=' after identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::CONST_INTEGER 
	&& token_.type_ != Token::CONST_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after '='\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA && token_.type_ != Token::SEMICOLON)	// ������һ�����Ż�ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �������壬������ű�
	if(tokentable_.SearchDefinitionInCurrentLevel(constIdentifier.value_.identifier))
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	else if(Token::CONST_INTEGER == token_.type_)
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::INTEGER, token_.value_.integer, level_);
	}
	else
	{
		tokentable_.AddConstItem(constIdentifier, TokenTableItem::CHAR, token_.value_.character, level_);
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= var <��������>;{<��������>;}
void SyntaxAnalyzer::VariablePart(int depth) throw()
{
	PrintFunctionFrame("VariablePart()", depth);

	assert(Token::VAR == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	do
	{
		VariableDefinition(depth + 1);
		if(token_.type_ != Token::SEMICOLON)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ';' after variable definition\n";
			is_successful_ = false;
			if(Token::IDENTIFIER == token_.type_)	// ���Ǽӷֺţ����滹�б�������
			{
				continue;
			}
			else
			{
				// ���Ǽӷֺ��ˣ��������޷��������ϱ������壬ֻ����ת����һ����
				while(Token::NIL != token_.type_ && Token::SEMICOLON != token_.type_ && Token::PROCEDURE != token_.type_ && Token::FUNCTION != token_.type_ && Token::BEGIN == token_.type_)
				{
					lexical_analyzer_.GetNextToken(token_);
				}
				if(Token::SEMICOLON == token_.type_)
				{
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
		}
		lexical_analyzer_.GetNextToken(token_);
	}while(token_.type_ == Token::IDENTIFIER);
}

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
void SyntaxAnalyzer::VariableDefinition(int depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of variable definition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER&& token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(token_.type_ != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
		{
			return;
		}
		// �����˱�ʶ���������ִ��
	}
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(token_.type_ == Token::COMMA)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be identifier at the beginning of variable definition\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::IDENTIFIER && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš���һ����ʶ����PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			if(token_.type_ != Token::IDENTIFIER)	// �������Ĳ��Ǳ�ʶ�����򷵻���һ�㴦��
			{
				return;
			}
			// �����˱�ʶ���������ִ��
		}
		tokenbuffer_.push_back(token_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ':' after identifier to specify the type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::RW_INTEGER && token_.type_ != Token::RW_CHAR && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β������˵�������ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::RW_INTEGER == token_.type_ || Token::RW_CHAR == token_.type_)	// ������������˵����
		{
			TypeSpecification(depth + 1);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <����> ::= [array'['<�޷�������>']'of]<��������>
void SyntaxAnalyzer::TypeSpecification(int depth) throw()
{
	PrintFunctionFrame("TypeSpecification()", depth);

	TokenTableItem::ItemType itemtype_ = TokenTableItem::VARIABLE;
	int arrayLength = 0;
	if(token_.type_ == Token::ARRAY)
	{
		itemtype_ = TokenTableItem::ARRAY;
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::LEFT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '[' after \"array\"\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;	// ��������ĳ�����̫���鷳����ֱ��������䣬���ؽ����
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::CONST_INTEGER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a constant integer after '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		arrayLength = token_.value_.integer;
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::OF)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" after []\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	if(token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)	// ��û������˵��
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" for type specification\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ������ű�
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;// ���η�����
	if(TokenTableItem::ARRAY == itemtype_)	// ��������
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))	// �ض������Ȼ���뵱ǰ���壨Ҳ�ɲ����룩
			{
				std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
				is_successful_ = false;
			}
			tokentable_.AddArrayItem(*iter, decoratetype_, arrayLength, level_);
		}
	}
	else									// ����һ�����
	{
		for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
		{
			if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
			{
				std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
				is_successful_ = false;
			}
			tokentable_.AddVariableItem(*iter, decoratetype_, level_);
		}
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::ProcedurePart(int depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		int proc_index = ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		// ���ɹ��̵�END��Ԫʽ
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, proc_index));
		// TODO �޸���һ��BEGIN��Ԫʽ�е���ʱ��������
//		SetTempVarCount(proc_index, max_local_temp_count_);
//		max_local_temp_count_ = 0;	// ��ʼ����������ʱ�����������
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �ֳ��������Ӧ����ֺ�
	}while(Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
int SyntaxAnalyzer::ProcedureHead(int depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// �������ڷ��ű��е�λ�ã��±꣩

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a procedure name after \"procedure\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// ������ж�λ����Ϊ�˵���ProcedurePart�е��ض�λ
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��������������ű�
	string proc_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĹ��̷ֳ�����﷨���������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	proc_index = tokentable_.AddProcedureItem(token_, level_++);// ������֮��leveҪ+1
	// ���ɹ��̵�BEGIN��Ԫʽ
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, 
		Quaternary::NIL_ADDRESSING, 0, 
		Quaternary::IMMEDIATE_ADDRESSING, 0,	// ����Ĳ�����Ӧ���Ǹù���Ҫ�õ�����ʱ�������������ȹ��̷��������
		Quaternary::IMMEDIATE_ADDRESSING, proc_index));
	// ��λ��������������ڵ�λ����Ϊ�ֲ����������ʼ�㣩
	tokentable_.Locate();	
	// ������ȡ����
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)	// û�ж��������ţ�����û�в���
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth);
		tokentable_.SetParameterCount(proc_name, parameterCount);
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  need ')' to match '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure head\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return proc_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return proc_index;
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::FunctionPart(int depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		int func_index = FunctionHead(depth + 1);	// ���к���ͷ���������õ��������ڷ��ű��е�λ��
		SubRoutine(depth + 1);
		// ���ɺ�����END��Ԫʽ
		quaternarytable_.push_back(Quaternary(Quaternary::END, Quaternary::NIL_ADDRESSING, 0, Quaternary::NIL_ADDRESSING, 0, Quaternary::IMMEDIATE_ADDRESSING, func_index));
		// TODO �޸���һ��BEGIN��Ԫʽ�е���ʱ��������
//		SetTempVarCount(func_index, max_local_temp_count_);
//		max_local_temp_count_ = 0;	// ��ʼ����������ʱ��������±�
		tokentable_.Relocate();
		--level_;
		if(Token::SEMICOLON != token_.type_)	// �ֳ��������Ӧ����ֺ�
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of procedure\n";
			is_successful_ = false;
		}
		lexical_analyzer_.GetNextToken(token_);	// �����β�ķֺ�
	}while(Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
int SyntaxAnalyzer::FunctionHead(int depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// �������ڷ��ű��е�λ��

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be a function name after \"function\"\n";
		is_successful_ = false;
		tokentable_.Locate();	// ������ж�λ����Ϊ�˵���FunctionPart�е��ض�λ
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ���뺯���������ű�
	string func_name = token_.value_.identifier;
	if(tokentable_.SearchDefinitionInCurrentLevel(token_.value_.identifier))	// �ض���ʱ����Ȼ�����ض���Ĺ��̶��壨��Ϊ��Ȼ�����Ӱ�첻�󣬶�������Ļ�����Ӱ��ôεĺ����ֳ�����﷨���������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
	}
	func_index = tokentable_.AddFunctionItem(token_, level_++);// ������֮��leveҪ+1
	// ���ɺ�����BEGIN��Ԫʽ
	quaternarytable_.push_back(Quaternary(Quaternary::BEGIN, 
		Quaternary::NIL_ADDRESSING, 0, 
		Quaternary::IMMEDIATE_ADDRESSING, 0, // ����Ĳ�����Ӧ���Ǹú���Ҫ�õ�����ʱ�������������Ⱥ������������
		Quaternary::IMMEDIATE_ADDRESSING, func_index));
	// ��λ
	tokentable_.Locate();
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  redifinition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		int parameterCount = ParameterList(depth + 1);
		tokentable_.SetParameterCount(func_name, parameterCount); 
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  need ')' to match '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)	// ����������ð��
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ':' after Parameter Statement\n";
		is_successful_ = false;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" after ':' to specify the return type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::SEMICOLON == token_.type_)	// �ֺź��ٶ�һ�����ʣ����ܻ����ֳ���
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return func_index;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	tokentable_.SetFunctionReturnType(func_name, decoratetype_);
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// �����п��������˷ֺţ����Բ��ټ�����
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ';' at the end of function head\n";
		is_successful_ = false;
		return func_index;
	}
	lexical_analyzer_.GetNextToken(token_);
	return func_index;
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
int SyntaxAnalyzer::ParameterList(int depth) throw()		// �βα�
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
int SyntaxAnalyzer::ParameterTerm(int depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);

	if(Token::VAR == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::IDENTIFIER)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be parameter name surrounded by parentheses\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::COMMA  && token_.type_ != Token::SEMICOLON)	// ������β���ֺš�BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::IDENTIFIER != token_.type_)
		{
			return 0;
		}
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ������ѹջ׼�������ű�
	tokenbuffer_.clear();
	tokenbuffer_.push_back(token_);
	lexical_analyzer_.GetNextToken(token_);
	int sum = 1;
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be parameter name after ','\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON  && token_.type_ != Token::IDENTIFIER)	// ������β���ֺš�BEGIN
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			if(Token::IDENTIFIER != token_.type_)
			{
				return 0;
			}
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		tokenbuffer_.push_back(token_);
		++sum;
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  lost ':' to specify the type after parameter name\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return 0;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"integer\" or \"char\" after ':' to specify the parameter type\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::PROCEDURE && token_.type_ != Token::FUNCTION && token_.type_ != Token::BEGIN)	// ������β���ֺš�PROCEDURE FUNCTION BEGIN
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return 0;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	// ��������ű�
	TokenTableItem::DecorateType decoratetype_ = (token_.type_ == Token::RW_INTEGER) ? TokenTableItem::INTEGER : TokenTableItem::CHAR;
	for(vector<Token>::const_iterator iter = tokenbuffer_.begin(); iter != tokenbuffer_.end(); ++iter)
	{
		if(tokentable_.SearchDefinitionInCurrentLevel(iter->value_.identifier))
		{
			std::cout << "line " << iter->lineNumber_ << ":  " << iter->toString() << "  redifinition\n";
			is_successful_ = false;
		}
		tokentable_.AddParameterItem(*iter, decoratetype_, level_);
	} // end ����ű�

	lexical_analyzer_.GetNextToken(token_);
	return sum;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
vector<TokenTableItem::DecorateType> SyntaxAnalyzer::ArgumentList(int depth) throw()			// ʵ�α�
{
	PrintFunctionFrame("ArgumentList()", depth);

	vector<TokenTableItem::DecorateType> attribute_buffer;
	ExpressionAttribute argument_attribute = Expression(depth + 1);
	attribute_buffer.push_back(argument_attribute.decoratetype_);
	// �������ò�������Ԫʽ
	Quaternary q_addpara(Quaternary::SETP, 
		Quaternary::NIL_ADDRESSING, 0, 
		argument_attribute.offset_addressingmethod_, argument_attribute.offset_, 
		argument_attribute.addressingmethod_, argument_attribute.value_);
	quaternarytable_.push_back(q_addpara);
	// ������ʱ����
	if(Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == argument_attribute.offset_addressingmethod_)	// ���߲�����ͬʱ��������д��һ��
	{
		--tempvar_index_;
	}

	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		argument_attribute = Expression(depth + 1);
		attribute_buffer.push_back(argument_attribute.decoratetype_);
		// �������ò�������Ԫʽ
		q_addpara.method2_ = argument_attribute.offset_addressingmethod_;
		q_addpara.offset2_ = argument_attribute.offset_;
		q_addpara.method3_ = argument_attribute.addressingmethod_;
		q_addpara.dst_ = argument_attribute.value_;
		quaternarytable_.push_back(q_addpara);
		// ������ʱ����
		if(Quaternary::TEMPORARY_ADDRESSING == argument_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == argument_attribute.offset_addressingmethod_)	// ���߲�����ͬʱ��������д��һ��
		{
			--tempvar_index_;
		}
	}
	return attribute_buffer;
}

// <�������> ::= begin <���>{;<���>} end
void SyntaxAnalyzer::StatementBlockPart(int depth) throw()	// �������
{
	PrintFunctionFrame("StatementBlockPart()", depth);

	assert(Token::BEGIN == token_.type_);
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}while(token_.type_ == Token::SEMICOLON);
	if(token_.type_ != Token::END)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"end\" at the end of Statement Block\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::END == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>
// |<�����>|<д���>|<whileѭ�����>|<forѭ�����>|<ѭ���������>|<ѭ���˳����>|<��>
void SyntaxAnalyzer::Statement(int depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// ��token_�����ǹ��������ȼ��£�����
	TokenTable::iterator iter;
	TokenTableItem::DecorateType l_value_type = TokenTableItem::VOID;
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		iter = tokentable_.SearchDefinition(token_);	// ���ҷ��ű��еĶ���
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// ���̵���
		{
			if(iter->itemtype_ != TokenTableItem::PROCEDURE)	// ����������Ƿ�Ϊ����
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a procedure\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return;
			}
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			
			// ��ProcedureCallStatement�����ò���
			ProcedureCallStatement(idToken, decorate_types, depth + 1);
			// ���ɹ��̵�����Ԫʽ
			Quaternary q_procedurecall(Quaternary::PROC_CALL, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::IMMEDIATE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_procedurecall);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(idToken, iter, depth + 1);		
		}
		else if(Token::COLON == token_.type_)	// �˷�֧ר��Ϊ�˼�鸳ֵ��д������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  please check the spelling of the assigning operator\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		else
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error after identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
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
		// ��������Ƿ�Ϸ���Ӧ�úϷ���
	case Token::SEMICOLON:	// �����
	case Token::END:		// �����
	default:
		break;
	//default:
	//	std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  syntax error at the beginning of Statement\n";
	//	is_successful_ = false;
	//	while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
	//	{ 
	//		lexical_analyzer_.GetNextToken(token_);
	//	}
	//	break;
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void SyntaxAnalyzer::AssigningStatement(const Token &idToken, TokenTable::iterator &iter, int depth)			// ��ֵ���
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// Ϊ��Ԫʽ���ɶ�����ı���
	bool assign2array= false;	// �Ƿ�Ϊ������ĸ�ֵ����
	
	ExpressionAttribute offset_attribute;	// ��������Ԫ�ظ�ֵʱ���洢ƫ�����������±꣩������

	if(Token::LEFT_BRACKET == token_.type_)	// ������Ԫ�ظ�ֵ
	{
		assign2array = true;
		// ������
		if(iter->itemtype_ != TokenTableItem::ARRAY)	// ����Ƿ�Ϊ������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		// �����ʾ�±�ı��ʽ
		lexical_analyzer_.GetNextToken(token_);
		offset_attribute = Expression(depth + 1);
		// �������±���������Ԫ��
		// �������Ԫʽ���������±�ֵ����һ����ʱ����
		SimplifyArrayOperand(offset_attribute);
		// �﷨���
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	// �����飺ʣ��ֻ������������������������Ǻ�������ֵ
	else if(iter->itemtype_ != TokenTableItem::VARIABLE
		&& iter->itemtype_ != TokenTableItem::PARAMETER
		&& iter->itemtype_ != TokenTableItem::FUNCTION)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �﷨���
	if(token_.type_ != Token::ASSIGN)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  \":=\" doesn't occur in the Assigning Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��������Ϊ�׳��쳣��׼��������GetNextTokenǰ��Ϊ��׼ȷ��¼��ֵ�ŵ��к�
	static Token errToken;
	errToken.type_ = Token::ASSIGN;
	errToken.lineNumber_ = token_.lineNumber_;

	// ���븳ֵ���ұߵı��ʽ
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);

	// �����飺����ת��
	if(iter->decoratetype_ < right_attribute.decoratetype_)	// С�ڱ�ʾ���ܴ��������ת��
	{
		std::cout << "warning: line " << idToken.lineNumber_ << ":  " << idToken.toString() 
			<< " convert from " << TokenTableItem::DecorateTypeString[right_attribute.decoratetype_] 
			<< " to " << TokenTableItem::DecorateTypeString[iter->decoratetype_] << "\n";
	}

	// �м��������
	if(assign2array)	// ������Ԫ�ظ�ֵ
	{
		// ���right_attribute������Ԫ�صĻ���Ҫ���丳ֵ����ʱ����
		SimplifyArrayOperand(right_attribute);
		// �������鸳ֵ
		Quaternary q_asg;
		q_asg.op_ = Quaternary::AASG;
		q_asg.method1_ = right_attribute.addressingmethod_;
		q_asg.src1_ = right_attribute.value_;
		q_asg.method2_ = offset_attribute.addressingmethod_;
		q_asg.offset2_ = offset_attribute.value_;
		q_asg.method3_ = Quaternary::ARRAY_ADDRESSING;
		q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
		quaternarytable_.push_back(q_asg);

		// ����Ҳ���������ʱ�������ɻ���
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
		// �������ֵ�������±�Ҳ����ʱ�������ɻ���
		if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
	}
	else if(TokenTableItem::PARAMETER == iter->itemtype_
			|| TokenTableItem::VARIABLE == iter->itemtype_)	// ��ͨ����/�����ĸ�ֵ
	{
		// ����Ҳ���������ʱ���������Ż�����ǰ�ĸ�ֵ��䡣�����Appendix1 ��Ʊ�ע��
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			Quaternary &q_last = quaternarytable_.back();
			q_last.method3_ = Quaternary::VARIABLE_ADDRESSING;
			q_last.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			//quaternarytable_.pop_back();
			//quaternarytable_.push_back(q_last);
			// �����Ҳ���������ʱ����
			--tempvar_index_;
			// �����max_local_temp_count_���Ż��ǲ�׼ȷ��
			// ��Ϊ��ʹ���ε���ʱ�������Ż����ˣ���������ǰ���õ�����ʱ����
			// ���Բ����������Ż�max_local_temp_count_
			//// ��飬�Ƿ���Ϊ�Ż���������һ����ʱ����
			//// ����ǵĻ���Ҫ��max_local_temp_count_���и���
			//if((Quaternary::TEMPORARY_ADDRESSING != q_last.method1_ || tempvar_index_ != q_last.src1_)		// ��src1���������ǻ��յ���ʱ����
			//	&& (Quaternary::TEMPORARY_ADDRESSING != q_last.method2_ || tempvar_index_ != q_last.src2_))	// ��src2���������ǻ��յ���ʱ����
			//{
			//	// ˵�����յ���ʱ��������һ����Ԫʽ�������
			//	--max_local_temp_count_;
			//}
		}
		else
		{
			Quaternary q_asg;
			q_asg.op_ = Quaternary::ASG;
			q_asg.method1_ = right_attribute.addressingmethod_;
			q_asg.src1_ = right_attribute.value_;
			q_asg.method2_ = right_attribute.offset_addressingmethod_;
			q_asg.offset2_ = right_attribute.offset_;
			q_asg.method3_ = Quaternary::VARIABLE_ADDRESSING;
			q_asg.dst_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			quaternarytable_.push_back(q_asg);
			//// ����Ҳ���������ʱ�������ɻ���
			//if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
			//{
			//	--tempvar_index_;
			//}
			// ����Ҳ����������飬���������±�����ʱ�������ɻ���
			if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_)
			{
				// ������һ������³���Եļٶ��������addressingmethod_����ARRAY�Ļ�����ôaddressingmethod_һ����NIL_ADDRESSING
				// �����������assert���һ�³����߼���������
				assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
				--tempvar_index_;
			}
		}
	}
	else	// ��������ֵ
	{
		Quaternary q_ret;
		q_ret.op_ = Quaternary::RET;
		q_ret.method2_ = right_attribute.offset_addressingmethod_;
		q_ret.offset2_ = right_attribute.offset_;
		q_ret.method3_ = right_attribute.addressingmethod_;
		q_ret.dst_ = right_attribute.value_;
		quaternarytable_.push_back(q_ret);
		// ����Ҳ���������ʱ�������ɻ���
		if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
		{
			--tempvar_index_;
		}
		// ����Ҳ����������飬���������±�����ʱ�������ɻ���
		else if(Quaternary::TEMPORARY_ADDRESSING == offset_attribute.offset_addressingmethod_)
		{
			// ������һ������³���Եļٶ��������addressingmethod_����ARRAY�Ļ�����ôaddressingmethod_һ����NIL_ADDRESSING
			// �����������assert���һ�³����߼���������
			assert(Quaternary::ARRAY_ADDRESSING == offset_attribute.addressingmethod_);
			--tempvar_index_;
		}
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
ExpressionAttribute SyntaxAnalyzer::Expression(int depth) throw()				// ���ʽ
{
	PrintFunctionFrame("Expression()", depth);

	Quaternary q_neg;	// �������ɵ�NEG��Ԫʽ
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		if(Token::MINUS == token_.type_)	// ����Ǽ��ţ���Ҫ����һ����Ԫʽ
		{
			q_neg.op_ = Quaternary::NEG;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	ExpressionAttribute first_term = Term(depth + 1);
	if(Quaternary::NEG == q_neg.op_)	// ���֮ǰ������һ������
	{
		// ����ȡ�����Ż�
		if(Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_)
		{
			first_term.value_ = -first_term.value_;
		}
		else		// ����NEG����Ԫʽ
		{
			q_neg.method2_ = first_term.offset_addressingmethod_;
			q_neg.offset2_ = first_term.offset_;
			q_neg.method3_ = first_term.addressingmethod_;
			q_neg.dst_ = first_term.value_;
			quaternarytable_.push_back(q_neg);
		}
	}

	Quaternary q_term;
	ExpressionAttribute new_term;
	bool is_first_operator = true;
	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// ��һ��ʱ�����new_term������Ԫ�أ���Ҫ���丳ֵ����ʱ����
		if(is_first_operator)
		{
			SimplifyArrayOperand(first_term);
		}

		// ȷ����Ԫʽ�Ĳ�����
		q_term.op_ = Token::PLUS == token_.type_ ? Quaternary::ADD : Quaternary::SUB;
		
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_term_decoratetype = is_first_operator ? first_term.decoratetype_ : new_term.decoratetype_;
		new_term = Term(depth + 1);

		// ���������ִ������ת��
		new_term.decoratetype_ = TokenTableItem::TypeConversionMatrix[last_term_decoratetype][new_term.decoratetype_];

		// ���������new_term��������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
		// ������Ԫ�ص�ֵ������ʱ����
		SimplifyArrayOperand(new_term);

		// ȷ����Ԫʽ�Ĳ�����
		// src1��ȷ����
		// ��һ�μ�/��ʱ��src1����while֮ǰ������Ǹ�term
		// ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
		if(is_first_operator)
		{
			// �����������/�����Ż���ֱ���ڱ���ʱ������
			if(Quaternary::IMMEDIATE_ADDRESSING == first_term.addressingmethod_
				&& Quaternary::IMMEDIATE_ADDRESSING == new_term.addressingmethod_)
			{
				first_term.decoratetype_ = new_term.decoratetype_;
				first_term.value_ = (Quaternary::ADD == q_term.op_) ? 
						(first_term.value_ + new_term.value_) : (first_term.value_ - new_term.value_);
				continue;
			}
			// ��������
			q_term.method1_ = first_term.addressingmethod_;
			q_term.src1_ = first_term.value_;
			is_first_operator = false;
		}
		else
		{
			q_term.method1_ = q_term.method3_;
			q_term.src1_ = q_term.dst_;
		}
		// src2��ȷ����
		// src2���Ƕ������µ�term
		q_term.method2_ = new_term.addressingmethod_;
		q_term.src2_ = new_term.value_;
		// dst��ȷ����
		// ���src1����ʱ����������dstΪsrc1
		// �������src2����ʱ����������dstΪsrc2
		// ������dstΪ�µ���ʱ����
		if(Quaternary::TEMPORARY_ADDRESSING == q_term.method1_)
		{
			q_term.method3_ = q_term.method1_;
			q_term.dst_ = q_term.src1_;
			// ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
			if(Quaternary::TEMPORARY_ADDRESSING == q_term.method2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_ADDRESSING == q_term.method2_)
		{
			q_term.method3_ = q_term.method2_;
			q_term.dst_ = q_term.src2_;
		}
		else
		{
			q_term.method3_ = Quaternary::TEMPORARY_ADDRESSING;
			q_term.dst_ = tempvar_index_++;
			//// ���������ʱ��������
			//if(tempvar_index_ > max_local_temp_count_)
			//{
			//	max_local_temp_count_ = tempvar_index_;
			//}
		}
		// ������Ԫʽ
		quaternarytable_.push_back(q_term);
	}

	

	// �������յı��ʽ����
	if(is_first_operator)	// ֻ��һ������
	{
		new_term = first_term;
	}
	else	// �ж���������Ҫ����new_term�����ԡ�����new_term����decoratetype_�⣬���඼�����һ�������
	{
		new_term.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		new_term.value_ = q_term.dst_;
		new_term.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		new_term.offset_ = 0;
	}
	return new_term;
}

// <��> ::= <����>{<�˷������><����>}
ExpressionAttribute SyntaxAnalyzer::Term(int depth) throw()						// ��
{
	PrintFunctionFrame("Term()", depth);

	ExpressionAttribute first_factor = Factor(depth + 1);

	ExpressionAttribute new_factor;
	Quaternary q_factor;
	bool is_first_operator = true;
	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		// ��һ��ʱ�����first_factor������Ԫ�أ���Ҫ���丳ֵ����ʱ����
		if(is_first_operator)
		{
			SimplifyArrayOperand(first_factor);
		}

		// ȷ����Ԫʽ�Ĳ�����
		q_factor.op_ = Token::MUL == token_.type_ ? Quaternary::MUL : Quaternary::DIV;

		// �﷨��������ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		TokenTableItem::DecorateType last_factor_decoratetype = is_first_operator ? first_factor.decoratetype_ : new_factor.decoratetype_;
		new_factor = Factor(depth + 1);
		// ���������ִ������ת��
		new_factor.decoratetype_ = TokenTableItem::TypeConversionMatrix[last_factor_decoratetype][new_factor.decoratetype_];

		// ����������������Ԫ�أ���ô��Ȼ��Ҫһ��ת��
		// ������Ԫ�ص�ֵ������ʱ����
		SimplifyArrayOperand(new_factor);

		// ȷ����Ԫʽ�Ĳ�����
		// src1��ȷ����
		// ��һ�γ�/����src1����while֮ǰ������Ǹ�factor
		// ֮���/��ʱ��src1������һ����Ԫʽ�Ľ��
		if(is_first_operator)
		{
			// �����������/�����Ż���ֱ���ڱ���ʱ������
			if(Quaternary::IMMEDIATE_ADDRESSING == first_factor.addressingmethod_
				&& Quaternary::IMMEDIATE_ADDRESSING == new_factor.addressingmethod_)
			{
				first_factor.decoratetype_ = new_factor.decoratetype_;
				first_factor.value_ = (Quaternary::MUL == q_factor.op_) ? 
						(first_factor.value_ * new_factor.value_) : (first_factor.value_ / new_factor.value_);
				continue;
			}
			// ��������
			q_factor.method1_ = first_factor.addressingmethod_;
			q_factor.src1_ = first_factor.value_;
			is_first_operator = false;
		}
		else
		{
			q_factor.method1_ = q_factor.method3_;
			q_factor.src1_ = q_factor.dst_;
		}
		// src2��ȷ����
		// src2�Ƕ������µ�factor
		q_factor.method2_ = new_factor.addressingmethod_;
		q_factor.src2_ = new_factor.value_;
		// dst��ȷ����
		// ���src1����ʱ����������dstΪsrc1
		// �������src2����ʱ����������dstΪsrc2
		// ������dstΪ�µ���ʱ����
		if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method1_)
		{
			q_factor.method3_ = q_factor.method1_;
			q_factor.dst_ = q_factor.src1_;
			// ��ʱ�����src2Ҳ����ʱ��������ô�Ϳ�����ִ���������Ԫʽ�󣬰������ʱ�����ı�Ż���
			if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_)
			{
				--tempvar_index_;
			}
		}
		else if(Quaternary::TEMPORARY_ADDRESSING == q_factor.method2_)
		{
			q_factor.method3_ = q_factor.method2_;
			q_factor.dst_ = q_factor.src2_;
		}
		else
		{
			q_factor.method3_ = Quaternary::TEMPORARY_ADDRESSING;
			q_factor.dst_ = tempvar_index_++;
		}
		// ������Ԫʽ
		quaternarytable_.push_back(q_factor);
	}

	// �����������
	if(is_first_operator)	// ֻ��һ������
	{
		new_factor = first_factor;
	}
	else
	{
		// ����new_factor������
		// ����new_factor��decoratetype�⣬�������Ծ����������һ�����ӵ�����
		new_factor.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		new_factor.value_ = q_factor.dst_;
		new_factor.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		new_factor.offset_ = 0;
	}
	return new_factor;
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
ExpressionAttribute SyntaxAnalyzer::Factor(int depth) throw()					// ����
{
	PrintFunctionFrame("Factor()", depth);
	ExpressionAttribute factor_attribute;	// ��¼��factor���ӵ���Ϣ

	// �﷨��飺��ʶ��������������������������顢�������á�
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		// ������
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);	// Ѱ�Ҷ���
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return factor_attribute;
		}
		// ���壺��¼�������ͣ������·��ű�
		factor_attribute.decoratetype_ = iter->decoratetype_;	
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		Token idToken = token_;	// ���£�����
		lexical_analyzer_.GetNextToken(token_);
		// �﷨���
		if(Token::LEFT_BRACKET == token_.type_)	// �������ţ�����Ԫ��
		{
			// factor_attribute�Լ���������ֵ
			factor_attribute.addressingmethod_ = Quaternary::ARRAY_ADDRESSING;
			factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
			// �����飺�Ƿ�Ϊ������
			if(iter->itemtype_ != TokenTableItem::ARRAY)	
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  subscript requires array or pointer type\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// �﷨��������Ϊ�±�ı��ʽ
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute offset_attribute = Expression(depth + 1);
			// �м��������
			// ȷ��factor_attribute���±�
			// �����offset_attribute����������Ԫ�أ�����ṹ��Ƕ�׵������±꣨�������±�����һ������Ԫ�أ����޷��������Ԫʽ
			// ��������飬��Ҫ��offset_attribute�浽��ʱ�����У���Ϊ��ǰfactor���±�
			SimplifyArrayOperand(offset_attribute);
			factor_attribute.offset_addressingmethod_ = offset_attribute.addressingmethod_;
			factor_attribute.offset_ = offset_attribute.value_;
			// �﷨���
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ']' to match '['\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// �����ţ���������
		{
			// �����飺�Ƿ�Ϊ����
			if(iter->itemtype_ != TokenTableItem::FUNCTION)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not declared as a function\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// ���壺����ƥ��
			// �ӷ��ű���ȡ�������Ĳ������ͣ���FunctionCallStatementȥƥ�����
			vector<TokenTableItem::DecorateType> decorate_types = tokentable_.GetProcFuncParameter(iter);
			// ��Ԫʽ���Ƚ���FunctionCallStatement�����úò�����Ȼ�������ɺ����������
			FunctionCallStatement(idToken, decorate_types, depth + 1);
			// ���ɺ������õ���Ԫʽ
			Quaternary q_functioncall(Quaternary::FUNC_CALL, 
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
			quaternarytable_.push_back(q_functioncall);
			// ���������������У�ϣ���������ķ���ֵ������temp#tempvar_index��λ��
			// �����Ӻ����У��޷��ж�temp#tempvar_index��λ��
			// �����Ӻ���������ֵ�洢��EAX�У��ټ�һ��ָ���EAX��ֵ�����ʱ������
			Quaternary q_store(Quaternary::STORE, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0, 
				Quaternary::TEMPORARY_ADDRESSING, tempvar_index_++);
			quaternarytable_.push_back(q_store);
			// ����factor_attribute����������ʱ����
			factor_attribute.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
			factor_attribute.value_ = q_store.dst_;
			factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
			factor_attribute.offset_ = 0;
		}
		else	// ����һ����ʶ��
		{
			// �����飺�Ƿ�Ϊ���������������/�����Ĳ���
			if(iter->itemtype_ != TokenTableItem::VARIABLE && iter->itemtype_ != TokenTableItem::PARAMETER && iter->itemtype_ != TokenTableItem::CONST)
			{
				std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  single token Factor should be varaible or constant\n";
				is_successful_ = false;
				while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
				{ 
					lexical_analyzer_.GetNextToken(token_);
				}
				return factor_attribute;
			}
			// factor_attribute������
			if(TokenTableItem::CONST == iter->itemtype_)	// ������
			{
				// ����ֱ�ӽ�������ת�������������ͣ������Appendix1 ��Ʊ�ע��
				factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
				factor_attribute.value_ = iter->value_;
				factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
				factor_attribute.offset_ = 0;
			}
			else	// һ�����
			{
				factor_attribute.addressingmethod_ = Quaternary::VARIABLE_ADDRESSING;
				factor_attribute.value_ = std::distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter));
				factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
				factor_attribute.offset_ = 0;
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
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return factor_attribute;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)	// �������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute.decoratetype_ = TokenTableItem::INTEGER;
		factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		factor_attribute.value_ = token_.value_.integer;
		factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// �ַ������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		factor_attribute.decoratetype_ = TokenTableItem::CHAR;
		factor_attribute.addressingmethod_ = Quaternary::IMMEDIATE_ADDRESSING;
		factor_attribute.value_ = token_.value_.character;
		factor_attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		factor_attribute.offset_ = 0;
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// �﷨��������
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  not a legal Factor\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
	}
	return factor_attribute;
}

// <�������> ::= if<����>then<���>[else<���>]
void SyntaxAnalyzer::IfStatement(int depth) throw()				// �������
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// ������һ��label
	int label1 = label_index_++;
	// ��ȡ�������
	lexical_analyzer_.GetNextToken(token_);
	Condition(label1, depth + 1);	// ��condition��������ת���
	if(Token::THEN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"then\" after Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡif�ɹ�������
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// ��ȡelse�����
	if(Token::ELSE == token_.type_)
	{
		// ����ڶ���label
		int label2 =  label_index_++;
		// ������������ת���
		Quaternary q_jmp(Quaternary::JMP, 
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, label2);
		quaternarytable_.push_back(q_jmp);
		// ���õ�һ��label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, label1);
		quaternarytable_.push_back(q_label1);
		// ��ȡelse�е����
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
		// ���õڶ���label
		Quaternary q_label2(Quaternary::LABEL, 
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, label2);
		quaternarytable_.push_back(q_label2);
	}
	else	// ���û��else��䣬����if���������ʱ�����õ�һ��label
	{
		// ���õ�һ��label
		Quaternary q_label1(Quaternary::LABEL, 
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::NIL_ADDRESSING, 0,
				Quaternary::IMMEDIATE_ADDRESSING, label1);
		quaternarytable_.push_back(q_label1);
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>
// ��if��for����д�������label��������ʶif�����forѭ����Ľ���
// �����ڴ���conditionʱ������ת���
void SyntaxAnalyzer::Condition(int endlabel, int depth) throw()				// ����
{
	PrintFunctionFrame("Condition()", depth);

	ExpressionAttribute left_attribute = Expression(depth + 1);
	// ��������Ԫ��Ϊ��ʱ����
	SimplifyArrayOperand(left_attribute);
	if(		Token::LT	!= token_.type_
		&&	Token::LEQ	!= token_.type_
		&&	Token::GT	!= token_.type_
		&&	Token::GEQ	!= token_.type_
		&&	Token::EQU	!= token_.type_
		&&	Token::NEQ	!= token_.type_
		)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be logic operator in the middle of Condition\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	Token compare_token = token_;
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute right_attribute = Expression(depth + 1);
	// ��������Ϊ��ʱ����
	SimplifyArrayOperand(right_attribute);

	// ��������������ת���
	// ����������ʱ�Ż���ת����������Ĳ������������tokenҪ��һ��
	Quaternary q_jmp_condition;
	switch(compare_token.type_)
	{
	case Token::LT:
		q_jmp_condition.op_ = Quaternary::JNL;
		break;
	case Token::LEQ:
		q_jmp_condition.op_ = Quaternary::JG;
		break;
	case Token::GT:
		q_jmp_condition.op_ = Quaternary::JNG;
		break;
	case Token::GEQ:
		q_jmp_condition.op_ = Quaternary::JL;
		break;
	case Token::EQU:
		q_jmp_condition.op_ = Quaternary::JNE;
		break;
	case Token::NEQ:
		q_jmp_condition.op_ = Quaternary::JE;
		break;
	// ��Ϊ֮ǰ�Ѿ������ˣ�������������²�������default
	default:
		assert(false);
		break;
	}
	// ������
	q_jmp_condition.method1_ = left_attribute.addressingmethod_;
	q_jmp_condition.src1_ = left_attribute.value_;
	q_jmp_condition.method2_ = right_attribute.addressingmethod_;
	q_jmp_condition.src2_ = right_attribute.value_;
	q_jmp_condition.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
	q_jmp_condition.dst_ = endlabel;
	// ������Ԫʽ
	quaternarytable_.push_back(q_jmp_condition);
	// ������ʱ����
	if(Quaternary::TEMPORARY_ADDRESSING == left_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_ADDRESSING == right_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SyntaxAnalyzer::CaseStatement(int depth) throw()			// ������
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute exp_attribute = Expression(depth + 1);
	// �����ʽ��Ϊ������Ԫ��
	SimplifyArrayOperand(exp_attribute);

	if(Token::OF != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"of\" to specify the certain case\n";
		is_successful_ = false;
		// �������������дof���ʲ�����
	}
	// ΪEND������һ��label
	int endlabel = label_index_++;
	// case���ʽ֮�����ת���Ĳ���λ��
	int jmp_insertion_location = quaternarytable_.size();
	// ���ɸ�JE����ת���
	Quaternary q_jmp;
	q_jmp.op_ = Quaternary::JE;
	q_jmp.method1_ = exp_attribute.addressingmethod_;
	q_jmp.src1_ = exp_attribute.value_;
	q_jmp.method2_ = Quaternary::IMMEDIATE_ADDRESSING;
	q_jmp.method3_ = Quaternary::IMMEDIATE_ADDRESSING;
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		// Ϊ��ǰ�����Ԫ������һ��label
		int caselabel = label_index_++;
		vector<int> constant_list = CaseElement(exp_attribute, caselabel, endlabel, depth + 1);
		// ����JE��ת���
		q_jmp.dst_ = caselabel;
		for(vector<int>::const_iterator c_iter = constant_list.begin();
			c_iter != constant_list.end(); ++c_iter)
		{
			q_jmp.src2_ = *c_iter;
			quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location++, q_jmp);
		}
	}while(Token::SEMICOLON == token_.type_);

	// ִ��������JE����ת��END��
	Quaternary q_endjmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.insert(quaternarytable_.begin() + jmp_insertion_location, q_endjmp);
	
	// ���������label
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);

	// ����case��Expression����ʱ����
	if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}

	// ��������־
	if(Token::END != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"end\" at the end of case Statement\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		if(Token::END == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <�����Ԫ��> ::= <���������>:<���>
// <���������> ::=  <���� | ������>{, <���� | ������>}
vector<int> SyntaxAnalyzer::CaseElement(const ExpressionAttribute &exp_attribute, int caselabel, int endlabel, int depth) throw()					// �����Ԫ��
{
	PrintFunctionFrame("CaseElement()", depth);

	vector<int> constant_list;

	TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
	if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_
		&& tokentable_.end() == iter)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character or constant variable after \"case\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺ�
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return constant_list;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif

	// ��¼�ó���
	if(Token::CONST_INTEGER == token_.type_)
	{
		constant_list.push_back(token_.value_.integer);
	}
	else if(Token::CONST_CHAR == token_.type_)
	{	
		constant_list.push_back(token_.value_.character);
	}
	else	// ������
	{
		constant_list.push_back(iter->value_);		
	}

	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		iter = tokentable_.SearchDefinition(token_);
		if(Token::CONST_INTEGER != token_.type_
			&& Token::CONST_CHAR != token_.type_
			&& tokentable_.end() == iter)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be constant integer or character after \"case\"\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return constant_list;
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		// ��¼�ó���
		if(Token::CONST_INTEGER == token_.type_)
		{
			constant_list.push_back(token_.value_.integer);
		}
		else if(Token::CONST_CHAR == token_.type_)
		{	
			constant_list.push_back(token_.value_.character);
		}
		else	// ������
		{
			constant_list.push_back(iter->value_);		
		}
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}
	if(Token::COLON != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ':' after constant to specify the certain action\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return constant_list;
	}
	// ���������Ԫ�ص����֮ǰ������һ��label
	Quaternary q_caselabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, caselabel);
	quaternarytable_.push_back(q_caselabel);

	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// �����Ԫ�ص����ִ�������ת��END��
	Quaternary q_endjmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endjmp);
	// ���ؼ�¼�������Ԫ�صĳ�������
	return constant_list;
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// ����չ���������֧��
void SyntaxAnalyzer::ReadStatement(int depth) throw()			// �����
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);

	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the arguments\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::IDENTIFIER != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be variable name in the location of read argument\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		TokenTable::iterator iter = tokentable_.SearchDefinition(token_);
		if(iter == tokentable_.end())
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
		if(iter->itemtype_ != TokenTableItem::VARIABLE
		&& iter->itemtype_ != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������������
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in read call\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		// ����READ���õ���Ԫʽ
		Quaternary q_read(Quaternary::READ,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(iter)));
		quaternarytable_.push_back(q_read);
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}while(Token::COMMA == token_.type_);
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void SyntaxAnalyzer::WriteStatement(int depth) throw()			// д���
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the arguments\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::CONST_STRING == token_.type_)
	{

#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
		
		vector<string>::const_iterator iter = std::find(stringtable_.begin(), stringtable_.end(), token_.value_.identifier);
		// ����WRITE���õ���Ԫʽ
		Quaternary q_read(Quaternary::WRITE,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::STRING_ADDRESSING, distance(static_cast<vector<string>::const_iterator>(stringtable_.begin()), iter));
		quaternarytable_.push_back(q_read);

		lexical_analyzer_.GetNextToken(token_);
		if(Token::COMMA == token_.type_)
		{
			lexical_analyzer_.GetNextToken(token_);
			ExpressionAttribute exp_attribute = Expression(depth + 1);	// ��ȡ�ڶ������������ʽ��
			// ����WRITE���õ���Ԫʽ
			Quaternary q_write(Quaternary::WRITE,
				Quaternary::NIL_ADDRESSING, 0,
				exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
				exp_attribute.addressingmethod_, exp_attribute.value_);
			// Imp������������ʽ��ֻ��һ������������ô���������洢һ��decoratetype�����ԣ��������ʱ�������Ƶ�
			if(Quaternary::IMMEDIATE_ADDRESSING == exp_attribute.addressingmethod_)
			{
				q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
			}
			quaternarytable_.push_back(q_write);
			// ������ʱ����
			if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_
			|| Quaternary::TEMPORARY_ADDRESSING == exp_attribute.offset_addressingmethod_)	// ���߲�����ͬʱ��������д��һ��
			{
				--tempvar_index_;
			}
		}
	}
	else
	{
		ExpressionAttribute exp_attribute = Expression(depth + 1);
		// ����WRITE���õ���Ԫʽ
		Quaternary q_write(Quaternary::WRITE,
			Quaternary::NIL_ADDRESSING, 0,
			exp_attribute.offset_addressingmethod_, exp_attribute.offset_,
			exp_attribute.addressingmethod_, exp_attribute.value_);
		// Imp������������ʽ��ֻ��һ������������ô���������洢һ��decoratetype�����ԣ��������ʱ�������Ƶ�
		if(Quaternary::IMMEDIATE_ADDRESSING == exp_attribute.addressingmethod_)
		{
			q_write.dst_decoratetype_ = exp_attribute.decoratetype_;
		}
		quaternarytable_.push_back(q_write);
		// ������ʱ����
		if(Quaternary::TEMPORARY_ADDRESSING == exp_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == exp_attribute.offset_addressingmethod_)	// ���߲�����ͬʱ��������д��һ��
		{
			--tempvar_index_;
		}
	}
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <whileѭ�����> ::= while <����> do <���>
// <whileѭ�����> ::= while @Label<check> <����> @JZLabel<end> do <���> @JMPLabel<check> @Label<end>
void SyntaxAnalyzer::WhileLoopStatement(int depth) throw()			// whileѭ�����
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// �����������ǰ���label<check>�ͽ���ʱ��label<end>
	int checklabel = label_index_++;
	int endlabel = label_index_++;
	// ����continue_label_ջ��break_label_ջ
	continue_label_.push(checklabel);
	break_label_.push(endlabel);
	// ����label<check>
	Quaternary q_checklabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_checklabel);
	
	// ��ȡ��һ�����ʣ��������������
	lexical_analyzer_.GetNextToken(token_);
	Condition(endlabel, depth + 1);	// ��������л�ִ�ж���@JZLabel<end>
	// �﷨���
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"do\" before loop body\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ����ѭ����ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// ����ѭ����
	Statement(depth + 1);
	// ����continue_label_ջ��break_label_ջ
	continue_label_.pop();
	break_label_.pop();
	// ѭ������������ת
	Quaternary q_jmp(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_jmp);
	// ���½�����label
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);
}

// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to�� <���ʽ> do <���>
// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to��
// @ASG<init> @JMPLABEL<check> @Label<vary> @ASG<vary> @Label<check> <���ʽ> @JZLABEL<end> 
// do <���>@JMPLABEL<vary>@Label<end>
void SyntaxAnalyzer::ForLoopStatement(int depth) throw()			// forѭ�����
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);

	// ��ȡ��ʶ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::IDENTIFIER != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be loop variable name after for\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_info_buffer_ << syntax_assist_buffer_ << "  " << token_.toString() << std::endl;
#endif
	TokenTable::iterator loopvar_iter = tokentable_.SearchDefinition(token_);
	if(loopvar_iter == tokentable_.end())
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  undeclared identifier\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	loopvar_iter->AddUsedLine(token_.lineNumber_);		// �ڷ��ű��в��������м�¼
	if(loopvar_iter->itemtype_ != TokenTableItem::VARIABLE
	&& loopvar_iter->itemtype_ != TokenTableItem::PARAMETER)	// ����Ƿ�Ϊ���������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot be assigned in for loop\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡ��ֵ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::ASSIGN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \":=\" after loop variable\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute init_attribute = Expression(depth + 1);

	// ���to/downto
	if(Token::DOWNTO != token_.type_
		&& Token::TO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be \"to\" or \"downto\" after variable assigning\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �������/������
	Token vary_token = token_;
	// ����ѭ�������ĵ���/����label<vary>, �߽����label<check>���Լ�ĩβ��label<end>
	int varylabel = label_index_++;
	int checklabel = label_index_++;
	int endlabel = label_index_++;
	// ����continue_label_ջ��break_label_ջ
	continue_label_.push(varylabel);
	break_label_.push(endlabel);
	// ����for��ѭ�������ĳ�ʼ������ֵ����Ԫʽ
	Quaternary q_init(Quaternary::ASG,
		init_attribute.addressingmethod_, init_attribute.value_,
		init_attribute.offset_addressingmethod_, init_attribute.offset_,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)));
	quaternarytable_.push_back(q_init);
	// ������ת���߽����JMP��Ԫʽ
	Quaternary q_jmpcheck(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_jmpcheck);
	// ����ѭ����������/����label<vary>
	Quaternary q_varylabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, varylabel);
	quaternarytable_.push_back(q_varylabel);
	// ����ѭ����������/������Ԫʽ
	Quaternary q_vary(Token::TO == vary_token.type_ ? Quaternary::ADD : Quaternary::SUB,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)),
		Quaternary::IMMEDIATE_ADDRESSING, 1,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)));
	quaternarytable_.push_back(q_vary);
	// ����ѭ�������߽����label<check>
	Quaternary q_checklabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, checklabel);
	quaternarytable_.push_back(q_checklabel);

	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	ExpressionAttribute bound_attribute = Expression(depth + 1);

	// ������������Ԫ�ر��ʽ
	SimplifyArrayOperand(bound_attribute);
	// ���ɱ߽����JMP��Ԫʽ
	Quaternary q_check(Token::TO == vary_token.type_ ? Quaternary::JG : Quaternary::JL,
		Quaternary::VARIABLE_ADDRESSING, distance(tokentable_.begin(), static_cast<TokenTable::const_iterator>(loopvar_iter)),
		bound_attribute.addressingmethod_, bound_attribute.value_,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_check);

	// ��������Expression����ʱ����
	if(	Quaternary::TEMPORARY_ADDRESSING == init_attribute.addressingmethod_
		|| Quaternary::TEMPORARY_ADDRESSING == init_attribute.offset_addressingmethod_)
	{
		--tempvar_index_;
	}
	if(Quaternary::TEMPORARY_ADDRESSING == bound_attribute.addressingmethod_)
	{
		--tempvar_index_;
	}

	// ��ȡDO
	if(Token::DO != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be do after loop head\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	// ��ȡѭ����
	Statement(depth + 1);
	// ����continue_label_ջ��break_label_ջ
	continue_label_.pop();
	break_label_.pop();
	// ��ת��ѭ����������/����label<vary>
	Quaternary q_jmpvary(Quaternary::JMP,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, varylabel);
	quaternarytable_.push_back(q_jmpvary);
	// forѭ��������label<end>
	Quaternary q_endlabel(Quaternary::LABEL,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::NIL_ADDRESSING, 0,
		Quaternary::IMMEDIATE_ADDRESSING, endlabel);
	quaternarytable_.push_back(q_endlabel);
}

void SyntaxAnalyzer::ContinueStatement(int depth) throw()	// continue
{
	assert(Token::CONTINUE == token_.type_);
	if(continue_label_.size() != 0)
	{
		Quaternary q_continue(Quaternary::JMP,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, continue_label_.top());
		quaternarytable_.push_back(q_continue);
	}
	else
	{
		// ����
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  no loop body found around \"continue\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}
void SyntaxAnalyzer::BreakStatement(int depth) throw()		// break
{
	assert(Token::BREAK == token_.type_);
	if(break_label_.size() != 0)
	{
		Quaternary q_break(Quaternary::JMP,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::NIL_ADDRESSING, 0,
			Quaternary::IMMEDIATE_ADDRESSING, break_label_.top());
		quaternarytable_.push_back(q_break);
	}
	else
	{
		// ����
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  no loop body found around \"break\"\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}


// <���̵������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::ProcedureCallStatement(const Token proc_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// ���̵������
{
	PrintFunctionFrame("ProcedureCallStatement()", depth);
	// �﷨���
	if(Token::LEFT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be '(' to specify the argument\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	if(parameter_decorate_types.size() == 0)	// ������������û�в����Ļ����Ͳ��ö�������
	{
		// �﷨���
		if(Token::RIGHT_PAREN != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);	// ������������Ϻ����һ������
		return;
	}
	// �﷨�������������������ò�������Ԫʽ
	vector<TokenTableItem::DecorateType> arg_decoratetypes = ArgumentList(depth + 1);
	// �﷨���
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);

	// �����飺���̲�������������Ƿ�ƥ��
	if(parameter_decorate_types.size() != arg_decoratetypes.size())	// ����������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  procedure does not take " << arg_decoratetypes.size() << " argument";
		if(arg_decoratetypes.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		//assert(parameter_decorate_types[i] != TokenTableItem::VOID);	// �������������ֻ��ΪCHAR��INTEGER
		//assert(arg_decoratetypes[i] != TokenTableItem::VOID);
		// ��С����С�ڱ�ʾ���ܴ��Ҳ���������ת�������������
		if(parameter_decorate_types[i] < arg_decoratetypes[i])
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() 
				<< "  cannot convert parameter " << i + 1 << " from " << TokenTableItem::DecorateTypeString[parameter_decorate_types[i]]
				<< " to " <<  TokenTableItem::DecorateTypeString[arg_decoratetypes[i]] <<"\n";
			is_successful_ = false;
			// ��������Ǽ���˲������Ͳ�ƥ�䣬�﷨�������ɼ������ʲ�����
		}
		//if(parameter_decorate_types[i] == TokenTableItem::CHAR && arg_decoratetypes[i] == TokenTableItem::INTEGER)
		//{
		//	std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  cannot convert parameter " << i + 1 << " from integer to char\n";
		//	is_successful_ = false;
		//	// ��������Ǽ���˲������Ͳ�ƥ�䣬ʵ���﷨�ɷַ������ɼ�����
		//}
	}
}

// <�����������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::FunctionCallStatement(const Token func_token, const vector<TokenTableItem::DecorateType> &parameter_decorate_types, int depth)	// �����������
{
	PrintFunctionFrame("FunctionCallStatement()", depth);

	assert(Token::LEFT_PAREN == token_.type_);

	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	if(parameter_decorate_types.size() == 0)	// ������������û�в����Ļ����Ͳ��ö�������
	{
		// �﷨���
		if(Token::RIGHT_PAREN != token_.type_)
		{
			std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
			is_successful_ = false;
			while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
			{ 
				lexical_analyzer_.GetNextToken(token_);
			}
			return;
		}
		lexical_analyzer_.GetNextToken(token_);	// ������������Ϻ����һ������
		return;
	}
	// �﷨��������
	vector<TokenTableItem::DecorateType> arg_decoratetypes = ArgumentList(depth + 1);
	// �﷨���
	if(Token::RIGHT_PAREN != token_.type_)
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  should be ')' to match the '('\n";
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	lexical_analyzer_.GetNextToken(token_);

	// �����飺���������뺯�������Ƿ�ƥ��
	if(parameter_decorate_types.size() != arg_decoratetypes.size())	// ����������
	{
		std::cout << "line " << token_.lineNumber_ << ":  " << token_.toString() << "  function does not take " << arg_decoratetypes.size() << " argument";
		if(arg_decoratetypes.size() > 1)
		{
			std::cout << "s\n";
		}
		else
		{
			std::cout << "\n";
		}
		is_successful_ = false;
		while(token_.type_ != Token::NIL && token_.type_ != Token::SEMICOLON && token_.type_ != Token::END)	// ������β��ֺŻ�END
		{ 
			lexical_analyzer_.GetNextToken(token_);
		}
		return;
	}
	// �����Ƿ�ƥ��
	for(int i = 0; i < static_cast<int>(parameter_decorate_types.size()); ++i)
	{
		// ��С����С�ڱ�ʾ���ܴ��Ҳ���������ת�������������
		if(parameter_decorate_types[i] < arg_decoratetypes[i])
		{
			std::cout << "warning: line " << token_.lineNumber_ << ":  " << token_.toString() 
				<< " convert parameter " << i + 1 << " from " << TokenTableItem::DecorateTypeString[parameter_decorate_types[i]]
				<< " to " <<  TokenTableItem::DecorateTypeString[arg_decoratetypes[i]] <<"\n";
			// ��������Ǽ���˲������Ͳ�ƥ�䣬�﷨�������ɼ������ʲ�����
		}
	}
	
}

// ������Ԫ�صĲ��������������������ͽ��仯��Ϊ��ʱ����
void SyntaxAnalyzer::SimplifyArrayOperand(ExpressionAttribute &attribute) throw()
{
	if(Quaternary::ARRAY_ADDRESSING == attribute.addressingmethod_)
	{
		Quaternary q_subscript2temp;
		q_subscript2temp.op_ = Quaternary::ASG;
		q_subscript2temp.method1_ = attribute.addressingmethod_;
		q_subscript2temp.src1_ = attribute.value_;
		q_subscript2temp.method2_ = attribute.offset_addressingmethod_;
		q_subscript2temp.offset2_ = attribute.offset_;
		q_subscript2temp.method3_ = Quaternary::TEMPORARY_ADDRESSING;
		// Ŀ�����ʱ������ŵ�ȷ��
		// ��������±������ʱ��������ô�����������
		// ������¿�һ����ʱ����
		if(Quaternary::TEMPORARY_ADDRESSING == attribute.offset_addressingmethod_)
		{
			q_subscript2temp.dst_ = attribute.offset_;
		}
		else
		{
			q_subscript2temp.dst_ = tempvar_index_++;
			//// ���������ʱ��������
			//if(tempvar_index_ > max_local_temp_count_)
			//{
			//	max_local_temp_count_ = tempvar_index_;
			//}
		}
		quaternarytable_.push_back(q_subscript2temp);
		attribute.addressingmethod_ = Quaternary::TEMPORARY_ADDRESSING;
		attribute.value_ = q_subscript2temp.dst_;
		attribute.offset_addressingmethod_ = Quaternary::NIL_ADDRESSING;
		attribute.offset_ = 0;
	}
}

//// �޸��ڷ��ű��λ��proc_func_index���Ĺ���/������Ӧ����Ԫʽ��BEGIN��䣬��������ʱ����������
//void SyntaxAnalyzer::SetTempVarCount(int proc_func_index, int max_tempvar_count) throw()
//{
//	for(vector<Quaternary>::reverse_iterator r_iter = quaternarytable_.rbegin();
//		r_iter != quaternarytable_.rend(); ++r_iter)
//	{
//		if(Quaternary::BEGIN == r_iter->op_
//			&& proc_func_index == r_iter->dst_)
//		{
//			r_iter->src2_ = max_tempvar_count;
//			return;
//		}
//	}
//}