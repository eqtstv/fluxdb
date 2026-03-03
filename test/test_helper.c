#include "test_helper.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

char *run_db_commands(const char **commands, int num_commands) {
  int pipe_in[2];
  int pipe_out[2];

  if (pipe(pipe_in) == -1) {
    perror("pipe");
    return NULL;
  }
  if (pipe(pipe_out) == -1) {
    perror("pipe");
    close(pipe_in[0]);
    close(pipe_in[1]);
    return NULL;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    return NULL;
  }

  if (pid == 0) {
    close(pipe_in[1]);
    close(pipe_out[0]);
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);

    char *args[] = {"./main", NULL};
    execvp("./main", args);
    perror("execvp");
    exit(1);
  }

  close(pipe_in[0]);
  close(pipe_out[1]);

  for (int i = 0; i < num_commands; i++) {
    size_t cmd_len = strlen(commands[i]);
    char command_with_newline[cmd_len + 2];
    strcpy(command_with_newline, commands[i]);
    strcat(command_with_newline, "\n");
    write(pipe_in[1], command_with_newline, strlen(command_with_newline));
  }

  write(pipe_in[1], ".exit\n", 6);
  close(pipe_in[1]);

  char buffer[4096];
  size_t total_read = 0;
  size_t buffer_capacity = 4096;
  char *output = malloc(buffer_capacity);

  ssize_t n;
  while ((n = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0) {
    if (total_read + n >= buffer_capacity) {
      buffer_capacity *= 2;
      output = realloc(output, buffer_capacity);
    }
    memcpy(output + total_read, buffer, n);
    total_read += n;
  }
  close(pipe_out[0]);

  int status;
  waitpid(pid, &status, 0);

  output[total_read] = '\0';
  return output;
}

char *run_db_command(const char *command) {
  const char *commands[1] = {command};
  return run_db_commands(commands, 1);
}

bool assert_string_contains(const char *haystack, const char *needle) {
  return strstr(haystack, needle) != NULL;
}

bool assert_string_equals(const char *expected, const char *actual) {
  return strcmp(expected, actual) == 0;
}

void free_command_result(CommandResult *result) {
  if (result && result->output) {
    free(result->output);
    result->output = NULL;
  }
}