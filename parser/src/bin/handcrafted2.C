#include "Scanner.h"
#include <iostream>
#include <cstring>
#include <list>
#include <boost/bind.hpp>
#include <loki/Sequence.h>
#include <loki/HierarchyGenerators.h>
#include <string>

namespace PGParse {
	
typedef TokenList::const_iterator iterator;

class NodeBase
{
public:
	virtual 
	~NodeBase()
	{}
	
	virtual std::string asString() const = 0;
};

template <PGParse::TokenId ID>
class T : public NodeBase
{
public:
	T(iterator token_) : token(token_)
	{}
	
	~T()
	{}
	
	std::string
	asString() const
	{
		std::cout << "Token" << std::endl;
		return std::string(token->idString());
	}
	
	iterator token;
	
	static T *
	parse (iterator &begin, const iterator &end)
	{
		std::cout << "parsing token " << idString(ID) << std::endl;
		if (begin == end) {
			std::cout << "begin == end" << std::endl;
		} else {
			std::cout << "begin is " << begin->idString() << std::endl;
		}
		if (begin == end || begin->id() != ID) {
			std::cout << "ID didn't match" << std::endl;
			return 0;
		}
		std::cout << "matched" << std::endl;
		T *ret = new T(begin);
		begin ++;
		return ret;
	}
};

/** 
 * DummySequence turns a typelist into something that looks
 * like a Loki::Seq.
 */
template <class TYPELIST>
struct DummySequence
{
	typedef TYPELIST Type;
};

template <class SEQUENCE>
class Node : public NodeBase
{
public:
	typedef typename SEQUENCE::Type::Head Head;
	typedef typename SEQUENCE::Type::Tail Tail;
	typedef Node< DummySequence<Tail> > TailNode;

private:
	Head *head_;
	TailNode *tail_;
	
	Node(Head *head = 0, TailNode *tail = 0) : head_(head), tail_(tail)
	{}

public:
	~Node()
	{
		delete head_;
		delete tail_;
	}
	
	std::string
	asString() const
	{
		//return std::string("hello world");
		
		std::cout << "node" << std::endl;
		return 
			std::string("(") 
			+ head_->asString() 
			+ " " 
			+ tail_->asString()
			+ ")";
	}
	
	static Node *
	parse (iterator &begin, const iterator &end)
	{
		Node *result = 0;
		Head *head = Head::parse(begin, end);
		TailNode *tail = 0;
		if (head) {
			std::cout << "head parsed" << std::endl;
			tail = TailNode::parse(begin, end);
			if (!tail) {
				std::cout << "tail didn't parse" << std::endl;
				delete head;
			} else {
				result = new Node(head, tail);
			}
		}
		return result;
	}
};

template <>
class Node< DummySequence<Loki::NullType> > : public NodeBase
{
public:
	~Node()
	{}
	
	std::string
	asString() const
	{
		std::cout << "dummy" << std::endl;
		return std::string("/");
	}
	
	static Node *
	parse (iterator &begin, const iterator &end)
	{
		return new Node();
	}
};

template <class NODE>
class ZeroOrMore : public NodeBase
{
private:
	std::list<NODE*> nodes_;
	
	ZeroOrMore() : nodes_()
	{}

public:
	~ZeroOrMore()
	{
		while (nodes_.size()) {
			delete nodes_.front();
			nodes_.pop_front();
		}
	}
	
	void
	append(NODE* node)
	{
		nodes_.push_back(node);
	}
	
	std::string
	asString() const
	{
		std::string ret("{");
		typename std::list<NODE*>::const_iterator i = nodes_.begin();
		for ( ; i != nodes_.end(); i++) {
			ret += (*i)->asString() + ", ";
		}
		ret += "}";
		return ret;
	}
	
	static ZeroOrMore *
	parse (iterator &begin, const iterator &end)
	{
		ZeroOrMore *ret = new ZeroOrMore();
		NODE *result = 0;
		while (result = NODE::parse(begin, end)) {
			ret->append(result);
		}
		return ret;
	}
};

typedef Node< Loki::Seq< T<DROP_KW>, T<TABLE_KW>, T<IDENTIFIER_T> > > DropTableNode;
typedef Node< Loki::Seq< DropTableNode, T<SEMI_COLON_T> > > StatementNode;
typedef ZeroOrMore< StatementNode > StatementsNode;
	

} // PGParse

int main()
{
	const char *bytes = 
		"Drop /* C-style comment*/  Table a_table_name;"
		" drop table another_table;"
	;
	
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes,len);
	
	PGParse::iterator begin = scanner.tokensBegin(PGParse::TOKEN_IS_IGNORED);
	PGParse::iterator end = scanner.tokensEnd();
	
	PGParse::StatementsNode *node = PGParse::StatementsNode::parse(begin, end);
	
	if (node) {
		std::cout << node->asString();
	} else {
		std::cout << "node didn't parse" << std::endl;
	}
	
	/*drop_table.parse(
		scanner.tokensBegin(),
		scanner.tokensEnd()
	);*/
}
