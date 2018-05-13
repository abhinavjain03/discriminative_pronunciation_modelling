// Author: Preethi Jyothi
// November 2011

//Compile on Linux
//g++ -I /home2/jyothi/openfst-1.2.7/src/include -g -O3 -o ./spath shortest_path.cc /home2/jyothi/openfst-1.2.7/src/lib/.libs/libfst.so -ldl -lpthread

#include <iostream>
#include <assert.h>
#include <fst/compose.h>
#include <fst/fstlib.h>
#include <fst/script/print.h>
#include <fst/shortest-path.h>
#include <vector>

using namespace std;
using namespace fst;

int main(int argc, char** argv) {

  //These are the aliases for all default composition options
  typedef StdArc A;
  typedef A::StateId S;
  typedef A::Weight W;
  typedef Matcher< Fst<A> > M;
  typedef SequenceComposeFilter<M> F;
  typedef GenericComposeStateTable<A, F::FilterState> T;
  typedef ComposeFstOptions<A, M, F, T> COpts;
  if (argc <= 1) {
    cerr << "Usage : cmd 1st_fst 2nd_fst .. nth_fst" << endl;
    cerr << "Output : shortest path from the composed fst" << endl;
    exit(0);
  }
  vector<StdFst*> fsts;
  vector<COpts*> copts;

  for (int i = 1; i != argc-1; ++i) {
    StdVectorFst* fst = StdVectorFst::Read(argv[i]);
    ArcSort(fst, StdOLabelCompare());
    if (fst == NULL)
      LOG(FATAL) << "Failed to read fst from : " << argv[i];
    fsts.push_back(fst);
  }

  StdVectorFst result;
  //Compose(*fsts[0], *fsts[1], &result);
  vector<StdFst*> cfsts;
  cfsts.push_back(fsts[0]);
  for (int i = 1; i != fsts.size(); i++) {
    COpts* opts = new COpts();
    opts->state_table = new T(*cfsts[i - 1], *fsts[i]);
    copts.push_back(opts);
    ComposeFst<A>* cfst = new ComposeFst<A>(*cfsts[i - 1], *fsts[i], *opts);
    LOG(INFO) << "Finished delayed composition for FSTs " << (i-1) << " and " << i;
    cfsts.push_back(cfst);
  }
  
  LOG(INFO) << "Finished delayed composition of all the input FSTs";  

  StdVectorFst path;

  ShortestPath(*(cfsts.back()), &path); 
  
  path.Write(argv[argc-1]);
  
  for (int i = copts.size() - 1; i >= 0; --i) {
    delete cfsts[i + 1];
    delete copts[i];
  }

  for (int i = 0; i != fsts.size(); ++i)
    delete fsts[i];
  
  return 0;

}
