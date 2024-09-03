#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <xmmintrin.h> // Para a implementação SSE

#define _GNU_SOURCE
#define TABLE_SIZE 10000

// Lookup Table para a raiz quadrada inversa
float lookup_table[TABLE_SIZE];

// Função para inicializar a Lookup Table
void init_lookup_table() {
  for (int i = 1; i < TABLE_SIZE; i++) {
    lookup_table[i] = 1.0f / sqrtf((float)i / TABLE_SIZE);
  }
}

// Função para obter a raiz quadrada inversa usando a Lookup Table
float inv_sqrt_lookup(float x) {
  // Inicializar a Lookup Table
  init_lookup_table();

  if (x < 0.0f)
    x = 0.0f;
  if (x > 1.0f)
    x = 1.0f;
  int index = (int)(x * (TABLE_SIZE - 1));

  return lookup_table[index];
}

// Função para obter a raiz quadrada inversa usando a otimização do Quake III
float inv_sqrt_quake(float x) {
  int i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = x * 0.5F;
  y = x;
  i = *(int *)&y; // Acessa os bits de ponto flutuante como um inteiro
  i = 0x5f3759df - (i >> 1); // Estimativa mágica
  y = *(float *)&i;
  y = y * (threehalfs - (x2 * y * y)); // Iteração de Newton-Raphson

  return y;
}

// Função para obter a raiz quadrada inversa usando instruções de hardware SSE
float inv_sqrt_sse(float x) {
  __m128 val = _mm_set_ss(x);        // Coloca 'x' em um registrador SSE
  __m128 result = _mm_rsqrt_ss(val); // Executa rsqrtss
  return _mm_cvtss_f32(result);      // Converte o resultado de volta para float
}

// Função para normalizar um vetor de características usando uma das abordagens
void normalize_feature_vector(float *features, int length, int method) {
  float sum = 0.0f;
  for (int i = 0; i < length; i++) {
    sum += features[i] * features[i];
  }

  float inv_sqrt;
  switch (method) {
  case 1: // Lookup Table
    inv_sqrt = inv_sqrt_lookup(sum);
    break;
  case 2: // Quake III Optimization
    inv_sqrt = inv_sqrt_quake(sum);
    break;
  case 3: // SSE
    inv_sqrt = inv_sqrt_sse(sum);
    break;
  default: // Default to standard sqrt
    inv_sqrt = 1.0f / sqrt(sum);
    break;
  }

  for (int i = 0; i < length; i++) {
    features[i] *= inv_sqrt;
  }
}

// Função para ler dados de um arquivo CSV
float **read_csv(const char *filename, int *num_elements, int *num_dimensions) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }

  *num_elements = 0;
  *num_dimensions = 0;
  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    if (*num_elements == 0) {
      char *token = strtok(line, ",");
      while (token) {
        (*num_dimensions)++;
        token = strtok(NULL, ",");
      }
    }
    (*num_elements)++;
  }
  rewind(file);

  float **features = (float **)malloc(*num_elements * sizeof(float *));
  for (int i = 0; i < *num_elements; i++) {
    features[i] = (float *)malloc(*num_dimensions * sizeof(float));
  }

  int i = 0;
  while (fgets(line, sizeof(line), file)) {
    int j = 0;
    char *token = strtok(line, ",");
    while (token) {
      features[i][j++] = atof(token);
      token = strtok(NULL, ",");
    }
    i++;
  }

  fclose(file);
  return features;
}

// Função para medir o tempo de execução usando a biblioteca 'resources'
void get_resource_usage(struct rusage *usage) { getrusage(RUSAGE_SELF, usage); }

void print_resource_usage(const char *label, struct rusage *usage) {
  printf("%s\n", label);
  printf("User time: %ld.%06ld seconds\n", usage->ru_utime.tv_sec,
         usage->ru_utime.tv_usec);
  printf("System time: %ld.%06ld seconds\n", usage->ru_stime.tv_sec,
         usage->ru_stime.tv_usec);
  printf("Maximum resident set size: %ld kilobytes\n", usage->ru_maxrss);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <method> <csv_file_path>\n", argv[0]);
    printf("Method: 1 for Lookup Table, 2 for Quake III, 3 for SSE and any "
           "other number to default\n");
    return 1;
  }

  int method = atoi(argv[1]);
  const char *csv_file_path = argv[2];

  int num_elements, num_dimensions;
  float **features = read_csv(csv_file_path, &num_elements, &num_dimensions);

  struct rusage start_usage, end_usage;

  get_resource_usage(&start_usage);
  for (int i = 0; i < num_elements; i++) {
    normalize_feature_vector(features[i], num_dimensions, method);
  }
  get_resource_usage(&end_usage);

  printf("Normalized features:\n");
  for (int i = 0; i < num_elements; i++) {
    for (int j = 0; j < num_dimensions; j++) {
      printf("%f ", features[i][j]);
    }
    printf("\n");
  }

  printf("Method: %s\n", method == 2   ? "Quake III"
                         : method == 3 ? "SSE"
                         : method == 1 ? "Lookup Table"
                                       : "Default invertion");
  printf("Execution time and resource usage:\n");
  print_resource_usage("Start Usage", &start_usage);
  print_resource_usage("End Usage", &end_usage);

  // Free allocated memory
  for (int i = 0; i < num_elements; i++) {
    free(features[i]);
  }
  free(features);

  return 0;
}
