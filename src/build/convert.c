#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "convert.h"

#define MAX_MATCHES 100000
#define KEY_WORDS_COUNT 20

// Function to parse a single C file
void parse_c_file(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Error: Could not open file %s\n", filename);
		return;
	}

	// Read the file content
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *file_contents = (char *)malloc(file_size + 1);
	if (!file_contents)
	{
		fprintf(stderr, "Error: Memory allocation failed\n");
		fclose(file);
		return;
	}

	fread(file_contents, 1, file_size, file);
	file_contents[file_size] = '\0';
	fclose(file);

	// Print the file content
	regexF(file_contents);

	free(file_contents);
}

void convert_files(BuildOptions *build_options)
{
	for (int i = 0; i < vec_size(build_options->files); i++)
	{
		parse_c_file(build_options->files[i]);
	}
}

// Function to initialize a token list
void init_token_list(TokenList *list)
{
	list->size = 0;
	list->capacity = 100; // Initial capacity
	list->tokens = malloc(list->capacity * sizeof(Token));
}

// Function to add a token to the list
void add_token(TokenList *list, TokenType type, const char *value)
{
	if (list->size >= list->capacity)
	{
		list->capacity *= 2;
		list->tokens = realloc(list->tokens, list->capacity * sizeof(Token));
	}
	list->tokens[list->size].type = type;
	list->tokens[list->size].value = strdup(value); // Allocate memory for the token value
	list->size++;
}

void free_token_list(TokenList *list)
{
	for (int i = 0; i < list->size; i++)
	{
		free(list->tokens[i].value);
	}
	free(list->tokens);
}

void regexF(const char *code)
{
	const char *keywords[KEY_WORDS_COUNT] = {
		"int", "float", "char", "double", "return", "if", "else", "while", "for", "void",
		"struct", "typedef", "static", "break", "continue", "switch", "case", "default",
		"#include", "define"};

	// Calculate the required size for the pattern
	size_t pattern_size = 256 * 2 * 2 * 2 * 2 * 2; // Start with a reasonable size
	char *pattern = malloc(pattern_size);
	if (pattern == NULL)
	{
		perror("Failed to allocate memory");
		return;
	}

	strcpy(pattern, "(\\/\\*(.|\n)*?\\*\\/)"); // MultiComments
	strcat(pattern, "|(\\/\\/.*$)");		   // SingleComments
	strcat(pattern, "|(");
	for (int i = 0; i < KEY_WORDS_COUNT; i++)
	{
		strcat(pattern, keywords[i]);
		if (i < KEY_WORDS_COUNT - 1)
		{
			strcat(pattern, "|");
		}
	}
	strcat(pattern, ")");
	strcat(pattern, "|(\"([^\"\\\\]|\\\\.)*\")");	   // Strings
	strcat(pattern, "|('([^'\\\\]|\\\\.)')");		   // Characters
	strcat(pattern, "|([a-zA-Z_][a-zA-Z0-9_]*)");	   // Identifiers
	strcat(pattern, "|([0-9]+(\\.[0-9]+)?)");		   // Numbers (including floating-point)
	strcat(pattern, "|([+=\\*%&|!^<>]|[<>]=?)");	   // Assignment and comparison operators
	strcat(pattern, "|(\\?|:)");					   // Ternary operator
	strcat(pattern, "|(&&|\\|\\||<<|>>|==|!=|<=|>=)"); // Logical and bitwise operators
	strcat(pattern, "|([;(){}.,])");				   // Symbols

	printf("Pattern: %s\n", pattern);
	regex_t regex;
	regmatch_t matches[MAX_MATCHES];

	if (regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE))
	{
		printf("Could not compile regex\n");
		free(pattern);
		return;
	}
	printf("Compiled regex\n");
	free(pattern);

	TokenList tokens;
	init_token_list(&tokens);
	const char *cursor = code;

	while (*cursor)
	{
		if (regexec(&regex, cursor, MAX_MATCHES, matches, 0) == 0)
		{
			int start = matches[0].rm_so;
			int end = matches[0].rm_eo;
			char match[end - start + 1];
			strncpy(match, cursor + start, end - start);
			match[end - start] = '\0';

			// Check for token types
			if (strstr(match, "//") || strstr(match, "/*"))
			{
				add_token(&tokens, TOKEN_COMMENT, match);
			}
			else if (strchr(match, '"'))
			{
				add_token(&tokens, TOKEN_STRING, match);
			}
			else if (strchr(match, '\''))
			{
				add_token(&tokens, TOKEN_CHAR, match);
			}
			else if (strspn(match, "0123456789") == strlen(match))
			{
				add_token(&tokens, TOKEN_NUMBER, match);
			}
			else if (strchr(";(){}.,", match[0]))
			{
				add_token(&tokens, TOKEN_SYMBOL, match);
			}
			else
			{
				// Check if the match is a keyword
				int is_keyword = 0;
				for (int i = 0; i < KEY_WORDS_COUNT; i++)
				{
					if (strcmp(match, keywords[i]) == 0)
					{
						add_token(&tokens, TOKEN_KEYWORD, match);
						is_keyword = 1;
						break;
					}
				}
				if (!is_keyword)
				{
					add_token(&tokens, TOKEN_IDENTIFIER, match);
				}
			}
			cursor += end; // Move the cursor forward
		}
		else
		{
			cursor++; // Move the cursor forward if no match
		}
	}

	// Free the token list
	free_token_list(&tokens);
	regfree(&regex);
}
