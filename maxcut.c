#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define TIME_OUT 180
#define TIME_PADDING 20
#define POPULATION_SIZE 40
#define CHILDREN_SIZE 80
#define CUT_POINT 0
#define MAX_VERTEX_SIZE 5000
#define MAX_CROSSOVER_NUM 1
#define MAX_EDGE_SIZE MAX_VERTEX_SIZE*(MAX_VERTEX_SIZE-1)/2
#define MUTATION_RATE 0.05

int vertex_size, edge_size;
int edges[MAX_EDGE_SIZE][3];
int adj_list[MAX_VERTEX_SIZE][MAX_VERTEX_SIZE];
int adj_list_weight[MAX_VERTEX_SIZE][MAX_VERTEX_SIZE];
int adj_list_len[MAX_VERTEX_SIZE];
int population_pool[POPULATION_SIZE][MAX_VERTEX_SIZE];
int children_pool[CHILDREN_SIZE][MAX_VERTEX_SIZE];
int next_gen_pool[POPULATION_SIZE][MAX_VERTEX_SIZE];
int score[POPULATION_SIZE];
int children_score[CHILDREN_SIZE];
int accum_score[POPULATION_SIZE];
char *output_path;

time_t start_time, end_time, deadline_time;

int delta (int *chromosome, int vertex) {

  int i, inv_group = 1 - chromosome[vertex];
  int gain = 0;

  for(i = 0; i < adj_list_len[vertex]; i++) {
    if (inv_group + chromosome[adj_list[vertex][i]] == 1) {
      gain += adj_list_weight[vertex][i];
    } else {
      gain -= adj_list_weight[vertex][i];
    }
  }
  return gain;
}

void local_optimize(int *chromosome) {

  int i, j, temp;
  int *permutation = malloc(vertex_size * sizeof(int));

  for (i = 0; i < vertex_size; i++) {
    permutation[i] = i;
  }

  for (i = vertex_size - 1; i >= 0; i--) {
    j = rand() % (i + 1);
    temp = permutation[j];
    permutation[j] = permutation[i];
    permutation[i] = temp;
  }

  int improved;
	int cnt = 0;

  do {
		cnt++;
    improved = 0;
    for (i = 0; i < vertex_size; i++) {
      j = permutation[i];
      if (delta(chromosome, j) > 0) {
        chromosome[j] = 1 - chromosome[j];
        improved = 1;
      }
    }
  } while(improved != 0 && cnt < 2);
}

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

int find_min_crossover_num(int *crossover_points, int crossover_num) {

  int i;
  int min_crossover_point = vertex_size, min_crossover_index = -1;
  for (i = 0; i < crossover_num; i++) {
    if (min_crossover_point > crossover_points[i]) {
      min_crossover_point = crossover_points[i];
      min_crossover_index = i;
    }
  }
  if (min_crossover_index != -1)
    crossover_points[min_crossover_index] = vertex_size;
  return min_crossover_point;
}

void crossover(int p1, int p2, int child) {

  int i;
  int crossover_num = (rand() % MAX_CROSSOVER_NUM) + 1;
  int *crossover_points = (int *)malloc(crossover_num * sizeof(int));

  for (i = 0; i < crossover_num; i++) {
    crossover_points[i] = (rand() % vertex_size) + 1;
  }
  int crossover_start = 0, crossover_end;
  int cur_population = p1;
  crossover_end = find_min_crossover_num(crossover_points, crossover_num);
  for (i = 0; i < crossover_num + 1; i++) {
    memcpy(children_pool[child] + crossover_start, population_pool[cur_population], (crossover_end - crossover_start) * sizeof(int));
    crossover_start = crossover_end;
    crossover_end = find_min_crossover_num(crossover_points, crossover_num);
    if (cur_population == p1)
      cur_population = p2;
    else
      cur_population = p1;
  }
//  memcpy(children_pool[child], population_pool[p1], crossover_point * sizeof(int));
//  memcpy(children_pool[child] + crossover_point, population_pool[p2] + crossover_point, (vertex_size - crossover_point) * sizeof(int));
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

  for (i = mutation_start_point; i <= mutation_end_point; i++) {
     population_pool[index][i] = rand() % 2;
  }
}

void check_time() {

	int i;
	time_t curr_time = clock();

  if (curr_time >= end_time) {
		if (curr_time >= deadline_time)
			printf("Warning! Deadline over!\n");
		int max_value = -1;
  	int max_index;
  	for (i = 0; i < POPULATION_SIZE; i++) {
    	if (max_value < score[i]) {
      	max_value = score[i];
      	max_index = i;
    	}
  	}
	  FILE *out = fopen(output_path, "w");
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
	  fclose(out);
  	exit(0);
	}

}

int main(int argc, char* argv[]) {
 
  int i, j, k;

  start_time = clock();
  end_time = (TIME_OUT - TIME_PADDING) * CLOCKS_PER_SEC;
	deadline_time = TIME_OUT * CLOCKS_PER_SEC;
	output_path = argv[2];

  if (argc != 3)
    return 0;

  srand(time(NULL));
  memset(adj_list_len, 0, MAX_VERTEX_SIZE * sizeof(int));

  // Read input file
  FILE *in = fopen(argv[1], "r");
  fscanf(in, "%d %d", &vertex_size, &edge_size);
  for (i = 0; i < edge_size; i++) {
    fscanf(in, "%d %d %d", &edges[i][0], &edges[i][1], &edges[i][2]);
    edges[i][0] -= 1;
    edges[i][1] -= 1;

    adj_list[edges[i][0]][adj_list_len[edges[i][0]]] = edges[i][1];
    adj_list_weight[edges[i][0]][adj_list_len[edges[i][0]]] = edges[i][2];
    adj_list[edges[i][1]][adj_list_len[edges[i][1]]] = edges[i][0];
    adj_list_weight[edges[i][1]][adj_list_len[edges[i][1]]] = edges[i][2];
    adj_list_len[edges[i][0]] += 1;
    adj_list_len[edges[i][1]] += 1;
//    printf("%d %d %d\n", edges[i][0], edges[i][1], edges[i][2]);
  }
	fclose(in);

  // Create first population
  for (i = 0; i < POPULATION_SIZE; i++) {
    for (j = 0; j < vertex_size; j++) {
      population_pool[i][j] = rand() % 2;
      if (population_pool[i][j] !=0 && population_pool[i][j] != 1) {
        printf("Error!\n");
        return 0;
      }
    }
 //   local_optimize(population_pool[i]);
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
 //     local_optimize(children_pool[i]);
			check_time();
    }
    calculate_children_score();
		check_time();
		// Replace using u + lambda strategy
    for (i = 0; i < CUT_POINT; i++) {
      int parent_max = find_max_chromosome();
			memcpy(next_gen_pool[i], population_pool[parent_max], vertex_size * sizeof(int));
			score[parent_max] = 0;
    } 
    for (i = CUT_POINT; i < POPULATION_SIZE; i++) {
      int child_max = find_max_child_chromosome();
      memcpy(next_gen_pool[i], children_pool[child_max], vertex_size * sizeof(int));
			/*
			if (i == CUT_POINT)
				printf("Children Max: %d\n", children_score[child_max]);*/
      children_score[child_max] = 0;
    }
		for (i = 0; i < POPULATION_SIZE; i++) {
	    memcpy(population_pool[i], next_gen_pool[i], vertex_size * sizeof(int));
		}
    // Mutation
    for (i = 0; i < POPULATION_SIZE * MUTATION_RATE; i++) {
      j = rand() % POPULATION_SIZE;
      mutate(j);
      local_optimize(population_pool[j]);
    }	
    calculate_score();
//		printf("After Children Max: %d\n", score[CUT_POINT]);
		check_time();
    double average = 0;
    int max_score = -1;
    for (i = 0; i < POPULATION_SIZE; i++) {
      average += score[i];
      if (score[i] > max_score) {
        max_score = score[i];
      }
    }
    average /= POPULATION_SIZE;
    printf("%d,%f\n", max_score, average);
  }
  return 0;
}
