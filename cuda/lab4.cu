/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: November 9th 2023
Description:
Evaluating speed of different memory management methods in CUDA
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <ctime>
#include <curand_kernel.h>

/*
* function that makes walker take steps
* @Param results: result of distance the walker has travel
* @Param steps: number of steps the walker has to take
* @Param seed: seed for random number generator
*/
__global__ void takeSteps(float* results, int steps, unsigned int seed) 
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    curandState state;
    curand_init(seed, tid, 0, &state);
    float x;
    float y;
    for (int i = 0; i < steps; ++i) {
        float dir = curand_uniform(&state);
        if (dir < 0.25) {
            ++x;
        } else if (dir <  0.5) {
        	--x;
        } else if (dir <  0.75) {
            ++y;
        } else {
            --y;
        }   
    }
    results[tid] = (x*x)+(y*y);
}

/*
* function that makes walker take steps using normal memory allocation
* @Param numWalkers: number of walkers
* @Param steps: number of steps the walker has to take
* @Param seed: seed for random number generator
* @Param threadsPerBlock: threads Per Block
* @Param blocksPerGrid: blocks Per Grid
* @Param print: if code should print the avg dist and time
*/
void cudaMallocNormal(int steps, int numWalkers, int threadsPerBlock, int blocksPerGrid, bool print) {
    float* d_results;
    float* h_results;
    int bytes = sizeof(float) * numWalkers;

    //timer starts
    auto start = std::chrono::high_resolution_clock::now();

    h_results = (float*)malloc(bytes);
    cudaMalloc((void**)&d_results, bytes);
    cudaMemcpy(d_results, h_results, bytes, cudaMemcpyHostToDevice);
    takeSteps<<<blocksPerGrid, threadsPerBlock>>>(d_results, steps, time(NULL));
    cudaMemcpy(h_results, d_results, bytes, cudaMemcpyDeviceToHost);    

    // calc avg distance
    float sum = 0;
    for (int i=0; i < numWalkers; ++i) {
        sum += sqrt(h_results[i]);
    }

    cudaFree(d_results);
    free(h_results);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;
    // prints
    if(print){
        std::cout << "\tTime to calculate(microsec): " << duration.count()*1000000 <<std::endl;
        std::cout << "\tAverage distance from origin: " << sum/numWalkers << std::endl;
    }
}

/*
* function that makes walker take steps using pinned memory allocation
* @Param numWalkers: number of walkers
* @Param steps: number of steps the walker has to take
* @Param seed: seed for random number generator
* @Param threadsPerBlock: threads Per Block
* @Param blocksPerGrid: blocks Per Grid
* @Param print: if code should print the avg dist and time
*/
void cudaMallocHost(int steps, int numWalkers, int threadsPerBlock, int blocksPerGrid, bool print) {
    float* d_results;
    float* h_results;
    int bytes = sizeof(float) * numWalkers;

    //timer starts
    auto start = std::chrono::high_resolution_clock::now();

    cudaMallocHost((void**)&h_results, bytes);
    cudaMalloc((void**)&d_results, bytes);
    //cudaMemcpy(d_results, h_results, bytes, cudaMemcpyHostToDevice);
    takeSteps<<<blocksPerGrid, threadsPerBlock>>>(d_results, steps, time(NULL));
    cudaMemcpy(h_results, d_results, bytes, cudaMemcpyDeviceToHost);  

    // calc avg distance
    float sum = 0;
    for (int i=0; i < numWalkers; ++i) {
        sum += sqrt(h_results[i]);
    }

    // free memory
    cudaFree(d_results);
    cudaFreeHost(h_results);


     // timer stops
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;

    // prints
   if(print){
        std::cout << "\tTime to calculate(microsec): " << duration.count()*1000000 <<std::endl;
        std::cout << "\tAverage distance from origin: " << sum/numWalkers << std::endl;
    }
}

/*
* function that makes walker take steps using managed memory allocation
* @Param numWalkers: number of walkers
* @Param steps: number of steps the walker has to take
* @Param seed: seed for random number generator
* @Param threadsPerBlock: threads Per Block
* @Param blocksPerGrid: blocks Per Grid
* @Param print: if code should print the avg dist and time
*/
void cudaMallocManaged(int steps, int numWalkers, int threadsPerBlock, int blocksPerGrid, bool print) {
    float* m_results;
    int bytes = sizeof(float) * numWalkers;

    //timer starts
    auto start = std::chrono::high_resolution_clock::now();
    cudaMallocManaged(&m_results, bytes);
    takeSteps<<<blocksPerGrid, threadsPerBlock>>>(m_results, steps, time(NULL));
    cudaDeviceSynchronize();;    

    // calc avg distance
    float sum = 0;
    for (int i=0; i < numWalkers; ++i) {
        sum += sqrt(m_results[i]);
    }

    cudaFree(m_results);

    // timer stops
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;
    // prints
    if(print){
        std::cout << "\tTime to calculate(microsec): " << duration.count()*1000000 <<std::endl;
        std::cout << "\tAverage distance from origin: " << sum/numWalkers << std::endl;
    }
    
}

// main program that executes and calls other functions
int main(int argc, char **argv) {
    int steps = 10000; //default
    int numWalkers = 1000;
    
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-I" && i + 1 < argc) {
            steps = std::atoi(argv[i + 1]);
            ++i;
        } else if (std::string(argv[i]) == "-W" && i + 1 < argc) {
            numWalkers = std::atoi(argv[i + 1]);
            ++i;
        } else {
            std::cerr << "Usage: " << argv[0] << " -I <integer> -W <integer>" << std::endl;
            return 1;
        }
    }
    std::cout << "\nLab 4 -W " << numWalkers << " -I " << steps << std::endl;
	int threadsPerBlock = 256;
	int blocksPerGrid = (numWalkers + threadsPerBlock) / threadsPerBlock;
    std::cout << "Normal CUDA memory Allocation: " <<std::endl;
    cudaMallocNormal(steps, numWalkers, threadsPerBlock, blocksPerGrid, false); // warm up
    cudaMallocNormal(steps, numWalkers, threadsPerBlock, blocksPerGrid, true);
    std::cout << "Pinned CUDA memory Allocation: " <<std::endl;
    cudaMallocHost(steps, numWalkers, threadsPerBlock, blocksPerGrid, false); // warm up
    cudaMallocHost(steps, numWalkers, threadsPerBlock, blocksPerGrid, true);
    std::cout << "Managed CUDA memory Allocation: " <<std::endl;
    cudaMallocManaged(steps, numWalkers, threadsPerBlock, blocksPerGrid, false); // warm up
    cudaMallocManaged(steps, numWalkers, threadsPerBlock, blocksPerGrid, true);
    return 0;
}