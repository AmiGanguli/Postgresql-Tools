#include "Scanner.h"
#include <iostream>
#include <cstring>

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

	const char *bytes = "Hello world B'ABCD' b'abcd' /*comment ";
	//                   0000000000111111111122222222223333333333
        //                   0123456789012345678901234567890123456789
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes, len+1);

	for (PGParse::TokenList::const_iterator i = scanner.tokensBegin(); i != scanner.tokensEnd(); i ++) {
		std::cout << "token: " << i->idString() << " at " << i->offset() << std::endl;
	}
	
	return 0;
}