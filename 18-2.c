#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"


#define MAX_N_SUMBERS 128

// sumber = snailfish number
typedef struct Sumber {
  i32 left_number;
  struct Sumber *left_sumber;
  i32 right_number;
  struct Sumber *right_sumber;
  struct Sumber *parent_sumber;
} Sumber;

typedef enum Direction {
  LEFT,
  RIGHT
} Direction;


Direction opposite_direction(Direction dir) {
  if (dir == LEFT) {
    return RIGHT;
  }
  return LEFT;
}


Sumber* create_sumber() {
  return (Sumber*)calloc(1, sizeof(Sumber));
}


void destroy_sumber(Sumber *sumber) {
  if (sumber->left_sumber) {
    destroy_sumber(sumber->left_sumber);
  }
  if (sumber->right_sumber) {
    destroy_sumber(sumber->right_sumber);
  }
  free(sumber);
}


Sumber* copy_sumber(Sumber *original) {
  Sumber *sumber = create_sumber();
  sumber->left_number = original->left_number;
  sumber->right_number = original->right_number;
  if (original->left_sumber) {
    sumber->left_sumber = copy_sumber(original->left_sumber);
    sumber->left_sumber->parent_sumber = sumber;
  }
  if (original->right_sumber) {
    sumber->right_sumber = copy_sumber(original->right_sumber);
    sumber->right_sumber->parent_sumber = sumber;
  }
  return sumber;
}


Sumber* read_sumber(FILE *f) {
  Sumber *sumber = create_sumber();
  char c;
  c = fgetc(f);
  if (c != '[') {
    return NULL;
  }
  assert(c == '[');

  c = fgetc(f);
  if (isdigit(c)) {
    sumber->left_number = c - '0';
  } else {
    ungetc(c, f);
    sumber->left_sumber = read_sumber(f);
    sumber->left_sumber->parent_sumber = sumber;
  }

  c = fgetc(f);
  assert(c == ',');

  c = fgetc(f);
  if (isdigit(c)) {
    sumber->right_number = c - '0';
  } else {
    ungetc(c, f);
    sumber->right_sumber = read_sumber(f);
    sumber->right_sumber->parent_sumber = sumber;
  }

  c = fgetc(f);
  assert(c == ']');

  return sumber;
}


void print_sumber(Sumber *sumber) {
  printf("[");
  if (sumber->left_sumber) {
    print_sumber(sumber->left_sumber);
  } else {
    printf("%d", sumber->left_number);
  }
  printf(",");
  if (sumber->right_sumber) {
    print_sumber(sumber->right_sumber);
  } else {
    printf("%d", sumber->right_number);
  }
  printf("]");
}


void add_outer(Sumber *sumber, i32 number, Direction dir, bool did_change_dir, Sumber *prev) {
  if (sumber == NULL) {
    return;
  }
  if (dir == LEFT && !sumber->left_sumber) {
    sumber->left_number += number;
    return;
  }
  if (dir == RIGHT && !sumber->right_sumber) {
    sumber->right_number += number;
    return;
  }
  if (dir == LEFT && sumber->left_sumber && sumber->left_sumber != prev) {
    if (did_change_dir) {
      add_outer(sumber->left_sumber, number, LEFT, true, sumber);
    } else {
      add_outer(sumber->left_sumber, number, RIGHT, true, sumber);
    }
    return;
  }
  if (dir == RIGHT && sumber->right_sumber && sumber->right_sumber != prev) {
    if (did_change_dir) {
      add_outer(sumber->right_sumber, number, RIGHT, true, sumber);
    } else {
      add_outer(sumber->right_sumber, number, LEFT, true, sumber);
    }
    return;
  }
  add_outer(sumber->parent_sumber, number, dir, did_change_dir, sumber);
}


bool explode_sumber(Sumber *sumber, i32 nesting) {
  if (nesting >= 4) {
    assert(!sumber->left_sumber);
    assert(!sumber->right_sumber);
    add_outer(sumber->parent_sumber, sumber->left_number, LEFT, false, sumber);
    add_outer(sumber->parent_sumber, sumber->right_number, RIGHT, false, sumber);
    if (sumber->parent_sumber->left_sumber == sumber) {
      sumber->parent_sumber->left_sumber = 0;
    }
    if (sumber->parent_sumber->right_sumber == sumber) {
      sumber->parent_sumber->right_sumber = 0;
    }
    destroy_sumber(sumber);
    return true;
  }
  if (sumber->left_sumber) {
    if (explode_sumber(sumber->left_sumber, nesting + 1)) {
      return true;
    }
  }
  if (sumber->right_sumber) {
    if (explode_sumber(sumber->right_sumber, nesting + 1)) {
      return true;
    }
  }
  return false;
}


bool split_sumber(Sumber *sumber) {
  if (!sumber->left_sumber && sumber->left_number > 9) {
    sumber->left_sumber = create_sumber();
    sumber->left_sumber->parent_sumber = sumber;
    sumber->left_sumber->left_number = floor((f32)sumber->left_number / 2.0f);
    sumber->left_sumber->right_number = ceil((f32)sumber->left_number / 2.0f);
    sumber->left_number = 0;
    return true;
  }
  if (sumber->left_sumber) {
    if (split_sumber(sumber->left_sumber)) {
      return true;
    }
  }
  if (!sumber->right_sumber && sumber->right_number > 9) {
    sumber->right_sumber = create_sumber();
    sumber->right_sumber->parent_sumber = sumber;
    sumber->right_sumber->left_number = floor((f32)sumber->right_number / 2.0f);
    sumber->right_sumber->right_number = ceil((f32)sumber->right_number / 2.0f);
    sumber->right_number = 0;
    return true;
  }
  if (sumber->right_sumber) {
    if (split_sumber(sumber->right_sumber)) {
      return true;
    }
  }
  return false;
}


bool reduce_sumber(Sumber *sumber) {
  if (explode_sumber(sumber, 0)) {
    return true;
  }
  if (split_sumber(sumber)) {
    return true;
  }
  return false;
}


Sumber* add_sumbers(Sumber *a, Sumber *b) {
  Sumber *sumber = create_sumber();
  sumber->left_sumber = a;
  a->parent_sumber = sumber;
  sumber->right_sumber = b;
  b->parent_sumber = sumber;
  return sumber;
}


i32 get_magnitude(Sumber *sumber) {
  i32 submagnitude = 0;
  if (sumber->left_sumber) {
    submagnitude += 3 * get_magnitude(sumber->left_sumber);
  } else {
    submagnitude += 3 * sumber->left_number;
  }
  if (sumber->right_sumber) {
    submagnitude += 2 * get_magnitude(sumber->right_sumber);
  } else {
    submagnitude += 2 * sumber->right_number;
  }
  return submagnitude;
}


int main() {
  FILE *f = fopen("data/18", "r");

  Sumber *sumbers[MAX_N_SUMBERS] = {};
  i32 n_sumbers = 0;

  while (true) {
    sumbers[n_sumbers++] = read_sumber(f);
    char c = fgetc(f);
    assert(c == '\n');
    c = fgetc(f);
    if (c == EOF) {
      break;
    } else {
      ungetc(c, f);
    }
  }

  for (i32 i = 0; i < n_sumbers; i++) {
    print_sumber(sumbers[i]);
    printf("\n");
  }

  i32 max_magnitude = 0;
  i32 max_magnitude_a_idx = -1;
  i32 max_magnitude_b_idx = -1;

  for (i32 i = 0; i < n_sumbers; i++) {
    for (i32 j = 0 ; j < n_sumbers; j++) {
      Sumber *a = copy_sumber(sumbers[i]);
      Sumber *b = copy_sumber(sumbers[j]);
      Sumber *sum = add_sumbers(a, b);

      while (true) {
        if (!reduce_sumber(sum)) {
          break;
        }
      };

      i32 magnitude = get_magnitude(sum);
      if (magnitude > max_magnitude) {
        max_magnitude = magnitude;
        max_magnitude_a_idx = i;
        max_magnitude_b_idx = j;
      }
    }
  }

  printf("max magnitude: %d\n", max_magnitude);
  print_sumber(sumbers[max_magnitude_a_idx]);
  printf("\n+\n");
  print_sumber(sumbers[max_magnitude_b_idx]);
  printf("\n");

  return 0;
}
