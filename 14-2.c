#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "common.h"


#define MAX_SEQUENCE_SIZE 128
char sequence[MAX_SEQUENCE_SIZE] = {};
#define MAX_N_RULES 128
char rules_from[MAX_N_RULES][2] = {};
char rules_to[MAX_N_RULES] = {};
i32 n_rules = 0;
#define N_STEPS 4
i32 step = 0;


int main() {
  FILE *f = fopen("data/14", "r");

  assert(fscanf(f, "%s\n\n", sequence) > 0);

  char rule_from[3];
  char rule_to[1];
  while (fscanf(f, "%s -> %c\n", rule_from, rule_to) > 0) {
    rules_from[n_rules][0] = rule_from[0];
    rules_from[n_rules][1] = rule_from[1];
    rules_to[n_rules] = rule_to[0];
    n_rules++;
  }

  while (step < N_STEPS) {
    printf("=== Step %d\n", step);
    step++;
  }

  return 0;
}
