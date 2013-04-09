#include "Scanner.h"
#include <iostream>
#include <cstring>
#include <list>
#include <boost/bind.hpp>
#include <loki/Typelist.h>
#include <loki/TypelistMacros.h>

namespace PGParse {
	
class Statement
{
};

class DropTableStatement : public Statement
{
public:
	DropTableStatement(TokenList::const_iterator table_name)	{}
};

class StatementBlock
{
public:
	std::list<Statement*> statements;
};
	
/**
 * A really simple recursive descent parser that takes advantage 
 * of templates to eliminate boilerplate code.
 *
 */
class Parser
{
public:
	typedef TokenList::const_iterator iterator;
	struct Frame;
private:
	std::list<Frame*> frames;
	iterator current_;
	const iterator& end_;
public:
	Parser(
		iterator begin,
		const iterator& end
	):	current_(begin), end_(end)
	{}

	~Parser()
	{
		while(frames.size()) {
			end_frame();
		}
	}
	
	struct Frame
	{
		Frame(iterator& begin_) : begin(begin_) {}
		
		iterator begin;
	};

	void
	start_frame()
	{
		frames.push_back(new Frame(current_));
	}
	
	void
	end_frame()
	{
		Frame * frame = frames.back();
		frames.pop_back();
		delete frame;
	}
	
	void
	abort_frame()
	{
		Frame * frame = frames.back();
		frames.pop_back();
		current_ = frame->begin;
		delete frame;
	}

	template <PGParse::TokenId ID>
	class T
	{
	public:
		bool 
		operator () (iterator& current, iterator* out = 0)
		{
			if (current->id() != ID) {
				return false;
			}
			if (out) {
				*out = current;
			}
			current ++;
			return true;
		}
	};

	
	bool
	token(PGParse::TokenId token_id, iterator* out = 0)
	{
		if (current_->id() != token_id) {
			return false;
		}
		if (out) {
			*out = current_;
		}
		current_ ++;
		return true;
	}

/*	
	template <
		class ITEM,
		bool (ITEM::*FUNC)(ITEM**)
	>
	bool
	zero_or_more_of(std::list<ITEM*>& out)
	{
		ITEM *item = 0;
		while(boost::bind(FUNC, this, &item)) {
			out.push_back(item);
		}
		// Always successful, since an empty sequence is valid.
		return true;
	}

	template <
		class ITEM,
		bool (Parser::*FUNC)(ITEM**) 
	>
	bool
	one_or_more_of(std::list<ITEM*>& out)
	{
		ITEM *item = 0;
		while(boost::bind(FUNC, this, &item)) {
			out.push_back(item);
		}
		// Item still holds the last item found.
		return item != 0;
	}
*/	

	template<class ITEM>
	bool
	parse(iterator *out = 0)
	{
		ITEM p;
		return p(current_, out);
	}
	
	typedef LOKI_TYPELIST_3(T<DROP_KW>, T<TABLE_KW>, T<IDENTIFIER_T>) DropTable_rule;
	
	bool
	drop_table(Statement **out)
	{
		//TokenParser<DROP_KW> drop_kw;
		iterator identifier;
		if (
			parse< T<DROP_KW> >()  //drop_kw(current_)   // token(DROP_KW)
			&& parse< T<TABLE_KW> >() //&& token(TABLE_KW)
			&& parse< T<IDENTIFIER_T> >(&identifier) // && token(IDENTIFIER_T, &identifier)
		) {
			std::cout << "identifier " << identifier->idString() << std::endl;
			*out = new DropTableStatement(identifier);
			return true;
		}
		return false;
	}
	
	bool
	statement(Statement** out)
	{
		Statement *out_ = 0;
		if (drop_table(&out_)) {
			if (token(SEMI_COLON_T)) {
				*out = out_;
				return true;
			}
			delete out_;
		}
		return false;
	}
/*	
	bool
	statement_block(StatementBlock **out)
	{
		*out = new StatementBlock();
		return zero_or_more_of<Statement,&Parser::statement>((*out)->statements);
	}
*/
};

void 
parse(
	TokenList::const_iterator begin, 
	const TokenList::const_iterator& end
)
{
	Statement *statement;
	Parser parser(begin, end);
	if (parser.statement(&statement)) {
		std::cout << "match!" << std::endl;
		delete statement;
	} else {
		std::cout << "fail!" << std::endl;
	}
	/*
	while (begin != end) 
	{
		std::cout << begin->idString() << std::endl;
		begin ++;
	}*/
}

} // PGParse

int main()
{
	const char *bytes = "Drop /* C-style comment*/  Table a_table_name;";
	
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes,len);
	
	PGParse::parse(
		scanner.tokensBegin(PGParse::TOKEN_IS_IGNORED),
		scanner.tokensEnd()
	);
}
