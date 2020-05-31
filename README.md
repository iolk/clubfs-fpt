### Compiling

```bash
g++ -std=c++17 -Wall -pedantic -O3 main.cpp -o main
```

### Run

```bash
./main testcases/16x4.in &>debug.out
```

### Generating

```bash
cd generator && ./gen.sh NUMBER_OF_NODES && cd ..
```
