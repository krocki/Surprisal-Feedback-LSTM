

# Surprisal-Feedback LSTM
Implementation of Surprisal-Feedback LSTM from "Surprisal-Driven Feedback in Recurrent Networks" (implementation src/layers/surprisalLSTM.h)

https://arxiv.org/abs/1608.06027

```
@article{DBLP:journals/corr/Rocki16b,
  author    = {Kamil Rocki},
  title     = {Surprisal-Driven Feedback in Recurrent Networks},
  journal   = {CoRR},
  volume    = {abs/1608.06027},
  year      = {2016},
  url       = {http://arxiv.org/abs/1608.06027},
  timestamp = {Fri, 02 Sep 2016 17:46:24 +0200},
  biburl    = {http://dblp.uni-trier.de/rec/bib/journals/corr/Rocki16b},
  bibsource = {dblp computer science bibliography, http://dblp.org}
}
```
And Surprisal-Feedback Zoneout (NIPS 2016 Workshop: Continual Learning and Deep Networks - poster: https://github.com/krocki/Surprisal-Feedback-LSTM/blob/master/nips-2016-surprisal-poster.pdf

(implementation of SF-Zoneout in src/layers/surprisalrLSTM.h)

https://arxiv.org/pdf/1610.07675.pdf
```
@article{rocki2016surprisal,
  title={Surprisal-Driven Zoneout},
  author={Rocki, Kamil and Kornuta, Tomasz and Maharaj, Tegan},
  journal={arXiv preprint arXiv:1610.07675},
  year={2016}
}
```
## Getting Started
to compile (with CUDA):

```
make PRECISE_MATH=0 cuda
```
or for precise FP64 (useful for debugging)

```
make PRECISE_MATH=1 cuda
```

CPU versions are outdated, OpenCL version is not fully implemented

If you still wish to compile the CPU version run:

```
make cpu
```

## Usage
 
run like this
```
./deeplstm N B S GPU (test_every_seconds)
```
example (512 hidden nodes, 100 BPTT steps, batchsize = 64, GPU id = 0, run test every 1000s):

```
./deeplstm 512 100 64 0 1000
```

## Author
Kamil M Rocki (kmrocki@us.ibm.com)

## License
Copyright (c) 2016, IBM Corporation. All rights reserved.
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
