#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

int token;           // current token
char *src, *old_src; // pointer to source code string;
int poolsize;        // default size of text/data/stack
int line;            // line number

void next() {
  token = *src++;
  return;
}

void expression(int level) {
  // do nothing
}

void program() {
  next(); // get next token
  while (token > 0) {
    printf("token is: %c\n", token);
    next();
  }
}

int eval() {
  // do nothing yet
  return 0;
}

// read file into string
int loadfile(const char *filename) {
  int fd, i;

  if ((fd = open(filename, 0)) < 0) {
    printf("could not open(%s)\n", filename);
    return -1;
  }

  if (!(src = old_src = malloc(poolsize))) {
    printf("could not malloc(%d) for source area\n", poolsize);
    close(fd);
    return -1;
  }
  // read the source file
  if ((i = read(fd, src, poolsize - 1)) <= 0) {
    printf("read() returned %d\n", i);
    close(fd);
    return -1;
  }

  src[i] = 0; // add EOF character

  close(fd);
  return 0;
}

int main(int argc, char **argv) {
  argc--;
  argv++;

  poolsize = 256 * 1024; // arbitrary size
  line = 1;

  if (loadfile(*argv))
    return -1;

  program();

  return eval();
}
