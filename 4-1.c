#include <stdio.h>
#include "common.h"


#define MAX_N_BOARDS 256
#define MAX_N_DRAWS 512
#define BOARD_SIZE 5


typedef struct {
  i8 number;
  b8 is_marked;
} Tile;


int main() {
  i32 n_draws = 0;
  i8 draws[MAX_N_DRAWS];
  i32 n_boards = 0;
  Tile boards[MAX_N_BOARDS][BOARD_SIZE][BOARD_SIZE];
  i32 current = -1;

  FILE *f = fopen("data/4", "r");

  // Read draws
  while (fscanf(f, "%d,", &current) > 0) {
    draws[n_draws++] = current;

    char c = fgetc(f);
    if (c == '\n') {
      fgetc(f);
      break;
    }
    ungetc(c, f);
  }

  // Read boards
  while (true) {
    for (i32 i = 0; i < BOARD_SIZE; i++) {
      for (i32 j = 0; j < BOARD_SIZE; j++) {
        if (fscanf(f, "%d", &current) < 1) {
          goto end_read_boards;
        }
        Tile *tile = &boards[n_boards][i][j];
        tile->number = current;
        tile->is_marked = false;
      }
    }
    n_boards++;
  }
end_read_boards:

  printf("n_boards = %d\n", n_boards);

  return 0;
}
