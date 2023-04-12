# P1

Compilamos el programa con:  
```
mpicc main.c -lm
```
>_compilar el programa acepta tambien la opción -o_
  
Y se ejecuta con:  
```
mpirun -np 2 ./a.out
```  
> _-np es el numero de procesos que vamos a crear_