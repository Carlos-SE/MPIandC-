# FILÓSOFOS

## PRÁCTICA 3 SCD
Consiste en un programa con 5 filósofos y 5 tenedores. Cada filósofo pide el de su izq. y der. y comparten tenedores de a uno con el filósofo que tengan al lado.

### COMPILE
```
mpicxx -std=c++11 file.cpp
```
### RUN
```
mpirun -np 10 file
```
->if previous doesn't work try next:
```
mpirun --oversubscribe -np 10 file
```
