#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2){

  if((base1 == 4) || (base2 == 4)){
    return 3;
  }

  if(base1 == base2) {
    return 0;
  }

  if((base1 == 0) && (base2 == 3)) {
    return 1;
  }

  if((base2 == 0) && (base1 == 3)) {
    return 1;
  }

  if((base1 == 1) && (base2 == 2)) {
    return 1;
  }

  if((base2 == 2) && (base1 == 1)) {
    return 1;
  }

  return 2;
}

int main(int argc, char *argv[] ) {

  int i, j, numprocs, rank, mpp;  //mpp = sequences per process
  int *data1, *data2, *subdata1, *subdata2;
  int *result, *subresult;
  struct timeval  tPreSc, tPosSc, tPosComp, tPosGath;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  mpp = M/numprocs;
  //if M is not a multiple of numprocs, we add 1, otherwise
  //we would lose the last M % numprocs sequences
  if(M % numprocs) mpp +=1;


  if (!rank){
    //only process 0 allocs memory and initializes the matrixes

    data1 = (int *) malloc(M*N*sizeof(int));
    data2 = (int *) malloc(M*N*sizeof(int));
    result = (int *) malloc(M*sizeof(int));

    /* Initialize Matrices */
    for(i=0;i<M;i++) {
      for(j=0;j<N;j++) {
        /* random with 20% gap proportion */
        data1[i*N+j] = fast_rand();
        data2[i*N+j] = fast_rand();
      }
    }
  }

  //each process initializes their own submatrixes
  //to store the part of the data they have to process
  subdata1 = (int *) malloc(mpp*N*sizeof(int));
  subdata2 = (int *) malloc(mpp*N*sizeof(int));
  subresult = (int *) malloc(mpp * sizeof(int));

  //start time for communication
  gettimeofday(&tPreSc, NULL);

  //sends from process 0 to every process mpp*N elements of data to subdata
  MPI_Scatter(data1, mpp * N, MPI_INT, subdata1, mpp * N, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Scatter(data2, mpp * N, MPI_INT, subdata2, mpp * N, MPI_INT, 0, MPI_COMM_WORLD);

  //end time for communication and start time for computation
  gettimeofday(&tPosSc, NULL);

  int mpplocal = mpp;
  //if M % numprocs != 0, then the last process will have
  //less than mpp sequences to process
  if(rank == numprocs-1) mpp = M - mpp*(numprocs-1);


  //calculates the base distance of each subdata from each process
  for(i=0;i<mpplocal;i++) {
    subresult[i]=0;
    for(j=0;j<N;j++) {
      subresult[i] += base_distance(subdata1[i*N+j], subdata2[i*N+j]);
    }
  }

  //end time for computation and start time for communication
  gettimeofday(&tPosComp, NULL);
    
  //retrieves the data of subresult from every process into result
  MPI_Gather(subresult, mpp, MPI_INT, result, mpp, MPI_INT, 0, MPI_COMM_WORLD);   //result
  
  //end time for communication
  gettimeofday(&tPosGath, NULL);


  //(tPostSc - tPreSc) + (tPosGath - tPosComp) 
  int tComm = (tPosSc.tv_usec + tPosGath.tv_usec - (tPreSc.tv_usec + tPosComp.tv_usec))+ 1000000 * (tPosSc.tv_sec + tPosGath.tv_sec - (tPreSc.tv_sec + tPosComp.tv_sec));
  //tPosComp - tPosSc
  int tComp = (tPosComp.tv_usec - tPosSc.tv_usec)+ 1000000 * (tPosComp.tv_sec - tPosSc.tv_sec);


  //debugging options
  if(!rank && DEBUG == 1){       
    /* Display result */
    int checksum = 0;
    for(i=0;i<M;i++) {
      checksum += result[i];
    }
    printf("Checksum: %d\n", checksum);
  } else if (!rank && DEBUG == 2){
    for(i=0;i<M;i++) {
      printf(" %d \t ",result[i]);
    }
  }

  //process 0 prints the results
  if(!rank){
    printf("Process 0 -> Communication time: %lfs | Computation time: %lfs\n", (double) tComm/1E6, (double) tComp/1E6);
    for(int k = 1; k < numprocs; k++){
      //receives the times from each process
      MPI_Recv(&tComm, 1, MPI_INT, k, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&tComp, 1, MPI_INT, k, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Process %d -> Communication time: %lfs | Computation time: %lfs\n", k, (double) tComm/1E6, (double) tComp/1E6);
    }
  } else {
    //if the process is not 0, then sends its time to process 0
    MPI_Send(&tComm, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&tComp, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);  
  }

  //since process 0 created these variables, only it has to free them
  if(!rank){
    free(data1);
    free(data2);
    free(result);
  }

  //every process has to free their own local variables
  free(subdata1);
  free(subdata2);
  free(subresult);
  
  MPI_Finalize();

  return 0;
}