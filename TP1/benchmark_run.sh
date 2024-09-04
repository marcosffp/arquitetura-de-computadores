#!/bin/bash

# Nomes dos arquivos CSV de entrada
csv_files=("test_data_1.csv" "test_data_10.csv" "test_data_100.csv" "test_data_1000.csv" "test_data_10000.csv")

# Métodos de normalização (1: Lookup Table, 2: Quake III, 3: SSE, 0: Default)
methods=(1 2 3 0)

# Executa o programa 10 vezes para cada combinação de arquivo CSV e método
for csv_file in "${csv_files[@]}"; do
  for method in "${methods[@]}"; do
    for i in {1..10}; do
      ./benchmark "$method" "$csv_file"
    done
  done
done

