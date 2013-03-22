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
	HEX_STRING_T,
	UNI_STRING_T,
	STRING_T,
	DOLQ_STRING_T,
	NCHAR_FLAG_T,
	IDENTIFIER_T,
	DQ_IDENTIFIER_T,
	UNICODE_IDENTIFIER_T,
	WHITESPACE_T,
	COMMENT_T,
	TOKEN_SENTINAL,
	UNTERMINATED_C_COMMENT_E,
	UNTERMINATED_BIT_STRING_E,
	UNTERMINATED_HEX_STRING_E,
	UNTERMINATED_QUOTED_STRING_E,
	UNTERMINATED_QUOTED_IDENTIFIER_E,
	UNTERMINATED_DOLQUOTE_STRING_E,
	STANDARD_CONFORMING_STRINGS_DISABLED_E,
	INVALID_UNICODE_ESCAPE_CHAR_E,
	INVALID_UNICODE_SURROGATE_PAIR_E,
	MALFORMED_DOLLAR_QUOTE_E,
	ZERO_LENGTH_QUOTED_IDENTIFIER_E,
	ZERO_LENGTH_UNICODE_IDENTIFIER_E,
	ERROR_SENTINAL,
	FINAL_SENTINAL
};
#undef PG_KEYWORD

enum TokenCategory {
	INVALID_TOKEN,
	LITERAL_TOKEN,
	IDENTIFIER_TOKEN,
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