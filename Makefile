CFLAGS= -std=gnu99 -Werror -pedantic

all:	parser
parser:	parser.c
	gcc $(CFLAGS) main.c parser.c symtable.c scanner.c expression.c interpreter.c -g -o main
run	:	
	./main < ifj21.tl > test.out
clean: 
	rm -f *.o main test.out