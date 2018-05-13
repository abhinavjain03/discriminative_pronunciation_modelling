CXX=g++
CXXFLAGS=-g \
-O3
CPPFLAGS=-I/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/include \
-I/home/abhinavj/Documents/seminarAsr/src \
-I/home/abhinavj/Documents/seminarAsr/tools/CLAPACK/
LDLIBS=-L/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs \
-L/home/abhinavj/Documents/seminarAsr/src/lib/ \
-L/usr/lib/atlas-base \
-latlas -lcblas -lf77blas -llapack_atlas -ldl -lpthread \
-lfst -lkaldi-base -lkaldi-ivector -lkaldi-online2 -lkaldi-chain \
-lkaldi-kws -lkaldi-sgmm2 -lkaldi-cudamatrix -lkaldi-lat \
-lkaldi-sgmm -lkaldi-decoder -lkaldi-lm -lkaldi-thread -lkaldi-feat \
-lkaldi-matrix -lkaldi-transform -lkaldi-fstext -lkaldi-nnet2 -lkaldi-tree -lkaldi-gmm \
-lkaldi-nnet3 -lkaldi-util -lkaldi-hmm -lkaldi-nnet

run: main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDLIBS) main.cpp -o run 