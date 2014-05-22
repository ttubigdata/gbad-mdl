CC = gcc
FLAGS = -Wall
LIBS = 

LEX = flex --yylineno --header-file=lex.yy.h
YACC = bison -y -d

LEXER = graph_lexer.l
GRAMMAR = graph_grammar.y

LEX_H = lex.yy.h
LEX_C = lex.yy.c
LEX_O = lex.yy.o

TAB_H = y.tab.h
TAB_C = y.tab.c
TAB_O = y.tab.o

ACTIONS_H = actions.h
ACTIONS_C = actions.c
ACTIONS_O = actions.o

TEST_HDR = $(ACTIONS_H)
TEST_SRC = $(ACTIONS_C)
TEST_OBJ = $(ACTIONS_O)

PARSER_HDR = $(LEX_H) $(TAB_H)
PARSER_SRC = $(LEX_C) $(TAB_C)
PARSER_OBJ = $(LEX_O) $(TAB_O)

TEST = test

all: $(TEST)

$(TEST) : $(LEX_O) $(TAB_O) $(ACTIONS_O)
	$(CC) $(FLAGS) -o $(TEST) $(TEST_OBJ) $(PARSER_OBJ)

$(ACTIONS_O) : $(LEX_H) $(TAB_H) $(ACTIONS_H) $(ACTIONS_C)
	$(CC) $(FLAGS) -o $(ACTIONS_O) -c $(ACTIONS_C)

$(LEX_O) : $(LEX_H) $(LEX_C) $(TAB_H)
	$(CC) $(FLAGS) -o $(LEX_O) -c $(LEX_C)

$(LEX_H) $(LEX_C) : $(LEXER)
	$(LEX) $(LEXER)

$(TAB_O) : $(TAB_H) $(TAB_C)
	$(CC) $(FLAGS) -o $(TAB_O) -c $(TAB_C)

$(TAB_H) $(TAB_C) : $(GRAMMAR) $(ACTIONS_H)
	$(YACC) $(GRAMMAR)

clean:
	rm -f $(PARSER_HDR) $(PARSER_SRC) $(TEST_OBJ) $(PARSER_OBJ)
