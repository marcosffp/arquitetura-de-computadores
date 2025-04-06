```markdown
# Benchmark de Normalização Vetorial com Raiz Quadrada Inversa

Este projeto em C tem como objetivo comparar diferentes métodos de cálculo da raiz quadrada inversa para normalização de vetores, medindo o desempenho de cada abordagem.

---

## 📂 Estrutura Geral do Código

- **Leitura de dados** a partir de um arquivo CSV.
- **Cálculo da norma** (soma dos quadrados dos elementos do vetor).
- **Normalização** do vetor com raiz quadrada inversa.
- **Métodos de otimização** utilizados:
  - Lookup Table
  - Algoritmo do Quake III
  - Instruções SSE (Single Instruction, Multiple Data)
- **Medição de desempenho** com `getrusage` da biblioteca `sys/resource.h`.
- **Gravação dos resultados** em arquivo CSV.

---

## ⚙️ Métodos de Cálculo da Raiz Quadrada Inversa

### 1. Lookup Table

```c
lookup_table[i] = 1.0f / sqrtf((float)i / TABLE_SIZE);
```

- Pré-calcula os valores da raiz quadrada inversa e armazena na memória.
- Muito rápido para consultas.
- Menos preciso.

---

### 2. Otimização do Quake III

```c
i = 0x5f3759df - (i >> 1);
y = y * (threehalfs - (x2 * y * y));
```

- Famosa técnica do Quake III Arena.
- Faz uma aproximação rápida com refinamento por Newton-Raphson.
- Excelente desempenho com boa precisão.

---

### 3. Instruções SSE

```c
__m128 val = _mm_set_ss(x);
__m128 result = _mm_rsqrt_ss(val);
```

- Utiliza instruções específicas de hardware.
- Rápido e razoavelmente preciso.
- Requer suporte ao SSE.

---

### 4. Método Padrão

```c
1.0f / sqrt(sum);
```

- Utiliza a função padrão `sqrt` da biblioteca `math.h`.
- Mais preciso, mas mais lento comparado aos outros.

---

## 📥 Leitura do Arquivo CSV

- Cada linha representa um vetor de características.
- O programa detecta automaticamente o número de dimensões e elementos.

```c
float **read_csv(const char *filename, int *num_elements, int *num_dimensions);
```

---

## 🧪 Normalização dos Vetores

```c
void normalize_feature_vector(float *features, int length, int method);
```

1. Calcula a soma dos quadrados dos elementos.
2. Aplica um dos métodos para obter a raiz quadrada inversa.
3. Multiplica cada elemento pela inversa da norma.

---

## ⏱️ Medição de Desempenho

```c
getrusage(RUSAGE_SELF, &usage);
```

Mede:
- Tempo de CPU em modo usuário
- Tempo de CPU em modo sistema
- Memória máxima alocada (resident set size)

---

## 📊 Exportação dos Resultados

Resultados são gravados no arquivo `benchmark_results.csv` com as seguintes colunas:

```
Método,Tempo_Usuário,Tempo_Sistema,Memória_Max_KB
```

---

## ▶️ Como Executar

```bash
./programa <método> <arquivo_csv>
```

### Onde:
- `<método>`:
  - `1` = Lookup Table
  - `2` = Quake III
  - `3` = SSE
  - Qualquer outro número = Método padrão
- `<arquivo_csv>`: Caminho para o arquivo com os dados de entrada.

### Exemplo:

```bash
./normalize 2 dados.csv
```

---

## 🧹 Liberação de Memória

No final do programa, toda a memória alocada dinamicamente é liberada com `free`.

---

## 📌 Requisitos

- Compilador compatível com SSE (como `gcc`).
- Sistema Unix-like (por usar `getrusage`).
- Arquivo CSV válido com apenas números separados por vírgula.

---

## ✍️ Desenvolvido para fins acadêmicos de benchmark e otimização numérica.
```
!
