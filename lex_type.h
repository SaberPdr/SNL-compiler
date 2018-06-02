#ifndef LEX_TYPE_H
#define LEX_TYPE_H

#define LEX_TYPE_SIZE 42
#define MAX_RESERVED 21

typedef enum LexType
{
	PROGRAM,	PROCEDURE,	TYPE,		VAR,		IF,
	THEN,		ELSE,		FI,		    WHILE,		DO,
	ENDWH,		BEGIN,		END,		READ,		WRITE,
	ARRAY,		OF,		    RECORD,		RETURN,		INTEGER,
	CHAR,

	ENDFILE,	ERROR,

	ID,		    INTC,		CHARC,

	ASSIGN,		EQ,		    LT,		    PLUS,		MINUS,
	TIMES,		OVER,		LPAREN,		RPAREN,		DOT,
	COLON,		SEMI,		COMMA,		LMIDPAREN,	RMIDPAREN,
	UNDERANGE
} LexType;

extern char *ReservedWord[LEX_TYPE_SIZE];

int get_lex_type(char *s);

#endif
