# TSP_GA
Implementation of Genetic Algorithm(GA) for Travelling Salesman Problem (TSP).

GA parameters:
<br>Selection for crossover - binary tournament
<br>Crossover - Partially Mapped Crossover(PMX)
<br>Mutation - random swapping 2 vertexies
<br>Selection for survival - N best individuals from all parents and childs

Task samples from http://softlib.rice.edu/pub/tsplib/

# Build
<br>mkdir build/
<br>cd build
<br>cmake ../
<br>make

# Run
Single task:
<br>build/tsp <task_file> <initial_solution_file>

All tasks:
<br>./run_tasks.sh
