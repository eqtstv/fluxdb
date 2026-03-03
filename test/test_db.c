#include "test_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  printf("insert_and_select_two_records: ");

  const char *commands[] = {"insert 1 testuser test@example.com",
                            "insert 2 anotheruser another@example.com",
                            "select"};

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

  if (!assert_string_contains(output,
                              "(2, anotheruser, another@example.com)")) {
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
  printf("test_select_on_empty_db: ");

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

void test_table_full(TestResults *results) {
  printf("test_table_full_error: ");

  const int MAX_ROWS = 1400;
  const char *commands[1402];
  char inserts[1400][64];

  for (int i = 0; i < MAX_ROWS; i++) {
    snprintf(inserts[i], sizeof(inserts[i]), "insert %d user%d test%d@test.com",
             i + 1, i + 1, i + 1);
    commands[i] = inserts[i];
  }
  commands[MAX_ROWS] = "insert 9999 overflow overflow@test.com";
  commands[MAX_ROWS + 1] = "select";

  char *output = run_db_commands(commands, MAX_ROWS + 2);
  if (!output) {
    printf(FAIL " Could not run commands\n");
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "Error: Table full")) {
    printf(FAIL " Expected 'Error: Table full'\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  free(output);
  printf(PASS "\n");
  results->passed++;
}

void test_max_length_username_and_email(TestResults *results) {
  printf("test_max_length_username_and_email: ");

  char username[33];
  memset(username, 'a', 32);
  username[32] = '\0';

  char email[256];
  memset(email, 'b', 255);
  email[255] = '\0';

  char insert_cmd[512];
  snprintf(insert_cmd, sizeof(insert_cmd), "insert 1 %s %s", username, email);

  const char *commands[] = {insert_cmd, "select"};
  char *output = run_db_commands(commands, 2);

  if (!output) {
    printf(FAIL " Could not run commands\n");
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "Executed")) {
    printf(FAIL " Insert did not return 'Executed'\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  char expected_output[512];
  snprintf(expected_output, sizeof(expected_output), "(1, %s, %s)", username,
           email);
  if (!assert_string_contains(output, expected_output)) {
    printf(FAIL " Expected output not found\n");
    printf("  Expected: %s\n", expected_output);
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  char unexpected[64];
  snprintf(unexpected, sizeof(unexpected), "(1, %saa", username);
  if (assert_string_contains(output, unexpected)) {
    printf(
        FAIL
        " Output contains garbage after username - missing null terminator\n");
    printf("  Output: %s\n", output);
    free(output);
    results->failed++;
    return;
  }

  free(output);
  printf(PASS "\n");
  results->passed++;
}

void test_too_long_username_and_email(TestResults *results) {
  printf("test_too_long_username_and_email: ");

  char username[40];
  memset(username, 'x', 39);
  username[39] = '\0';

  char email[300];
  memset(email, 'y', 299);
  email[299] = '\0';

  char insert_cmd[512];
  snprintf(insert_cmd, sizeof(insert_cmd), "insert 1 %s %s", username, email);

  const char *commands[] = {insert_cmd, "select"};
  char *output = run_db_commands(commands, 2);

  if (!output) {
    printf(FAIL " Could not run commands\n");
    results->failed++;
    return;
  }

  if (!assert_string_contains(output, "String is too long")) {
    printf(FAIL " Expected 'String is too long' error\n");
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

  printf("=== Running Tests ===\n\n");

  clock_t start = clock();

  test_insert_and_select(&results);
  test_select_empty_db(&results);
  test_table_full(&results);
  test_max_length_username_and_email(&results);
  test_too_long_username_and_email(&results);

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

  int total = results.passed + results.failed;

  if (results.failed > 0) {
    printf("\n=== Finished: Total: %d | " GREEN "Passed: %d" RESET " | " RED
           "Failed: %d" RESET " | In: %.3fs ===\n",
           total, results.passed, results.failed, elapsed);
  } else {
    printf("\n=== Finished: Total: %d | " GREEN "Passed: %d" RESET
           " | Failed: %d | In: %.3fs ===\n",
           total, results.passed, results.failed, elapsed);
  }

  return results.failed > 0 ? 1 : 0;
}
