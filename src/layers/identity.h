/*
*
* 	@Author: kmrocki
* 	@Date:   2016-02-19 15:25:43
* 	@Last Modified by:   kmrocki
* 	@Last Modified time: 2016-03-24 18:36:57
*
* 	a dummy layer template
*	copy this into a new file
*	and implement missing parts
*
*/

#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <vector>
#include <parameters.h>
#include <timelayer.h>
#include <state.h>
#include <containers/cu_matrix.h>

template <typename T>
class Identity : public Timelayer<T> {

public:

	/* main constructor */
	Identity ( size_t _in, size_t _out, size_t _B, size_t _S ) :
		Timelayer<T> ( _in, _out, _B, _S,

	{	"dummy_softmax"		},

	{
		/* define states */

	}, {

		/* define params */

	} ) {


	}

	virtual void forward ( bool dropout, size_t t = 1 ) {

		s ( t, x ).sync_device();
		//s ( t, y ) = s(t, x);
		cudaMemcpy ( s ( t, y ).cu_data, s ( t, x ).cu_data, s ( t, x ).cu_bytes_allocated, cudaMemcpyDeviceToDevice );
		s ( t, y ).sync_host();
	}

	virtual void backward ( bool dropout, size_t t ) {

		g ( t, y ).sync_device();
		//g ( t, x ) = g(t, y);
		cudaMemcpy ( g ( t, x ).cu_data, g ( t, y ).cu_data, g ( t, y ).cu_bytes_allocated, cudaMemcpyDeviceToDevice );
		g ( t, x ).sync_host();

	}

	virtual void reset ( dtype std ) {};

	/* optional */
	/* copy constr */
	// Softmax( const Softmax& src) : Timelayer(src.M, src.N, src.B, src.S) { }

	/* assignent operator */
	// Softmax& operator=( const Softmax& src) { Timelayer::operator=(src); return *this; }

};


#endif