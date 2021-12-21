#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"


#define BLUE "\033[0;34m"
#define RED "\033[0;31m"
#define WHITE "\033[0;37m"

i32 const start_boardpos[2] = {4, 8};
i32 const roll_probabilities[10] = {0, 0, 0, 1, 3, 6, 7, 6, 3, 1};
u64 universes_with_score[2][22][11] = {};


bool do_non_winning_scores_exist(i32 idx_player) {
  for (i32 score = 0; score < 21; score++) {
    for (i32 boardpos = 1; boardpos <= 10; boardpos++) {
      if (universes_with_score[idx_player][score][boardpos] > 0) {
        return true;
      }
    }
  }
  return false;
}


i32 mod10(i32 n) {
  while (n > 10) {
    n -= 10;
  }
  return n;
}


void init_universes() {
  for (i32 idx_player = 0; idx_player < 2; idx_player++) {
    for (i32 score = 3; score <= 9; score++) {
      i32 new_boardpos = mod10(start_boardpos[idx_player] + score);
      universes_with_score[idx_player][score][new_boardpos] = roll_probabilities[score];
    }
  }
}


void update_universes(i32 idx_player) {
  u64 universes_with_score_original[2][22][11];
  memcpy(universes_with_score_original, universes_with_score, sizeof(universes_with_score));
  for (i32 score = 0; score < 21; score++) {
    for (i32 boardpos = 1; boardpos <= 10; boardpos++) {
      for (i32 roll = 3; roll <= 9; roll++) {
        i32 roll_probability = roll_probabilities[roll];
        i32 new_boardpos = mod10(boardpos + roll);
        i32 new_score = score + new_boardpos;
        if (new_score > 21) {
          new_score = 21;
        }
        universes_with_score[idx_player][new_score][new_boardpos] +=
          roll_probability * universes_with_score_original[idx_player][score][boardpos];
      }

      universes_with_score[idx_player][score][boardpos] -=
        universes_with_score_original[idx_player][score][boardpos];
    }
  }
}


void print_universes() {
  printf("----------------------------------------------------\n\n");
  for (i32 idx_player = 0; idx_player < 2; idx_player++) {
    printf(BLUE"Player %d:\n"WHITE, idx_player + 1);

    // Score row header
    printf("     ");
    for (i32 score = 3; score <= 21; score++) {
      printf(RED"%12d"WHITE, score);
    }
    printf(RED"       total"WHITE);
    printf("\n");

    for (i32 boardpos = 1; boardpos <= 10; boardpos++) {
      // Board column header
      printf(RED"%5d"WHITE, boardpos);

      u64 total_for_boardpos = 0;

      // Score row
      for (i32 score = 3; score <= 21; score++) {
        total_for_boardpos += universes_with_score[idx_player][score][boardpos];
        printf("%12lu", universes_with_score[idx_player][score][boardpos]);
      }

      // Plus the total column
      printf("%12lu", total_for_boardpos);

      printf("\n");
    }

    // Total row
    u64 total_for_all = 0;
    printf(RED"total"WHITE);
    for (i32 score = 3; score <= 21; score++) {
      u64 total_for_score = 0;
      for (i32 boardpos = 1; boardpos <= 10; boardpos++) {
        total_for_score += universes_with_score[idx_player][score][boardpos];
      }
      total_for_all += total_for_score;
      printf("%12lu", total_for_score);
    }
    printf("%12lu", total_for_all);
    printf("\n\n");
  }
}


int main() {
  init_universes();
  print_universes();
  usleep(500 * 1000);

  i32 idx_step = 2;

  while (do_non_winning_scores_exist(0) && do_non_winning_scores_exist(1)) {
    printf("Step %d\n", idx_step);
    for (i32 idx_player = 0; idx_player < 2; idx_player++) {
      update_universes(idx_player);
    }
    print_universes();
    idx_step++;
    usleep(500 * 1000);
  }
  return 0;
}
