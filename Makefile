CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -Wno-int-conversion -Wno-implicit-function-declaration

all: analyzer code_correc

analyzer: lex.yy.c parser.tab.c inter_code_gen.c
	$(CC) $(CFLAGS) lex.yy.c parser.tab.c inter_code_gen.c -o analyzer

code_correc: code_correc.c
	$(CC) $(CFLAGS) code_correc.c -o code_correc

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

optimize: analyzer
	python optimiser.py icg.txt --print
	
clean:
	del /f /q lex.yy.c parser.tab.c parser.tab.h analyzer.exe icg.txt optimized_icg.txt code_correc.exe errors.txt temp_source.c corrected_source.c