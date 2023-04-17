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

int MPI_FlattreeBCast(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op, int root, MPI_Comm comm){
  int result, numprocs, rank;

    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);

    if (rank == root) {
    for (int i = 0; i < numprocs; i++)
            if (i != root) {
                result = MPI_Send(buffer, count, datatype, i, 0, comm);
                if (error != MPI_SUCCESS) return error;
            }
    } else {
        error = MPI_Recv(buffer, count, datatype, root, 0, comm, &status);
        if (error != MPI_SUCCESS) return error;
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
  int countrec, numprocs, rank, i, n, count=0;
  char *cadena;
  char L;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  while (1) {
    if (rank == 0){
        printf("Enter string length (0 to exit):\n");
        scanf("%d", &n);
        printf("Enter character to count (A, C, G o T):\n");
        scanf("%c", &L);
    }

    if (n == 0) break;

    MPI_BinomialBCast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_BinomialBCast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&count, 1, MPI_INT, rank, MPI_COMM_WORLD);

    count = 0;
    counrec = 0;

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);


    if (rank > 0) {
     for (i = rank-1; i <= n; i += numprocs-1){ 
         if(cadena[i] == L){
         count++;
       }
     }
    }

    MPI_FlattreeBCast(&count, &countrec, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //MPI_Reduce(&count, &countrec, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) printf("El numero de apariciones de la letra %c es %d -> %d\n", L, count, rank);
    free(cadena);
    }


  MPI_Finalize();
}
