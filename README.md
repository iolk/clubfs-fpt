### Compiling

```bash
g++ -std=c++17 -Wall -pedantic -O3 main.cpp -o main
```
or 
`CTRL+SHIFT+B` on VS Code on Windows (check mingw path)

### Generating

```bash
generator.py [OPTIONS] file_name
```
see `generator.py -h` for all options

### Run

```bash
main testcases/gen_FILE_NAME.in
plot_result testcases/gen_FILE_NAME.in
plot.gexf
```