#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_OUT 180
#define TIME_PADDING 20
#define POPULATION_SIZE 100
#define MAX_VERTEX_SIZE 1000
#define MAX_EDGE_SIZE 10000
#define CHILDREN_RATE 1.0

int edges[MAX_EDGE_SIZE][3];

int main(int argc, char* argv[]) {

  int vertex_size, edge_size; 
  time_t start_time, end_time;
  int i;

  start_time = clock();
  end_time = (float) (TIME_OUT - TIME_PADDING) * CLOCKS_PER_SEC;

  if (argc != 4)
    return 0;

  // Read input file
  FILE *in = fopen(argv[2], "r");
  fscanf(in, "%d %d", &vertex_size, &edge_size);
  for (i = 0; i < edge_size; i++) {
    fscanf(in, "%d %d %d", &edges[i][0], &edges[i][1], &edges[i][2]);
  }

  while(1) {
    time_t curr_time = clock();
    if (curr_time >= end_time)
      break;
  }

  return 0;
}
