#include <stdio.h>
#include "external/hashmap.h"
#include "external/hashmap.c"
#include "common.h"


typedef struct Point {
  i64 x;
  i64 y;
  i64 z;
} Point;

typedef struct Volume {
  i64 x0;
  i64 x1;
  i64 y0;
  i64 y1;
  i64 z0;
  i64 z1;
} Volume;
#define MAX_N_VOLUMES 10000
Volume volumes[MAX_N_VOLUMES] = {};
i32 n_volumes = 0;


int point_compare(const void *a, const void *b, void *udata) {
    const Point *pa = a;
    const Point *pb = b;
    return pa->x == pb->x && pa->y == pb->y && pa->z == pb->z;
}


uint64_t point_hash(const void *item, u64 seed0, u64 seed1) {
    const Point *point = item;
    return hashmap_sip(point, sizeof(point), seed0, seed1);
}


int main() {
  FILE *f = fopen("data/22-test", "r");

  char on_or_off[4];
  i64 x0, x1, y0, y1, z0, z1;
  while (fscanf(f, "%s x=%ld..%ld,y=%ld..%ld,z=%ld..%ld", on_or_off, &x0, &x1, &y0, &y1, &z0, &z1) > 0) {
    volumes[n_volumes++] = (Volume){x0, x1, y0, y1, z0, z1};
  }

  for (i32 idx_volume = 0; idx_volume < n_volumes; idx_volume++) {
    Volume *v = &volumes[idx_volume];
    printf("(%ld..%ld) (%ld..%ld) (%ld..%ld)\n", v->x0, v->x1, v->y0, v->y1, v->z0, v->z1);
  }

  struct hashmap *map = hashmap_new(sizeof(Point), 0, 0, 0, point_hash, point_compare, NULL, NULL);

  return 0;
}
