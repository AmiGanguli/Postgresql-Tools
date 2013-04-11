#include "Scanner.h"
#include <iostream>
#include <cstring>
#include <list>
#include <boost/bind.hpp>
#include <loki/Sequence.h>
#include <loki/HierarchyGenerators.h>

namespace PGParse {
	
typedef TokenList::const_iterator iterator;
	
template <PGParse::TokenId ID>
class T
{
public:
	bool 
	operator () (iterator& current, iterator* out = 0)
	{
		if (current->id() != ID) {
			return false;
		}
		if (out) {
			*out = current;
		}
		current ++;
		return true;
	}
};

/*
template <class RULE>
struct Rule
{
	typename RULE::Type::Head *head;
	typename RULE::Type::Tail tail;
	typedef Loki::Tuple<RULE> Result;
		~Rule()
	{
		delete head;
	}
};
	
template <>
struct Rule<Loki::NullType>
{};
*/

template <class ITEMS>
void
parse(Loki::Tuple<ITEMS> &items, iterator begin, const iterator& end)
{
	Loki::Field<0>(items) = 0;
	
}

template <>
void
parse<Loki::NullType>(Loki::Tuple<Loki::NullType> &items, iterator begin, const iterator& end)
{
	return;
}

template <class ITEMS>
class Node : public Loki::Tuple<ITEMS>
{
public:
	void 
	parse(iterator begin, const iterator& end)
	{
		PGParse::parse<ITEMS>(*this, begin,end);
		
	}
};


typedef Loki::Seq< T<DROP_KW>, T<TABLE_KW>, T<IDENTIFIER_T> > DropTableRule;
typedef Node< Loki::Seq< T<DROP_KW>, T<TABLE_KW>, T<IDENTIFIER_T> > > DropTableNode;
	

} // PGParse

int main()
{
	const char *bytes = "Drop /* C-style comment*/  Table a_table_name;";
	
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes,len);
	
	PGParse::DropTableNode drop_table;
	
	drop_table.parse(
		scanner.tokensBegin(),
		scanner.tokensEnd()
	);
}
