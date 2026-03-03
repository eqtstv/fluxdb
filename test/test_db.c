#include "test_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_MAX_OUTPUT 16384

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define PASS "[" GREEN "✓" RESET "]"
#define FAIL "[" RED "✗" RESET "]"

typedef struct {
  int passed;
  int failed;
} TestResults;

void test_insert_and_select(TestResults *results) {
  printf("insert and select two records... ");

  const char *commands[] = {
    "insert 1 testuser test@example.com",
    "insert 2 anotheruser another@example.com",
    "select"
  };

  char *output = run_db_commands(commands, 3);
  if (!output) {
    printf(FAIL " Could not run commands\n");
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "Executed")) {
    printf(FAIL " Commands did not return 'Executed'\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "(1, testuser, test@example.com)")) {
    printf(FAIL " First record not found in output\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "(2, anotheruser, another@example.com)")) {
    printf(FAIL " Second record not found in output\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  free(output);
  printf(PASS "\n");
  results->passed++;
}

void test_select_empty_db(TestResults *results) {
  printf("select on empty db... ");

  const char *commands[] = {"select"};

  char *output = run_db_commands(commands, 1);
  if (!output) {
    printf(FAIL " Could not run commands\n");
    results->failed++;
    return;
  }

  if (assert_string_contains(output, "(")) {
    printf(FAIL " Expected no rows, but got output\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  free(output);
  printf(PASS "\n");
  results->passed++;
}

int main() {
  TestResults results = {0, 0};

  printf("=== Database Tests ===\n\n");

  test_insert_and_select(&results);
  test_select_empty_db(&results);

  if (results.failed > 0) {
    printf("\n=== Results: " RED "%d passed, %d failed" RESET " ===\n", results.passed, results.failed);
  } else {
    printf("\n=== Results: " GREEN "%d passed, %d failed" RESET " ===\n", results.passed, results.failed);
  }

  return results.failed > 0 ? 1 : 0;
}