### Compiling

```bash
g++ -std=c++17 -Wall -pedantic -O3 main.cpp -o main
```
or 
`CTRL+SHIFT+B` on VS Code on Windows (check mingw path)

### Generating

```bash
python3 generator.py [OPTIONS] file_name
```
see `generator.py -h` for all options. Example:
```bash
python3 generator.py -gen lfr-benchmark -n 1500 -t1 2.6 -t2 1.4 -mu 0.1 -mind 5 -minc 106 -maxd 300 -maxc 300 lfr_example
```

### Run
Please make sure that python3 is in the Path

```bash
main INPUT_FILE OUTPUT_FILE
python3 plot_result.py INPUT_FILE OUTPUT_FILE PLOT_FILE.gexf
plot.gexf
```

### To run all testcases in tests/testcases
Please first set the number of thread in resolver.py and than

```bash
python3 resolver.py
```