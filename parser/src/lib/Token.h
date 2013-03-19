#if !defined (PGPARSE_TOKEN_H)
#define PGPARSE_TOKEN_H

#include <vector>
#include <cstddef>

namespace PGParse {

// Extract token identifiers from kwlist.h
//
#define PG_KEYWORD(text, id, category)		id##_KW,
enum TokenId {
	INVALID,
#include "kwlist.h"
	KW_SENTINAL,
	BIT_STRING_T,
	WHITESPACE_T,
	COMMENT_T,
	TOKEN_SENTINAL,
	UNTERMINATED_C_COMMENT_E,
	ERROR_SENTINAL,
	FINAL_SENTINAL
};
#undef PG_KEYWORD

enum TokenCategory {
	INVALID_TOKEN,
	LITERAL_TOKEN,
	UNRESERVED_KEYWORD,
	RESERVED_KEYWORD,
	TYPE_FUNC_NAME_KEYWORD,
	COL_NAME_KEYWORD,
	WHITESPACE_TOKEN,
	COMMENT_TOKEN,
	ERROR_TOKEN
};

class Token
{
private:
	std::size_t offset_;
	TokenId id_;
public:
	Token(std::size_t offset, TokenId id)
		: offset_(offset), id_(id)
	{}

	std::size_t
	offset() const 
	{
		return offset_;
	}
	
	TokenId
	id() const
	{
		return id_;
	}

	static TokenId keywordToId(
		const char *text,
		TokenId from = TokenId(INVALID +1),
		TokenId to = KW_SENTINAL
	);
	
	TokenCategory category() const;
	const char * idString() const;
	const char * categoryString() const;
};
typedef std::vector<Token> TokenList;

}

#endif // PGPARSE_TOKEN_H