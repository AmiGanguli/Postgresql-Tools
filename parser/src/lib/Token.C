#include "Token.h"

#include <cctype>

namespace PGParse {



struct TokenMeta {
	const char* text;
	TokenCategory category;
};

/**
 * Macro for extracting keyword data from kwlist.h
 */
#define PG_KEYWORD(text, id, category)		{text, category},

/**
 * All token ids.  Some of these aren't used, since we just import the whole
 * list blindly from Postgres.  We generate tokens for whitespace, comments,
 * and errors, which is unusual for a parser but makes sense if you need to
 * annotate a source file.
 */
const TokenMeta token_data[] = {
	{"invalid", 				INVALID_TOKEN},
/**
 * Keywords imported from the Postgresql parser kwlist
 */
#include "kwlist.h"
	{"kw sentinal", 			INVALID_TOKEN},

/**
 * Other token types.
 */
	{"bit string",				LITERAL_TOKEN},
	{"hex string",				LITERAL_TOKEN},
	{"national character flag",		LITERAL_TOKEN},
	{"whitespace",	 			WHITESPACE_TOKEN},
	{"comment",	 			COMMENT_TOKEN},
	{"token types sentinal", 		INVALID_TOKEN},
	{"unterminated c-style comment",	ERROR_TOKEN},
	{"unterminated bit string",		ERROR_TOKEN},
	{"unterminated hex string",		ERROR_TOKEN},
	{"error sentinal",			INVALID_TOKEN},
	{"final sentinal", 			INVALID_TOKEN}
};
#undef PG_KEYWORD

const char *token_category_strings[] = {
	"INVALID_TOKEN",
	"LITERAL_TOKEN",
	"UNRESERVED_KEYWORD",
	"RESERVED_KEYWORD",
	"TYPE_FUNC_NAME_KEYWORD",
	"COL_NAME_KEYWORD",
	"WHITESPACE_TOKEN",
	"COMMENT_TOKEN",
	"ERROR_TOKEN"
};

// Couldn't find this in the standard library.  I'm sure it used to exist.
// For a slight efficiency improviment, only the second param is converted
// to lower case.
//
int
strcmpi(const char *a, const char *b)
{
	int diff = 0;
	while (*a && *b && !diff) {
		diff = tolower(*b) - *a;
		a ++;
		b ++;
	}
	if (!diff) {
		if (*a && !*b) {
			return -1;
		} else if (!*a && *b) {
			return 1;
		} else {
			return 0;
		}
	}
	return diff;
}

TokenId
Token::keywordToId(const char *text, TokenId from, TokenId to)
{
	TokenId middle = TokenId((from + to)/2);
	int compare = strcmpi(token_data[middle].text, text);
	if (compare == 0) {
		return middle;
	}
	if (middle == from) {
		return INVALID;
	}
	if (compare > 0) {
		return keywordToId(text, middle, to);
	}
	return keywordToId(text, from, middle);
}

TokenCategory
Token::category() const
{
	return token_data[id_].category;
}

const char *
Token::idString() const
{
	return token_data[id_].text;
}

const char *
Token::categoryString() const
{
	return token_category_strings[category()];
}


} // PGParse