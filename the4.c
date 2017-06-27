#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

//You can compile the program with given command below //
//mpicc the4.c -o twin_primes -lm//
//and post a job with given command//
//sbatch slurmscript.h// 

int main(int argc, char *argv[]){
double start_time;
int *list1; //same first list to keep  numbers from 2 through sqrtN // same for all processes
int *list2; // different second list for each process to keep remaining numbers in a splitted way
unsigned long int s = 0; // split size
unsigned long int rem = 0; // remainder size
unsigned long int l = 0; // lowest bound of split
unsigned long int h = 0; // highest bound of split, it may be greater on last process coz of remainder
int r = 0; // current rank of process
int p = 0; // total number of processes


MPI_Init(&argc,&argv);
if(argc<=1) {
        printf("No arguments, no fun...");
        exit(1);
     }

unsigned long int n = strtoul(argv[1],NULL,0);  //primes till N
MPI_Comm_rank(MPI_COMM_WORLD, &r);
MPI_Comm_size(MPI_COMM_WORLD, &p);

if (r==0)
  start_time=MPI_Wtime();//start time for performance measurement

// some calculations //
unsigned long int sqrtN = (unsigned long int)(sqrt(n));
s = (n - (sqrtN+1)) / p;
rem = (n-(sqrtN+1)) % p;
l = sqrtN + r*s + 1;
h = l + s -1;


list1 = (int*)malloc((sqrtN+1) * sizeof(int));
list2 = (int*)malloc((h-l+1) * 2 *sizeof(int)); // get some more space since there will be future buffer overflows due to last process

//if it is last process, add remainder to it
if(r == p-1){
  h += rem;
}


unsigned long int c,m;
for(c=0; c<=sqrtN; c++){
   //set each number as unmarked
   list1[c] = 0;
}

//specific loop for each process
for(c=l; c<=h; c++){
   //set each number as unmarked 
   list2[c-l] = 0;
}

for(c=2; c<=sqrtN; c++){
  // in case number is unmarked
  if(list1[c] == 0){
    //run through numbers in list1 bigger than c
    for(m=c+1; m<=sqrtN; m++){
      //if m is a multiple of c
      if(m%c==0){
	list1[m] = 1;
      }
    }
    // run through numbers in list2 bigger than c
    for(m=l; m <=h; m++){
      if(m%c == 0){
	list2[m-l] = 1;
      }
    }
  }
}

//printers
if(r==0){
  unsigned long int prime_counter = 0;
  unsigned long int list3[n-1]; // list for holding all primes till N
  unsigned long int **final_list = malloc( (n/2) * sizeof(unsigned long int *));

  //run through list1
  for(c=2; c <= sqrtN; c++){
    if(list1[c] == 0){
      //print the unmarked number in list1 -> primes
      //printf("%lu \n",c);
      list3[prime_counter] = c;
      prime_counter++;
    }
  }
  for(c=l; c <= h; c++){
    if(list2[c-l] == 0){
      //print the unmarked number in list2 -> primes
      // printf("%lu \n",c);
      list3[prime_counter] = c;
      prime_counter++;
    }
  }

  //run through each other processes
  for(r=1; r <= p-1; r++){
    //calculate new boundaries
    l = sqrtN + r*s + 1;
    h = l+s-1;
    if (r==p-1){
      h += rem;
    }
    //receive the list2 from  particular process
    MPI_Recv(list2, h-l+1, MPI_INT, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for(c=l; c <= h; c++){
      if(list2[c-l] == 0){
	//print the prime in list2 coming from particular process
	//printf("%lu \n", c);
	list3[prime_counter] = c;
	prime_counter++;
      }
    }
  }
  // build prime_twins
  unsigned long int i,j=0;
  for(i=0; i < prime_counter - 1 ; i++){
    if( list3[i+1] - list3[i] == 2 ){
      final_list[j] = malloc(2 * sizeof(unsigned long int));
      final_list[j][0] = list3[i];
      final_list[j][1] = list3[i+1];
      j++;
    }
  }

  printf("Here comes twin primes!\n");
  for(i=0; i < j; i++){
    printf("%lu %lu \n",final_list[i][0],final_list[i][1]);
  }

  free(final_list);
  printf("Running time is: %f\n",(MPI_Wtime()-start_time));
  //printf("Total number of primes... %lu \n", prime_counter);
}
//if process is not the first one
else{
  MPI_Send(list2, h-l+1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

//deallocation
free(list1);
free(list2);

MPI_Finalize();

}
