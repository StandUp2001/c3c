#ifndef BUILD_CONVERT_H
#define BUILD_CONVERT_H
#include <stdio.h>
#include "build_internal.h"


void parse_c_file(const char *filename);
void convert_files(BuildOptions *build_options);
void regexF(const char *code);

#endif // BUILD_CONVERT_H