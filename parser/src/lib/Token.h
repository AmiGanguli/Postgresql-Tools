#if !defined (PGPARSE_TOKEN_H)
#define PGPARSE_TOKEN_H

#include <vector>
#include <cstddef>
#include <boost/iterator/iterator_facade.hpp>

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

	int 
	category() const
	{
		return PGParse::category(id_);
	}
	
	const char * 
	idString() const
	{
		return PGParse::idString(id_);
	}
	
	std::string
	categoryString() const
	{
		return PGParse::categoryString(category());
	}
	
	bool
	is(int flags) const
	{
		return category() & flags;
	}
};

//typedef std::vector<Token> TokenList;

typedef std::vector<Token> TokenListBase;

class TokenList : public TokenListBase
{
public:
	class const_iterator : public boost::iterator_facade<
		const_iterator,
		Token const,
		boost::forward_traversal_tag
	>
	{
	public:
		const_iterator()
		: 	tokens_(0), flag_filter_(0), iterator_()
		{
		}

		explicit 
		const_iterator(const TokenList* tokens, int flag_filter)
		: 	tokens_(tokens), 
			flag_filter_(flag_filter),
			iterator_(tokens->TokenListBase::begin())
		{
		}

		explicit 
		const_iterator(const TokenList* tokens)
		: 	tokens_(tokens), 
			flag_filter_(0),
			iterator_(tokens->TokenListBase::end())
		{
		}
		
	private:
		friend class boost::iterator_core_access;
		
		void
		increment()
		{
			do {
				iterator_ ++;
			} while (iterator_!= tokens_->TokenListBase::end() && iterator_->is(flag_filter_));
		}
		
		bool
		equal(const_iterator const& other) const
		{
			return iterator_ == other.iterator_;
		}
		
		const Token&
		dereference() const
		{
			return *iterator_;
		}
		
		const TokenList* tokens_;
		int flag_filter_;
		TokenListBase::const_iterator iterator_;
	};
	
	const_iterator 
	begin(int filter = 0) const 
	{
		return const_iterator(this, filter);
	}
	
	const_iterator
	end() const
	{
		return const_iterator(this);
	}
};

}

#endif // PGPARSE_TOKEN_H
