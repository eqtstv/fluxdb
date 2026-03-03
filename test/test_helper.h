#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *output;
  size_t output_size;
} CommandResult;

char *run_db_command(const char *command);
char *run_db_commands(const char **commands, int num_commands);
bool assert_string_contains(const char *haystack, const char *needle);
bool assert_string_equals(const char *expected, const char *actual);
void free_command_result(CommandResult *result);

#endif