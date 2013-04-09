#include "Scanner.h"
#include <iostream>
#include <cstring>
#include <list>
#include <boost/bind.hpp>

namespace PGParse {
	
class Statement
{
};

class DropTableStatement : public Statement
{
public:
	DropTableStatement(TokenList::const_iterator table_name)	{}
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
		bool (ITEM::*FUNC)(ITEM**) 
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
	
	bool
	drop_table(Statement **out)
	{
		iterator identifier;
		if (
			token(DROP_KW)
			&& token(TABLE_KW)
			&& token(IDENTIFIER_T, &identifier)
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
