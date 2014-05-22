#include <stdio.h>
#include <stdlib.h>

#include "actions.h"
#include "y.tab.h"
#include "lex.yy.h"

int yyparse(void *arg);

long unsigned int GP_line = 1;
char *GP_file_name = NULL;

void GP_add_xp(void *arg, int num)
{
	printf("XP # %d\n", num);
}

void GP_add_ps(void *arg, int num)
{
	printf("PX # %d\n", num);
}

void GP_add_vertex_i(void *arg, int v, int label)
{
	printf("v %d %d\n", v, label);
}

void GP_add_vertex_f(void *arg, int v, double label)
{
	printf("v %d %f\n", v, label);
}

void GP_add_vertex_s(void *arg, int v, char *label)
{
	printf("v %d %s\n", v, label);
	
	free(label);
}

void GP_add_edge_i(void *arg, int type, int src, int dst, int label)
{
	if(type == E_EDGE)
	{
		printf("e %d %d %d\n", src, dst, label);
	}
	else if(type == D_EDGE)
	{
		printf("d %d %d %d\n", src, dst, label);
	}
	else if(type == U_EDGE)
	{
		printf("u %d %d %d\n", src, dst, label);
	}
	else
	{
		
	}
}

void GP_add_edge_f(void *arg, int type, int src, int dst, double label)
{
	if(type == E_EDGE)
	{
		printf("e %d %d %f\n", src, dst, label);
	}
	else if(type == D_EDGE)
	{
		printf("d %d %d %f\n", src, dst, label);
	}
	else if(type == U_EDGE)
	{
		printf("u %d %d %f\n", src, dst, label);
	}
	else
	{
		
	}
}

void GP_add_edge_s(void *arg, int type, int src, int dst, char *label)
{
	if(type == E_EDGE)
	{
		printf("e %d %d %s\n", src, dst, label);
	}
	else if(type == D_EDGE)
	{
		printf("d %d %d %s\n", src, dst, label);
	}
	else if(type == U_EDGE)
	{
		printf("u %d %d %s\n", src, dst, label);
	}
	else
	{
		
	}
	
	free(label);
}

void GP_read_graph(FILE *input)
{
	FILE *tmp = yyin;
	yyin = input;
	yyparse(NULL);
	yyin = tmp;
}

int main()
{
	GP_file_name = "";
	GP_read_graph(stdin);
	
	return 0;
}
