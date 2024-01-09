/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 12/3/2023
Description:
Using monte carlo simulation to estimate an integral based on user input.
Use MPI to run code on different processors with different memories.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include "mpi.h"

using namespace std;

int main(int argc, char**argv) {

	int numtasks, rank, sendcount, recvcount, source;
	int sendbuf, recvbuf;

	if (argc != 5) {
    	fprintf(stderr, "not enough args provided\n");
    	exit(1);
    }

	MPI_Init(&argc,&argv);

	

	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
    int integralCase = 1;
    int number = 1000000;
    if(std::string(argv[1]) == "-P" && std::string(argv[3]) == "-N") {
    	integralCase = std::atoi(argv[2]);
    	number = std::atoi(argv[4]);
    } else if (std::string(argv[1]) == "-N" && std::string(argv[3]) == "-P") {
    	integralCase = std::atoi(argv[4]);
    	number = std::atoi(argv[2]);
    }

    std::mt19937 gen(rank);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

	// Sum the numbers locally
	float local_sum = 0;
	int i;
	int iterations = number/numtasks;
	if (integralCase == 1) {
		for (i = 0; i < iterations; i++) {
			double random_number = dis(gen);
	    	local_sum += random_number*random_number;
		}
	} else {
		for (i = 0; i < iterations; i++) {
			double random_number = dis(gen);
	    	local_sum += exp(-1*random_number*random_number);
		}
	}

	// Reduce all of the local sums into the global sum
	float global_sum;
	MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0,
             MPI_COMM_WORLD);

	// Print the result
	if (rank == 0) {
	    printf("The estimate for integral %d is %f\n", integralCase,
	           global_sum / number);
	    std:cout <<"Bye!" << std::endl;
    }


	MPI_Finalize();
	return 0;
}