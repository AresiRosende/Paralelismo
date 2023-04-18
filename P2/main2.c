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

int MPI_FlattreeBCast(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){
  int result, numprocs, rank;
  int *aux;

    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);

    if (rank != root) {
      for (int i = 1; i < numprocs; i++){
        result = MPI_Send(sendbuf, count, datatype, root, 0, comm);
        if (result != MPI_SUCCESS) return result;
      }
    } else {
      aux = ((int *) sendbuf); //add the value of process with rank 0
      for (int k = 1; k < numprocs; k++){
        result = MPI_Recv(recvbuf, count, datatype, k, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);
        if (result != MPI_SUCCESS) return result;
        *aux += *((int *) recvbuf);
      }
      recvbuf = aux;
    }
    return MPI_SUCCESS;
}


//WIP




int MPI_BinomialBCast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
  int numprocs, rank, result;

  MPI_Comm_size(comm, &numprocs);
  MPI_Comm_rank(comm, &rank);

  if (rank > 0)
    result = MPI_Recv(buffer, count, datatype, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
  for(int i = 1; ; i*=2){
    if(rank < i) {
      if (i+rank >= numprocs) break;
      result = MPI_Send(buffer, count, datatype, i+rank, 0, comm);
    }
  }
  return result;
}


//WIP



int main(int argc, char *argv[])
{   

  if(argc != 3){
    printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
    exit(1); 
  }

  int countrec, numprocs, rank, i, n, count=0;
  char *cadena;
  char L;


  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  n = atoi(argv[1]);
  L = *argv[2];

  //MPI_BinomialBCast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  //MPI_BinomialBCast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Bcast(&count, 1, MPI_INT, rank, MPI_COMM_WORLD);

  cadena = (char *) malloc(n*sizeof(char));
  inicializaCadena(cadena, n);


  for (i = rank; i <= n; i += numprocs){ 
    if(cadena[i] == L){
    count++;
   }
  }

  MPI_FlattreeBCast(&count, &countrec, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  //MPI_Reduce(&count, &countrec, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) printf("El numero de apariciones de la letra %c es %d\n", L, count);
  free(cadena);

  MPI_Finalize();
}
