//export LD_LIBRARY_PATH=/home/abhinavj/Documents/seminarAsr/src/lib:/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs:$LD_LIBRARY_PATH
//export LD_LIBRARY_PATH=/usr/lib/atlas-base:$LD_LIBRARY_PATH
//Compile on Linux
//g++ -I/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/include -I/home/abhinavj/Documents/seminarAsr/src -I/home/abhinavj/Documents/seminarAsr/tools/CLAPACK/ -g -O3 -o ./fstutils fst-utils.cpp /home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs/libfst.so /home/abhinavj/Documents/seminarAsr/src/lib/*.so -ldl -lpthread


#define HAVE_CLAPACK 1

//For LeftCompose
#include <fstext/context-fst.h>
#include <fstext/fstext-utils.h>
#include <fstext/kaldi-fst-io.h>


//FOr Determinizestar()
#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <fst/fstlib.h>
#include <fstext/determinize-star.h>
#include <fstext/kaldi-fst-io.h>
//Forminimize
//#include <fst/script/minimize.h>
#include <util/text-utils.h>
//For compose
#include <fst/script/compose.h>
#include <fst/script/connect.h>
//For fstprint
#include <fst/script/print.h>
//For fstcompile
#include <fst/script/compile.h>

#include <fstream>
#include <iostream>
#include <string>
#include <map>

using namespace std;

#include "utils.cpp"

//FOr Determinizestar()
using namespace kaldi;
using namespace fst;
//using kaldi::int32;
//Forminimize
//For compose
//For fstprint
//For fstcompile
using namespace fst::script;
using fst::ifstream;


/*
typedef StdArc A;
typedef A::StateId S;
typedef A::Weight W;
typedef Matcher< Fst<A> > M;
typedef SequenceComposeFilter<M> F;
typedef GenericComposeStateTable<A, F::FilterState> T;
typedef ComposeFstOptions<A, M, F, T> COpts;*/


/* 
For each state in the fst, it adds self loops for the input symbols 
present in the disambigs
*/
void AddSelfLoopsToX(StdVectorFst* fst, vector<int> disambigs)
{
  for (StateIterator<StdVectorFst> siter(*fst); !siter.Done(); siter.Next()) 
  {
    int stateId = siter.Value();
    TropicalWeight w = TropicalWeight(0);
    for(int i=0;i<disambigs.size();i++)
    {
      int sym=disambigs[i];
      fst->AddArc(stateId,StdArc(sym, sym, w, stateId));
    }
  }
  
}



/*
For each state, for each arc, of fst, this sets the weight of the 
arc to 0.
*/
void SetFstWeightsToZero(StdVectorFst* fst)
{
  StateIterator<StdVectorFst> siter(*fst);
  for (StateIterator<StdVectorFst> siter(*fst); !siter.Done(); siter.Next()) 
  {
    int stateId = siter.Value();
    for (MutableArcIterator<StdVectorFst> aiter(fst, stateId); !aiter.Done(); aiter.Next())
    {
      StdArc arc = aiter.Value();
      
      float newWeight = 0.0;
      TropicalWeight wAveraged = TropicalWeight(newWeight);     
      arc.weight = wAveraged;
      aiter.SetValue(arc);  
    }
  }
}

/*
This prints the fst in the mentioned filename
*/
void PrintFstWeights(StdVectorFst* fst, const char *fileName)
{

  ofstream myFile;
  myFile.open(fileName);
  string delimiter=" ";
  for (StateIterator<StdVectorFst> siter(*fst); !siter.Done(); siter.Next()) 
  {
    int stateId = siter.Value();
    for (MutableArcIterator<StdVectorFst> aiter(fst, stateId); !aiter.Done(); aiter.Next())
    {
      StdArc arc = aiter.Value();
      myFile << stateId << delimiter << arc.nextstate << delimiter << arc.ilabel << delimiter << arc.ilabel << delimiter << arc.weight.Value();
      myFile << endl;
    }
  }
   myFile.close();
}


/*
This function creates the map state -> phone, as it is a bigram model, 
an input phone will make the model transition to a specific state
*/
void GetStateToPhoneMap(StdVectorFst* fst, map<int, int>& stateToPhone )
{
  //taking the state as 0 which is the backoff state
  for (MutableArcIterator<StdVectorFst> aiter(fst, 0); !aiter.Done(); aiter.Next())
  {
    StdArc arc = aiter.Value();
    stateToPhone[arc.nextstate]=arc.ilabel;
  } 
}


void ComposeTwoFsts(StdVectorFst *fst1, StdVectorFst *fst2, StdVectorFst *cfst)
{
  ArcSort(fst1, StdOLabelCompare());
  ArcSort(fst2, StdILabelCompare());
  ComposeOptions opts(true, fst::SEQUENCE_FILTER);
  Compose(*fst1, *fst2, cfst, opts);
}


void RemoveDisambigSymbols(string disambigFileName, StdVectorFst* fst)
{
  using kaldi::int32;

    std::vector<int32> disambigIn;
    if (!ReadIntegerVectorSimple(disambigFileName, &disambigIn))
      KALDI_ERR << "fstrmsymbols: Could not read disambiguation symbols from "
                << (disambigFileName == "" ? "standard input" : disambigFileName);

    
      RemoveSomeInputSymbols(disambigIn, fst);

    //WriteFstKaldi(fst, fst_wxfilename);

}


void RemoveLocalEpsilons(StdVectorFst *fst)
{
  RemoveEpsLocal(fst);
}





void RemoveEpsilonAndDeterminize(bool useLog, StdVectorFst* fst, StdVectorFst *detFst)
{
  float delta = kDelta;
  int maxStates = -1;
  bool debugLocation = false;

  if(useLog)
  {
    DeterminizeStarInLog(fst, delta, &debugLocation, maxStates);
    *detFst=*fst;
  }
  else
  {
    DeterminizeStar(*fst, detFst, delta, &debugLocation, maxStates);
  }

}




/*void RemoveEpsilonAndDeterminize(StdFst* fst, StdVectorFst& detFst)
{
  float delta = kDelta;
  int maxStates = -1;
  bool debugLocation = false;

  //ArcSort(fst, ILabelCompare<StdArc>());
  DeterminizeStar(*fst, &detFst, delta, &debugLocation, maxStates);
}*/


/*void ComposeTwoFsts(StdVectorFst *fst1, StdVectorFst *fst2, ComposeFst<A>* &cfst)
{
  COpts* opts = new COpts();
  opts->state_table = new T(*fst1, *fst2);
  cfst = new ComposeFst<A>(*fst1, *fst2, *opts);
}*/





void MinimizeFst(StdVectorFst* fst)
{
    using kaldi::int32;
    float delta = kDelta;
    MinimizeEncoded(fst, delta);
 }


void PrintFstText(StdVectorFst* fst, const char* fileName)
{
  string fileNameString (fileName);
  ofstream myFile;
  myFile.open(fileName);
  fst::ostream *ostrm = &myFile;
    string dest = "To file " + fileNameString;
    ostrm->precision(9);
  const fst::SymbolTable *isyms = fst->InputSymbols();
  const fst::SymbolTable *osyms = fst->OutputSymbols();
  const fst::SymbolTable *ssyms = 0;
    PrintFst(*fst, *ostrm, dest, isyms, osyms, ssyms);
    myFile.close();
}

void TempCompileFromFst(string fileName, string dest)
{
  string command="fstcompile ";
  command+=fileName;
  command+=" ";
  command+=dest;
  //cout<<endl<<command<<endl;
  //getchar();
  int c=system(command.c_str());
}

void CompileFstFromText(char* fileName, string dest)
{
  ifstream myFile(fileName);
  fst::istream& istrm = myFile;
    const string source = fileName;
    const string FLAGS_fst_type="vector";
    const string FLAGS_arc_type="standard";
    bool FLAGS_allow_negative_labels=true;
    bool FLAGS_keep_state_numbering = true;
    bool FLAGS_keep_isymbols=false;
    bool FLAGS_keep_osymbols=false;
    bool FLAGS_acceptor=true;
    const fst::SymbolTable *isyms = 0;
    const fst::SymbolTable *osyms = 0;
    const fst::SymbolTable *ssyms = 0;
    /*CompileFst(istrm ,source, dest, FLAGS_fst_type, FLAGS_arc_type,
                isyms, osyms, ssyms,
                FLAGS_acceptor, FLAGS_keep_isymbols,
                FLAGS_keep_osymbols,FLAGS_keep_state_numbering, FLAGS_allow_negative_labels);
*/
}

void LeftComposeWithC(StdVectorFst* fst, string disambigOutFilename, StdVectorFst* result)
{
    bool binary = true;
    std::string disambig_rxfilename="exp/lang/phones/disambig.int";
    std::string disambig_wxfilename="exp/lang/tmp/disambig_ilabels_3_1.int";
    int32 N = 3, P = 1;
    std::string ilabels_out_filename = disambigOutFilename;

    //VectorFst<StdArc> *fst = ReadFstKaldi(fst_in_filename);

    if ( (disambig_wxfilename != "") && (disambig_rxfilename == "") )
      KALDI_ERR << "fstcomposecontext: cannot specify --write-disambig-syms if "
          "not specifying --read-disambig-syms\n";

    std::vector<int32> disambig_in;
    if (disambig_rxfilename != "")
      if (!ReadIntegerVectorSimple(disambig_rxfilename, &disambig_in))
        KALDI_ERR << "fstcomposecontext: Could not read disambiguation symbols from "
                  << PrintableRxfilename(disambig_rxfilename);

    if (disambig_in.empty()) {
      KALDI_WARN << "Disambiguation symbols list is empty; this likely "
                 << "indicates an error in data preparation.";
    }
    
    std::vector<std::vector<int32> > ilabels;

    // Work gets done here (see context-fst.h)
    ComposeContext(disambig_in, N, P, fst, result, &ilabels);

    WriteILabelInfo(Output(ilabels_out_filename, binary).Stream(),
                    binary, ilabels);

    if (disambig_wxfilename != "") {
      std::vector<int32> disambig_out;
      for (size_t i = 0; i < ilabels.size(); i++)
        if (ilabels[i].size() == 1 && ilabels[i][0] <= 0)
          disambig_out.push_back(static_cast<int32>(i));
      if (!WriteIntegerVectorSimple(disambig_wxfilename, disambig_out)) {
        std::cerr << "fstcomposecontext: Could not write disambiguation symbols to "
                  << PrintableWxfilename(disambig_wxfilename) << '\n';
      }
    }


} 


/*void UpdateFstWeights(string in, bool whatToDo, int importance, int backOffState, int dataPoints, StdVectorFst& fst, StdVectorFst& averagedFst)
{
  Matcher<StdVectorFst> matcher(fst, MATCH_INPUT);
  Matcher<StdVectorFst> matcherAveraged(averagedFst, MATCH_INPUT);
  int state=fst.Start();
  matcher.SetState(state);
  matcherAveraged.SetState(state);
  string delimiter=" ";
  string token;
  int pos=0;
  float input;
  float newWeight;
  while((pos=in.find(delimiter))!=string::npos)
  {
    token=in.substr(0,pos);
    in.erase(0,pos+delimiter.length());
    input=atof(token.c_str());
    if(matcher.Find(input))
    {
      matcherAveraged.Find(input);
       StdArc arc = matcher.Value(); 
       StdArc arcAveraged = matcherAveraged.Value(); 
       if(whatToDo)
      {
         newWeight=arc.weight.Value() + importance;
      }
      else
      {
          newWeight=arc.weight.Value() - importance;
      } 
      float newAveragedWeight = (arcAveraged.weight.Value()*dataPoints+newWeight)/(dataPoints+1);
      //averagedFst.DeleteArcs(state, arc.Value());
     // averagedFst.AddArc(state, StdArc(input, input, newAveragedWeight, arc.nextstate));    
      arcAveraged.weight=TropicalWeight(newAveragedWeight);
      arc.weight=TropicalWeight(newWeight);

      //fst.DeleteArcs(state, arc.Value());
      //fst.AddArc(state, StdArc(input, input, newWeight, arc.nextstate));
      state=arc.nextstate;
      matcher.SetState(state);
      matcherAveraged.SetState(state);     
    }
    else
    {
      matcher.SetState(backOffState);
       matcherAveraged.SetState(backOffState);
      matcher.Find(input);
      matcherAveraged.Find(input);
      const StdArc &arc = matcher.Value(); 
       const StdArc &arcAveraged = matcherAveraged.Value(); 
      if(whatToDo)
      {
         newWeight=arc.weight.Value() + importance;
      }
      else
      {
          newWeight=arc.weight.Value() - importance;
      }
      float newAveragedWeight = (arcAveraged.weight.Value()*dataPoints+newWeight)/(dataPoints+1);
      //averagedFst.DeleteArcs(state, arc.Value());
     // averagedFst.AddArc(state, StdArc(input, input, newAveragedWeight, arc.nextstate));    
      arcAveraged.weight=TropicalWeight(newAveragedWeight);
      arc.weight=TropicalWeight(newWeight);

      //fst.DeleteArcs(state, arc.Value());
      //fst.AddArc(state, StdArc(input, input, newWeight, arc.nextstate));
      state=arc.nextstate;
      matcher.SetState(state);
      matcherAveraged.SetState(state);  
    }

    
  }
}*/




void UpdateFstWeights(string strUntrimmed, bool whatToDo, float importance,
                      int backOffState, StdVectorFst* fst, int epoch_count, int dataPoint, 
                      map<int, int>& stateToPhone, map<int, string>& indexToPhone)
{
  string fileName;
  if(whatToDo)
  {
      fileName="exp/weightMod/ref_"+ConvertToString(epoch_count)+"_"+ConvertToString(dataPoint);
  }
  else
  {
    fileName="exp/weightMod/dec_"+ConvertToString(epoch_count)+"_"+ConvertToString(dataPoint);
  }
  const char* cfileName=fileName.c_str();
  ofstream myFile;
  myFile.open(cfileName);

  string str=trim(strUntrimmed);
  string in=str+" ";
  int currentState=fst->Start();
  //cout<<currentState<<endl;
  //getchar();
  string delimiter=" ";
  string token;
  int pos=0;
  float input;
  float newWeight;
  while((pos=in.find(delimiter))!=string::npos)
  {
    token=in.substr(0,pos);
    //cout<<in<<endl;
    //cout<<"Token:"<<token<<endl;
    //getchar();
    in.erase(0,pos+delimiter.length());
    input=atoi(token.c_str());
    
    //update monophone weight
/*    for (MutableArcIterator<StdVectorFst> aiter(fst, backOffState); !aiter.Done(); aiter.Next())
    {
      //cout<<"Ilabel:"<<aiter.Value().ilabel<<endl;
      //cout<<"input:"<<input<<endl;
      StdArc arc = aiter.Value();
      if(arc.ilabel==input)
      {
          float newWeight;
          if(whatToDo)
          {
            newWeight=arc.weight.Value() - importance; 
          }
          else
          {
           newWeight=arc.weight.Value() + importance; 
          }
          //myFile<<"For Monogram:"<<endl;
          myFile<<indexToPhone[stateToPhone[backOffState]]<<" "<<indexToPhone[input]<<" "<<arc.nextstate<<" "<<arc.weight.Value()<<" "<<newWeight<<endl;
          TropicalWeight w = TropicalWeight(newWeight);     
          arc.weight = w;
          aiter.SetValue(arc);
          //currentState=arc.nextstate;
          break;
      }
    }*/

    //update biphone weights
    bool flag=false;
    for (StateIterator<StdVectorFst> siter(*fst); !siter.Done(); siter.Next()) 
    {
      //cout<<"Siter Value:"<<siter.Value()<<endl;
      int stateId = siter.Value();
      if(stateId!=currentState)
      {
        continue;
      }
      for (MutableArcIterator<StdVectorFst> aiter(fst, stateId); !aiter.Done(); aiter.Next())
      {
         //cout<<"Aiter Value:"<<aiter.Value().ilabel<<endl;
        StdArc arc = aiter.Value();
        if(arc.ilabel==input)
        {
            flag=true;
            float newWeight;
            if(whatToDo)
            {
              newWeight=arc.weight.Value() - importance; 
            }
            else
            {
             newWeight=arc.weight.Value() + importance; 
            }
            //myFile<<"For bigram:"<<endl;
            myFile<<indexToPhone[stateToPhone[stateId]]<<" "<<indexToPhone[input]<<" "<<arc.nextstate<<" "<<arc.weight.Value()<<" "<<newWeight<<endl;
            TropicalWeight w = TropicalWeight(newWeight);     
            arc.weight = w;
            aiter.SetValue(arc);
            currentState=arc.nextstate;
            //cout<<"New Weight:"<<arc.weight.Value()<<endl;
            break;
        }
        else
        {
          continue;
        }
      }
      if(flag)
      {
        break;
      }
        
    }
      

  }
  myFile.close();
}


void GetAveragedFst(float averageFor, StdVectorFst* fst, StdVectorFst* fstAveraged, int dataPoint)
{/*
  string fileName="exp/averageWeights/iter_"+ConvertToString(dataPoint);
  const char* cfileName=fileName.c_str();
  ofstream myFile;
  myFile.open(cfileName);
*/

  StateIterator<StdVectorFst> siter(*fst);
  for (StateIterator<StdVectorFst> siterAveraged(*fstAveraged); !siterAveraged.Done(); siterAveraged.Next()) 
  {
    int stateIdAveraged = siterAveraged.Value();
    int stateId = siter.Value();
    MutableArcIterator<StdVectorFst> aiter(fst, stateId);
    for (MutableArcIterator<StdVectorFst> aiterAveraged(fstAveraged, stateIdAveraged); !aiterAveraged.Done(); aiterAveraged.Next())
    {
      StdArc arc = aiter.Value();
      StdArc arcAveraged = aiterAveraged.Value();
      
      float newAveragedWeight = ((arcAveraged.weight.Value()*averageFor) + arc.weight.Value())/(averageFor+1.0);
      //myFile<<newAveragedWeight<<" ";
      TropicalWeight wAveraged = TropicalWeight(newAveragedWeight);         
      arcAveraged.weight = wAveraged;
      aiterAveraged.SetValue(arcAveraged); 
      aiter.Next();
    }
    siter.Next();
      
  }

 // myFile.close();
    
}









/*int main(int argc, char** argv) 
{
  
  StdVectorFst* fst1 = StdVectorFst::Read(argv[1]);
  StdVectorFst* fst2 = StdVectorFst::Read(argv[2]);
  StdVectorFst result;

  char* fileName="composed_minimized.txt";

  //RemoveEpsilonAndDeterminize(fst1, result);
  //ComposeTwoFsts(fst1,fst2,result);
  //MinimizeFst(fst1, result);
  //PrintFstText(fst1, fileName);
  //result.Write("composed_minimized.fst");


}*/


