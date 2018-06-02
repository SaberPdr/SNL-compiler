#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

#define BUFFER_SIZE 1000
#define LF '\n' //0x0a
#define CR '\r' //0x0d
#define TAB '\t'

States state = START;//DFA路径初值
TokenNode *list_curr;//当前节点
char lex_buffer[BUFFER_SIZE];
char *start_pos = lex_buffer;
char *pos = lex_buffer;
int line = 1;
FILE *fp, *fp_token;

void free_token(TokenNode *token_list)
{
	TokenNode *curr_list = NULL;
	curr_list = token_list->nextToken;
	free(token_list);
	token_list = curr_list;
	while (token_list != NULL) {
		curr_list = token_list->nextToken;
		free(token_list->token);
		free(token_list);
		token_list = curr_list;
	}
}

void print_token(TokenNode *token_list)
{

	list_curr = token_list->nextToken;
	while (list_curr != NULL) {

		fprintf(fp_token, "%d ", list_curr->token->line_show);
		if (list_curr->token->lex_type <= MAX_RESERVED) {
			char *tmp = list_curr->token->sem_info;
			int length = strlen(list_curr->token->sem_info);
			int i;
			for (i = 0; i < length; i++) {
				fprintf(fp_token, "%c", (*tmp)^32);
				tmp++;
			}
			if (list_curr->token->lex_type != ENDFILE)
                fprintf(fp_token, ", ");
		} else if (list_curr->token->lex_type == ID) {
			fprintf(fp_token, "ID, name = ");
		} else if (list_curr->token->lex_type == INTC) {
			fprintf(fp_token, "INTC, val = ");
		} else if (list_curr->token->lex_type == CHARC) {
			fprintf(fp_token, "CHARC, char = ");
		}
		fprintf(fp_token, "%s\n", list_curr->token->sem_info);

		list_curr = list_curr->nextToken;
	}
}

char *add_token(LexType lex_type)
{
        //创建新节点，memset为0
	list_curr->nextToken = (TokenNode *)malloc(sizeof(TokenNode));
	memset(list_curr->nextToken, 0x00, sizeof(TokenNode));
	list_curr = list_curr->nextToken;
	list_curr->token = (TokenType *)malloc(sizeof(TokenType));
	memset(list_curr->token, 0x00, sizeof(TokenType));
	list_curr->nextToken = NULL;
	//行号
	list_curr->token->line_show = line;
	//id长度，给id赋值
	size_t id_length = pos + 1 - start_pos;
	memset(list_curr->token->sem_info, 0x00, MAX_INFO_SIZE);
	strncpy(list_curr->token->sem_info, start_pos, id_length);
	int tmp;
	if (lex_type == ID) {
		tmp = get_lex_type(list_curr->token->sem_info);
		if (tmp >= 0)
			lex_type = (LexType)tmp;
	}
	list_curr->token->lex_type = lex_type;
	return (pos + 1);
}


int scan()
{
	switch (state)
	{
		case START:
			if (isalpha(*pos)) {
                                //英文字母
				state = INID;
				break;
			} else if (isdigit(*pos)) {
			        //十进制数字字符
				state = INNUM;
				break;
			} else if (*pos == ' ' || *pos == LF
					|| *pos == CR || *pos == TAB) {
                                // 空格 /n /r /t
				start_pos = pos + 1;
				break;
			} else if (*pos == '{') {
				//注释
				start_pos = pos + 1;
				state = INCOMMENT;
				break;
			}
			switch (*pos)
			{
				case ':':
					state = INASSIGN;
					break;
				case '.':
					state = INRANGE;
					if (*(pos + 1) == '\0')
					{
						start_pos = add_token(DOT);
						state = START;
					}
					break;
				case '+':
					start_pos = add_token(PLUS);
					state = START;
					break;
				case '-':
					start_pos = add_token(MINUS);
					state = START;
					break;
				case '*':
					start_pos = add_token(TIMES);
					state = START;
					break;
				case '/':
					start_pos = add_token(OVER);
					state = START;
					break;
				case '(':
					start_pos = add_token(LPAREN);
					state = START;
					break;
				case ')':
					start_pos = add_token(RPAREN);
					state = START;
					break;
				case '[':
					start_pos = add_token(LMIDPAREN);
					state = START;
					break;
				case ']':
					start_pos = add_token(RMIDPAREN);
					state = START;
					break;
				case '=':
					start_pos = add_token(EQ);
					state = START;
					break;
				case '<':
					start_pos = add_token(LT);
					state = START;
					break;
				case ',':
					start_pos = add_token(COMMA);
					state = START;
					break;
				case ';':
					start_pos = add_token(SEMI);
					state = START;
					break;
				case '\'':
					start_pos = pos + 1;
					state = INQUOTE;
					break;
				case EOF:
					printf("where is EOF\n");
					start_pos = add_token(ENDFILE);
					state = START;
					break;
				default:
					//声明有错时添加token
					//start_pos = add_token(ERROR);
					//不加
					start_pos = pos + 1;
					state = FAULT;
			}
			break;
		case INID:
			if (!isalnum(*pos)) {
				pos--;
				start_pos = add_token(ID);
				state = START;
			}
			break;
		case INNUM:
			if (!isdigit(*pos)) {
				pos--;
				start_pos = add_token(INTC);
				state = START;
			}
			break;
		case INASSIGN:
			if (*pos == '=') {
                                //赋值
				start_pos = add_token(ASSIGN);
				state = START;
			} else {
				//其他情况出错
				start_pos = pos + 1;
				state = FAULT;
			}
			break;
		case INCOMMENT:
		        //注释
			if (*pos == '}') {
				start_pos = pos+1;
				state = START;
			}
			break;
		case INRANGE:
			if (*pos == '.') {
				start_pos = add_token(UNDERANGE);
			} else {
				pos--;
				start_pos = add_token(DOT);
			}
			state = START;
			break;
		case INQUOTE:
			if (isalnum(*pos)) {
				state = INCHAR;
			} else {
				//start_pos = add_token(ERROR);
				start_pos = pos + 1;
				state = FAULT;
			}
			break;
		case INCHAR:
			if (*pos == '\'') {
				pos--;
				start_pos = add_token(CHARC);
				pos++;
				state = START;
			} else {
				//start_pos = add_token(ERROR);
				state = INCHAR_ERR;
			}
			break;
		case INCHAR_ERR:
			if (*pos == '\'') {
				fprintf(stderr, "Error Char Value: ");
				while ((pos - start_pos) >= 0) {
					fprintf(stderr, "%c", *start_pos);
					start_pos++;
				}
				state = START;
			} else {
				state = INCHAR_ERR;
			}
			break;
		case FAULT:
			break;
		default:
			break;
	}

	return state;
}

TokenNode *lex(char *src_file, char *token_file)
{

	if (!(fp_token = fopen(token_file, "w"))) {
		fprintf(stderr, "Token写入 %s 失败\n", token_file);
		exit(-1);
	}

	if (!(fp = fopen(src_file, "r"))) {
		fprintf(stderr, "打开%s, 失败\n", src_file);
		exit(-1);
	}

	TokenNode *list_token = (TokenNode *)malloc(sizeof(TokenNode));
	memset(list_token, 0x00, sizeof(TokenNode));
	//list_token 是头结点， list_curr 是当前节点
	list_curr = list_token;

	// 注意EOF
	// 每次读取一行
	while (fgets(lex_buffer, BUFFER_SIZE, fp)) {
		if (*lex_buffer == 0x0d || *lex_buffer == 0x0a) {
                        //0x0d回车  0x0a换行
			line++;
			continue;
		}
		start_pos = lex_buffer;
		pos = lex_buffer;
		while (!(*pos == '\0')) {
			state = (States)scan();
			if (state == FAULT) {
				state = START;
				fprintf(stdout, "在%d行 : %x 有未定义字符!!\n", line, *pos);
			}
			pos++;
		}

		if (lex_buffer[strlen(lex_buffer)-1] == 0x0d ||
				lex_buffer[strlen(lex_buffer)-1] == 0x0a) {
			line++;
		}
	}
	//pos = start_pos+1;
	add_token(ENDFILE);

	print_token(list_token);

	fclose(fp_token);
	fclose(fp);

	return list_token;

}
