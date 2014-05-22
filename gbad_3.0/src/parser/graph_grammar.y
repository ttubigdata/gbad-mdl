%{
#include <stdio.h>
#include <string.h>
#include "actions.h"

#define YYERROR_VERBOSE 1

int yylex(void *arg);
int yyparse(void *arg);

extern int yylineno;
extern char *GP_file_name;

void yyerror(void *arg, const char *str);
int yywrap();

%}

//%define api.pure
//%pure_parser

%parse-param { void *arg }
%lex-param { void *arg }

%union {
	int num;
	double flt;
	char *str;
}

%start graph

%token <num> CHARACTER
%token <num> ML_CMT SL_CMT XP PS T HASH VERTEX E_EDGE D_EDGE U_EDGE INT WS
%token <flt> FLOAT
%token <str> SQ_STRING DQ_STRING

%type <str> string

%destructor { printf("free %s\n", $$); free($$); } DQ_STRING SQ_STRING

%%
graph
	:	xp_instance
	|	ps_instance
	;

xp_instance
	:	xp_instance XP HASH INT {GP_add_xp(arg, $4);} element
	|	XP HASH INT {GP_add_xp(arg, $3);} element
	;

ps_instance
	:	ps_instance PS HASH INT {GP_add_ps(arg, $4);} element
	|	PS HASH INT {GP_add_ps(arg, $3);} element
	;

element
	:	element vertex 
	|	element edge 
	|	vertex
	|	edge
	;

vertex
	:	VERTEX INT INT {GP_add_vertex_i(arg, $2, $3);}
	|	VERTEX INT FLOAT {GP_add_vertex_f(arg, $2, $3);}
	|	VERTEX INT string {GP_add_vertex_s(arg, $2, $3);}
	;

edge
	:	E_EDGE INT INT INT {GP_add_edge_i(arg, $1, $2, $3, $4);}
	|	E_EDGE INT INT FLOAT {GP_add_edge_f(arg, $1, $2, $3, $4);}
	|	E_EDGE INT INT string {GP_add_edge_s(arg, $1, $2, $3, $4);}
	|	D_EDGE INT INT INT {GP_add_edge_i(arg, $1, $2, $3, $4);}
	|	D_EDGE INT INT FLOAT {GP_add_edge_f(arg, $1, $2, $3, $4);}
	|	D_EDGE INT INT string {GP_add_edge_s(arg, $1, $2, $3, $4);}
	|	U_EDGE INT INT INT {GP_add_edge_i(arg, $1, $2, $3, $4);}
	|	U_EDGE INT INT FLOAT {GP_add_edge_f(arg, $1, $2, $3, $4);}
	|	U_EDGE INT INT string {GP_add_edge_s(arg, $1, $2, $3, $4);}
	;

string
	:	SQ_STRING {$$ = $1;}
	|	DQ_STRING {$$ = $1;}
	;

%%

void yyerror(void *arg, const char *str)
{
	fprintf(stderr,"%s:%d: error: %s\n", GP_file_name, yylineno, str);
	// yyerror();
	// YYERROR;
}
 
int yywrap()
{
	return 1;
}
