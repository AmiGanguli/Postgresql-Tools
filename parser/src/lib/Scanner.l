%{

/*-------------------------------------------------------------------------
 *
 * Scanner.l
 *	  lexical scanner for PostgreSQL
 *
 * Mostly copied from the PostgreSQL source, available here:
 *
 *   https://github.com/postgres/postgres/blob/master/src/backend/parser/scan.l
 *
 * I've left as much of the original as possible intact in order make it
 * easier to stay in sync with the PostgreSQL version.
 *
 * Portions Copyright (c) 2013, Amitavo Ganguli
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/lib/Scanner.l
 *
 *-------------------------------------------------------------------------
 */

#include <cstdio>
#include <cstring>
#include <cctype>
#include <list>

#include "Token.h"

namespace PGParse { struct ScannerState
{
	ScannerState(TokenList& tokens_)
		: tokens(tokens_),
		  xcdepth(0),
		  position(0),
		  start_of_token(-1),
		  standard_conforming_strings(false),
		  dolqstart(0)
	{}

	~ScannerState()
	{
		if (dolqstart) {
			free dolqstart;
		}
	}

	void
	saveDolq(const char *dolq_)
	{
		if (dolqstart) {
			free dolqstart;
		}
		dolqstart = strdup(dolq_);
	}

	TokenList& 	tokens;
	int 		xcdepth;
	size_t 		position;
	yyscan_t 	scanner;
	size_t 		start_of_token;
	bool		standard_conforming_strings;
	char *		dolqstart;
};}


static bool check_uescapechar(unsigned char escape);
static bool scanner_isspace(char ch);


#define YY_EXTRA_TYPE PGParse::ScannerState *

/**
 * The requirements for this lexer are much simpler than for the 'real' PostgreSQL parser.
 *
 * The rules are:
 *
 *   1. Every input character must belong to a token.
 *   2. Every token must encode the token id and the position.
 *
 * This way we can use the token list together with the original input buffer to recover
 * the text of each token.  We can also step through the original input buffer and added
 * annotations - markup if we're emitting HTML, or colours if we're formatting text form
 * a text editor.
 *
 * The following three macros do most of the work:
 *
 *   ADD_TOKEN       Add a token in one step - used if the rule matches the entire token.
 *   START_TOKEN     Save the starting position of the token, but don't add it yet.
 *                   Used in rules that match only the start of the token.
 *   CONTINUE_TOKEN  Used within rules that match the inside of a token.
 *   END_TOKEN       Used when a rule closes-off a token that began previously.
 *
 * IMPORTANT: - One of these macros must be called within each rule.
 *            - yyless updates the token length, and we need that to calculate the 
 *              correct position, so if yyless is called it must happen before the
 *              macros are used.
 */

#define ADD_TOKEN(id)		yyextra->tokens.push_back(PGParse::Token(yyextra->position, id)); \
				yyextra->start_of_token = yyextra->position;\
				yyextra->position += yyleng

#define START_TOKEN()		yyextra->start_of_token = yyextra->position;\
				yyextra->position += yyleng
				
#define CONTINUE_TOKEN()	yyextra->position += yyleng

#define END_TOKEN(id)		yyextra->tokens.push_back(PGParse::Token(yyextra->start_of_token, id)); \
				yyextra->position += yyleng

%}

%option reentrant
%option noyywrap
/*
%option bison-bridge
%option bison-locations
*/
%option 8bit
/*
%option never-interactive
*/
%option nodefault
/*
%option noinput
%option nounput
%option noyyalloc
%option noyyrealloc
%option noyyfree
%option warn
%option prefix="core_yy"
*/

/**
 * From here to %% is copied verbatim from the PostgreSQL source
 * tree.
 *
 * Last copied on March 19, 2013 from the Github repository.
 */

/*
 * OK, here is a short description of lex/flex rules behavior.
 * The longest pattern which matches an input string is always chosen.
 * For equal-length patterns, the first occurring in the rules list is chosen.
 * INITIAL is the starting state, to which all non-conditional rules apply.
 * Exclusive states change parsing rules while the state is active.  When in
 * an exclusive state, only those rules defined for that state apply.
 *
 * We use exclusive states for quoted strings, extended comments,
 * and to eliminate parsing troubles for numeric strings.
 * Exclusive states:
 *  <xb> bit string literal
 *  <xc> extended C-style comments
 *  <xd> delimited identifiers (double-quoted identifiers)
 *  <xh> hexadecimal numeric string
 *  <xq> standard quoted strings
 *  <xe> extended quoted strings (support backslash escape sequences)
 *  <xdolq> $foo$ quoted strings
 *  <xui> quoted identifier with Unicode escapes
 *  <xuiend> end of a quoted identifier with Unicode escapes, UESCAPE can follow
 *  <xus> quoted string with Unicode escapes
 *  <xusend> end of a quoted string with Unicode escapes, UESCAPE can follow
 *  <xeu> Unicode surrogate pair in extended quoted string
 */

%x xb
%x xc
%x xd
%x xh
%x xe
%x xq
%x xdolq
%x xui
%x xuiend
%x xus
%x xusend
%x xeu

/*
 * In order to make the world safe for Windows and Mac clients as well as
 * Unix ones, we accept either \n or \r as a newline.  A DOS-style \r\n
 * sequence will be seen as two successive newlines, but that doesn't cause
 * any problems.  Comments that start with -- and extend to the next
 * newline are treated as equivalent to a single whitespace character.
 *
 * NOTE a fine point: if there is no newline following --, we will absorb
 * everything to the end of the input as a comment.  This is correct.  Older
 * versions of Postgres failed to recognize -- as a comment if the input
 * did not end with a newline.
 *
 * XXX perhaps \f (formfeed) should be treated as a newline as well?
 *
 * XXX if you change the set of whitespace characters, fix scanner_isspace()
 * to agree, and see also the plpgsql lexer.
 */

space			[ \t\n\r\f]
horiz_space		[ \t\f]
newline			[\n\r]
non_newline		[^\n\r]

comment			("--"{non_newline}*)

whitespace		({space}+|{comment})

/*
 * SQL requires at least one newline in the whitespace separating
 * string literals that are to be concatenated.  Silly, but who are we
 * to argue?  Note that {whitespace_with_newline} should not have * after
 * it, whereas {whitespace} should generally have a * after it...
 */

special_whitespace		({space}+|{comment}{newline})
horiz_whitespace		({horiz_space}|{comment})
whitespace_with_newline	({horiz_whitespace}*{newline}{special_whitespace}*)

/*
 * To ensure that {quotecontinue} can be scanned without having to back up
 * if the full pattern isn't matched, we include trailing whitespace in
 * {quotestop}.  This matches all cases where {quotecontinue} fails to match,
 * except for {quote} followed by whitespace and just one "-" (not two,
 * which would start a {comment}).  To cover that we have {quotefail}.
 * The actions for {quotestop} and {quotefail} must throw back characters
 * beyond the quote proper.
 */
quote			'
quotestop		{quote}{whitespace}*
quotecontinue	{quote}{whitespace_with_newline}{quote}
quotefail		{quote}{whitespace}*"-"

/* Bit string
 * It is tempting to scan the string for only those characters
 * which are allowed. However, this leads to silently swallowed
 * characters if illegal characters are included in the string.
 * For example, if xbinside is [01] then B'ABCD' is interpreted
 * as a zero-length string, and the ABCD' is lost!
 * Better to pass the string forward and let the input routines
 * validate the contents.
 */
xbstart			[bB]{quote}
xbinside		[^']*

/* Hexadecimal number */
xhstart			[xX]{quote}
xhinside		[^']*

/* National character */
xnstart			[nN]{quote}

/* Quoted string that allows backslash escapes */
xestart			[eE]{quote}
xeinside		[^\\']+
xeescape		[\\][^0-7]
xeoctesc		[\\][0-7]{1,3}
xehexesc		[\\]x[0-9A-Fa-f]{1,2}
xeunicode		[\\](u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})
xeunicodefail	[\\](u[0-9A-Fa-f]{0,3}|U[0-9A-Fa-f]{0,7})

/* Extended quote
 * xqdouble implements embedded quote, ''''
 */
xqstart			{quote}
xqdouble		{quote}{quote}
xqinside		[^']+

/* $foo$ style quotes ("dollar quoting")
 * The quoted string starts with $foo$ where "foo" is an optional string
 * in the form of an identifier, except that it may not contain "$",
 * and extends to the first occurrence of an identical string.
 * There is *no* processing of the quoted text.
 *
 * {dolqfailed} is an error rule to avoid scanner backup when {dolqdelim}
 * fails to match its trailing "$".
 */
dolq_start		[A-Za-z\200-\377_]
dolq_cont		[A-Za-z\200-\377_0-9]
dolqdelim		\$({dolq_start}{dolq_cont}*)?\$
dolqfailed		\${dolq_start}{dolq_cont}*
dolqinside		[^$]+

/* Double quote
 * Allows embedded spaces and other special characters into identifiers.
 */
dquote			\"
xdstart			{dquote}
xdstop			{dquote}
xddouble		{dquote}{dquote}
xdinside		[^"]+

/* Unicode escapes */
uescape			[uU][eE][sS][cC][aA][pP][eE]{whitespace}*{quote}[^']{quote}
/* error rule to avoid backup */
uescapefail		[uU][eE][sS][cC][aA][pP][eE]{whitespace}*"-"|[uU][eE][sS][cC][aA][pP][eE]{whitespace}*{quote}[^']|[uU][eE][sS][cC][aA][pP][eE]{whitespace}*{quote}|[uU][eE][sS][cC][aA][pP][eE]{whitespace}*|[uU][eE][sS][cC][aA][pP]|[uU][eE][sS][cC][aA]|[uU][eE][sS][cC]|[uU][eE][sS]|[uU][eE]|[uU]

/* Quoted identifier with Unicode escapes */
xuistart		[uU]&{dquote}

/* Quoted string with Unicode escapes */
xusstart		[uU]&{quote}

/* Optional UESCAPE after a quoted string or identifier with Unicode escapes. */
xustop1		{uescapefail}?
xustop2		{uescape}

/* error rule to avoid backup */
xufailed		[uU]&


/* C-style comments
 *
 * The "extended comment" syntax closely resembles allowable operator syntax.
 * The tricky part here is to get lex to recognize a string starting with
 * slash-star as a comment, when interpreting it as an operator would produce
 * a longer match --- remember lex will prefer a longer match!  Also, if we
 * have something like plus-slash-star, lex will think this is a 3-character
 * operator whereas we want to see it as a + operator and a comment start.
 * The solution is two-fold:
 * 1. append {op_chars}* to xcstart so that it matches as much text as
 *    {operator} would. Then the tie-breaker (first matching rule of same
 *    length) ensures xcstart wins.  We put back the extra stuff with yyless()
 *    in case it contains a star-slash that should terminate the comment.
 * 2. In the operator rule, check for slash-star within the operator, and
 *    if found throw it back with yyless().  This handles the plus-slash-star
 *    problem.
 * Dash-dash comments have similar interactions with the operator rule.
 */
xcstart			\/\*{op_chars}*
xcstop			\*+\/
xcinside		[^*/]+

digit			[0-9]
ident_start		[A-Za-z\200-\377_]
ident_cont		[A-Za-z\200-\377_0-9\$]

identifier		{ident_start}{ident_cont}*

typecast		"::"
dot_dot			\.\.
colon_equals	":="

/*
 * "self" is the set of chars that should be returned as single-character
 * tokens.  "op_chars" is the set of chars that can make up "Op" tokens,
 * which can be one or more characters long (but if a single-char token
 * appears in the "self" set, it is not to be returned as an Op).  Note
 * that the sets overlap, but each has some chars that are not in the other.
 *
 * If you change either set, adjust the character lists appearing in the
 * rule for "operator"!
 */
self			[,()\[\].;\:\+\-\*\/\%\^\<\>\=]
op_chars		[\~\!\@\#\^\&\|\`\?\+\-\*\/\%\<\>\=]
operator		{op_chars}+

/* we no longer allow unary minus in numbers.
 * instead we pass it separately to parser. there it gets
 * coerced via doNegate() -- Leon aug 20 1999
 *
* {decimalfail} is used because we would like "1..10" to lex as 1, dot_dot, 10.
*
 * {realfail1} and {realfail2} are added to prevent the need for scanner
 * backup when the {real} rule fails to match completely.
 */

integer			{digit}+
decimal			(({digit}*\.{digit}+)|({digit}+\.{digit}*))
decimalfail		{digit}+\.\.
real			({integer}|{decimal})[Ee][-+]?{digit}+
realfail1		({integer}|{decimal})[Ee]
realfail2		({integer}|{decimal})[Ee][-+]

param			\${integer}

other			.

/*
 * Dollar quoted strings are totally opaque, and no escaping is done on them.
 * Other quoted strings must allow some special characters such as single-quote
 *  and newline.
 * Embedded single-quotes are implemented both in the SQL standard
 *  style of two adjacent single quotes "''" and in the Postgres/Java style
 *  of escaped-quote "\'".
 * Other embedded escaped characters are matched explicitly and the leading
 *  backslash is dropped from the string.
 * Note that xcstart must appear before operator, as explained above!
 *  Also whitespace (comment) must appear before operator.
 */


/*
 * #### END SECTION COPIED DIRECTLY FROM POSTGRESQL SOURCE TREE. ####
 *
 * The code below is still based on the original sources, but significanly
 * mangled so that changes must be ported manually.
 *
 **/
 
%%

{space}+	{
			/** 
			 * ########## White space ##########
			 * The original lexer handles simple whitespace and 
			 * SQL-style comments identically.  That doesn't work 
			 * for us.  In order to keep the token section above
			 * identical to the original, we deal with this in 
			 * the rules.
			 */
			ADD_TOKEN(PGParse::WHITESPACE_T);
		}
{comment}	{
			/**
			 * ########## SQL-Style comments ##########
			 */
			ADD_TOKEN(PGParse::COMMENT_T);
		}
{xcstart}	{
			/**
			 * ########## C-Style comments ##########
			 */
			/* Put back any characters past slash-star; see above */
			yyless(2);
			START_TOKEN();
			yyextra->xcdepth = 0;
			BEGIN(xc);
		}
		
<xc>{xcstart}	{
			/* Put back any characters past slash-star; see above */
			yyless(2);
			CONTINUE_TOKEN();
			yyextra->xcdepth ++;
		}

<xc>{xcstop}	{
			if (yyextra->xcdepth <= 0) {
				END_TOKEN(PGParse::COMMENT_T);
				BEGIN(INITIAL);
			} else {
				CONTINUE_TOKEN();
				yyextra->xcdepth --;
			}
		}

<xc>{xcinside}	{
			CONTINUE_TOKEN();
		}

<xc>{op_chars}	{
			CONTINUE_TOKEN();
		}

<xc>\*+		{
			CONTINUE_TOKEN();
		}

<xc><<EOF>>	{
			END_TOKEN(PGParse::UNTERMINATED_C_COMMENT_E);
			yyterminate();
		}


{xbstart}	{
			/**
			 * ########## Binary bit type ##########
			 *
			 * At some point we should simply pass the string
			 * forward to the parser and label it there.
			 * In the meantime, place a leading "b" on the string
			 * to mark it for the input routine as a binary string.
			 */
			START_TOKEN();
			BEGIN(xb);
			
		}
<xb>{quotestop}	|
<xb>{quotefail} {
			yyless(1);
			END_TOKEN(PGParse::BIT_STRING_T);
			BEGIN(INITIAL);
		}
<xh>{xhinside}	|
<xb>{xbinside}	{
			/**
			 * FIXME: the original lexer doesn't validate
			 * the contents of bitstrings and hexstrings.  We
			 * should probably change this.
			 */
			CONTINUE_TOKEN();
		}
		
<xb><<EOF>>	{
			END_TOKEN(PGParse::UNTERMINATED_BIT_STRING_E);
			yyterminate();
		}

{xhstart}	{
			/** ########## Hexadecimal bit type ##########
			 *
			 * At some point we should simply pass the string
			 * forward to the parser and label it there.
			 * In the meantime, place a leading "x" on the string
			 * to mark it for the input routine as a hex string.
			 */
			START_TOKEN();
			BEGIN(xh);
		}
<xh>{quotestop}	|
<xh>{quotefail} {
			yyless(1);
			END_TOKEN(PGParse::HEX_STRING_T);
			BEGIN(INITIAL);
		}
<xh><<EOF>>	{
			END_TOKEN(PGParse::UNTERMINATED_HEX_STRING_E);
			yyterminate();
		}
{xnstart}	{
			/* ########## National character literal flag ##########
			 * 
			 * This is the "n" in front of n'some text'.  We treat it
			 * as a regular string, but we need to return something for
			 * the "n" regardless.
			 *
			 * FIXME: this should probably be treated identically to
			 * the other strings.
			 */
			yyless(1);
			ADD_TOKEN(PGParse::NCHAR_FLAG_T);
		}

{xqstart}	{
			/* ########## Quoted strings ##########
			 *
			 * We do not support "escape_string_warning"
			 * (see: http://www.postgresql.org/docs/9.2/static/runtime-config-compatible.html
			 * but we do support the standard_conforming_strings flag.
			 *
			 * FIXME: The original lexer checks the validity of multi-byte strings.  That would
			 * require pulling in more code than I want to deal with at the moment, so I've
			 * pulled this out.  Might want to revisit this later - either bring in the mbstr
			 * functions from Postgres, or find a library that does something similar.
			 */
			START_TOKEN();
			if (yyextra->standard_conforming_strings) {
				BEGIN(xq);
			} else {
				BEGIN(xe);
			}
		}
{xestart}	{
			START_TOKEN();
			BEGIN(xe);
		}
{xusstart}	{
			START_TOKEN();
			BEGIN(xus);
		}
<xq,xe>{quotestop}	|
<xq,xe>{quotefail} {
			yyless(1);
			// FIXME: what's this about quotefail?
			END_TOKEN(PGParse::STRING_T);
			BEGIN(INITIAL);
		}
<xus>{quotestop} |
<xus>{quotefail} {
			/* throw back all but the quote */
			yyless(1);
			CONTINUE_TOKEN();
			/* handle possible UESCAPE in xusend mode */
			BEGIN(xusend);
		}
<xusend>{whitespace}
<xusend>{other} |
<xusend>{xustop1} {
			/* no UESCAPE after the quote, throw back everything */
			yyless(0);
			if (!yyextra->standard_conforming_strings) {
				END_TOKEN(PGParse::STANDARD_CONFORMING_STRINGS_DISABLED_E);
			} else {
				END_TOKEN(PGParse::UNI_STRING_T);
			}

			BEGIN(INITIAL);
		}
<xusend>{xustop2} {
			/* found UESCAPE after the end quote */
			BEGIN(INITIAL);
			if (!check_uescapechar(yytext[yyleng-2])) {
				END_TOKEN(PGParse::STANDARD_CONFORMING_STRINGS_DISABLED_E);
			} else if (!yyextra->standard_conforming_strings) {
				END_TOKEN(PGParse::STANDARD_CONFORMING_STRINGS_DISABLED_E);
			} else {
				END_TOKEN(PGParse::STANDARD_CONFORMING_STRINGS_DISABLED_E);
			}
		}
<xq,xe,xus>{xqdouble} {
			CONTINUE_TOKEN();
		}
<xq,xus>{xqinside}  {
			CONTINUE_TOKEN();
		}
<xe>{xeinside}  {
			CONTINUE_TOKEN();
		}
<xe>{xeunicode} {
			CONTINUE_TOKEN();
		}
<xeu>{xeunicode} {
			CONTINUE_TOKEN();
			BEGIN(xe);
		}
<xeu>.		{
			END_TOKEN(PGParse::INVALID_UNICODE_SURROGATE_PAIR_E);
		}
<xeu>\n		{
			END_TOKEN(PGParse::INVALID_UNICODE_SURROGATE_PAIR_E);
		}
<xeu><<EOF>>	{
			END_TOKEN(PGParse::INVALID_UNICODE_SURROGATE_PAIR_E);
			yyterminate();
		}
<xe,xeu>{xeunicodefail}	{
			END_TOKEN(PGParse::INVALID_UNICODE_ESCAPE_CHAR_E);
		}
<xe>{xeescape}  {
			CONTINUE_TOKEN();
		}
<xe>{xeoctesc}  {
			CONTINUE_TOKEN();
		}
<xe>{xehexesc}  {
			CONTINUE_TOKEN();
		}
<xq,xe,xus>{quotecontinue} {
			CONTINUE_TOKEN();
		}
<xe>.			{
			CONTINUE_TOKEN();
		}
<xq,xe,xus><<EOF>>	{
			END_TOKEN(PGParse::UNTERMINATED_QUOTED_STRING_E);
			yyterminate();
		}

{dolqdelim}	{
			/* ########## Dollar-quoted strings ########## */
			START_TOKEN();
			yyextra->saveDolq(yytext);
			BEGIN(xdolq);
		}
{dolqfailed}	{
			SET_YYLLOC();
			/* throw back all but the initial "$" */
			yyless(1);
			/* and treat it as {other} */
			return yytext[0];
		}
<xdolq>{dolqdelim} {
			if (strcmp(yytext, yyextra->dolqstart) == 0)
			{
				pfree(yyextra->dolqstart);
				yyextra->dolqstart = NULL;
				BEGIN(INITIAL);
				yylval->str = litbufdup(yyscanner);
				return SCONST;
			}
			else
			{
				/*
				 * When we fail to match $...$ to dolqstart, transfer
				 * the $... part to the output, but put back the final
				 * $ for rescanning.  Consider $delim$...$junk$delim$
				 */
				addlit(yytext, yyleng-1, yyscanner);
				yyless(yyleng-1);
			}
		}
<xdolq>{dolqinside} {
			addlit(yytext, yyleng, yyscanner);
		}
<xdolq>{dolqfailed} {
			addlit(yytext, yyleng, yyscanner);
		}
<xdolq>.	{
			/* This is only needed for $ inside the quoted text */
			addlitchar(yytext[0], yyscanner);
		}
<xdolq><<EOF>>	{ yyerror("unterminated dollar-quoted string"); }

















		
<<EOF>>		{
			yyterminate();
		}
		
{other}		{
			ADD_TOKEN(PGParse::INVALID);
		}
%%

#include "Scanner.h"


/* is 'escape' acceptable as Unicode escape character (UESCAPE syntax) ? */
static bool
check_uescapechar(unsigned char escape)
{
	if (isxdigit(escape)
		|| escape == '+'
		|| escape == '\''
		|| escape == '"'
		|| scanner_isspace(escape))
	{
		return false;
	} else {
		return true;
	}
}

/*
 * Taken from scansup.c
 *
 * scanner_isspace() --- return TRUE if flex scanner considers char whitespace
 *
 * This should be used instead of the potentially locale-dependent isspace()
 * function when it's important to match the lexer's behavior.
 *
 * In principle we might need similar functions for isalnum etc, but for the
 * moment only isspace seems needed.
*/
static bool
scanner_isspace(char ch)
{
	/* This must match scan.l's list of {space} characters */
	if (ch == ' ' ||
	    ch == '\t' ||
	    ch == '\n' ||
	    ch == '\r' ||
	    ch == '\f'
	) {
		return true;
	}
	return false;
}

namespace PGParse {

Scanner::Scanner()
{
	scanner_state_ = new ScannerState(tokens_);
	yylex_init_extra(scanner_state_, &scanner_state_->scanner);
}

Scanner::~Scanner()
{
	yylex_destroy ( scanner_state_->scanner );
	delete scanner_state_;
}

void
Scanner::scan(const char *bytes, std::size_t len)
{
	YY_BUFFER_STATE buf;

	buf = yy_scan_bytes(bytes, len, scanner_state_->scanner);
	yylex ( scanner_state_->scanner );
	yy_delete_buffer(buf,scanner_state_->scanner);
}

} // PGParse

