%{
    #include "globals.h"

    #include "peanut.tab.h"
    #include <string.h>
    //#include <math.h>

    #include "list.h"

    extern YYSTYPE yylval;
    int yyerror(char* msg);

    int quote_string();

    #define YY_NO_INPUT
%}

%option noyywrap
%option nounput

digit [0-9]
integer \-?{digit}+
real \-?({digit}+|{digit}+\.{digit}+)

identifier [a-zA-Z_][a-zA-Z0-9_]*(\?)?

whitespace [ \t]+

bin_op [+\-*/%]
paren [\(\)\{\}\[\]]
seperator [:,\.\|]

comment_oneline (#[^\r\n]+)|(\/\/[^\r\n]+)

/* http://wordaligned.org/articles/string-literals-and-regular-expressions */
/* double_quote_string \"([^\"\\\\]|\\\\.)*\" */
double_quote_string \"([^\"\\]|\\.)*\"

%%

"import"                { return IMPORT; }
"return"                { return RETURN; }
"if"                    { return IF; }
"elif"                  { return ELIF; }
"else"                  { return ELSE; }
"for"                   { return FOR; }
"in"                    { return IN; }
"while"                 { return WHILE; }
"def"                   { return DEF; }
"end"                   { return END; }
"class"                 { return CLASS; }
"null"                  { return PN_NULL; }

"!"                     { yylval.text = "!";  return NOT; }
"="                     { yylval.text = "=";  return ASSIGN; }
"=="                    { yylval.text = "=="; return EQ; }
"!="                    { yylval.text = "!="; return NEQ; }
"&&"                    { yylval.text = "&&"; return AND; }
"||"                    { yylval.text = "||"; return OR; }
"<"                     { yylval.text = "<";  return LT; }
"<="                    { yylval.text = "<="; return LTEQ; }
">"                     { yylval.text = ">";  return GT; }
">="                    { yylval.text = ">="; return GTEQ; }

"+"                     { yylval.text = "+";  return PLUS; }
"-"                     { yylval.text = "-";  return MINUS; }
"*"                     { yylval.text = "*";  return MULT; }
"/"                     { yylval.text = "/";  return DIVIDE; }
"%"                     { yylval.text = "%";  return MOD; }

{paren}                 { return (int)yytext[0]; }
{seperator}             { return (int)yytext[0]; }

"\n"                    { return ENDL; }
{whitespace}            { ; }

{identifier}            { yylval.text = strdup(yytext); return ID; }
{integer}               { yylval.text = strdup(yytext); return INTEGER; }
{real}                  { yylval.text = strdup(yytext); return REAL; }
{double_quote_string}   { return quote_string(); }

{comment_oneline}       { ; }
.                       { yyerror(yytext); }

%%

/**
 * remove escape chars
 */
char* escape_string(char* s)
{
    // TODO: remove escape chars
    return s;
}

/**
 * create new node, remove " char both sides.
 */
int quote_string()
{
    // always, 2+ chars. cause it must have ""
    // remove two " chars
    yytext = &yytext[1];
    yytext[strlen(yytext) - 1] = 0;
    char * escaped = escape_string(yytext);
    yylval.text = strdup(escaped);
    return STRING;
}

void lex_push_buffer_file()
{
    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
}

// create new buffer and push it to lex's stack
void lex_push_buffer(char *code)
{
    yypush_buffer_state(yy_scan_string(code));
}

// end current buffer, and pop
void lex_pop_buffer()
{
    yypop_buffer_state();
    //if (!YY_CURRENT_BUFFER)
    //{
    //    yyterminate();
    //}
}

