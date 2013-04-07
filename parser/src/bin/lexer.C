#include "Scanner.h"
#include <iostream>
#include <cstring>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("keywordToId/round-trip", "Convert from keyword id to string and back again")
{
	for (PGParse::TokenId i = PGParse::INVALID; i < PGParse::KW_SENTINAL; i = PGParse::TokenId(i + 1)) {
		PGParse::Token token(0, 0, i);
		const char *text = token.idString();
		PGParse::TokenId id = PGParse::keywordToId(text);
		REQUIRE (id == i);
	}
}

TEST_CASE("keywordToId/case-insensitive", "SQL keywords are not case-sensitive")
{
	REQUIRE (PGParse::keywordToId("aCcEss") == PGParse::ACCESS_KW);
}


TEST_CASE("Scanner::scan/sql-comments1", "SQL-style comments")
{
	const char *bytes = "    -- This is a comment\n   -- And another";
	//                   0000000000111111111122222 222223
	//                   0123456789012345678901234 567890
	PGParse::Token correct[] = {
		{0, 4, PGParse::WHITESPACE_T},
		{4, 20, PGParse::COMMENT_T},
		{24, 4, PGParse::WHITESPACE_T},
		{28, 14, PGParse::COMMENT_T}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 4);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 4);
}

TEST_CASE("Scanner::scan/c-comments1", "C-style comments")
{
	const char *bytes = " /*comment 1*/  /* comment 2*/ /*  /*  */  */  /*  /*  */ ";
	//                   000000000011111111112222222222333333333344444444445555555555
	//                   012345678901234567890123456789012345678901234567890123456789
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 13, PGParse::COMMENT_T},
		{14, 2, PGParse::WHITESPACE_T},
		{16, 14, PGParse::COMMENT_T},
		{30, 1, PGParse::WHITESPACE_T},
		{31, 14, PGParse::COMMENT_T},
		{45, 2, PGParse::WHITESPACE_T},
		{47, 11, PGParse::UNTERMINATED_C_COMMENT_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 9);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 8);
}

TEST_CASE("Scanner::scan/bit-string1", "Binary bit strings.")
{
	// NOTE: the lexer doesn't validate the contents of the
	// string, so we don't test for this here.
	//
	const char *bytes = " b'010101' B'1111222'  b'11";
	//                   0000000000111111111122222222
	//                   0123456789012345678901234567
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 9, PGParse::BIT_STRING_T},
		{10, 1, PGParse::WHITESPACE_T},
		{11, 10, PGParse::BIT_STRING_T},
		{21, 2, PGParse::WHITESPACE_T},
		{23, 4, PGParse::UNTERMINATED_BIT_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 7);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 6);
}

TEST_CASE("Scanner::scan/hex-string1", "Binary hex strings.")
{
	// NOTE: the lexer doesn't validate the contents of the
	// string, so we don't test for this here.
	//
	const char *bytes = " x'12abc3' X'1111222'  x'11";
	//                   0000000000111111111122222222
	//                   0123456789012345678901234567
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 9, PGParse::HEX_STRING_T},
		{10, 1, PGParse::WHITESPACE_T},
		{11, 10, PGParse::HEX_STRING_T},
		{21, 2, PGParse::WHITESPACE_T},
		{23, 4, PGParse::UNTERMINATED_HEX_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		REQUIRE(j < 7);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 6);

}

TEST_CASE("Scanner::scan/quote1", "Simple single quotes.")
{
	const char *bytes = " 'hello world' 'unterminated";
	//                   0000000000111111111122222222223
	//                   0123456789012345678901234567890
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 13, PGParse::STRING_T},
		{14, 1, PGParse::WHITESPACE_T},
		{15, 13, PGParse::UNTERMINATED_QUOTED_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 5);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 4);
}

TEST_CASE("Scanner::scan/quote2", "Extended single quotes.")
{
	const char *bytes = " E'hello world' e'unterminated";
	//                   0000000000111111111122222222223
	//                   0123456789012345678901234567890
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 14, PGParse::STRING_T},
		{15, 1, PGParse::WHITESPACE_T},
		{16, 14, PGParse::UNTERMINATED_QUOTED_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 5);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 4);
}

// FIXME: lots of escape code cases to test.  Probably need to revisit this
//        anyway, since we should either pass the escape sequences untouched
//        (simplifying the lexer) or actually parse them properly here and
//        save the value.
//

TEST_CASE("Scanner::scan/dolquote1", "Dollar quotes.")
{
	const char *bytes = " $hello$ $world$  $hello$ $stuff$ this is some new $un$stuff$ $jump $jump$ ";
	//                   0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889
	//                   0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 24, PGParse::DOLQ_STRING_T},
		{25, 1, PGParse::WHITESPACE_T},
		{26, 35, PGParse::DOLQ_STRING_T},
		{61, 1, PGParse::WHITESPACE_T},
		{62, 1, PGParse::MALFORMED_DOLLAR_QUOTE_E},
		{63, 4, PGParse::IDENTIFIER_T},
		{67, 1, PGParse::WHITESPACE_T},
		{68, 7, PGParse::UNTERMINATED_DOLQUOTE_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 9);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 9);
}

TEST_CASE("Scanner::scan/quoted-identifier", "Quoted identifiers")
{
	const char *bytes = " \"hello world\"  \"one";
	//                   0 000000000111 111 11112222
	//                   0 123456789012 345 67890127
	PGParse::Token correct[] = {
		{0, 1, PGParse::WHITESPACE_T},
		{1, 13, PGParse::DQ_IDENTIFIER_T},
		{14, 2, PGParse::WHITESPACE_T},
		{16, 4, PGParse::UNTERMINATED_QUOTED_IDENTIFIER_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 4);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 4);
}

TEST_CASE("Scanner::scan/operators1", "Misc operators")
{
	const char *bytes = ":: .. := , ( ) [ ] . ; : + - * / % ^ < > = ~!@#^&|`?+-*/%<>=";
	//                   0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889
	//                   0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
	PGParse::Token correct[] = {
		{0, 2, PGParse::TYPECAST_T},	// 0
		{2, 1, PGParse::WHITESPACE_T},	// 1
		{3, 2, PGParse::DOTDOT_T},		// 2
		{5, 1, PGParse::WHITESPACE_T},	// 3
		{6, 2, PGParse::COLONEQUALS_T},	// 4
		{8, 1, PGParse::WHITESPACE_T},	// 5
		{9, 1, PGParse::COMMA_T},	// 6
		{10, 1, PGParse::WHITESPACE_T},	// 7
		{11, 1, PGParse::OPEN_PAREN_T},	// 8
		{12, 1, PGParse::WHITESPACE_T},	// 9
		{13, 1, PGParse::CLOSE_PAREN_T},	// 10
		{14, 1, PGParse::WHITESPACE_T},	// 11
		{15, 1, PGParse::OPEN_BRACKET_T},	// 12
		{16, 1, PGParse::WHITESPACE_T},	// 13
		{17, 1, PGParse::CLOSE_BRACKET_T},	// 14
		{18, 1, PGParse::WHITESPACE_T},	// 15
		{19, 1, PGParse::DOT_T},	// 16
		{20, 1, PGParse::WHITESPACE_T},	// 17
		{21, 1, PGParse::SEMI_COLON_T},	// 18
		{22, 1, PGParse::WHITESPACE_T},	// 19
		{23, 1, PGParse::COLON_T},	// 20
		{24, 1, PGParse::WHITESPACE_T},	// 21
		{25, 1, PGParse::PLUS_T},	// 22
		{26, 1, PGParse::WHITESPACE_T},	// 23
		{27, 1, PGParse::MINUS_T},	// 24
		{28, 1, PGParse::WHITESPACE_T},	// 25
		{29, 1, PGParse::STAR_T},	// 26
		{30, 1, PGParse::WHITESPACE_T},	// 27
		{31, 1, PGParse::SLASH_T},	// 28
		{32, 1, PGParse::WHITESPACE_T},	// 29
		{33, 1, PGParse::PERCENT_T},	// 30
		{34, 1, PGParse::WHITESPACE_T},	// 31
		{35, 1, PGParse::CARET_T},	// 32
		{36, 1, PGParse::WHITESPACE_T},	// 33
		{37, 1, PGParse::LESS_THAN_T},	// 34
		{38, 1, PGParse::WHITESPACE_T},	// 35
		{39, 1, PGParse::GREATER_THAN_T},	// 36
		{40, 1, PGParse::WHITESPACE_T},	// 37
		{41, 1, PGParse::EQUAL_T},	// 38
		{42, 1, PGParse::WHITESPACE_T},	// 39
		{43, 17, PGParse::OPERATOR_T}	// 40
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 41);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 41);
}

TEST_CASE("Scanner::scan/param1", "Numbered parameters")
{
	const char *bytes = "$1 $2 $3";
	//                   00000000001111111111
	//                   01234567890123456789
	PGParse::Token correct[] = {
		{0, 2, PGParse::PARAM_T},		// 0
		{2, 1, PGParse::WHITESPACE_T},	// 1
		{3, 2, PGParse::PARAM_T},		// 2
		{5, 1, PGParse::WHITESPACE_T},	// 3
		{6, 2, PGParse::PARAM_T}		// 4
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 5);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 5);
}

TEST_CASE("Scanner::scan/numbers1", "Numeric literals")
{
	const char *bytes = "123 0 31.322 3.222e32 4.11e-3";
	//                   000000000011111111112222222222
	//                   012345678901234567890123456789
	PGParse::Token correct[] = {
		{0, 3, PGParse::INTEGER_T},	// 0
		{3, 1, PGParse::WHITESPACE_T},	// 1
		{4, 1, PGParse::INTEGER_T},	// 2
		{5, 1, PGParse::WHITESPACE_T},	// 3
		{6, 6, PGParse::FLOAT_T},		// 4
		{12, 1, PGParse::WHITESPACE_T},	// 5
		{13, 8, PGParse::FLOAT_T},		// 6
		{21, 1, PGParse::WHITESPACE_T},	// 7
		{22, 7, PGParse::FLOAT_T}		// 8
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 9);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 9);
}

TEST_CASE("Scanner::scan/identifiers1", "Bare identifiers and keywords")
{
	const char *bytes = "if then end if hello world";
	//                   000000000011111111112222222222
	//                   012345678901234567890123456789
	PGParse::Token correct[] = {
		{0, 2, PGParse::IF_P_KW},		// 0
		{2, 1, PGParse::WHITESPACE_T},	// 1
		{3, 4, PGParse::THEN_KW},		// 2
		{7, 1, PGParse::WHITESPACE_T},	// 3
		{8, 3, PGParse::END_P_KW},		// 4
		{11, 1, PGParse::WHITESPACE_T},	// 5
		{12, 2, PGParse::IF_P_KW},		// 6
		{14, 1, PGParse::WHITESPACE_T},	// 7
		{15, 5, PGParse::IDENTIFIER_T},	// 8
		{20, 1, PGParse::WHITESPACE_T},	// 9
		{21, 5, PGParse::IDENTIFIER_T}	// 10
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 11);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 11);
}

TEST_CASE("Scanner::scan/filters1", "Iterate through tokens with a filter")
{
	const char *bytes = "if then end if hello world";
	//                   000000000011111111112222222222
	//                   012345678901234567890123456789
	PGParse::Token correct[] = {
		{0, 2, PGParse::IF_P_KW},		// 0
		{3, 4, PGParse::THEN_KW},		// 1
		{8, 3, PGParse::END_P_KW},		// 2
		{12, 2, PGParse::IF_P_KW},		// 3
		{15, 5, PGParse::IDENTIFIER_T},		// 4
		{21, 5, PGParse::IDENTIFIER_T}		// 5
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(PGParse::TOKEN_IS_IGNORED);
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 6);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->length() == correct[j].length());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 6);
}
