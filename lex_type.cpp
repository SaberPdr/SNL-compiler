#include <stdio.h>
#include <string.h>
#include "lex_type.h"

char *ReservedWord[LEX_TYPE_SIZE] = {
	// reserver words
	"program", "procedure", "type", "var", "if",
	"then", "else", "fi", "while", "do",
	"endwh", "begin", "end", "read", "write",
	"array", "of", "record", "return", "integer",
	"char",
	// others
	"EOF", "ERROR",
	"ID name", "INTC integer", "CHARC char",
	":=", "=", "<", "+", "-",
	"*", "/", "(", ")", ".",
	":", ";", ",", "[", "]",
	"...",
};

int get_lex_type(char *s)
{
	int i, flag;
	for (i = 0; i < MAX_RESERVED; i++) {
		flag = -1;
		char *tmp = ReservedWord[i];
		if (strcmp(tmp, s) == 0) {
			flag = i;
			break;
		} else {
			flag = -1;
		}
	}
	return flag;
}
