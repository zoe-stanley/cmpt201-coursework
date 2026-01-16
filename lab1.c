#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  char *line = NULL; // point to buffer for last input line, getline allocates
  size_t n = 0;      // buffer size in bytes (to grow with getline)
  ssize_t numchars;  // num chars getline reads each time, or -1 for EOF/err

  for (;;) { // forever

    printf("Please enter some text: \n"); // get input

    numchars = getline(&line, &n,
                       stdin); // reads until EOF, allocates line where NULL set
                               // stores buffer address in line
                               // stores buffer size as n
                               // returns numchars read or -1 (EOF)

    if (numchars == -1) { // check for errors/EOF within curr_line
      perror("getline failed");
      exit(EXIT_FAILURE); // exit loop --> MUST STILL FREE(LINE) OUTSIDE LOOP
    }

    printf("Full line: %s", line); // print line

    char *saveptr = NULL; // MUST SET STDIN TO SAVEPTR = NULL, else new string
    char *token = strtok_r(
        line, " ", &saveptr); // SAVEPTR returns pointer to token start in line
    // pass string buffer, set delim to <space>
    // str_tok_r stores continuation point after delimiter to saveptr
    // return ptr to token start or NULL
    int i = 1;              // track num_tokens for print
    while (token != NULL) { // until EOF / end of line reached or ERR
      printf("Token %d: %s\n", i, token); // print tokens line by line
      i++;

      token = strtok_r(NULL, " ", &saveptr);
      // must set NULL as line, otherwise not same line
      // saveptr tracks last place
    }
  }
  free(line); // ensure that OUTSIDE all loops, we free space allocated for
              // buffer
  return 0;
}
