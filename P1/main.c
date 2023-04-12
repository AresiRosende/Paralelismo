#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void inicializaCadena(char *cadena, int n){
  int i;
  for(i=0; i<n/2; i++){
    cadena[i] = 'A';
  }
  for(i=n/2; i<3*n/4; i++){
    cadena[i] = 'C';
  }
  for(i=3*n/4; i<9*n/10; i++){
    cadena[i] = 'G';
  }
  for(i=9*n/10; i<n; i++){
    cadena[i] = 'T';
  }
}

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
    exit(1); 
  }
    
  int countrec, numprocs, rank, i, n, count=0;
  char *cadena;
  char L;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  n = atoi(argv[1]);
  L = *argv[2];
  
  cadena = (char *) malloc(n*sizeof(char));
  inicializaCadena(cadena, n);


  if (rank > 0) {
    for (i = rank-1; i <= n; i += numprocs-1){
      if(cadena[i] == L){
        count++;
      }
    }
    MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
  else {
    for (int k = 1; k < numprocs; k++){
      MPI_Recv(&countrec, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      count += countrec;
    }
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    free(cadena);
  }

  MPI_Finalize();
}
