#include "SyntaxAnalyzer.h"
#include <assert.h>
#include <sstream>

#define SYNTAXDEBUG

SyntaxAnalyzer::SyntaxAnalyzer(LexicalAnalyzer &lexical_analyzer) throw()
	: lexical_analyzer_(lexical_analyzer), token_(), is_successful_(true), syntax_process_buffer_(), syntax_format_string_()
{
	
}


bool SyntaxAnalyzer::Parse() throw()
{
	size_t depth = 0;
	lexical_analyzer_.ResetTokenPos();
	syntax_process_buffer_.str("");
	syntax_format_string_.clear();
	PrintFunctionFrame("Parse()", depth);
	lexical_analyzer_.GetNextToken(token_);
	Routine(depth + 1);
	return is_successful_;
}

string SyntaxAnalyzer::toString() const throw()
{
	return syntax_process_buffer_.str();
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
	output << syntax_process_buffer_.str() << std::endl;
}

static string syntax_format_string_;	// ע�������̰߳�ȫ��
void SyntaxAnalyzer::PrintFunctionFrame(const char *func_name, size_t depth) throw()
{
	
	if(depth * 4 == syntax_format_string_.size())
	{
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else if(depth * 4 > (int)syntax_format_string_.size())
	{
		syntax_format_string_.append("|");
		syntax_format_string_.append(depth * 4 - syntax_format_string_.size(), ' ');	// ���ﲻ�ܼ�1
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
	else // depth * 4 < syntax_format_string_.size()
	{
		syntax_format_string_.resize(depth * 4);
		syntax_process_buffer_ << syntax_format_string_ << func_name << '\n';
	}
}
// <����> ::= <�ֳ���>.
void SyntaxAnalyzer::Routine(size_t depth) throw()
{
	PrintFunctionFrame("Routine()", depth);
	// �����ֳ���
	SubRoutine(depth + 1);
	// �жϽ�������
	if(token_.type_ != Token::PERIOD)
	{
		if(Token::NIL != token_.type_)	// ����û�н���
		{
			ErrorHandle(WRONGENDINGTOKEN);
		}
		else	// ����ȷʵ������
		{
			ErrorHandle(LACKENDINGPERIOD);
		}
		return;
	}
	// ��֤PERIOD�����Ƿ��е���
	if(lexical_analyzer_.GetNextToken(token_))
	{
		ErrorHandle(REDUNDANTTOOKEN);
	}
}

// <�ֳ���> ::= [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>
void SyntaxAnalyzer::SubRoutine(size_t depth) throw()
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
		ErrorHandle(NOSTATEMENTBLOCK);
	}
}

// <����˵������> ::= const<��������>{,<��������>};
void SyntaxAnalyzer::ConstantPart(size_t depth) throw()
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
		ErrorHandle(LACKSEMICOLON);
		//return;	// �����ݲ����أ�Ҫִ�����һ����䣬��ȡ�˷ֺź����һ�����ʺ��ٷ���
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <��������> ::= <��ʶ��>��<����>
void SyntaxAnalyzer::constantDefination(size_t depth) throw()
{
	PrintFunctionFrame("constantDefination()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��¼token_�Բ�����ű�
	Token constIdentifier = token_;
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::EQU)
	{
		ErrorHandle(LACKEQU);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::CONST_INTEGER 
	&& token_.type_ != Token::CONST_CHAR)
	{
		ErrorHandle(LACKCONSTANT);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= var <��������>;{<��������>;}
void SyntaxAnalyzer::VariablePart(size_t depth) throw()
{
	PrintFunctionFrame("VariablePart()", depth);

	assert(Token::VAR == token_.type_);

	lexical_analyzer_.GetNextToken(token_);
	do
	{
		VariableDefinition(depth + 1);
		if(token_.type_ != Token::SEMICOLON)
		{
			ErrorHandle(LACKSEMICOLON);
			//return;	// �ݲ����أ�����ȡ�ֺŵ���һ������
		}
		lexical_analyzer_.GetNextToken(token_);
	}while(token_.type_ == Token::IDENTIFIER);
}

// <��������> ::= <��ʶ��>{,<��ʶ��>}:<����>
void SyntaxAnalyzer::VariableDefinition(size_t depth) throw()
{
	PrintFunctionFrame("VariableDefinition()", depth);

	if(token_.type_ != Token::IDENTIFIER)
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(token_.type_ == Token::COMMA)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			ErrorHandle(LACKIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	TypeSpecification(depth + 1);
}
// <����> ::= [array'['<�޷�������>']'of]<��������>
void SyntaxAnalyzer::TypeSpecification(size_t depth) throw()
{
	PrintFunctionFrame("TypeSpecification()", depth);

	if(token_.type_ == Token::ARRAY)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::LEFT_BRACKET)
		{
			ErrorHandle(LACKLEFTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::CONST_INTEGER)
		{
			ErrorHandle(LACKCONSTINTEGER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			ErrorHandle(LACKRIGHTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::OF)
		{
			ErrorHandle(LACKOF);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}

	if(token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)	// ��û������˵��
	{
		ErrorHandle(LACKRWTYPE);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::ProcedurePart(size_t depth) throw()
{
	PrintFunctionFrame("ProcedurePart()", depth);

	do
	{
		ProcedureHead(depth + 1);
		SubRoutine(depth + 1);
		if(Token::SEMICOLON != token_.type_)
		{
			ErrorHandle(LACKSEMICOLON);
			//return;	// �ݲ����أ�����ȡ�ֺŵ���һ������
		}
		lexical_analyzer_.GetNextToken(token_);	// �ֳ��������Ӧ����ֺ�
	}while(Token::PROCEDURE == token_.type_);
}

// <�����ײ�> ::= procedure<���̱�ʶ��>'('[<��ʽ������>]')';
void SyntaxAnalyzer::ProcedureHead(size_t depth) throw()
{
	PrintFunctionFrame("ProcedureHead()", depth);

	assert(Token::PROCEDURE == token_.type_);

	int proc_index = -1;	// �������ڷ��ű��е�λ�ã��±꣩

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ������ȡ����
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)	// û�ж��������ţ�����û�в���
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		ParameterList(depth);
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)
	{
		ErrorHandle(LACKSEMICOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <����˵������> ::= <�����ײ�><�ֳ���>;{<�����ײ�><�ֳ���>;}
void SyntaxAnalyzer::FunctionPart(size_t depth) throw()
{
	PrintFunctionFrame("FunctionPart()", depth);

	do
	{
		FunctionHead(depth + 1);	// ���к���ͷ���������õ��������ڷ��ű��е�λ��
		SubRoutine(depth + 1);
		if(Token::SEMICOLON != token_.type_)	// �ֳ��������Ӧ����ֺ�
		{
			ErrorHandle(LACKSEMICOLON);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);	// �����β�ķֺ�
	}while(Token::FUNCTION == token_.type_);
}

// <�����ײ�> ::= function <������ʶ��>'('[<��ʽ������>]')':<��������>;
void SyntaxAnalyzer::FunctionHead(size_t depth) throw()
{
	PrintFunctionFrame("FunctionHead()", depth);

	assert(Token::FUNCTION == token_.type_);

	int func_index = -1;	// �������ڷ��ű��е�λ��

	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::IDENTIFIER)	// δ�ҵ�������
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::LEFT_PAREN)
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	Token::VAR == token_.type_
		|| Token::IDENTIFIER == token_.type_)
	{
		ParameterList(depth + 1);
	}
	if(token_.type_ != Token::RIGHT_PAREN)
	{
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		ErrorHandle(LACKRWTYPE);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	if(token_.type_ != Token::SEMICOLON)	// �����п��������˷ֺţ����Բ��ټ�����
	{
		ErrorHandle(LACKSEMICOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	return ;
}

// <��ʽ������> ::= <��ʽ������>{;<��ʽ������>}
// �����β�����
void SyntaxAnalyzer::ParameterList(size_t depth) throw()		// �βα�
{
	PrintFunctionFrame("ParameterList()", depth);

	ParameterTerm(depth + 1);
	while(Token::SEMICOLON == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		ParameterTerm(depth + 1);
	}
}

// <��ʽ������> ::= [var]<��ʶ��>{,<��ʶ��>}:<��������>
// ���ظ��βζε��β�����
void SyntaxAnalyzer::ParameterTerm(size_t depth) throw()		
{
	PrintFunctionFrame("ParameterTerm()", depth);
	bool isref = false;	// �Ƿ�Ϊ���ô���
	if(Token::VAR == token_.type_)
	{
		isref = true;
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::IDENTIFIER)
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(token_.type_ != Token::IDENTIFIER)
		{
			ErrorHandle(LACKIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	if(token_.type_ != Token::COLON)
	{
		ErrorHandle(LACKTYPECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	if(	token_.type_ != Token::RW_INTEGER
		&& token_.type_ != Token::RW_CHAR)
	{
		ErrorHandle(LACKRWTYPE);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);
	return ;
}


// <�������> ::= begin <���>{;<���>} end
void SyntaxAnalyzer::StatementBlockPart(size_t depth) throw()	// �������
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
		ErrorHandle(LACKSEMICOLON);	// ע������󲿷ֳ���ԭ����ȱ���˷ֺţ��������������дend
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <���> ::= <��ʶ��>(<��ֵ���>|<���̵������>)|<�������>|<������>|<�������>
// |<�����>|<д���>|<whileѭ�����>|<forѭ�����>|<ѭ���������>|<ѭ���˳����>|<��>
void SyntaxAnalyzer::Statement(size_t depth) throw()
{
	PrintFunctionFrame("Statement()", depth);

	Token idToken = token_;	// ��token_�����ǹ��������ȼ��£�����
	switch(token_.type_)
	{
	case Token::IDENTIFIER:
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_PAREN == token_.type_)	// ���̻�������
		{
			ProcFuncCallStatement(depth + 1);
		}
		else if(Token::ASSIGN == token_.type_
			|| Token::LEFT_BRACKET == token_.type_)
		{
			AssigningStatement(depth + 1);		
		}
		else
		{
			ErrorHandle(LACKASSIGNINGTOKEN);
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
	}
}

// <��ֵ���> ::= ['['<���ʽ>']']:=<���ʽ>
// idToken�Ǹ�ֵ���֮ǰ��ʶ����token��iter�����ڷ��ű��еĵ�����
void SyntaxAnalyzer::AssigningStatement(size_t depth)			// ��ֵ���
{
	PrintFunctionFrame("AssigningStatement()", depth);

	// Ϊ��Ԫʽ���ɶ�����ı���
	bool assign2array= false;	// �Ƿ�Ϊ������ĸ�ֵ����
	
	ExpressionAttribute offset_attribute;	// ��������Ԫ�ظ�ֵʱ���洢ƫ�����������±꣩������

	if(Token::LEFT_BRACKET == token_.type_)	// ������Ԫ�ظ�ֵ
	{
		assign2array = true;
		// �����ʾ�±�ı��ʽ
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
		// �﷨���
		if(token_.type_ != Token::RIGHT_BRACKET)
		{
			ErrorHandle(LACKRIGHTBRACKET);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	// �﷨���
	if(token_.type_ != Token::ASSIGN)
	{
		ErrorHandle(LACKASSIGNINGTOKEN);
		return;
	}

	// ���븳ֵ���ұߵı��ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
void SyntaxAnalyzer::Expression(size_t depth) throw()				// ���ʽ
{
	PrintFunctionFrame("Expression()", depth);
	
	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}
	Term(depth + 1);

	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		Term(depth + 1);
	}
}

// <��> ::= <����>{<�˷������><����>}
void SyntaxAnalyzer::Term(size_t depth) throw()						// ��
{
	PrintFunctionFrame("Term()", depth);

	Factor(depth + 1);

	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		lexical_analyzer_.GetNextToken(token_);
		Factor(depth + 1);
	}
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
void SyntaxAnalyzer::Factor(size_t depth) throw()					// ����
{
	PrintFunctionFrame("Factor()", depth);

	// �﷨��飺��ʶ��������������������������顢�������á�
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		//Token idToken = token_;	// ���£�����
		lexical_analyzer_.GetNextToken(token_);
		// ����Ԫ��
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// �﷨��������Ϊ�±�ı��ʽ
			lexical_analyzer_.GetNextToken(token_);
			Expression(depth + 1);
			// �﷨���
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				ErrorHandle(LACKRIGHTBRACKET);
				return;
			}
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// �����ţ���������
		{
			ProcFuncCallStatement(depth + 1);			
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// �����������ı��ʽ
	{
		// bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
		// �����ʽ�ĵ�һ������
		lexical_analyzer_.GetNextToken(token_);
		// �ٶ�ȡ���ʽ
		Expression(depth + 1);	// ��¼����
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			ErrorHandle(LACKRIGHTPAREN);
			return;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)	// �������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// �ַ������泣��
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// �﷨��������
		ErrorHandle(WRONGFACTOR);
		return;
	}
	return ;
}

// <�������> ::= if<����>then<���>[else<���>]
void SyntaxAnalyzer::IfStatement(size_t depth) throw()				// �������
{
	PrintFunctionFrame("IfStatement()", depth);

	assert(Token::IF == token_.type_);

	// ��ȡ�������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// ��condition��������ת���
	// �﷨���
	if(Token::THEN != token_.type_)
	{
		ErrorHandle(LACKTHEN);
		return;
	}
	// ��ȡif�ɹ�������
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);

	// ��ȡelse����䣨����еĻ���
	if(Token::ELSE == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Statement(depth + 1);
	}
}

//// <����> ::= <���ʽ><��ϵ�����><���ʽ>
//void SyntaxAnalyzer::Condition(size_t depth) throw()				// ����
//{
//	PrintFunctionFrame("Condition()", depth);
//
//	Expression(depth + 1);
//	switch(token_.type_)	// ������ʿ����ǹ�ϵ���������Ҳ�п�����THEN���������н���һ�����ʽʱ��
//	{
//	case Token::LT:
//	case Token::LEQ:
//	case Token::GT:
//	case Token::GEQ:
//	case Token::EQU:
//	case Token::NEQ:
//	case Token::THEN:
//		break;
//	// ��Ϊ֮ǰ�Ѿ������ˣ�������������²�������default
//	default:
//		ErrorHandle(LACKLOGICOPERATOR);
//		return;
//		break;
//	}
//	if(Token::THEN != token_.type_)	// ���������һ�����ʽ���ټ�����ȡ
//	{
//		lexical_analyzer_.GetNextToken(token_);
//		Expression(depth + 1);
//	}
//}

// <����> ::= <�������ʽ>
void SyntaxAnalyzer::Condition(size_t depth) throw()				// ����
{
	PrintFunctionFrame("Condition()", depth);
	BoolExpression(depth + 1);
}

// <�������ʽ> ::= <������> [ <�߼���> <������>]
void SyntaxAnalyzer::BoolExpression(size_t depth) throw()	// �������ʽ
{
	PrintFunctionFrame("BoolExpression()", depth);
	bool isfirst = true;
	do
	{
		if(!isfirst)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		else
		{
			isfirst = false;
		}
		BoolTerm(depth + 1);
	}while(Token::LOGICOR == token_.type_);
}

// <������> ::= <��������> [<�߼���><��������>]
void SyntaxAnalyzer::BoolTerm(size_t depth) throw()			// ������
{
	PrintFunctionFrame("BoolTerm()", depth);
	bool isfirst = true;
	do
	{
		if(!isfirst)
		{
			lexical_analyzer_.GetNextToken(token_);
		}
		else
		{
			isfirst = false;
		}
		BoolFactor(depth + 1);
	}while(Token::LOGICAND == token_.type_);
}

// <��������> ::= <���ʽ>[<��ϵ�����><���ʽ>] | ��(��<�������ʽ>��)��
void SyntaxAnalyzer::BoolFactor(size_t depth) throw()		// ��������
{
	PrintFunctionFrame("BoolFactor()", depth);
	// �ȿ��Ƿ������ŵ����
	if(Token::LEFT_PAREN != token_.type_ || IsExpression(depth + 1))
	{
		Expression(depth + 1);
		switch(token_.type_)	// ������ʿ����ǹ�ϵ���������Ҳ�п����������Ż�THEN��OR��AND���������н���һ�����ʽʱ��
		{
		case Token::LT:
		case Token::LEQ:
		case Token::GT:
		case Token::GEQ:
		case Token::EQU:
		case Token::NEQ:
		case Token::RIGHT_PAREN:
		case Token::THEN:
		case Token::LOGICOR:
		case Token::LOGICAND:
			break;
		default:
			ErrorHandle(LACKLOGICOPERATOR);
			return;
			break;
		}
		if(Token::RIGHT_PAREN != token_.type_
			&& Token::THEN != token_.type_
			&& Token::LOGICOR != token_.type_
			&& Token::LOGICAND != token_.type_)	// ���������һ�����ʽ���ټ�����ȡ
		{
			lexical_analyzer_.GetNextToken(token_);	// ��ȡ��ϵ���������һ������
			Expression(depth + 1);
		}
	}
	else	// ��һ���������ţ��Ҳ���ʶ��Ϊ���ʽ�����
	{
		lexical_analyzer_.GetNextToken(token_);	// �������ŵ���һ������
		BoolExpression(depth + 1);
		lexical_analyzer_.GetNextToken(token_);	// �������ŵ���һ������
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
bool SyntaxAnalyzer::IsExpression(size_t depth) throw()
{
	PrintFunctionFrame("IsExpression()", depth);
	vector<Token>::const_iterator iter = lexical_analyzer_.GetTokenPosition();
	Token tmp = token_;
	bool result = ExpressionTest(depth + 1);
	token_ = tmp;
	lexical_analyzer_.SetTokenPosition(iter);
	return result;
}


// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
bool SyntaxAnalyzer::ExpressionTest(size_t depth) throw()
{
	PrintFunctionFrame("ExpressionTest()", depth);

	if(	Token::PLUS == token_.type_
		|| Token::MINUS == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
	}
	if(!TermTest(depth + 1))
	{
		return false;
	}

	while(	Token::PLUS == token_.type_
		||	Token::MINUS == token_.type_)
	{
		// ��ȡ��һ��
		lexical_analyzer_.GetNextToken(token_);
		if(!TermTest(depth + 1))
		{
			return false;
		}
	}
	return true;
}

// <��> ::= <����>{<�˷������><����>}
bool SyntaxAnalyzer::TermTest(size_t depth) throw()						// ��
{
	PrintFunctionFrame("TermTest()", depth);

	if(!FactorTest(depth + 1))
	{
		return false;
	}

	while(	token_.type_ == Token::MUL
		||	token_.type_ == Token::DIV)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(!FactorTest(depth + 1))
		{
			return false;
		}
	}
	return true;
}

// <����> ::= <��ʶ��>(['['<���ʽ>']'] | [<�����������>])
//          | '('<���ʽ>')' 
//          | <�޷�������> 
//          | <�ַ�>
bool SyntaxAnalyzer::FactorTest(size_t depth) throw()					// ����
{
	PrintFunctionFrame("FactorTest()", depth);

	// �﷨��飺��ʶ��������������������������顢�������á�
	if(Token::IDENTIFIER == token_.type_)
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
		// ����Ԫ��
		if(Token::LEFT_BRACKET == token_.type_)
		{
			// �﷨��������Ϊ�±�ı��ʽ
			lexical_analyzer_.GetNextToken(token_);
			if(!ExpressionTest(depth + 1))
			{
				return false;
			}
			// �﷨���
			if(token_.type_ != Token::RIGHT_BRACKET)
			{
				return false;
			}
			// �����������ŵ���һ������ <bug fixed by mxf at 21:28 1.29 2016>
			lexical_analyzer_.GetNextToken(token_);
		}
		else if(Token::LEFT_PAREN == token_.type_)	// �����ţ���������
		{
			ProcFuncCallStatementTest(depth + 1);			
		}
	}
	else if(Token::LEFT_PAREN == token_.type_)	// �����������ı��ʽ
	{
		// bug fixed by mxf at 0:42 1/31 2016�������ʽ֮ǰû�ж�ȡ���ź�ĵ�һ�����ʡ�
		// �����ʽ�ĵ�һ������
		lexical_analyzer_.GetNextToken(token_);
		// �ٶ�ȡ���ʽ
		if(!ExpressionTest(depth + 1))	// ��¼����
		{
			return false;
		}
		if(token_.type_ != Token::RIGHT_PAREN)
		{
			return false;
		}
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_INTEGER == token_.type_)	// �������泣��
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else if(Token::CONST_CHAR == token_.type_)	// �ַ������泣��
	{
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		lexical_analyzer_.GetNextToken(token_);
	}
	else
	{
		// �﷨��������
		return false;
	}
	return true;
}

// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
bool SyntaxAnalyzer::ProcFuncCallStatementTest(size_t depth)	// ���̵������
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	// �﷨���
	if(Token::LEFT_PAREN != token_.type_)
	{
		return false;
	}
	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// �﷨��������
	if(Token::RIGHT_PAREN != token_.type_)
	{
		if(!ArgumentListTest(depth + 1))
		{
			return false;
		}
		// �﷨���
		if(Token::RIGHT_PAREN != token_.type_)
		{
			return false;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
	return true;
}

// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
bool SyntaxAnalyzer::ArgumentListTest(size_t depth) throw()			// ʵ�α�
{
	PrintFunctionFrame("ArgumentList()", depth);

	if(!ExpressionTest(depth + 1))
	{
		return false;
	}
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(!ExpressionTest(depth + 1))
		{
			return false;
		}
	}
	return true;
}

// <������> ::= case <���ʽ> of <�����Ԫ��>{; <�����Ԫ��>}end
void SyntaxAnalyzer::CaseStatement(size_t depth) throw()			// ������
{
	PrintFunctionFrame("CaseStatement()", depth);

	assert(Token::CASE == token_.type_);

	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);
	if(Token::OF != token_.type_)
	{
		ErrorHandle(LACKOF);
		return;
	}
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		CaseElement(depth + 1);
	}while(Token::SEMICOLON == token_.type_);

	// ��������־
	if(Token::END != token_.type_)
	{
		ErrorHandle(LACKSEMICOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <�����Ԫ��> ::= <���������>:<���>
// <���������> ::=  <���� | ������>{, <���� | ������>}
void SyntaxAnalyzer::CaseElement(size_t depth) throw()					// �����Ԫ��
{
	PrintFunctionFrame("CaseElement()", depth);

	if(Token::CONST_INTEGER != token_.type_
	&& Token::CONST_CHAR != token_.type_
	&& Token::IDENTIFIER != token_.type_)
	{
		ErrorHandle(LACKCONSTANTORIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif

	lexical_analyzer_.GetNextToken(token_);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::CONST_INTEGER != token_.type_
		&& Token::CONST_CHAR != token_.type_
		&& Token::IDENTIFIER != token_.type_)
		{
			ErrorHandle(LACKCONSTANTORIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// ��ȡ��һ������
		lexical_analyzer_.GetNextToken(token_);
	}
	if(Token::COLON != token_.type_)
	{
		ErrorHandle(LACKCASECOLON);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	Statement(depth + 1);
}

// <�����> ::= read'('<��ʶ��>{,<��ʶ��>}')'
// TODO ��չ���������֧��
void SyntaxAnalyzer::ReadStatement(size_t depth) throw()			// �����
{
	PrintFunctionFrame("ReadStatement()", depth);

	assert(Token::READ == token_.type_);
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);

	if(Token::LEFT_PAREN != token_.type_)
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	
	do
	{
		lexical_analyzer_.GetNextToken(token_);
		if(Token::IDENTIFIER != token_.type_)
		{
			ErrorHandle(LACKIDENTIFIER);
			return;
		}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
		// ��ȡ��һ�����ʣ��ж��Ƿ�Ϊ����Ԫ��
		lexical_analyzer_.GetNextToken(token_);
		if(Token::LEFT_BRACKET == token_.type_)	// ��������Ԫ��
		{
			// ���������±�ĵ�һ������
			lexical_analyzer_.GetNextToken(token_);
			// �������±�ı��ʽ
			Expression(depth + 1);
			// �ж�������
			if(Token::RIGHT_BRACKET != token_.type_)
			{
				ErrorHandle(LACKRIGHTBRACKET);
				return;
			}
			else
			{
				// ������һ������
				lexical_analyzer_.GetNextToken(token_);
			}
		}
	}while(Token::COMMA == token_.type_);
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ErrorHandle(LACKLEFTBRACKET);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <д���> ::= write'(' (<�ַ���>[,<���ʽ>] | <���ʽ>) ')'
void SyntaxAnalyzer::WriteStatement(size_t depth) throw()			// д���
{
	PrintFunctionFrame("WriteStatement()", depth);

	assert(Token::WRITE == token_.type_);

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::LEFT_PAREN != token_.type_)
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	
	if(Token::CONST_STRING == token_.type_)
	{

#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
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
	
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ErrorHandle(LACKRIGHTPAREN);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
}

// <whileѭ�����> ::= while <����> do <���>
void SyntaxAnalyzer::WhileLoopStatement(size_t depth) throw()			// whileѭ�����
{
	PrintFunctionFrame("WhileLoopStatement()", depth);
	assert(Token::WHILE == token_.type_);

	// ��ȡ��һ�����ʣ��������������
	lexical_analyzer_.GetNextToken(token_);
	Condition(depth + 1);	// ��������л�ִ�ж���@JZLabel<end>
	// �﷨���
	if(Token::DO != token_.type_)
	{
		ErrorHandle(LACKDO);
		return;
	}
	// ����ѭ����ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// ����ѭ����
	Statement(depth + 1);
}

// <forѭ�����> ::= for <��ʶ��> := <���ʽ> ��downto | to�� <���ʽ> do <���>
void SyntaxAnalyzer::ForLoopStatement(size_t depth) throw()			// forѭ�����
{
	PrintFunctionFrame("ForLoopStatement()", depth);

	assert(Token::FOR == token_.type_);

	// ��ȡ��ʶ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::IDENTIFIER != token_.type_)
	{
		ErrorHandle(LACKIDENTIFIER);
		return;
	}
#ifdef SYNTAXDEBUG
	syntax_process_buffer_ << syntax_format_string_ << "  " << token_.toString() << std::endl;
#endif
	// ��ȡ��ֵ��
	lexical_analyzer_.GetNextToken(token_);
	if(Token::ASSIGN != token_.type_)
	{
		ErrorHandle(LACKASSIGNINGTOKEN);
		return;
	}
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// ���to/downto
	if(Token::DOWNTO != token_.type_
		&& Token::TO != token_.type_)
	{
		ErrorHandle(LACKTO_DOWNTO);
		return;
	}
	// �������/������
	Token vary_token = token_;
	// ��ȡ���ʽ
	lexical_analyzer_.GetNextToken(token_);
	Expression(depth + 1);

	// ��ȡDO
	if(Token::DO != token_.type_)
	{
		ErrorHandle(LACKDO);
		return;
	}
	lexical_analyzer_.GetNextToken(token_);
	// ��ȡѭ����
	Statement(depth + 1);
}

void SyntaxAnalyzer::ContinueStatement(size_t depth) throw()	// continue
{
	PrintFunctionFrame("ContinueStatement()", depth);
	assert(Token::CONTINUE == token_.type_);
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}
void SyntaxAnalyzer::BreakStatement(size_t depth) throw()		// break
{
	PrintFunctionFrame("BreakStatement()", depth);
	assert(Token::BREAK == token_.type_);
	// ������һ�����ʲ�����
	lexical_analyzer_.GetNextToken(token_);
}


// <����/�����������> ::= '('[<ʵ�ڲ�����>]')'
void SyntaxAnalyzer::ProcFuncCallStatement(size_t depth)	// ���̵������
{
	PrintFunctionFrame("ProcFuncCallStatement()", depth);
	// �﷨���
	if(Token::LEFT_PAREN != token_.type_)
	{
		ErrorHandle(LACKLEFTPAREN);
		return;
	}
	// �﷨�����������Ż������ĵ�һ������
	lexical_analyzer_.GetNextToken(token_);
	// �﷨��������
	if(Token::RIGHT_PAREN != token_.type_)
	{
		ArgumentList(depth + 1);
		// �﷨���
		if(Token::RIGHT_PAREN != token_.type_)
		{
			ErrorHandle(LACKRIGHTPAREN);
			return;
		}
	}
	lexical_analyzer_.GetNextToken(token_);
}



// <ʵ�ڲ�����> ::= <���ʽ>{,<���ʽ>}
void SyntaxAnalyzer::ArgumentList(size_t depth) throw()			// ʵ�α�
{
	PrintFunctionFrame("ArgumentList()", depth);

	Expression(depth + 1);
	while(Token::COMMA == token_.type_)
	{
		lexical_analyzer_.GetNextToken(token_);
		Expression(depth + 1);
	}
	return ;
}



void SyntaxAnalyzer::ErrorHandle(ErrorType error_type) throw()
{
	using std::cout;
	is_successful_ = false;
	int offset = lexical_analyzer_.GetLine(token_.lineNumber_).find_first_not_of(" \t\n");
	int count = lexical_analyzer_.GetLine(token_.lineNumber_).find_last_not_of(" \t\n") - offset + 1;
	string errline = offset == string::npos ? "" : lexical_analyzer_.GetLine(token_.lineNumber_).substr(offset, count);

	cout << "syntax error(" << error_type << ")";
	//switch(error_type)
	//{
	//case LACKENDINGPERIOD:
	//	cout << "\nerror info: lack '.' at the end of program\n";
	//	break;
	//case WRONGENDINGTOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '.' at the end of program\n";
	//	break;
	//case REDUNDANTTOOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "redundant token after '.'\n";
	//	break;
	//case NOSTATEMENTBLOCK:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "didn't find statement block in subroutine\n";
	//	break;
	//case LACKSEMICOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ';' after the sentence(or do you forget ','?)\n";
	//	break;
	//case LACKIDENTIFIER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be an identifier\n";
	//	break;
	//case LACKEQU:	// ֻ�ڳ�������ʱ����
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '=' after identifier\n";
	//	break;
	//case LACKCONSTANT:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be constant integer or character\n";
	//	break;
	//case LACKTYPECOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the type\n";
	//	break;
	//case LACKLEFTBRACKET:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '[' to specify an array\n";
	//	break;
	//case LACKRIGHTBRACKET:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ']' to match '['\n";
	//	break;
	//case LACKCONSTINTEGER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "need a constant integer\n";
	//	break;
	//case LACKOF:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"of\" after [] to specify array type\n";
	//	break;
	//case LACKRWTYPE:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"integer\" or \"char\" for type specification\n";
	//	break;
	//case LACKLEFTPAREN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be '('\n";
	//	break;
	//case LACKRIGHTPAREN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ')' to match '('\n";
	//	break;
	//case LACKASSIGNINGTOKEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \":=\"\n";
	//	break;
	//case WRONGFACTOR:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "illegal factor, should be a value or variable\n";
	//	break;
	//case LACKTHEN:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "need \"then\" after if condition\n";
	//	break;
	//case LACKLOGICOPERATOR:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be a logic operator or \"then\"\n";
	//	break;
	//case LACKCONSTANTORIDENTIFIER:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be constant integer or character or identifier\n";
	//	break;
	//case LACKCASECOLON:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the case action\n";
	//	break;
	//case LACKDO:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"do\" before loop body\n";
	//	break;
	//case LACKTO_DOWNTO:
	//	cout << "  line " << token_.lineNumber_ << ": " << lexical_analyzer_.GetLine(token_.lineNumber_)
	//		 << "  error token: " << token_.toString() << "\nerror info: " << "should be \"to\" or \"downto\" after variable assigning\n";
	//	break;
	//default:
	//	// δ֪����
	//	assert(false);
	//	break;
	//}
	switch(error_type)
	{
	case LACKENDINGPERIOD:
		cout << "\nerror info: lack '.' at the end of program\n";
		break;
	case WRONGENDINGTOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '.' at the end of program\n";
		break;
	case REDUNDANTTOOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "redundant token after '.'\n";
		break;
	case NOSTATEMENTBLOCK:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "didn't find statement block in subroutine\n";
		break;
	case LACKSEMICOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ';' after the sentence(or do you forget ','?)\n";
		break;
	case LACKIDENTIFIER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be an identifier\n";
		break;
	case LACKEQU:	// ֻ�ڳ�������ʱ����
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '=' after identifier\n";
		break;
	case LACKCONSTANT:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be constant integer or character\n";
		break;
	case LACKTYPECOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the type\n";
		break;
	case LACKLEFTBRACKET:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '[' to specify an array\n";
		break;
	case LACKRIGHTBRACKET:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ']' to match '['\n";
		break;
	case LACKCONSTINTEGER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "need a constant integer\n";
		break;
	case LACKOF:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"of\" after [] to specify array type\n";
		break;
	case LACKRWTYPE:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"integer\" or \"char\" for type specification\n";
		break;
	case LACKLEFTPAREN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be '('\n";
		break;
	case LACKRIGHTPAREN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ')' to match '('\n";
		break;
	case LACKASSIGNINGTOKEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \":=\"\n";
		break;
	case WRONGFACTOR:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "illegal factor, should be a value or variable\n";
		break;
	case LACKTHEN:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "need \"then\" after if condition\n";
		break;
	case LACKLOGICOPERATOR:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be a logic operator or \"then\", ')' or \"||\" or \"&&\"\n";
		break;
	case LACKCONSTANTORIDENTIFIER:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be constant integer or character or identifier\n";
		break;
	case LACKCASECOLON:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be ':' to specify the case action\n";
		break;
	case LACKDO:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"do\" before loop body\n";
		break;
	case LACKTO_DOWNTO:
		cout << "  line " << token_.lineNumber_ << ": " << errline
			 << " \terror token: " << token_.toString() << "\nerror info: " << "should be \"to\" or \"downto\" after variable assigning\n";
		break;
	default:
		// δ֪����
		assert(false);
		break;
	}
	while(Token::NIL != token_.type_ && Token::SEMICOLON != token_.type_)	// �����������
	{
		lexical_analyzer_.GetNextToken(token_);
	}
}