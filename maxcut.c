#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TIME_OUT 180
#define TIME_PADDING 10
#define POPULATION_SIZE 500
#define CHILDREN_SIZE 1000
#define MAX_VERTEX_SIZE 1000
#define MAX_EDGE_SIZE 10000
#define MUTATION_RATE 0.005

int vertex_size, edge_size;
int edges[MAX_EDGE_SIZE][3];
int population_pool[POPULATION_SIZE][MAX_VERTEX_SIZE];
int children_pool[CHILDREN_SIZE][MAX_VERTEX_SIZE];
int next_gen_pool[POPULATION_SIZE][MAX_VERTEX_SIZE];
int score[POPULATION_SIZE];
int children_score[CHILDREN_SIZE];
int accum_score[POPULATION_SIZE];

void calculate_score() {

  int i, j; 

  for(i = 0; i < POPULATION_SIZE; i++) {
    score[i] = 0;
    for(j = 0; j < edge_size; j++) {
      if (population_pool[i][edges[j][0]] + population_pool[i][edges[j][1]] == 1) {
        score[i] += edges[j][2];
      }
    }
    if (score[i] < 0)
      score[i] = 0;
  }
  accum_score[0] = score[0];
  for(i = 1; i < POPULATION_SIZE; i++) {
    accum_score[i] = accum_score[i-1] + score[i];
  }
}

void calculate_children_score() {

  int i, j; 

  for(i = 0; i < CHILDREN_SIZE; i++) {
    children_score[i] = 0;
    for(j = 0; j < edge_size; j++) {
      if (children_pool[i][edges[j][0]] + children_pool[i][edges[j][1]] == 1) {
        children_score[i] += edges[j][2];
      }
    }
    if (children_score[i] < 0)
      children_score[i] = 0;
  }
}

void crossover(int p1, int p2, int child) {

  int crossover_point = rand() % vertex_size;

  memcpy(children_pool[child], population_pool[p1], crossover_point * sizeof(int));
  memcpy(children_pool[child] + crossover_point, population_pool[p2] + crossover_point, (vertex_size - crossover_point) * sizeof(int));
}

int find_max_chromosome() {

  int i, max_value, max_index;

  max_value = -1;
  for (i = 0; i < POPULATION_SIZE; i++) {
    if (max_value < score[i]) {
      max_value = score[i];
      max_index = i;
    }
  }
  return max_index;
}

int find_max_child_chromosome() {
  
  int i, max_value, max_index;

  max_value = -1;
  for (i = 0; i < CHILDREN_SIZE; i++) {
    if (max_value < children_score[i]) {
      max_value = children_score[i];
      max_index = i;
    }
  }
  return max_index;
}

void mutate(int index) {

  int mutation_start_point = rand() % vertex_size;
  int mutation_end_point = rand() % vertex_size;
  int i;

  if (mutation_end_point < mutation_start_point) {
    int temp;
    temp = mutation_start_point;
    mutation_end_point = mutation_start_point;
    mutation_start_point = mutation_end_point;
  }

  int length = mutation_end_point - mutation_start_point + 1;

  int *rev_seq = (int *)malloc(length * sizeof(int));

  for (i = mutation_start_point; i <= mutation_end_point; i++) {
    rev_seq[length - i + mutation_start_point - 1] = population_pool[index][i];
  }
  memcpy(population_pool[index] + mutation_start_point, rev_seq, length * sizeof(int));
  free(rev_seq);
}

int main(int argc, char* argv[]) {

  time_t start_time, end_time;
  int i, j, k;

  start_time = clock();
  end_time = (TIME_OUT - TIME_PADDING) * CLOCKS_PER_SEC;

  if (argc != 3)
    return 0;

  srand(time(NULL));

  // Read input file
  FILE *in = fopen(argv[1], "r");
  fscanf(in, "%d %d", &vertex_size, &edge_size);
  for (i = 0; i < edge_size; i++) {
    fscanf(in, "%d %d %d", &edges[i][0], &edges[i][1], &edges[i][2]);
    edges[i][0] -= 1;
    edges[i][1] -= 1;
//    printf("%d %d %d\n", edges[i][0], edges[i][1], edges[i][2]);
  }

  // Create first population
  for (i = 0; i < POPULATION_SIZE; i++) {
    for (j = 0; j < vertex_size; j++) {
      population_pool[i][j] = rand() % 2;
      if (population_pool[i][j] !=0 && population_pool[i][j] != 1) {
        printf("Error!\n");
        return 0;
      }
    }
  }
  calculate_score();

  int gen = 1;
  while(1) {
    // Selection & Crossover step
    int score_sum = accum_score[POPULATION_SIZE-1];
    for(i = 0; i < CHILDREN_SIZE; i++) {
      // Selection
      int random_choice[2];
      random_choice[0] = rand() % score_sum;
      random_choice[1] = rand() % score_sum;
      int parent[2];
      parent[0] = -1;
      parent[1] = -1;
      for (j = 0; j < 2; j++) {
        if (random_choice[j] < accum_score[0]) {
          parent[j] = 0;
        }
      }
      for (j = 1; j < vertex_size; j++) {
        for (k = 0; k < 2; k++) {
          if (random_choice[k] >= accum_score[j-1] && random_choice[k] < accum_score[j]) {
            parent[k] = j;
          }
        }
        if (parent[0] != -1 && parent[1] != -1)
          break;
      }
      // Crossover
      crossover(parent[0], parent[1], i);
    }
    calculate_children_score();
    // Replace using u + lambda strategy
    for (i = 0; i < CHILDREN_SIZE; i++) {
      int parent_max = find_max_chromosome();
      int child_max = find_max_child_chromosome();
      if (score[parent_max] > children_score[child_max]) {
        memcpy(next_gen_pool[i], population_pool[parent_max], vertex_size * sizeof(int));
        score[parent_max] = 0;
      } else {
        memcpy(next_gen_pool[i], children_pool[child_max], vertex_size * sizeof(int));
        children_score[child_max] = 0;
      }
    }
    memcpy(population_pool, next_gen_pool, vertex_size * POPULATION_SIZE * sizeof(int));
    // Mutation
    for (i = 0; i < POPULATION_SIZE * MUTATION_RATE; i++)
      mutate(rand() % POPULATION_SIZE);
    calculate_score();
    time_t curr_time = clock();
    if (curr_time >= end_time)
      break;
 /*  
    double average = 0;
    int max_score = -1;
    for (i = 0; i < POPULATION_SIZE; i++) {
      average += score[i];
      if (score[i] > max_score) {
        max_score = score[i];
      }
    }
    average /= POPULATION_SIZE;
    printf("%d,%f\n", max_score, average);*/
  }
  int max_value = -1;
  int max_index;
  for (i = 0; i < POPULATION_SIZE; i++) {
    if (max_value < score[i]) {
      max_value = score[i];
      max_index = i;
    }
  }

  FILE *out = fopen(argv[2], "w");
  int *result = (int *)malloc(vertex_size * sizeof(int));
  int result_length = 0;
  for (i = 0; i < vertex_size; i++) {
    if (population_pool[max_index][i] == 1) {
      result[result_length] = i + 1;
      result_length++;
    }
  }
  for (i = 0; i < result_length - 1; i++) {
    fprintf(out, "%d ", result[i]);
  }
  fprintf(out, "%d", result[result_length-1]);
  free(result);
  // Should be deleted!
/*  printf("Result: %d\n", max_value);
  printf("max_index: %d\n", max_index);*/
  fclose(in);
  fclose(out);
  return 0;
}
