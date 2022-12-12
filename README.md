# tscp-solver

## Dependencies
The solver uses [libconfig](http://hyperrealm.github.io/libconfig/) library for parsing input files.

## Compilation
You can use `cmake` and the provided CMakeLists.txt:
```bash
mkdir build && cd build
cmake ..
make
```
The resulting binary will be in `tscp_solver` file.

## Running
Solver is run as:
`./tscp_solver [OPTION...] PROBLEM_FILE`

Default run without options runs the simulated annealing algorithm for 2 million steps with a feasible solution using only the second trainset as the initial solution.

### Options
- `-v`,   `--verbose`              Produce verbose output
- `-s`, `--steps=N`                Number of steps of the algorithm (default is 2 million)
- `-e`,   `--empty_init`           Use empty initial solution
- `-i`,   `--init_file=FILE`       Load the initial solution from csv file
- `-o`,   `--output_sol=FILE`      Write the solution to file as csv
- `-t`,   `--tabu`                 Use tabu search (default is simulated annealing)
- `-n`,   `--neigh=N`              Number of generated neighbors for tabu search
- `-u`,   `--tabu_len=N`           Tabu list length
- `-?`,   `--help`                 Give this help list
- `--usage`                        Give a short usage message

### Problem file format
Solver requires problem description in the libconfig format. Examples of such files can be found in the `data` directory.
