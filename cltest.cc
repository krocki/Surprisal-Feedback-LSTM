// #include <sys/types.h>
// #include <stdio.h>
// #include <iostream>

// /* Include the clBLAS header. It includes the appropriate OpenCL headers */
// #include <clBLAS.h>
// #include <clUtils.h>

// /* This example uses predefined matrices and their characteristics for
//  * simplicity purpose.
// */

// #define M  4
// #define N  3
// #define K  5

// static const cl_float alpha = 10;

// static const cl_float A[] = {
// 	11, 12, 13, 14, 15,
// 	21, 22, 23, 24, 25,
// 	31, 32, 33, 34, 35,
// 	41, 42, 43, 44, 45,
// };
// static const size_t lda = K;        /* i.e. lda = K */

// static const cl_float B[] = {
// 	11, 12, 13,
// 	21, 22, 23,
// 	31, 32, 33,
// 	41, 42, 43,
// 	51, 52, 53,
// };
// static const size_t ldb = N;        /* i.e. ldb = N */

// static const cl_float beta = 20;

// static cl_float C[] = {
// 	11, 12, 13,
// 	21, 22, 23,
// 	31, 32, 33,
// 	41, 42, 43,
// };
// static const size_t ldc = N;        /* i.e. ldc = N */

// static cl_float result[M * N];

// int main ( void ) {

// 	cl_int err;
// 	cl_platform_id platform = 0;
// 	cl_device_id device = 0;
// 	cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
// 	cl_context ctx = 0;
// 	cl_command_queue queue = 0;
// 	cl_mem bufA, bufB, bufC;
// 	cl_event event = NULL;
// 	int ret = 0;

// 	/* Setup OpenCL environment. */
// 	CL_SAFE_CALL ( clGetPlatformIDs ( 1, &platform, NULL ) );


// 	CL_SAFE_CALL ( clGetDeviceIDs ( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL ) );


// 	props[1] = ( cl_context_properties ) platform;
// 	ctx = clCreateContext ( props, 1, &device, NULL, NULL, &err );

// 	if ( err != CL_SUCCESS ) {
// 		printf ( "clCreateContext() failed with %d\n", err );
// 		return 1;
// 	}

// 	queue = clCreateCommandQueue ( ctx, device, 0, &err );

// 	if ( err != CL_SUCCESS ) {
// 		printf ( "clCreateCommandQueue() failed with %d\n", err );
// 		clReleaseContext ( ctx );
// 		return 1;
// 	}

// 	/* Setup clblas. */
// 	CL_SAFE_CALL ( clblasSetup() );

// 	if ( err != CL_SUCCESS ) {
// 		printf ( "clblasSetup() failed with %d\n", err );
// 		clReleaseCommandQueue ( queue );
// 		clReleaseContext ( ctx );
// 		return 1;
// 	}

// 	/* Prepare OpenCL memory objects and place matrices inside them. */
// 	bufA = clCreateBuffer ( ctx, CL_MEM_READ_ONLY,
// 							M * K * sizeof ( cl_float ),
// 							NULL, &err );

// 	bufB = clCreateBuffer ( ctx, CL_MEM_READ_ONLY,
// 							K * N * sizeof ( cl_float ),
// 							NULL, &err );

// 	bufC = clCreateBuffer ( ctx, CL_MEM_READ_WRITE,
// 							M * N * sizeof ( cl_float ),
// 							NULL, &err );

// 	if ( err != CL_SUCCESS ) {
// 		printf ( "clCreateBuffer failed with %d\n", err );
// 		return 1;
// 	}

// 	CL_SAFE_CALL ( clEnqueueWriteBuffer ( queue, bufA, CL_TRUE, 0, ( M * K * sizeof ( cl_float ) ), A, 0, NULL, NULL ) );

// 	CL_SAFE_CALL ( clEnqueueWriteBuffer ( queue, bufB, CL_TRUE, 0, K * N * sizeof ( cl_float ), B, 0, NULL, NULL ) );

// 	CL_SAFE_CALL ( clEnqueueWriteBuffer ( queue, bufC, CL_TRUE, 0,
// 										  M * N * sizeof ( cl_float ), C, 0, NULL, NULL ) );

// 	/* Call clBLAS extended function. Perform gemm for the lower right sub-matrices */
// 	CL_SAFE_CALL ( clblasSgemm ( clblasRowMajor, clblasNoTrans,
// 								 clblasNoTrans,
// 								 M, N, K,
// 								 alpha, bufA, 0, lda,
// 								 bufB, 0, ldb, beta,
// 								 bufC, 0, ldc,
// 								 1, &queue, 0, NULL, &event ) );

// 	/* Wait for calculations to be finished. */
// 	CL_SAFE_CALL ( clWaitForEvents ( 1, &event ) );

// 	/* Fetch results of calculations from GPU memory. */
// 	CL_SAFE_CALL ( clEnqueueReadBuffer ( queue, bufC, CL_TRUE, 0,
// 										 M * N * sizeof ( *result ),
// 										 result, 0, NULL, NULL ) );

// 	/* Release OpenCL memory objects. */
// 	clReleaseMemObject ( bufC );
// 	clReleaseMemObject ( bufB );
// 	clReleaseMemObject ( bufA );

// 	/* Finalize work with clBLAS */
// 	clblasTeardown( );

// 	/* Release OpenCL working objects. */
// 	clReleaseCommandQueue ( queue );
// 	clReleaseContext ( ctx );

// 	return ret;
// }

/* ************************************************************************
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include CLBLAS header. It automatically includes needed OpenCL header,
 * so we can drop out explicit inclusion of cl.h header.
 */
#include <clBLAS.h>
#include <opencl/clUtils.h>

/* This example uses predefined matrices and their characteristics for
 * simplicity purpose.
 */


static const size_t N = 9;
static const cl_float alpha = 10;

static cl_float X[] = {
	11,
	21,
	31,
	41,
	51,
	61,
	71,
	81,
	91,
};

static const int incx = 1;

static cl_float Y[] = {
	15,
	11,
	1,
	2,
	1,
	8,
	1,
	4,
	1,
};

static cl_float result[9];

static const int incy = 1;

static void
printResult ( void ) {
	size_t i;
	printf ( "\nResult:\n" );
	
	printf ( "Y\n" );
	
	for ( i = 0; i < N; i++ )
		printf ( "\t%f\n", result[i] );
}

int
main ( void ) {
	cl_int err;
	cl_platform_id platform = 0;
	cl_device_id device = 0;
	cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
	cl_context ctx = 0;
	cl_command_queue queue = 0;
	cl_mem bufX, bufY, bufC;
	cl_event event = NULL;
	int ret = 0;
	int lenX = 1 + ( N - 1 ) * abs ( incx );
	int lenY = 1 + ( N - 1 ) * abs ( incy );
	
	/* Setup OpenCL environment. */
	CL_SAFE_CALL ( clGetPlatformIDs ( 1, &platform, NULL ) );
	
	
	CL_SAFE_CALL ( clGetDeviceIDs ( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL ) );
	
	
	props[1] = ( cl_context_properties ) platform;
	ctx = clCreateContext ( props, 1, &device, NULL, NULL, &err );
	
	if ( err != CL_SUCCESS ) {
		printf ( "clCreateContext() failed with %d\n", err );
		return 1;
	}
	
	queue = clCreateCommandQueue ( ctx, device, 0, &err );
	
	if ( err != CL_SUCCESS ) {
		printf ( "clCreateCommandQueue() failed with %d\n", err );
		clReleaseContext ( ctx );
		return 1;
	}
	
	/* Setup clblas. */
	CL_SAFE_CALL ( clblasSetup() );
	
	if ( err != CL_SUCCESS ) {
		printf ( "clblasSetup() failed with %d\n", err );
		clReleaseCommandQueue ( queue );
		clReleaseContext ( ctx );
		return 1;
	}
	
	/* Prepare OpenCL memory objects and place matrices inside them. */
	bufX = clCreateBuffer ( ctx, CL_MEM_READ_ONLY, ( lenX * sizeof ( cl_float ) ), NULL, &err );
	bufY = clCreateBuffer ( ctx, CL_MEM_READ_ONLY, ( lenY * sizeof ( cl_float ) ), NULL, &err );
	bufC = clCreateBuffer ( ctx, CL_MEM_READ_WRITE, ( lenY * sizeof ( cl_float ) ), NULL, &err );
	
	CL_SAFE_CALL ( clEnqueueWriteBuffer ( queue, bufX, CL_TRUE, 0, ( lenX * sizeof ( cl_float ) ), X, 0, NULL, NULL ) );
	CL_SAFE_CALL ( clEnqueueWriteBuffer ( queue, bufY, CL_TRUE, 0, ( lenY * sizeof ( cl_float ) ), Y, 0, NULL, NULL ) );
	
	/* Call clblas function. */
	CL_SAFE_CALL ( clblasSgemm ( clblasRowMajor, clblasNoTrans,
								 clblasNoTrans,
								 3, 3, 3,
								 1, bufX, 0, 3,
								 bufY, 0, 3, 0,
								 bufC, 0, 3,
								 1, &queue, 0, NULL, &event ) );
								 
	if ( err != CL_SUCCESS ) {
		printf ( "clblasSaxpy() failed with %d\n", err );
		ret = 1;
		
	}
	else {
		/* Wait for calculations to be finished. */
		CL_SAFE_CALL ( clWaitForEvents ( 1, &event ) );
		
		/* Fetch results of calculations from GPU memory. */
		CL_SAFE_CALL ( clEnqueueReadBuffer ( queue, bufC, CL_TRUE, 0, ( lenY * sizeof ( cl_float ) ),
											 result, 0, NULL, NULL ) );
											 
		/* At this point you will get the result of SAXPY placed in vector Y. */
		printResult();
	}
	
	/* Release OpenCL events. */
	clReleaseEvent ( event );
	
	/* Release OpenCL memory objects. */
	clReleaseMemObject ( bufY );
	clReleaseMemObject ( bufX );
	
	/* Finalize work with clblas. */
	clblasTeardown();
	
	/* Release OpenCL working objects. */
	clReleaseCommandQueue ( queue );
	clReleaseContext ( ctx );
	
	return ret;
}