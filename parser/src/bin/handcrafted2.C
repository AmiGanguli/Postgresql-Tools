#include <iostream>

#include "Node.h"

namespace PGParse { namespace Rules {

struct Foo;	

typedef OneOf< 
	S< T<PASSWORD_KW>, Sconst> >, 
	S< T<PASSWORD_KW>, T<NULL_P_KW> >, 
	S< T<ENCRYPTED_KW>, T<PASSWORD_KW>, Sconst >, 
	T<INHERIT_KW>, 
	S< T<CONNECTION_KW>, T<LIMIT_KW>, SignedIconst> >, 
	S< T<VALID_KW>, T<UNTIL_KW>, Sconst >, 
	S< T<USER_KW>, name_list> >, 
	T<IDENT_KW>
> 	AlterOptRoleElem;

typedef OneOf< 
	AlterOptRoleElem, 
	S< T<SYSID_KW>, Iconst >, 
	S< T<ADMIN_KW>, name_list >, 
	S< T<ROLE_KW>, name_list >, 
	S< T<IN_P_KW>, name_list >, 
	S< T<IN_P_KW>, T<GROUP_P_KW>, name_list >
> 	CreateOptRoleElem;
	
typedef S< 
	T<CREATE_KW>, 
	T<ROLE_KW>, 
	RoleId, 
	ZeroOrOne( T<WITH_KW> ), 
	ZeroOrMore( CreateOptRoleElem ),
> 	CreateRole;

typedef S< 
	T<CREATE_KW>, 
	T<USER_KW>, 
	RoleId, 
	ZeroOrOne( T<WITH_KW> ), 
	ZeroOrMore( CreateOptRoleElem ),
> 	CreateUser;

typedef S< 
	T<ALTER_KW>, 
	T<ROLE_KW>, 
	RoleId, 
	ZeroOrOne( T<WITH_KW> ), 
	ZeroOrMore( AlterOptRoleElem ),
> 	AlterRole;

struct Foo : public S< T<CREATE_KW>, T<ROLE_KW>, T<IDENTIFIER_T> > { 
public:
/*
	static Foo *
	parse (token_iterator &begin, const token_iterator &end)
	{
		return 0;
	}
*/
	static constexpr char const *description = "Foo statement";
};


typedef S< T<DROP_KW>, T<TABLE_KW>, T<IDENTIFIER_T> > 		DropTable;
typedef S< OneOf< CreateRole, DropTable >, T<SEMI_COLON_T> > 	Statement;

typedef ZeroOrMore< Statement > 				Statements;
	

} } // PGParse::Rules

int main()
{
	const char *bytes = 
		"Drop /* C-style comment*/  Table a_table_name;"
		" drop table another_table;"
	;
	
	PGParse::Scanner scanner;
	std::size_t len = strlen(bytes);
	scanner.scan(bytes,len);
	
	PGParse::token_iterator begin = scanner.tokensBegin(PGParse::TOKEN_IS_IGNORED);
	PGParse::token_iterator end = scanner.tokensEnd();
	
	PGParse::Rules::Statements *node = PGParse::Rules::Statements::parse(begin, end);
	
	if (node) {
		std::cout << node->asString();
	} else {
		std::cout << "node didn't parse" << std::endl;
	}
	
	std::cout << "crate role node: " << PGParse::Rules::CreateRole::ruleString() << std::endl;
	std::cout << "Drop table node: " << PGParse::Rules::DropTable::ruleString() << std::endl;
	std::cout << "Statement node: " << PGParse::Rules::Statement::ruleString() << std::endl;
	std::cout << "Statements node: " << PGParse::Rules::Statements::ruleString() << std::endl;
	
	/*drop_table.parse(
		scanner.tokensBegin(),
		scanner.tokensEnd()
	);*/
}
