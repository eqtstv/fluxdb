CC = cc
CFLAGS = -Wall -Wextra

.PHONY: test run clean

main: main.c
	@$(CC) $(CFLAGS) -o main main.c

test: main
	@$(CC) $(CFLAGS) -c test/test_helper.c -o test/test_helper.o
	@$(CC) $(CFLAGS) -c test/test_db.c -o test/test_db.o
	@$(CC) $(CFLAGS) -o test_ test/test_helper.o test/test_db.o
	@./test_ || true
	@rm -f test_ test/*.o

run: main
	@./main

clean:
	@rm -rf main main.dSYM test_
