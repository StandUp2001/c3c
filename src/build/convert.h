#ifndef BUILD_CONVERT_H
#define BUILD_CONVERT_H
#include <stdio.h>
#include "build_internal.h"

// Token types
typedef enum
{
	TOKEN_KEYWORD,
	TOKEN_STRING,
	TOKEN_CHAR,
	TOKEN_NUMBER,
	TOKEN_SYMBOL,
	TOKEN_IDENTIFIER,
	TOKEN_COMMENT,
	TOKEN_OPERATOR
} TokenType;

// Token structure
typedef struct
{
	TokenType type;
	char *value;
} Token;

// Token list structure
typedef struct
{
	Token *tokens;
	int size;
	int capacity;
} TokenList;

void parse_c_file(const char *filename);
void convert_files(BuildOptions *build_options);
void init_token_list(TokenList *list);
void add_token(TokenList *list, TokenType type, const char *value);
void free_token_list(TokenList *list);
void regexF(const char *code);

#endif // BUILD_CONVERT_H
