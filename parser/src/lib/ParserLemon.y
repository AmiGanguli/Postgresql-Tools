%token_prefix TK_
%token_type {int}  
   
%left PLUS MINUS.   
%left SLASH STAR.  
   
%include {   
#include <iostream>  
#include <assert.h>
#include "ParserLemon.h"
#include "Token.h"

//using namespace PGParse;

}  
   
%syntax_error {  
  std::cout << "Syntax error!" << std::endl;  
}   
   
program ::= expr(A).   { std::cout << "Result=" << A << std::endl; }  
   
expr(A) ::= expr(B) MINUS  expr(C).   { A = B - C; }  
expr(A) ::= expr(B) PLUS  expr(C).   { A = B + C; }  
expr(A) ::= expr(B) STAR  expr(C).   { A = B * C; }  
expr(A) ::= expr(B) SLASH expr(C).  { 

		if(C != 0){
			A = B / C;
		}else{
			std::cout << "divide by zero" << std::endl;
		}
}  /* end of DIVIDE */

expr(A) ::= INTEGER(B). { A = B; } 