
/*
*   @Author: kmrocki
*   @Date:   2016-02-19 15:25:43
*   @Last Modified by:   kmrocki
*   @Last Modified time: 2016-03-24 18:36:57
*
*   Implementation of surprisalLSTM layer
*
*/

#ifndef _surprisalLSTM_H_
#define _surprisalLSTM_H_

#include <vector>
#include <parameters.h>
#include <timelayer.h>
#include <state.h>

#include <containers/cu_matrix.h>

/* gates:

[  0:    N-1 ] = i
[  N:   2N-1 ] = o
[ 2N:   3N-1 ] = f
[ 3N:   4N-1 ] = u

*/


#define N this->N
#define B this->B

template <typename T>
class surprisalLSTM : public Timelayer<T> {

	public:
	
		using Timelayer<T>::s;
		
		//temp storage for probability sums, softmax
		T sums;
		T maxima;
		T rands;
		
		/* main constructor */
		surprisalLSTM ( size_t _M, size_t _N, size_t _B,
						size_t _S ) :
						
			Timelayer<T> ( _M, _N, _B, _S,
			
		{   "surprisal_lstm"        },
		
		{
			/* define states */
			std::make_tuple ( "h", _B, _N ),
			std::make_tuple ( "c", _B, _N ),
			std::make_tuple ( "ct", _B, _N ),
			std::make_tuple ( "g", _B, 4 * _N ),
			std::make_tuple ( "g2", _B, 4 * _N ),
			std::make_tuple ( "g3", _B, 4 * _N ),
			
			//from softmax
			std::make_tuple ( "p", _B, _M ),
			std::make_tuple ( "q", _B, _M ),
			std::make_tuple ( "qsum", _B, 1 ),
			
		}, {
		
			/* define params */
			std::make_tuple ( "W", _M, 4 * _N ),
			std::make_tuple ( "U", _N, 4 * _N ),
			std::make_tuple ( "V", 1, 4 * _N ),
			std::make_tuple ( "b", 1, 4 * _N ),
			
			std::make_tuple ( "B_ones", _B, 1 ),
			
			//from softmax
			std::make_tuple ( "Wy", _N, _M ),
			std::make_tuple ( "by", 1, _M ),
			std::make_tuple ( "M_ones", _M, 1 )
			
			
		} ) {
		
			/*init*/
			matrix_init ( p ( W ) );
			matrix_init ( p ( U ) );
			matrix_init ( p ( V ) );
			
			rands = T ( _B, 4 * _N );
			
			// set biases of f gates to -1
			// (http://jmlr.org/proceedings/papers/v37/jozefowicz15.pdf)
			p ( b ).block_forall ( 2 * N, N, [ = ] () { return -1; } );
			
			//rand
			//p ( b ).block_rand ( 2 * N, N, 0, 4 );
			
			p ( B_ones ).forall ( [ = ] () { return 1; } );
			p ( B_ones ).sync_device();
			
			/*softmax*/
			matrix_init ( p ( Wy ) );
			sums = T ( _B, 1 );
			maxima = T ( _B, 1 );
			
			p ( M_ones ).forall ( [ = ] () { return 1; } );
			p ( M_ones ).sync_device();
			
		}
		
		~surprisalLSTM() {
		
		}
		
#define c_gates 3 * N * s(t, g).rows()
#define i_gates 0 * N * s(t, g).rows()
#define f_gates 2 * N * s(t, g).rows()
#define o_gates 1 * N * s(t, g).rows()
		
		virtual void forward ( bool apply_dropout, size_t t = 1 ) {
		
			s ( t, x ).sync_device();
			s ( t - 1, h ).sync_device();
			s ( t - 1, c ).sync_device();
			s ( t - 1, p ).sync_device();
			s ( t - 1, q ).sync_device();
			
			// put in 2 separate streams
			cublasSetStream ( handle, streams[1] );
			CU_GEMM ( s ( t, g ), s ( t, x ), p ( W ), false, false, 1, 0 );
			
			cublasSetStream ( handle, streams[2] );
			CU_GEMM ( s ( t, g2 ), s ( t - 1, h ), p ( U ), false, false, 1, 0 );
			
			cu_elementwise_surprisal_lstm_forward0 ( s ( t, q ).cu_data, s ( t - 1, p ).cu_data, s ( t, x ).cu_data, this->M, s ( t,
					q ).rows() );
					
			cublasSetStream ( handle, streams[3] );
			CU_GEMM ( s ( t, qsum ), s ( t, q ), p ( M_ones ), false, false, 1, 0 );
			CU_GEMM ( s ( t, g3 ), s ( t, qsum ), p ( V ), false, false, 1, 0 );
			
			//cu_rand ( rands.cu_data, rands.size() );
			
			// fmad
			// cublasSetStream ( handle, streams[3] );
			// CU_GEMM ( s ( t, g3 ), s ( t - 1, h ), p ( V ), false, false, 1, 0 );
			
			sync_stream ( 1 );
			sync_stream ( 2 );
			sync_stream ( 3 );
			
			//fused
			cu_elementwise_surprisal_lstm_forward (
				& ( s ( t, g ).cu_data[0] ),
				& ( s ( t, g2 ).cu_data[0] ),
				& ( s ( t, g3 ).cu_data[0] ),
				& ( p ( b ).cu_data[0] ),
				& ( s ( t, h ).cu_data[0] ),
				& ( s ( t, c ).cu_data[0] ),
				& ( s ( t, ct ).cu_data[0] ),
				& ( s ( t - 1, c ).cu_data[0] ),
				& ( s ( t - 1, h ).cu_data[0] ),
				& ( rands.cu_data[0] ),
				N, s ( t, c ).rows() );
				
			/* softmax */
			//s ( t, x ).sync_device();
			//cudaMemcpy ( s ( t, q ).cu_data, s ( t, h ).cu_data, s ( t, h ).cu_bytes_allocated, cudaMemcpyDeviceToDevice );
			
			CU_GEMM ( s ( t, p ), s ( t, h ), p ( Wy ), false, false, 1, 0 );
			
			cu_add_row_vector ( & ( s ( t, p ) ).cu_data[0], & ( p ( by ) ).cu_data[0], this->M, s ( t, p ).rows() );
			
			// for numerical stability
			cu_row_max ( maxima.cu_data,  s ( t, p ).cu_data, this->M, s ( t, p ).rows() );
			cu_sub_col_vector ( & ( s ( t, p ) ).cu_data[0], maxima.cu_data, this->M, s ( t, p ).rows() );
			
			cu_exp ( & ( s ( t, p ) ).cu_data[0], this->M * s ( t, p ).rows() );
			
			CU_GEMM ( sums, s ( t, p ), p ( M_ones ), false, false, 1, 0 );
			
			cu_div_col_vector ( & ( s ( t, p ) ).cu_data[0], & ( sums.cu_data[0] ), this->M, s ( t, p ).rows() );
			
			s ( t, c ).sync_host();
			s ( t, h ).sync_host();
			s ( t, p ).sync_host();
			//s ( t, q ).sync_host();
			s ( t, qsum ).sync_host();
			
		}
		
		virtual void backward ( bool apply_dropout, size_t t ) {
		
			//softmax
			cu_sub ( & ( g ( t, p ).cu_data[0] ), & ( s ( t, p ).cu_data[0] ), & ( g ( t, y ).cu_data[0] ), this->M * s ( t,
					 p ).rows() );
					 
			// propagate through linear layer h->y
			cublasSetStream ( handle, streams[1] );
			CU_GEMM ( d ( Wy ), s ( t, h ), g ( t, p ), true, false );
			
			cublasSetStream ( handle, streams[2] );
			CU_GEMM ( d ( by ), p ( B_ones ), g ( t, p ), true, false );
			
			// propagate through linear layer x->h
			cublasSetStream ( handle, streams[3] );
			CU_GEMM ( g ( t, h ), g ( t, p ), p ( Wy ), false, true );
			
			sync_stream ( 1 );
			sync_stream ( 2 );
			sync_stream ( 3 );
			
			// error coming from higher layers: dh[t] = dy[t]
			
			cu_elementwise_surprisal_lstm_backward ( & ( g ( t, g ).cu_data[0] ), & ( g ( t, g2 ).cu_data[0] ), & ( g ( t,
					g3 ).cu_data[0] ),
					& ( g ( t, h ).cu_data[0] ),
					& ( s ( t, c ).cu_data[0] ),
					& ( s ( t, ct ).cu_data[0] ),
					& ( g ( t, c ).cu_data[0] ),
					& ( s ( t, g ).cu_data[0] ),
					& ( s ( t, g2 ).cu_data[0] ),
					& ( s ( t, g3 ).cu_data[0] ),
					& ( s ( t - 1, c ).cu_data[0] ),
					& ( g ( t - 1, c ).cu_data[0] ),
					& ( s ( t , h ).cu_data[0] ),
					& ( s ( t - 1, h ).cu_data[0] ),
					& ( g ( t - 1, h ).cu_data[0] ),
					N, g ( t, c ).rows() );
					
			//backprop through linear part (forward pass step 1)
			//these are computed in parallel
			cublasSetStream ( handle, streams[1] );
			CU_GEMM ( d ( b ), p ( B_ones ), g ( t, g ), true, false );
			
			cublasSetStream ( handle, streams[2] );
			//cu_elementwise_mult ( g ( t, g2 ).cu_data, g ( t, g ).cu_data, s ( t, g3 ).cu_data, 4 * N * g ( t, c ).rows());
			CU_GEMM ( d ( U ), s ( t - 1, h ), g ( t, g2 ), true, false );
			
			cublasSetStream ( handle, streams[3] );
			CU_GEMM ( d ( W ), s ( t, x ), g ( t, g ), true, false );
			
			cublasSetStream ( handle, streams[4] );
			//cu_elementwise_mult ( g ( t, g3 ).cu_data, g ( t, g ).cu_data, s ( t, g2 ).cu_data, 4 * N * g ( t, c ).rows());
			CU_GEMM ( d ( V ), s ( t, qsum ), g ( t, g3 ), true, false );
			
			cublasSetStream ( handle, streams[5] );
			//backprop into inputs for lower layers
			CU_GEMM ( g ( t, x ), g ( t, g ), p ( W ), false, true );
			
			cublasSetStream ( handle, streams[6] );
			
			//carry - h state
			CU_GEMM ( g ( t - 1, h ), g ( t, g2 ), p ( U ), false , true );
			
			//fmad
			// CU_GEMM ( g ( t - 1, h ), g ( t, g3 ), p ( V ), false , true );
			
			cublasSetStream ( handle, streams[7] );
			
			//carry - y state
			CU_GEMM ( g ( t, qsum ), g ( t, g3 ), p ( V ), false , true, 1, 0 );
			
			//carry - y state
			cu_elementwise_surprisal_lstm_backward0 ( g ( t - 1, p ).cu_data,
					g ( t, qsum ).cu_data, s ( t, q ).cu_data, s ( t - 1, p ).cu_data, s ( t, x ).cu_data, this->M, g ( t, q ).rows() );
					
			CU_GEMM ( sums, g ( t - 1, p ), p ( M_ones ), false, false, 1, 0 );
			
			cu_elementwise_surprisal_lstm_backward1 ( g ( t - 1, p ).cu_data, s ( t - 1, p ).cu_data, s ( t, x ).cu_data,
					sums.cu_data, this->M, g ( t - 1, p ).rows() );
					
			sync_stream ( 1 );
			sync_stream ( 2 );
			sync_stream ( 3 );
			sync_stream ( 4 );
			sync_stream ( 5 );
			sync_stream ( 6 );
			//sync_stream ( 7 );
		}
		
		virtual void reset ( dtype std ) {
		
			randn ( s ( 0, p ), ( dtype ) 1.0f / 256.0f, ( dtype ) std );
			randn ( s ( 0, h ), ( dtype ) 0, ( dtype ) std );
			randn ( s ( 0, c ), ( dtype ) 0, ( dtype ) std );
			// s ( 0, h ).cu_zero();
			// s ( 0, c ).cu_zero();
		}
		
		/* optional */
		/* copy constr */
		//surprisalLSTM( const surprisalLSTM& src) : Timelayer(src.M, src.N, src.B, src.S)  { }
		
		/* assignent operator */
		//surprisalLSTM& operator=( const surprisalLSTM& src) { Timelayer::operator=(src); return *this; }
		
};

#undef N
#undef B

#undef i_gates
#undef o_gates
#undef c_gates
#undef f_gates

#endif /* _surprisalLSTM_H_ */