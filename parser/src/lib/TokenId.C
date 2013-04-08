#include <cctype>

#include "TokenId.h"
#include "ParserLemon.h"

namespace PGParse {

/** 
 * The column of the lookup table that contains the lemon 
 * token id is populated at runtime, as I couldn't think 
 * of a graceful way to do this.
 */
struct TokenMeta {
	const char *text;
	CategoryFlags category;
	int lemon_id;
};

/**
 * Macro for extracting keyword data from kwlist.h
 * 
 */
#define PG_KEYWORD(text, id, category)		{text, category, -1},

TokenMeta token_data[] = {
	{"invalid", 				INVALID_TOKEN, -1},
/**
 * Keywords imported from the Postgresql parser kwlist
 */
#include "kwlist.h"
	{"kw sentinal", 			INVALID_TOKEN, -1},

/**
 * Other token types.
 */
	{"bit string",					LITERAL_TOKEN, -1},
	{"hex string",					LITERAL_TOKEN, -1},
	{"unicode escape string", 			LITERAL_TOKEN, -1},
	{"string literal",	 			LITERAL_TOKEN, -1},
	{"integer literal",	 			LITERAL_TOKEN, -1},
	{"float literal",	 			LITERAL_TOKEN, -1},
	{"dollar quote string literal",	 		LITERAL_TOKEN, -1},
	{"national character flag",			LITERAL_TOKEN, -1},
	{"identifier",					IDENTIFIER_TOKEN, -1},
	{"double-quote identifier",			IDENTIFIER_TOKEN, -1},
	{"unicode identifier",				IDENTIFIER_TOKEN, -1},
	{"whitespace",	 				WHITESPACE_TOKEN, -1},
	{"comment",	 				COMMENT_TOKEN, -1},
	{"typecast",	 				OPERATOR_TOKEN, -1},
	{"dotdot",	 				OPERATOR_TOKEN, -1},
	{"colonequals",	 				OPERATOR_TOKEN, -1},
	{"comma",	 				OPERATOR_TOKEN, -1},
	{"open paren",	 				OPERATOR_TOKEN, -1},
	{"close paren",					OPERATOR_TOKEN, -1},
	{"open bracket", 				OPERATOR_TOKEN, -1},
	{"close bracket", 				OPERATOR_TOKEN, -1},
	{"dot",	 					OPERATOR_TOKEN, -1},
	{"semi-colon",	 				OPERATOR_TOKEN, -1},
	{"colon",	 				OPERATOR_TOKEN, -1},
	{"plus",	 				OPERATOR_TOKEN, -1},
	{"minus",	 				OPERATOR_TOKEN, -1},
	{"star",	 				OPERATOR_TOKEN, -1},
	{"slash",	 				OPERATOR_TOKEN, -1},
	{"percent",	 				OPERATOR_TOKEN, -1},
	{"caret",	 				OPERATOR_TOKEN, -1},
	{"less than",	 				OPERATOR_TOKEN, -1},
	{"greater than",				OPERATOR_TOKEN, -1},
	{"equal",	 				OPERATOR_TOKEN, -1},
	{"operator",	 				OPERATOR_TOKEN, -1},
	{"parameter",	 				PARAMETER_TOKEN, -1},
	{"token types sentinal", 			ERROR_TOKEN | INVALID_TOKEN, -1},
	{"unterminated c-style comment",		ERROR_TOKEN | COMMENT_TOKEN, -1},
	{"unterminated bit string",			ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"unterminated hex string",			ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"unterminated quoted string",			ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"unterminated quoted identifier",		ERROR_TOKEN | IDENTIFIER_TOKEN, -1},
	{"unterminated dollar quoted string",		ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"standard-conforming strings are disabled",	ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"invalid unicode escape character",		ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"invalid unicode surrogate pair",		ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"malformed dollar quote",			ERROR_TOKEN | LITERAL_TOKEN, -1},
	{"zero-length quoted identifier",		ERROR_TOKEN | IDENTIFIER_TOKEN, -1},
	{"zero-length unicode identifier",		ERROR_TOKEN | IDENTIFIER_TOKEN, -1},
	{"error sentinal",				ERROR_TOKEN | INVALID_TOKEN, -1},

	{"final sentinal", 				ERROR_TOKEN | INVALID_TOKEN, -1}
};
#undef PG_KEYWORD

struct CategoryMeta {
	const char *text;
	TokenCategory category;
};
const CategoryMeta token_categories[] = {
	{"INVALID_TOKEN", INVALID_TOKEN},
	{"LITERAL_TOKEN", LITERAL_TOKEN},
	{"IDENTIFIER_TOKEN", IDENTIFIER_TOKEN},
	{"UNRESERVED_KEYWORD", UNRESERVED_KEYWORD},
	{"RESERVED_KEYWORD", RESERVED_KEYWORD},
	{"TYPE_FUNC_NAME_KEYWORD", TYPE_FUNC_NAME_KEYWORD},
	{"COL_NAME_KEYWORD", COL_NAME_KEYWORD},
	{"WHITESPACE_TOKEN", WHITESPACE_TOKEN},
	{"COMMENT_TOKEN", COMMENT_TOKEN},
	{"OPERATOR_TOKEN", OPERATOR_TOKEN},
	{"PARAMETER_TOKEN", PARAMETER_TOKEN},
	{"ERROR_TOKEN", ERROR_TOKEN}
};

struct LemonMeta {
	int lemon_id;
	TokenId token_id;
};

const LemonMeta lemon_data[] = {
	{TK_PLUS, 	PLUS_T},
	{TK_MINUS,	MINUS_T},
	{TK_SLASH,	SLASH_T},
	{TK_STAR,	STAR_T},
	{TK_INTEGER,	INTEGER_T},
	{-1,		INVALID}
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
keywordToId(const char *text, TokenId from, TokenId to)
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

int
lemonId(TokenId id)
{
	static bool initialized = false;
	
	if (!initialized) {
		int i = 0;
		while (lemon_data[i].lemon_id != -1) {
			token_data[lemon_data[i].token_id].lemon_id = lemon_data[i].lemon_id;
			i ++;
		}
		initialized = true;
	}
	
	return token_data[id].lemon_id;
}

const char *
idString(TokenId id)
{
	return token_data[id].text;
}

std::string
categoryString(CategoryFlags cat)
{
	std::string ret;
	int i = 0;
	while (token_categories[i].category != CATEGORIES_SENTINAL)
	{
		if (cat & token_categories[i].category) {
			ret += token_categories[i].text;
			if (i) {
				ret += " ";
			}
		}
		i ++;
	}
	return ret;
}

int 	
category	(TokenId id)
{
	return token_data[id].category;
}


} // PGParse