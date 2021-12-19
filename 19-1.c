#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define CGLM_DEFINE_PRINTS
#include <cglm/cglm.h>
#include "common.h"

typedef vec2 v2;
typedef vec3 v3;
typedef vec4 v4;
typedef mat2 m2;
typedef mat3 m3;
typedef mat4 m4;

#define MAX_N_SCANNERS 32
#define MAX_N_SCANNER_POINTS 32


v3 points[MAX_N_SCANNERS][MAX_N_SCANNER_POINTS] = {};
i32 n_scanners = 0;
i32 n_points[MAX_N_SCANNERS] = {};
m3 cob_matrices[24] = {};


void print_scanners_and_points() {
  printf("%d scanners\n\n", n_scanners);
  for (i32 idx_scanner = 0; idx_scanner < n_scanners; idx_scanner++) {
    printf("scanner %d (%d points):\n", idx_scanner, n_points[idx_scanner]);
    for (i32 idx_point = 0; idx_point < n_points[idx_scanner]; idx_point++) {
      printf("  (%f, %f, %f)\n",
        points[idx_scanner][idx_point][0],
        points[idx_scanner][idx_point][1],
        points[idx_scanner][idx_point][2]);
    }
  }
  printf("\n");
}


void make_cob_matrices(m3 cob_matrices[24]) {
  /* [ newRight.x    newRight.y    newRight.z   ] */
  /* [ newUp.x       newUp.y       newUp.z      ] */
  /* [ newForward.x  newForward.y  newForward.z ] */
  i32 idx_matrix = 0;
  for (i32 axis = 0; axis <= 2; axis++) {
    for (i32 axis_sign = -1; axis_sign <= 1; axis_sign += 2) {
      for (i32 up_axis = 0; up_axis <= 1; up_axis += 1) {
        for (i32 up_axis_sign = -1; up_axis_sign <= 1; up_axis_sign += 2) {
          // forward
          cob_matrices[idx_matrix][axis][2] = axis_sign;

          // up
          cob_matrices[idx_matrix][(axis + up_axis) % 3][1] = up_axis_sign;

          // right
          v3 forward = {};
          forward[0] = cob_matrices[idx_matrix][0][2];
          forward[1] = cob_matrices[idx_matrix][1][2];
          forward[2] = cob_matrices[idx_matrix][2][2];
          v3 up = {};
          up[0] = cob_matrices[idx_matrix][0][1];
          up[1] = cob_matrices[idx_matrix][1][1];
          up[2] = cob_matrices[idx_matrix][2][1];
          v3 right = {};
          glm_vec3_cross(forward, up, right);
          cob_matrices[idx_matrix][0][0] = right[0];
          cob_matrices[idx_matrix][1][0] = right[1];
          cob_matrices[idx_matrix][2][0] = right[2];

          idx_matrix++;
        }
      }
    }
  }
}


void find_transformation(i32 idx_scanner_a, i32 idx_scanner_b) {
  printf("Finding transformation between scanner %d and scanner %d\n", idx_scanner_a, idx_scanner_b);

  for (i32 idx_matrix = 0; idx_matrix < 24; idx_matrix++) {
    printf("Trying cob matrix %d\n", idx_matrix);
    m3 *transform = &cob_matrices[idx_matrix];

    // Calculate deltas
    v3 deltas[MAX_N_SCANNER_POINTS * MAX_N_SCANNER_POINTS] = {};
    i32 n_deltas = 0;
    for (i32 idx_a_point = 0; idx_a_point < n_points[idx_scanner_a]; idx_a_point++) {
      for (i32 idx_b_point = 0; idx_b_point < n_points[idx_scanner_b]; idx_b_point++) {
        v3 transformed_b_point;
        glm_mat3_mulv(*transform, points[idx_scanner_b][idx_b_point], transformed_b_point);
        glm_vec3_sub(points[idx_scanner_a][idx_a_point], transformed_b_point, deltas[n_deltas]);
        n_deltas++;
      }
    }

    // Find delta counts
    v3 unique_deltas[MAX_N_SCANNER_POINTS * MAX_N_SCANNER_POINTS] = {};
    i32 delta_counts[MAX_N_SCANNER_POINTS * MAX_N_SCANNER_POINTS] = {};
    i32 n_unique_deltas = 0;
    for (i32 idx_delta = 0; idx_delta < n_deltas; idx_delta++) {
      // Check if we already have a count for this delta
      for (i32 idx_unique_delta = 0; idx_unique_delta < n_unique_deltas; idx_unique_delta++) {
        /* printf("Comparing (%f, %f, %f) and (%f, %f, %f)\n", */
        /*   unique_deltas[idx_unique_delta][0], */
        /*   unique_deltas[idx_unique_delta][1], */
        /*   unique_deltas[idx_unique_delta][2], */
        /*   deltas[idx_delta][0], */
        /*   deltas[idx_delta][1], */
        /*   deltas[idx_delta][2]); */
        if (glm_vec2_eqv_eps(unique_deltas[idx_unique_delta], deltas[idx_delta])) {
          delta_counts[idx_unique_delta]++;
          goto next_delta;
        }
      }

      // Add a count for this delta
      glm_vec2_copy(deltas[idx_delta], unique_deltas[n_unique_deltas]);
      delta_counts[n_unique_deltas]++;
      n_unique_deltas++;
next_delta:
      ;
    }
    printf("%d unique deltas, compared with %d total combinations\n",
      n_unique_deltas,
      MAX_N_SCANNER_POINTS * MAX_N_SCANNER_POINTS);

    // Find most common delta
    i32 idx_most_common_delta = -1;
    i32 biggest_delta_count = 0;
    for (i32 idx_unique_delta = 0; idx_unique_delta < n_unique_deltas; idx_unique_delta++) {
      if (delta_counts[idx_unique_delta] > biggest_delta_count) {
        idx_most_common_delta = idx_unique_delta;
        biggest_delta_count = delta_counts[idx_unique_delta];
      }
    }
    printf("Most common delta was (%f, %f, %f) with count %d\n",
      unique_deltas[idx_most_common_delta][0],
      unique_deltas[idx_most_common_delta][1],
      unique_deltas[idx_most_common_delta][2],
      biggest_delta_count);
  }
}


int main() {
  make_cob_matrices(cob_matrices);

  FILE *f = fopen("data/19-test", "r");

  while (true) {
    i32 idx_scanner;
    if (fscanf(f, "--- scanner %d ---\n", &idx_scanner) <= 0) {
      break;
    }
    n_scanners++;
    while (true) {
      i32 x;
      i32 y;
      if (
        fscanf(
          f, "%f,%f,%f",
          &points[n_scanners - 1][n_points[n_scanners - 1]][0],
          &points[n_scanners - 1][n_points[n_scanners - 1]][1],
          &points[n_scanners - 1][n_points[n_scanners - 1]][2]
        ) <= 0
      ) {
        break;
      }
      n_points[n_scanners - 1]++;

      char c;
      c = fgetc(f);
      assert(c == '\n');
      c = fgetc(f);
      if (c == '\n') {
        break;
      } else {
        ungetc(c, f);
      }
    }
  }

  print_scanners_and_points();

  find_transformation(0, 1);

  return 0;
}
