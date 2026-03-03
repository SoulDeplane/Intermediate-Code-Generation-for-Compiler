CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -Wno-int-conversion -Wno-implicit-function-declaration

all: analyzer

analyzer: lex.yy.c parser.tab.c inter_code_gen.c
	$(CC) $(CFLAGS) lex.yy.c parser.tab.c inter_code_gen.c -o analyzer

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

optimize: analyzer
	python optimiser.py icg.txt --print
	
clean:
	del /f /q lex.yy.c parser.tab.c parser.tab.h analyzer.exe icg.txt optimized_icg.txt