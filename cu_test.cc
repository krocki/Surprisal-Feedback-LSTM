/*

    cuda_matrix test

    Author: Kamil Rocki <kmrocki@us.ibm.com>

    Created on: 04/18/2016

*/

#include <iostream>
#include <cuda_matrix.h>

int main() {

	size_t N = 8;
	size_t M = 8;
	
	std::cout << "Setting up device..." << std::endl;
	cudaSetDevice ( 0 );
	std::cout << "Setting up curand..." << std::endl;
	init_curand();
	std::cout << "Setting up cublas..." << std::endl;
	init_cublas();
	
	cuda_matrix A ( N, M );
	cuda_matrix B ( N, M );
	cuda_matrix C ( N, M );
	
	randn ( A, 0, 0.1 );
	randn ( B, 1, 0.5 );
	
	std::cout << A << std::endl;
	std::cout << B << std::endl;
	std::cout << C << std::endl;
	
	std::cout << "************************" << std::endl;
	
	A.sync_device();
	B.sync_device();
	
	//add ( C, A, B );
	GEMM ( C, A, B );
	C.sync_host();
	
	std::cout << C << std::endl;
	
	teardown_cublas();
	std::cout << "Done." << std::endl;
	
	return 0;
	
}