
#include <iostream>
#include <iomanip>
#include <vector>
#include <timer.h>

// for now, define matrix type here

#include <containers/datatype.h>

#include <deeplstm.h>
#include <utils.h>

#include <containers/io.h>
// #include <serialization.h>

int main() {

	//openblas_set_num_threads ( 1 );
	#ifdef __USE_CLBLAS__
	init_clblas ();
	#endif
	
	cl_matrix<float> a;
	matrix<float> b;
	
	cl_matrix<float> c ( 100, 100 );
	
	a = c;
	// b = a;
	
	#ifdef __USE_CLBLAS__
	teardown_clblas ();
	#endif
	
	return 0;
	
}