# Evaluation of Performance Impact on `normalize_feature_vector` method 🚀

This repository contains the code, scripts, and data used to benchmark different square root inversion normalization methods. The methods compared are:

- **Lookup Table**: Precomputed values to optimize repeated function calls.
- **Quake III**: The famous Quake algorithm, known for balancing speed and precision.
- **SSE (Streaming SIMD Extensions)**: Hardware-level parallelism for improved performance.
- **Default**: Standard method without specific optimizations.

## Objective

The aim is to compare these methods based on three key metrics:

- **User time**
- **System time**
- **Memory usage** (maximum resident set size)

Each method was tested using datasets ranging from 50,000 to 250,000 lines, with multiple trials conducted for each dataset size.

## Project Structure

- **`data/`**: CSVs files for tests.
- **`benchmark.c`**: The C source code that runs the benchmarking tests.
- **`benchmark_run.sh`**: Shell script to automate multiple benchmarking runs for each method and dataset size.
- **`benchmark_plots.ipynb`**: A Jupyter Notebook to process the results from CSV files and generate performance graphs.
- **`README.md`**: This file.

**Note**: Files like `test_data_*.csv`, which were originally part of the dataset used in this project, were excluded from the final analysis in the report. These files have been discarded from the repository and are not needed for the conclusions presented.

## Usage

### Prerequisites

To run the benchmarks and plot results, ensure you have the following installed:

- **Python 3.x**
- **GCC** for compiling C code
- **Jupyter Notebook** for generating plots
- Python libraries: `matplotlib`, `numpy`, `pandas`

You can install the required libraries by running:

```bash
pip install matplotlib numpy pandas
```

### Running the Benchmarks

1. **Compile the benchmarking code**:

   ```bash
   gcc -o benchmark benchmark.c -O2
   ```

2. **Run the benchmark for each method and dataset**:

   ```bash
   ./benchmark <method> <input_csv>
   ```

   or use the provided shell script to automate the execution for multiple dataset sizes:

   ```bash
   chmod +x benchmark_run.sh
   ./benchmark_run.sh
   ```

### Plotting Results

Once the results are generated and saved in CSV files, you can visualize them using the `Jupyter Notebook`:

1. Open the notebook:

   ```bash
   jupyter notebook benchmark_plots.ipynb
   ```

2. Run the notebook to generate graphs that illustrate the performance of each method.

## Results Overview

The report (included in the repository as `technical_report.pdf`) provides detailed performance comparisons across the following metrics:

- **User Time**: How much time the CPU spent executing user instructions for each method.
- **System Time**: The time spent in system-level operations.
- **Memory Usage**: The amount of memory used during the execution of each method.

### Key Findings

- **Lookup Table**: Fastest for large datasets but consumes more memory.
- **Quake III**: A balanced approach that works well for moderate-sized datasets.
- **SSE**: Performs well with parallelism, but its efficiency depends on hardware.
- **Default**: Easy to implement but becomes less efficient with large datasets.
