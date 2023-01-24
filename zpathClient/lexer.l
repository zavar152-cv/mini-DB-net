/* lexer */
%{
#include "parser.h"
#include <string.h>
#include <math.h>

extern void yyerror(const char * message);
#define ECHO fwrite(yytext, yyleng, 1, yyout)
%}

%option noyywrap


DIGIT    [0-9]

%%

"/"            {return ABSOLUTE;}
"//"           {return RELATIVE;}
"["            {return LBRAC;}
"]"            {return RBRAC;}
"("            {return LROBRAC;}
")"            {return RROBRAC;}
","            {return COMMA;}
":"            {return COLON;}

"="            {return EQ;}
"!="           {return NOT_EQ;}
"<"            {return LS;}
">"            {return GR;}
">="           {return GR_EQ;}
"<="           {return LS_EQ;}
"contains"     {return CONT;}
"ON"           {return ON;}

"INT"          {return INT;}
"DBL"          {return DBL;}
"STR"          {return STR;}
"BOOL"         {return BOOL;}

"&"            {return AND_SYM;}
"|"            {return OR_SYM;}
"!"            {return NOT_SYM;}

";"            {return END;}

(\.{DIGIT}+)|({DIGIT}+(\.{DIGIT}*)?([eE][+-]?[DIGIT]+)?)   {
      double f = atof(yytext);
      if (ceil(f) == f) {
        yylval.number = atoi(yytext);
        return NUMBER;
      } else {
        yylval.decimal = f;
        return DNUMBER;
      }

}

"ADD"          {return ADD;}
"DELETE"       {return DELETE;}
"UPDATE"       {return UPDATE;}
"FIND"         {return FIND;}
"JOIN"         {return JOIN;}
"PARENT"       {return PARENT;}

\"[^"]*\"      {yylval.string = strdup(yytext); return STRING;}
@\"[^"]*\"     {yylval.string = strdup(yytext); return KEY;}

[ \t\r\n]+     {/* ignore spaces */}


.              {yyerror(yytext); yyterminate();}

%%