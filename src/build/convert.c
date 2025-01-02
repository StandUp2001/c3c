#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regex.h"
#include "convert.h"

const int MAX_MATCHES = 100;

// Function to parse a single C file
void parse_c_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }

    // Read the file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_contents = (char *)malloc(file_size + 1);
    if (!file_contents) {
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

void convert_files(BuildOptions *build_options) {
    for (int i = 0; i < vec_size(build_options->files); i++) {
        parse_c_file(build_options->files[i]);
    }
}

void regexF(const char *code) {
	const char *pattern = 	"(int|float|char|double|return|if|else|while|for|void|struct|typedef|static|break|continue|switch|case|default|#include|define)" // Keywords
							"|(\\b[a-zA-Z_][a-zA-Z0-9_]*)" // Identifiers
							"|(\"([^\"\\\\]|\\\\.)*\")" // Strings
							"|('([^'\\\\]|\\\\.)')" // Characters
							"|([0-9]+)" // Numbers
							"|([;(){}.,])"; // Symbols


    regex_t regex;
    regmatch_t matches[MAX_MATCHES];

    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        printf("Could not compile regex\n");
        return;
    }

    const char *cursor = code;
    while (*cursor) {
        if (regexec(&regex, cursor, MAX_MATCHES, matches, 0) == 0) {
            int start = matches[0].rm_so;
            int end = matches[0].rm_eo;
            char match[end - start + 1];
            strncpy(match, cursor + start, end - start);
            match[end - start] = '\0';

            if (strstr(match, "int") || strstr(match, "return") || strstr(match, "#include")) {
                printf("Keyword: %s\n", match);
            } else if (strchr(match, '"')) {
                printf("String: %s\n", match);
            } else if (strchr(match, '\'')) {
                printf("Character: %s\n", match);
            } else if (strspn(match, "0123456789") == strlen(match)) {
                printf("Number: %s\n", match);
            } else if (strchr(";(){}.,", match[0])) {
                printf("Symbol: %s\n", match);
            } else {
                printf("Identifier: %s\n", match);
            }
            cursor += end;
        } else {
            cursor++;
        }
    }

    regfree(&regex);
}

