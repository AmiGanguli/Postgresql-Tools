#if !defined (PGPARSE_TOKEN_H)
#define PGPARSE_TOKEN_H

#include <vector>
#include <cstddef>

#include "TokenId.h"

namespace PGParse {

class Token
{
private:
	std::size_t offset_;
	std::size_t length_;
	TokenId id_;
public:
	Token(std::size_t offset, std::size_t length, TokenId id)
		: offset_(offset), length_(length), id_(id)
	{}

	std::size_t
	offset() const 
	{
		return offset_;
	}

	std::size_t
	length() const 
	{
		return length_;
	}
	
	TokenId
	id() const
	{
		return id_;
	}

	TokenCategory 
	category() const
	{
		return PGParse::category(id_);
	}
	
	const char * 
	idString() const
	{
		return PGParse::idString(id_);
	}
	
	const char * 
	categoryString() const
	{
		return PGParse::categoryString(category());
	}
};
typedef std::vector<Token> TokenList;

}

#endif // PGPARSE_TOKEN_H
