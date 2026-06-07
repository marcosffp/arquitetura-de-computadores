<img width="1600" style="height:auto; border-radius: 12px;" alt="banner" src="images/banner.png" />

# Arquitetura de Computadores — Benchmark de Normalização Vetorial

> Trabalho prático da disciplina de Arquitetura de Computadores: um benchmark em C que compara quatro estratégias de cálculo da raiz quadrada inversa — Lookup Table, aproximação de Quake III, instruções SSE e a função `sqrt` padrão — medindo o impacto de cada uma no tempo de CPU e no consumo de memória durante a normalização de vetores de características.

---

## 🛠️ Stack Principal

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/GCC--O2-A42E2B?style=for-the-badge&logo=gnu&logoColor=white)
![SSE](https://img.shields.io/badge/SIMD-SSE-FF6F00?style=for-the-badge&logo=intel&logoColor=white)
![Bash](https://img.shields.io/badge/Bash-4EAA25?style=for-the-badge&logo=gnubash&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Python](https://img.shields.io/badge/Python-3.x-3776AB?style=for-the-badge&logo=python&logoColor=white)
![Jupyter](https://img.shields.io/badge/Jupyter-Notebook-F37626?style=for-the-badge&logo=jupyter&logoColor=white)
![Pandas](https://img.shields.io/badge/Pandas-150458?style=for-the-badge&logo=pandas&logoColor=white)
![NumPy](https://img.shields.io/badge/NumPy-013243?style=for-the-badge&logo=numpy&logoColor=white)
![Matplotlib](https://img.shields.io/badge/Matplotlib-Plots-3776AB?style=for-the-badge&logo=matplotlib&logoColor=white)

---

## 📑 Sumário

- [Sobre o projeto](#-sobre-o-projeto)
- [Pipeline do benchmark](#-pipeline-do-benchmark)
- [Métodos de cálculo da raiz quadrada inversa](#-métodos-de-cálculo-da-raiz-quadrada-inversa)
- [Estrutura de pastas](#-estrutura-de-pastas)
- [Conjunto de dados](#-conjunto-de-dados)
- [Medição de desempenho](#-medição-de-desempenho)
- [Como executar](#-como-executar)
- [Resultados e relatório técnico](#-resultados-e-relatório-técnico)
- [Requisitos](#-requisitos)

---

## 📖 Sobre o projeto

Este repositório reúne os trabalhos práticos da disciplina de **Arquitetura de Computadores**. O **TP1** investiga, na prática, como decisões de baixo nível — pré-computação, aproximações numéricas e instruções vetoriais de hardware — afetam o desempenho de uma operação simples, porém recorrente em aplicações de aprendizado de máquina e processamento de sinais: a **normalização de vetores via raiz quadrada inversa** (`1/√x`).

O programa em C (`benchmark.c`) lê vetores de características de um arquivo CSV, calcula a norma de cada vetor (soma dos quadrados dos elementos) e o normaliza usando uma de quatro estratégias selecionáveis por linha de comando. A cada execução, `getrusage` (`sys/resource.h`) mede tempo de CPU em modo usuário, tempo em modo sistema e pico de memória residente — métricas persistidas em `benchmark_results.csv` para análise posterior em `benchmark_plots.ipynb`. Ao final, toda a memória alocada dinamicamente para os vetores é liberada com `free`.

---

## 🔄 Pipeline do benchmark

```
┌──────────────────┐    ┌─────────────────────┐    ┌───────────────────────┐
│   data/*.csv     │ →  │   read_csv()        │ →  │ normalize_feature_    │
│ vetores de       │    │ detecta automatica- │    │ vector()              │
│ características  │    │ mente N elementos   │    │ soma dos quadrados →  │
│                  │    │ × M dimensões       │    │ raiz inversa (método) │
└──────────────────┘    └─────────────────────┘    └──────────┬────────────┘
                                                               │
                       ┌──────────────────────┐    ┌──────────▼────────────┐
                       │ benchmark_results.csv│ ←  │  getrusage()          │
                       │ método, tempo user,  │    │  amostra start/end →  │
                       │ tempo sys, RSS (KB)  │    │  user · sys · RSS     │
                       └──────────┬───────────┘    └───────────────────────┘
                                  │
                       ┌──────────▼───────────┐
                       │ benchmark_plots.ipynb│
                       │ pandas + matplotlib  │
                       │ → gráficos comparativos
                       └──────────────────────┘
```

---

## ⚙️ Métodos de cálculo da raiz quadrada inversa

Cada vetor é normalizado dividindo seus elementos pela raiz quadrada da soma dos quadrados (a norma). O cerne do benchmark está em **como** essa raiz quadrada inversa é calculada — cada abordagem troca precisão por velocidade de uma forma diferente.

---

#### 🔢 Método 1 — Lookup Table

> *"Pré-calcule uma vez, consulte muitas vezes."*

Constrói, na inicialização, uma tabela com `TABLE_SIZE = 100000` valores de `1/√x` pré-computados para `x` no intervalo `[0, 1]`. Em tempo de execução, a entrada é limitada (`clamp`) a esse intervalo e mapeada para um índice da tabela — trocando o cálculo por uma simples indexação de array.

```c
lookup_table[i] = 1.0f / sqrtf((float)i / TABLE_SIZE);
// ...
int index = (int)(x * (TABLE_SIZE - 1));
return lookup_table[index];
```

**Trade-off:** consulta em tempo constante, sem nenhuma chamada a `sqrt` em tempo de execução — mas a precisão fica limitada pela granularidade da tabela e pelo recorte do domínio de entrada em `[0, 1]`.

---

#### 🌀 Método 2 — Quake III

> *"A constante mágica `0x5f3759df` que entrou para o folclore da computação."*

Reaproveita a famosa aproximação bit a bit do código-fonte do *Quake III Arena*: reinterpreta os bits IEEE-754 do `float` como inteiro, subtrai de uma constante mágica para obter uma estimativa inicial de `1/√x`, e refina o resultado com **uma iteração do método de Newton-Raphson**.

```c
i = *(int *)&y;            // reinterpreta os bits do float como int
i = 0x5f3759df - (i >> 1); // estimativa inicial "mágica"
y = *(float *)&i;
y = y * (threehalfs - (x2 * y * y)); // refinamento de Newton-Raphson
```

**Trade-off:** ótimo equilíbrio entre velocidade e precisão sem depender de instruções específicas de hardware — por isso se tornou um clássico da otimização numérica.

---

#### 🧮 Método 3 — Instruções SSE

> *"Deixe a própria CPU calcular a aproximação."*

Delega o cálculo à instrução vetorial `rsqrtss` da extensão **SSE (Streaming SIMD Extensions)**, acessada via *intrinsics* (`xmmintrin.h`). O valor escalar é movido para um registrador de 128 bits, a aproximação por hardware é executada em uma única instrução, e o resultado é convertido de volta para `float`.

```c
__m128 val = _mm_set_ss(x);        // move x para um registrador SSE
__m128 result = _mm_rsqrt_ss(val); // executa a instrução rsqrtss
return _mm_cvtss_f32(result);      // converte de volta para float escalar
```

**Trade-off:** desempenho próximo do limite do hardware com boa precisão — mas exige uma CPU com suporte a SSE e a flag de compilação `-msse`.

---

#### 📐 Método padrão — `sqrt`

> *"O baseline: correção antes de otimização."*

Qualquer valor de método diferente de `1`, `2` ou `3` cai no caminho padrão, que usa diretamente `sqrt` da `math.h` — a implementação mais direta e precisa, usada como referência de comparação para as demais estratégias.

```c
inv_sqrt = 1.0f / sqrt(sum);
```

**Trade-off:** a maior precisão entre as quatro abordagens, ao custo do maior tempo de CPU — diferença que se acentua em datasets grandes.

---

## 📁 Estrutura de pastas

```
arquitetura-de-computadores/
├── images/
│   ├── banner.png
│   └── banner-institucional.svg
├── TP1/
│   ├── benchmark.c              # Código-fonte: leitura de CSV, normalização e medição
│   ├── benchmark_run.sh         # Roda 4 métodos × 5 datasets × 10 repetições
│   ├── benchmark_results.csv    # Saída acumulada: método, tempo user, tempo sys, RSS (KB)
│   ├── benchmark_plots.ipynb    # Notebook Jupyter — geração dos gráficos comparativos
│   ├── benchmark                # Binário compilado (gcc -O2 -msse)
│   ├── data/                    # Datasets de entrada (CSV)
│   │   ├── large_dataset_50000.csv … large_dataset_250000.csv
│   │   └── test_data_*.csv      # Conjuntos pequenos para testes/depuração rápida
│   ├── docs/
│   │   └── relatório.pdf        # Relatório técnico com a análise dos resultados
│   └── readme.md                # Documentação específica do TP1
└── README.md
```

---

## 📊 Conjunto de dados

Os arquivos em `TP1/data/` contêm vetores de características com **10 dimensões por linha**, em tamanhos crescentes para observar como cada método escala:

| Arquivo | Vetores (linhas) | Uso |
|---|---|---|
| `large_dataset_50000.csv` | 50 000 | Carga base |
| `large_dataset_100000.csv` | 100 000 | Carga intermediária |
| `large_dataset_150000.csv` | 150 000 | Carga intermediária |
| `large_dataset_200000.csv` | 200 000 | Carga alta |
| `large_dataset_250000.csv` | 250 000 | Carga máxima |
| `test_data_1.csv` … `test_data_10000.csv` | 1 – 10 000 | Conjuntos pequenos para testes/depuração rápida |

> `read_csv` detecta automaticamente o número de elementos (linhas) e dimensões (colunas) — não é preciso informar o formato manualmente.

---

## ⏱️ Medição de desempenho

Cada execução chama `getrusage(RUSAGE_SELF, &usage)` (de `sys/resource.h`) imediatamente antes e depois do laço de normalização, isolando o custo do método avaliado do custo de I/O (leitura do CSV e impressão dos resultados).

| Métrica | Campo de `rusage` | Descrição |
|---|---|---|
| Tempo de usuário | `ru_utime` | Tempo de CPU gasto executando instruções do programa |
| Tempo de sistema | `ru_stime` | Tempo de CPU gasto em chamadas de sistema |
| Memória residente máxima | `ru_maxrss` | Pico de memória física ocupada (RSS, em KB) |

Os resultados são apensados a `benchmark_results.csv` no formato:

```
Método,Tempo_Usuário,Tempo_Sistema,Memória_Max_KB
```

`benchmark_run.sh` automatiza a coleta: roda as **4 estratégias** sobre os **5 datasets grandes**, repetindo cada combinação **10 vezes** — gerando uma amostra estatisticamente robusta para o notebook de análise.

---

## ▶️ Como executar

### Pré-requisitos

- Compilador compatível com SSE (ex.: `gcc`)
- Sistema Unix-like (uso de `getrusage` / `sys/resource.h`)
- Python 3.x + Jupyter Notebook (apenas para gerar os gráficos)

### Compilar

```bash
cd TP1
gcc -o benchmark benchmark.c -lm -msse -O2
```

### Executar uma única combinação

```bash
./benchmark <método> <arquivo_csv>
# <método>: 1 = Lookup Table · 2 = Quake III · 3 = SSE · qualquer outro = Padrão (sqrt)

# Exemplo
./benchmark 2 data/large_dataset_50000.csv
```

### Executar a bateria completa de testes

```bash
chmod +x benchmark_run.sh
./benchmark_run.sh
# 4 métodos × 5 datasets × 10 repetições → benchmark_results.csv
```

### Gerar os gráficos comparativos

```bash
pip install matplotlib numpy pandas
jupyter notebook benchmark_plots.ipynb
```

---

## 📈 Resultados e relatório técnico

A análise completa — incluindo gráficos de tempo de usuário, tempo de sistema e memória por método e tamanho de dataset — está documentada em [`TP1/docs/relatório.pdf`](TP1/docs/relatório.pdf).

**Principais conclusões:**

| Método | Observação |
|---|---|
| Lookup Table | O mais rápido em datasets grandes, à custa de maior consumo de memória (tabela pré-computada) |
| Quake III | Equilíbrio sólido entre velocidade e precisão — desempenho consistente em qualquer porte de dataset |
| SSE | Aproveita paralelismo de hardware; eficiência depende do suporte da CPU |
| Padrão (`sqrt`) | Implementação mais simples e precisa, mas perde desempenho de forma perceptível em datasets grandes |

---

## 📌 Requisitos

- Compilador compatível com SSE (como `gcc`)
- Sistema Unix-like (por usar `getrusage`)
- Arquivo CSV válido contendo apenas números separados por vírgula
- Python 3.x, Jupyter Notebook e as bibliotecas `matplotlib`, `numpy`, `pandas` (apenas para gerar os gráficos)

---

<div align="center">
  <img width="70%" alt="pucminas" src="images/banner-institucional.svg"/>
</div>
<p align="center">Fonte do banner: <a href="https://github.com/joaopauloaramuni">João Paulo Carneiro Aramuni</a></p>

