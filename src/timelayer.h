/*
 *
 * Author: Kamil Rocki <kmrocki@us.ibm.com>
 *
 * Copyright (c) 2016, IBM Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * 	@Date:   2016-04-05
 * 	@Last Modified by:   kmrocki
 * 	@Last Modified time: 2016-04-05 18:36:57
 *
 *	Abstract Timelayer class: a static NN layer that hold history of its states (in s)
 *	Parameters p are the layer parameters - define the transitions between states
 *	they are independent of the size of s
 *
 *	The idea is that Timelayer does not care about specificity of data structures (state)
 *	or algorithms (forward, backward)
 *
 */

#ifndef __TIMELAYER_H__
#define __TIMELAYER_H__

#include <state.h>
#include <parameters.h>

template <typename T>
class Timelayer {

public:

	virtual ~Timelayer() {
		/* do something */
	};

	/* main constr */
	Timelayer ( size_t _M, size_t _N, size_t _B, size_t _S,

				std::string name,

				std::initializer_list<std::tuple<std::string, size_t, size_t>>
				state_definition,

				std::initializer_list<std::tuple<std::string, size_t, size_t>>
				param_definition )

		: M ( _M ), N ( _N ), S ( _S ), B ( _B ) {

		s.resize ( _S );
		g.resize ( _S );

		for ( size_t t = 0; t < S; t++ ) {

			s[t] = State<T> ( M, N, B, name, state_definition, "s" );
			g[t] = State<T> ( M, N, B, name, state_definition, "g" );

		}

		p  = Parameters<T> ( name, param_definition, "parameters" );
		d  = Parameters<T> ( name, param_definition, "gradients" );
		m  = Parameters<T> ( name, param_definition, "memory" );
		u  = Parameters<T> ( name, param_definition, "adadelta updates" );
		n  = Parameters<T> ( name, param_definition, "numerical" );

		std::cout << "Timelayer() : " << name << std::endl;

	};

	/* copy constr */
	Timelayer ( const Timelayer& t ) :
		p ( t.p ), d ( t.d ), m ( t.m ), n ( t.n ), u ( t.u ),
		S ( t.S ), N ( t.N ), M ( t.M ), B ( t.B ) {
		s = t.s;
		g = t.g;
	}

	/* assignment */
	Timelayer& operator= ( const Timelayer& t ) {

		/* just go over all members */
		p = t.p; d = t.d; m = t.m; n = t.n; u = t.u;
		S = t.S; B = t.B; M = t.M, N = t.N;
		s = t.s; g = t.g;
		return *this;

	}

	/* TODO: move constr */

	void forward ( bool apply_dropout, std::vector <State<T>>& input, char id ) {

		for ( size_t t = 1; t < S; t++ )
			s[t]['x'] = input[t][id];

		for ( size_t t = 1; t < S; t++ )
			forward ( apply_dropout, t );

	}

	void forward ( bool apply_dropout, std::vector<T>& x ) {

		// sequence -> -> ->
		for ( size_t t = 1; t < S; t++ )
			s[t]['x'] =  x[t];

		for ( size_t t = 1; t < S; t++ )
			forward ( apply_dropout, t );

	}

	void backward ( bool apply_dropout, std::vector<T>& dy ) {

		//d.zero();
		for ( size_t w = 0; w < d.matrices.size(); w++ )
			d.matrices[w].cu_zero();

		// for ( size_t w = 0; w < d.matrices.size(); w++ )
		// 	d.matrices[w].sync_device();


		for ( size_t t = 0; t < S; t++ ) {

			/* CPU */
			/* 			g[t].zero();
						g[t]['y'] = dy[t]; */
			g[t].cu_zero();
			g[t]['y'] = dy[t];
			g[t]['y'].sync_device();

		}

		// sequence <- <- <-
		for ( size_t t = S - 1; t > 0; t-- )
			backward ( apply_dropout, t );

		for ( size_t t = 0; t < S; t++ )

			g[t]['x'].sync_host();


	}

	// void sync_state(size_t t) {

	// 	for ( size_t w = 0; w < s[t].matrices.size(); w++ )
	// 		s[t].matrices[w].sync_device();
	// }
	void zero() {

		d.zero();

	}

	/* need to implement these in non-abstract derived classes */
	virtual void forward ( bool apply_dropout, size_t t ) = 0;
	virtual void backward ( bool apply_dropout, size_t t ) = 0;
	virtual void reset ( dtype std ) = 0;

	/*
		forward + backward states

		s is used during inference (actual states)
		g is used during learning (gradient states)

	*/

	/* change to State* and Parameter* */
	std::vector<State<T>> s, g;

	/* weights */
	Parameters<T> p, d, m, n, u;

	/*
		size params:

		M - number of inputs (fan-in)
		N - number of outputs (fan-out)
		B - batch size (how many sequences are being processed at once)
		S - sequence length (unrolling for BPTT)

		M and N are properties of the parameters
		S and B are needed for learning, can be changed for given M and N

	*/

	size_t S, B, M, N;

	void sync_all_host() {

		for ( size_t t = 0; t < S; t++ )

			s[t].sync_host();

		// 	g[t].sync_host();

		//}

		//p.sync_host();
		// d.sync_host();
		// m.sync_host();
		// n.sync_host();
	}

	template<class Archive>
	void serialize ( Archive& archive ) {

		for ( size_t t = 0; t < S; t++ ) {

			bool append = false;

			if ( t > 0 )
				append = true;

			std::map<std::string, size_t>::iterator it = s[t].namemap.begin();
			// int row_start = b;
			// int row_end = b + 1;

			while ( it != s[t].namemap.end() ) {
				s[t].matrices[it->second].sync_host();
				save_matrix_to_file ( s[t].matrices[it->second],
									  "snapshots/enwik8_rlstm_h_check_2_" + s[t].name + "_" + std::to_string ( it->second ) + "_" + it->first + ".txt",
									  append, 0,
									  0 );
				std::advance ( it, 1 );
			}

		}

		//archive ( p ); //, d, m, n);
		//archive(s, g);

	}

	/* FLOPS */
};

#define p(x) this->p[#x]
#define d(x) this->d[#x]
#define s(t, x) this->s[t][#x]
#define g(t, x) this->g[t][#x]

#endif /* __TIMELAYER_H__ */
