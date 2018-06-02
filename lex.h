#ifndef LEX_H
#define LEX_H

#include "lex_type.h"

#define MAX_INFO_SIZE 40

typedef enum States {
	START, INID, INNUM, INASSIGN, INCOMMENT, INRANGE, INQUOTE, INCHAR, FAULT, INCHAR_ERR
} States;

struct TokenType {
	int line_show;
	LexType lex_type;//词法信息，单词的枚举类型
	char sem_info[MAX_INFO_SIZE];//该单词的语义信息
};

struct TokenNode {
	TokenType *token;
	TokenNode *nextToken;
};

TokenNode *lex(char *src_file, char *token_file);

void print_token(TokenNode *token_list);

void free_token(TokenNode *token_list);

#endif
