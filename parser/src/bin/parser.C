#include "Scanner.h"
#include <iostream>
#include <cstring>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("keywordToId/round-trip", "Convert from keyword id to string and back again")
{
	for (PGParse::TokenId i = PGParse::INVALID; i < PGParse::KW_SENTINAL; i = PGParse::TokenId(i + 1)) {
		PGParse::Token token(0, i);
		const char *text = token.idString();
		PGParse::TokenId id = PGParse::Token::keywordToId(text);
		REQUIRE (id == i);
	}
}

TEST_CASE("keywordToId/case-insensitive", "SQL keywords are not case-sensitive")
{
	REQUIRE (PGParse::Token::keywordToId("aCcEss") == PGParse::ACCESS_KW);
}


TEST_CASE("Scanner::scan/sql-comments1", "SQL-style comments")
{
	const char *bytes = "    -- This is a comment\n   -- And another";
	//                   0000000000111111111122222 222223
	//                   0123456789012345678901234 567890
	PGParse::Token correct[] = {
		{0, PGParse::WHITESPACE_T},
		{4, PGParse::COMMENT_T},
		{24, PGParse::WHITESPACE_T},
		{28, PGParse::COMMENT_T}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 4);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::COMMENT_T},
		{14, PGParse::WHITESPACE_T},
		{16, PGParse::COMMENT_T},
		{30, PGParse::WHITESPACE_T},
		{31, PGParse::COMMENT_T},
		{45, PGParse::WHITESPACE_T},
		{47, PGParse::UNTERMINATED_C_COMMENT_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 9);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::BIT_STRING_T},
		{10, PGParse::WHITESPACE_T},
		{11, PGParse::BIT_STRING_T},
		{21, PGParse::WHITESPACE_T},
		{23, PGParse::UNTERMINATED_BIT_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 7);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::HEX_STRING_T},
		{10, PGParse::WHITESPACE_T},
		{11, PGParse::HEX_STRING_T},
		{21, PGParse::WHITESPACE_T},
		{23, PGParse::UNTERMINATED_HEX_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		REQUIRE(j < 7);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::STRING_T},
		{14, PGParse::WHITESPACE_T},
		{15, PGParse::UNTERMINATED_QUOTED_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin(); 
		i != scanner.tokensEnd(); 
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 5);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::STRING_T},
		{15, PGParse::WHITESPACE_T},
		{16, PGParse::UNTERMINATED_QUOTED_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 5);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::DOLQ_STRING_T},
		{25, PGParse::WHITESPACE_T},
		{26, PGParse::DOLQ_STRING_T},
		{61, PGParse::WHITESPACE_T},
		{62, PGParse::MALFORMED_DOLLAR_QUOTE_E},
		{63, PGParse::INVALID},
		{64, PGParse::INVALID},
		{65, PGParse::INVALID},
		{66, PGParse::INVALID},
		{67, PGParse::WHITESPACE_T},
		{68, PGParse::UNTERMINATED_DOLQUOTE_STRING_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 12);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 12);
}

TEST_CASE("Scanner::scan/quoted-identifier", "Quoted identifiers")
{
	const char *bytes = " \"hello world\"  \"one";
	//                   0 000000000111 111 11112222
	//                   0 123456789012 345 67890127
	PGParse::Token correct[] = {
		{0, PGParse::WHITESPACE_T},
		{1, PGParse::DQ_IDENTIFIER_T},
		{14, PGParse::WHITESPACE_T},
		{16, PGParse::UNTERMINATED_QUOTED_IDENTIFIER_E}
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << std::endl;
		REQUIRE(j < 4);
		REQUIRE(i->offset() == correct[j].offset());
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
		{0, PGParse::TYPECAST_T},	// 0
		{2, PGParse::WHITESPACE_T},	// 1
		{3, PGParse::DOTDOT_T},		// 2
		{5, PGParse::WHITESPACE_T},	// 3
		{6, PGParse::COLONEQUALS_T},	// 4
		{8, PGParse::WHITESPACE_T},	// 5
		{9, PGParse::OPERATOR_T},	// 6
		{10, PGParse::WHITESPACE_T},	// 7
		{11, PGParse::OPERATOR_T},	// 8
		{12, PGParse::WHITESPACE_T},	// 9
		{13, PGParse::OPERATOR_T},	// 10
		{14, PGParse::WHITESPACE_T},	// 11
		{15, PGParse::OPERATOR_T},	// 12
		{16, PGParse::WHITESPACE_T},	// 13
		{17, PGParse::OPERATOR_T},	// 14
		{18, PGParse::WHITESPACE_T},	// 15
		{19, PGParse::OPERATOR_T},	// 16
		{20, PGParse::WHITESPACE_T},	// 17
		{21, PGParse::OPERATOR_T},	// 18
		{22, PGParse::WHITESPACE_T},	// 19
		{23, PGParse::OPERATOR_T},	// 20
		{24, PGParse::WHITESPACE_T},	// 21
		{25, PGParse::OPERATOR_T},	// 22
		{26, PGParse::WHITESPACE_T},	// 23
		{27, PGParse::OPERATOR_T},	// 24
		{28, PGParse::WHITESPACE_T},	// 25
		{29, PGParse::OPERATOR_T},	// 26
		{30, PGParse::WHITESPACE_T},	// 27
		{31, PGParse::OPERATOR_T},	// 28
		{32, PGParse::WHITESPACE_T},	// 29
		{33, PGParse::OPERATOR_T},	// 30
		{34, PGParse::WHITESPACE_T},	// 31
		{35, PGParse::OPERATOR_T},	// 32
		{36, PGParse::WHITESPACE_T},	// 33
		{37, PGParse::OPERATOR_T},	// 34
		{38, PGParse::WHITESPACE_T},	// 35
		{39, PGParse::OPERATOR_T},	// 36
		{40, PGParse::WHITESPACE_T},	// 37
		{41, PGParse::OPERATOR_T},	// 38
		{42, PGParse::WHITESPACE_T},	// 39
		{43, PGParse::OPERATOR_T},	// 40
		{60, PGParse::INVALID}		// 41
	};
	REQUIRE (true);
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);
	int j = 0;
	for (
		PGParse::TokenList::const_iterator i = scanner.tokensBegin();
		i != scanner.tokensEnd();
		i ++, j ++
	) {
		//std::cout << i->offset() << i->idString() << std::endl;
		REQUIRE(j < 42);
		REQUIRE(i->offset() == correct[j].offset());
		REQUIRE(i->id() == correct[j].id());
	}
	REQUIRE(j == 42);
}


#if 0

int
main(int argc, char **argv)
{
	bool passed = true;
	for (PGParse::TokenId i = PGParse::INVALID; i < PGParse::KW_SENTINAL; i = PGParse::TokenId(i + 1)) {
		PGParse::Token token(0, i);
		const char *text = token.idString();
		PGParse::TokenId id = PGParse::Token::keywordToId(text);
		if (id != i) {
			passed = false;
		}
	}
	if (passed) {
		std::cout << "Round-trip conversions passed." << std::endl;
	} else {
		std::cout << "Round-trip conversions failed." << std::endl;
	}
	if (PGParse::Token::keywordToId("aCcEss") == PGParse::ACCESS_KW) {
		std::cout << "Case insensitive conversion passed." << std::endl;
	} else {
		std::cout << "Case insensitive conversion failed." << std::endl;
	}

	const char *bytes = " n'some text' Hello world B'ABCD' b'abcd' /*comment */ x'12ff'  /*  stuff /* nested */ */ x'22";
	//                   00000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999
        //                   01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);

	for (PGParse::TokenList::const_iterator i = scanner.tokensBegin(); i != scanner.tokensEnd(); i ++) {
		std::cout << "token: " << i->idString() << " at " << i->offset() << std::endl;
	}
	
	return 0;
}

#endif