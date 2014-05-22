D            [0-9]


%option nounput

%{
#include <stdio.h>

#include "y.tab.h"

#define YY_NO_INPUT
#define YY_DECL int yylex(void *arg)

extern long unsigned int GP_line;

%}

%option yylineno

%%
\/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+\/ { /* yylval.num=ML_CMT; return ML_CMT; */ }
\/\/.*\n?                                   { /* yylval.num=SL_CMT; return SL_CMT; */ }

("xp"|"XP")[ \t]                            { yylval.num=XP; return XP; }
("ps"|"PS")[ \t]                            { yylval.num=PS; return PS; }
"#"[ \t]                                    { yylval.num=HASH; return HASH; }
("v"|"V")[ \t]                              { yylval.num=VERTEX; return VERTEX; }
("e"|"E")[ \t]                              { yylval.num=E_EDGE; return E_EDGE; }
("d"|"D")[ \t]                              { yylval.num=D_EDGE; return D_EDGE; }
("u"|"U")[ \t]                              { yylval.num=U_EDGE; return U_EDGE; }
[0-9]+                                      { yylval.num=atoi(yytext); return INT; }
[0-9]*"."[0-9]+                             { yylval.flt=atof(yytext); return FLOAT; }
\"(\\.|[^\\"])*\"                           { yylval.str=strdup(yytext); return DQ_STRING; }
\'(\\.|[^\\'])*\'                           { yylval.str=strdup(yytext); return SQ_STRING; }
[ \t\v\f\r\n]                               { /* yylval.num=WS; return WS; */ }
%%

