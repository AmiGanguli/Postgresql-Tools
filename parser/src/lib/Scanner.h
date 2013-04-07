#if !defined (PGPARSE_SCANNER_H)
#define PGPARSE_SCANNER_H

#include "flex.h"
#include <list>

#include "Token.h"

namespace PGParse {

struct ScannerState;

class Scanner
{
private:
	ScannerState *scanner_state_;
	TokenList tokens_;
public:
	Scanner();
	~Scanner();
	void scan(const char *bytes, std::size_t len);
	
	TokenList::const_iterator tokensBegin(int filter = 0) const { return tokens_.begin(filter); }
	TokenList::const_iterator tokensEnd()   const { return tokens_.end(); }
};

}

#endif // PGPARSE_SCANNER_H