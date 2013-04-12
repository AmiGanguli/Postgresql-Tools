#if !defined( PGPARSE_NODE_H )
#define PGPARSE_NODE_H

#include "Scanner.h"
#include <cstring>
#include <ctype.h>
#include <list>
#include <loki/Sequence.h>
#include <loki/HierarchyGenerators.h>
#include <string>

namespace PGParse {
	
typedef TokenList::const_iterator token_iterator;

class Node
{
public:
	virtual 
	~Node()
	{}
	
	virtual std::string asString(int indent = 0) const = 0;
	
	static std::string 
	indentString(int i)
	{
		std::string tabs;
		while (i) {
			tabs += "\t";
			i --;
		}
		return tabs;
	}
};

template <PGParse::TokenId ID>
class T : public Node
{
public:
	T(token_iterator token_) : token(token_)
	{}
	
	~T()
	{}
	
	std::string
	asString(int indent = 0) const
	{
		std::string ret("<");
		const char *s = token->idString();
		while (*s) {
			ret +=toupper(*s);
			s ++;
		}
		ret += '>';
		return ret;
	}
	
	token_iterator token;
	
	static std::string
	ruleString(int indent = 0)
	{
		return std::string("<") + idString(ID) + ">";
	}

	static T *
	parse (token_iterator &begin, const token_iterator &end)
	{
		if (begin == end || begin->id() != ID) {
			return 0;
		}
		T *ret = new T(begin);
		begin ++;
		return ret;
	}
};

using namespace Loki;

template <
	class T01=NullType,class T02=NullType,class T03=NullType,class T04=NullType,class T05=NullType,
	class T06=NullType,class T07=NullType,class T08=NullType,class T09=NullType,class T10=NullType,
	class T11=NullType,class T12=NullType,class T13=NullType,class T14=NullType,class T15=NullType,
	class T16=NullType,class T17=NullType,class T18=NullType,class T19=NullType,class T20=NullType
>
class S : public Node
{
public:
	typedef T01 Head;
	typedef S<T02,T03,T04,T05,T06,T07,T08,T09,T10,
	             T11,T12,T13,T14,T15,T16,T17,T18,T19,T20> TailNode;

private:
	Node *head_;
	Node *tail_;
	
	S(Node *head = 0, Node *tail = 0) : head_(head), tail_(tail)
	{}

public:
	~S()
	{
		delete head_;
		delete tail_;
	}
	
	std::string
	asString(int indent = 0) const
	{
		return 
			std::string("") 
			+ head_->asString() 
			+ " " 
			+ tail_->asString();
	}
	
	static std::string
	ruleString(int indent = 0)
	{
		return Head::ruleString(indent) + " " + TailNode::ruleString(indent);
	}
	
	static S *
	parse (token_iterator &begin, const token_iterator &end)
	{
		S *result = 0;
		Node *head = Head::parse(begin, end);
		Node *tail = 0;
		if (head) {
			tail = TailNode::parse(begin, end);
			if (!tail) {
				delete head;
			} else {
				result = new S(head, tail);
			}
		}
		return result;
	}
};

template <>
class S< NullType > : public Node
{
public:
	~S()
	{}
	
	std::string
	asString(int indent = 0) const
	{
		return std::string("");
	}

	static std::string
	ruleString(int indent = 0)
	{
		return std::string("");
	}
	
	static S *
	parse (token_iterator &begin, const token_iterator &end)
	{
		return new S();
	}
};

template <class NODE>
class ZeroOrMore : public Node
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
	asString(int indent = 0) const
	{
		std::string ret;
		ret += indentString(indent) + " { \n";
		typename std::list<NODE*>::const_iterator i = nodes_.begin();
		for ( ; i != nodes_.end(); i++) {
			ret += indentString(indent) + (*i)->asString(indent +1) + "\n";
		}
		ret += indentString(indent) + " }*\n";
		return ret;
	}

	static std::string
	ruleString(int indent = 0)
	{
		return std::string(" { ") + NODE::ruleString(indent) + " }* ";
	}
	
	static ZeroOrMore *
	parse (token_iterator &begin, const token_iterator &end)
	{
		ZeroOrMore *ret = new ZeroOrMore();
		NODE *result = 0;
		while (result = NODE::parse(begin, end)) {
			ret->append(result);
		}
		return ret;
	}
};

template <class NODE>
class ZeroOrOne : public Node
{
private:
	NODE* node_;
	
	ZeroOrOne(NODE *node) : node_(node)
	{}

public:
	~ZeroOrOne()
	{
		delete node_;
	}
	
	std::string
	asString(int indent = 0) const
	{
		if (node_) {
			return std::string(" [ ") + node_->asString(indent) = " ]? " ;
		} else {
			return std::string("");
		}
	}

	static std::string
	ruleString(int indent = 0)
	{
		return std::string(" [ ") + NODE::ruleString(indent) + " ]? ";
	}
	
	static ZeroOrOne *
	parse (token_iterator &begin, const token_iterator &end)
	{
		return new ZeroOrOne(NODE::parse(begin, end));
	}
};

template <
	class T01=NullType,class T02=NullType,class T03=NullType,class T04=NullType,class T05=NullType,
	class T06=NullType,class T07=NullType,class T08=NullType,class T09=NullType,class T10=NullType,
	class T11=NullType,class T12=NullType,class T13=NullType,class T14=NullType,class T15=NullType,
	class T16=NullType,class T17=NullType,class T18=NullType,class T19=NullType,class T20=NullType
>
class OneOf : public Node
{
public:
	typedef T01 Head;
	typedef OneOf<T02,T03,T04,T05,T06,T07,T08,T09,T10,
	             T11,T12,T13,T14,T15,T16,T17,T18,T19,T20> TailNode;

private:
	Node *node_;
	
	OneOf(Node *node) : node_(node)
	{}

public:
	~OneOf()
	{
		delete node_;
	}
	
	std::string
	asString(int indent = 0) const
	{
		return 
			std::string("") 
			+ node_->asString();
	}
	
	static std::string
	ruleStringRest(int indent = 0)
	{
		return std::string(" | ")
			+ Head::ruleString(indent) 
			+ TailNode::ruleStringRest(indent);
	}
	
	static std::string
	ruleString(int indent = 0)
	{
		return std::string(" [ ")
			+ Head::ruleString(indent) 
			+ TailNode::ruleStringRest(indent)
			+ " ] ";
	}
	
	static Node *
	parseRest (token_iterator &begin, const token_iterator&end)
	{
		Node *result = Head::parse(begin, end);
		if (result) {
			return result;
		}
		return TailNode::parseRest(begin, end);
	}
	
	static OneOf *
	parse (token_iterator &begin, const token_iterator &end)
	{
		Node *node = parseRest(begin,end);
		if (node) {
			return new OneOf(node);
		}
		return 0;
	}
};


template <>
class OneOf< NullType > : public Node
{
public:
	~OneOf()
	{}
	
	std::string
	asString(int indent = 0) const
	{
		return std::string("");
	}

	static std::string
	ruleStringRest(int indent = 0)
	{
		return std::string("");
	}
	
	static std::string
	ruleString(int indent = 0)
	{
		return std::string("");
	}
	
	static Node *
	parseRest (token_iterator &begin, const token_iterator&end)
	{
		return 0;
	}

	static OneOf *
	parse (token_iterator &begin, const token_iterator &end)
	{
		return 0;
	}
};

} // PGParse

#endif

