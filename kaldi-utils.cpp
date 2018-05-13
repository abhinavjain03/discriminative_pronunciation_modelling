//g++ -I/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/include -I/home/abhinavj/Documents/seminarAsr/src -I/home/abhinavj/Documents/seminarAsr/tools/CLAPACK/ -g -O3 -o ./kaldiutils kaldi-utils.cpp /home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs/libfst.so /home/abhinavj/Documents/seminarAsr/src/lib/*.so -ldl -lpthread


#define HAVE_CLAPACK 1

//For AddSelfLoops
#include <tree/context-dep.h>
#include <fstext/table-matcher.h>
#include <fstext/context-fst.h>

//make h transducer
#include <fstext/fstext-utils.h>
#include <fstext/context-fst.h>



#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <fstext/fstext-lib.h>
#include <lat/kaldi-lattice.h>
#include <lat/lattice-functions.h>
#include <hmm/transition-model.h>
#include <hmm/hmm-utils.h>
#include <fst/fstlib.h>
#include <string>
#include <cstdlib>
#include <sstream>

#include "utils.cpp"

using namespace kaldi;
using fst::SymbolTable;
using fst::VectorFst;
using fst::StdArc;

typedef kaldi::int32 int32;
typedef kaldi::int64 int64;




StdVectorFst* MakeHTransducer(string disambig_out_filename,
                     string ilabel_info_filename,
                     string tree_filename,
                     string model_filename 
                     )
{
  HTransducerConfig hcfg;
    std::string fst_out_filename;
    std::vector<std::vector<int32> > ilabel_info;
    {
      bool binary_in;
      Input ki(ilabel_info_filename, &binary_in);
      fst::ReadILabelInfo(ki.Stream(), binary_in, &ilabel_info);
    }

    ContextDependency ctx_dep;
    ReadKaldiObject(tree_filename, &ctx_dep);

    TransitionModel trans_model;
    ReadKaldiObject(model_filename, &trans_model);

    std::vector<int32> disambig_syms_out;

    // The work gets done here.
    StdVectorFst *H = GetHTransducer (ilabel_info,
                                        ctx_dep,
                                         trans_model,
                                         hcfg,
                                        &disambig_syms_out);

    if (disambig_out_filename != "") {  // if option specified..
      if (disambig_out_filename == "-")
        disambig_out_filename = "";
      if (! WriteIntegerVectorSimple(disambig_out_filename, disambig_syms_out))
        KALDI_ERR << "Could not write disambiguation symbols to "
                   << (disambig_out_filename == "" ?
                       "standard output" : disambig_out_filename);
    }
    return H;

}



void AddSelfLoopsToGraph(string modelPath, StdVectorFst* fst)
{
    typedef kaldi::int32 int32;
    using fst::SymbolTable;
    using fst::VectorFst;
    using fst::StdArc;

    BaseFloat self_loop_scale = 0.1;
    bool reorder = true;
    std::string disambig_in_filename;

    std::string model_in_filename = modelPath;


    std::vector<int32> disambig_syms_in;
    if (disambig_in_filename != "") {
      if (disambig_in_filename == "-") disambig_in_filename = "";
      if (!ReadIntegerVectorSimple(disambig_in_filename, &disambig_syms_in))
        KALDI_ERR << "add-self-loops: could not read disambig symbols from "
                   <<(disambig_in_filename == "" ?
                      "standard input" : disambig_in_filename);
    }

    TransitionModel trans_model;
    ReadKaldiObject(model_in_filename, &trans_model);
    // The work gets done here.
    AddSelfLoops(trans_model,disambig_syms_in,self_loop_scale,reorder,fst);

}


void LatticeOneBest(string inputLattice, string outputLattice)
{
  using namespace kaldi;
    typedef kaldi::int32 int32;
    typedef kaldi::int64 int64;
    using fst::VectorFst;
    using fst::StdArc;

    inputLattice="ark:"+inputLattice;
    outputLattice="ark:"+outputLattice;
    BaseFloat acoustic_scale = 1.0;
    BaseFloat lm_scale = 1.0;

    std::string lats_rspecifier = inputLattice, lats_wspecifier = outputLattice;

    SequentialCompactLatticeReader clat_reader(lats_rspecifier);
    
    // Write as compact lattice.
    CompactLatticeWriter compact_1best_writer(lats_wspecifier); 

    int32 n_done = 0, n_err = 0;

    if (acoustic_scale == 0.0 || lm_scale == 0.0)
      KALDI_ERR << "Do not use exactly zero acoustic or LM scale (cannot be inverted)";
    for (; !clat_reader.Done(); clat_reader.Next()) {
      std::string key = clat_reader.Key();
      CompactLattice clat = clat_reader.Value();
      clat_reader.FreeCurrent();
      fst::ScaleLattice(fst::LatticeScale(lm_scale, acoustic_scale), &clat);

      CompactLattice best_path;
      CompactLatticeShortestPath(clat, &best_path);
      

        fst::ScaleLattice(fst::LatticeScale(1.0 / lm_scale, 1.0/acoustic_scale),
                          &best_path);
        compact_1best_writer.Write(key, best_path);
        n_done++;
    }
    cout << "Done converting " << n_done << " to best path, "
              << n_err << " had errors.";
}


void NBestToLinear(string latticePath, string alignmentPath)
{
  alignmentPath="ark,t:"+alignmentPath;
  latticePath="ark:"+latticePath;
  using namespace kaldi;
    typedef kaldi::int32 int32;
    typedef kaldi::int64 int64;
    using fst::SymbolTable;
    using fst::VectorFst;
    using fst::StdArc;


    std::string lats_rspecifier = latticePath,
        ali_wspecifier = alignmentPath;

    SequentialLatticeReader lattice_reader(lats_rspecifier);
    Int32VectorWriter ali_writer(ali_wspecifier);

    int32 n_done = 0, n_err = 0;

    for (; !lattice_reader.Done(); lattice_reader.Next()) {
      std::string key = lattice_reader.Key();
      Lattice lat = lattice_reader.Value();

      std::vector<int32> ilabels;
      std::vector<int32> olabels;
      LatticeWeight weight;

      if (!GetLinearSymbolSequence(lat, &ilabels, &olabels, &weight)) {
        KALDI_WARN << "Lattice/nbest for key " << key << " had wrong format: "
            "note, this program expects input with one path, e.g. from "
            "lattice-to-nbest.";
        n_err++;
      } else {
        if (ali_wspecifier != "") ali_writer.Write(key, ilabels);
        n_done++;
      }
    }
    cout << "Done " << n_done << " n-best entries, "
              << n_err  << " had errors.";
    //return (n_done != 0 ? 0 : 1);
}


void AliToPhones(string model, string alignmentPath, string transcriptionPath)
{
  using namespace kaldi;
  typedef kaldi::int32 int32;
  try {

    alignmentPath="ark:"+alignmentPath;
    transcriptionPath="ark,t:"+transcriptionPath;
    bool per_frame = false;
    bool write_lengths = false;
    bool ctm_output = false;

    std::string model_filename = model,
        alignments_rspecifier = alignmentPath;

    TransitionModel trans_model;
    ReadKaldiObject(model_filename, &trans_model);

    SequentialInt32VectorReader reader(alignments_rspecifier);
    std::string empty;
    Int32VectorWriter phones_writer(ctm_output ? empty :
                                    (write_lengths ? empty : transcriptionPath));
    Int32PairVectorWriter pair_writer(ctm_output ? empty :
                                      (write_lengths ? transcriptionPath : empty));

    std::string ctm_wxfilename(ctm_output ? transcriptionPath : empty);
    Output ctm_writer(ctm_wxfilename, false);


    int32 n_done = 0;

    for (; !reader.Done(); reader.Next()) {
      std::string key = reader.Key();
      const std::vector<int32> &alignment = reader.Value();

      std::vector<std::vector<int32> > split;
      SplitToPhones(trans_model, alignment, &split);

     if (!write_lengths) {
        std::vector<int32> phones;
        for (size_t i = 0; i < split.size(); i++) {
          KALDI_ASSERT(!split[i].empty());
          int32 phone = trans_model.TransitionIdToPhone(split[i][0]);
          int32 num_repeats = split[i].size();
          //KALDI_ASSERT(num_repeats!=0);
          if (per_frame)
            for(int32 j = 0; j < num_repeats; j++)
              phones.push_back(phone);
          else
            phones.push_back(phone);
        }
        phones_writer.Write(key, phones);
      } else {
        std::vector<std::pair<int32, int32> > pairs;
        for (size_t i = 0; i < split.size(); i++) {
          KALDI_ASSERT(split[i].size() > 0);
          int32 phone = trans_model.TransitionIdToPhone(split[i][0]);
          int32 num_repeats = split[i].size();
          //KALDI_ASSERT(num_repeats!=0);
          pairs.push_back(std::make_pair(phone, num_repeats));
        }
        pair_writer.Write(key, pairs);
      }
      n_done++;
    }
    cout << "Done " << n_done << " utterances.";
  } catch(const std::exception &e) {
    std::cerr << e.what();
  }
}


void DecodeUtterance(string HCLG, string rootDir, string latticePath, string tempDir, string uttId)
{
cout<<"-----------------------------------------------------------"<<endl;
//beam=10.0 lattice-beam=6
  string command="gmm-latgen-faster --max-active=2000 --beam=20.0 "
  "--lattice-beam=10 --acoustic-scale=0.083333 --allow-partial=true "
  "--word-symbol-table=exp/model/words.txt "
  "exp/model/final.alimdl exp/graph/HCXLG.fst \""
  "ark,s,cs:apply-cmvn --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats --left-context=3 "
  "--right-context=3 ark:- ark:- | transform-feats "
  "exp/model/final.mat ark:- ark:- |\" "
  "\"ark:|gzip -c > ";
  command+=latticePath;
  command+=".gz\"";
/*  command+=tempDir;
  command+="/lat.si.gz\""; */
 int c=system(command.c_str());
 cout<<"=======================";
 cout<<c<<endl;
 cout<<"=======================";


/*  command+=" > exp/out/log/";
  command+=uttId;
  command+="_decode1";

  //cout<<command<<endl;
 // getchar();
 

cout<<"-----------------------------------------------------------"<<endl;

  command="gunzip -c ";
  command+=tempDir;
  command+="/lat.si.gz | lattice-to-post --acoustic-scale=0.083333 "
  "ark:- ark:- | weight-silence-post 0.01 1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31:32:33:34:35:36:37:38:39:40:41:42:43:44:45:46:47:48:49:50:51:52:53:54:55 "
  "exp/model/final.alimdl ark:- ark:- "
  "| gmm-post-to-gpost exp/model/final.alimdl "
  "\"ark,s,cs:apply-cmvn --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats --left-context=3 "
  "--right-context=3 ark:- ark:- | transform-feats exp/model/final.mat "
  "ark:- ark:- |\" ark:- ark:- | gmm-est-fmllr-gpost --fmllr-update-type=full "
  "--spk2utt=ark:";
  command+=rootDir;
  command+="/spk2utt exp/model/final.mdl "
  "\"ark,s,cs:apply-cmvn  --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats --left-context=3 "
  "--right-context=3 ark:- ark:- | transform-feats exp/model/final.mat "
  "ark:- ark:- |\" ark,s,cs:- ark:";
  command+=tempDir;
  command+="/pre_trans";
/*  command+=" > exp/out/log/";
  command+=uttId;
  command+="_decode2";


  //cout<<command<<endl;
  //getchar();
  c=system(command.c_str());
  
cout<<"-----------------------------------------------------------"<<endl;

  //beam=13 lattice-beam=6.0
  command="gmm-latgen-faster --max-active=7000 --beam=20.0 "
  "--lattice-beam=10.0 --acoustic-scale=0.083333 "
  "--determinize-lattice=false --allow-partial=true "
  "--word-symbol-table=exp/model/words.txt "
  "exp/model/final.mdl exp/graph/HCXLG.fst "
  "\"ark,s,cs:apply-cmvn  --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats "
  "--left-context=3 --right-context=3 ark:- ark:- | "
  "transform-feats exp/model/final.mat ark:- ark:- | "
  "transform-feats --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk ark:";
  command+=tempDir;
  command+="/pre_trans ark:- ark:- |\" "
  "\"ark:|gzip -c > ";
  command+=tempDir;
  command+="/pre.lat.gz\"";
/*  command+=" > exp/out/log/";
  command+=uttId;
  command+="_decode3";

  //cout<<command<<endl;
  //getchar();
   c=system(command.c_str());
cout<<"-----------------------------------------------------------"<<endl;

  //beam=4.0
  command="lattice-determinize-pruned --acoustic-scale=0.083333 --beam=20.0 "
  "\"ark:gunzip -c ";
  command+=tempDir;
  command+="/pre.lat.gz |\" ark:- | lattice-to-post --acoustic-scale=0.083333 ark:- "
  "ark:- | weight-silence-post 0.01 1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31:32:33:34:35:36:37:38:39:40:41:42:43:44:45:46:47:48:49:50:51:52:53:54:55 "
  "exp/model/final.mdl ark:- ark:- | gmm-est-fmllr --fmllr-update-type=full --spk2utt=ark:";
  command+=rootDir;
  command+="/spk2utt exp/model/final.mdl "
  "\"ark,s,cs:apply-cmvn --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats --left-context=3 --right-context=3 "
  "ark:- ark:- | transform-feats exp/model/final.mat ark:- ark:- | "
  "transform-feats --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk ark:";
  command+=tempDir;
  command+="/pre_trans ark:- ark:- |\" ark,s,cs:- ark:";
  command+=tempDir;
  command+="/trans_tmp && compose-transforms --b-is-affine=true ark:";
  command+=tempDir;
  command+="/trans_tmp ark:";
  command+=tempDir;
  command+="/pre_trans ark:";
  command+=tempDir;
  command+="/trans"; 

  //cout<<command<<endl;
  //getchar();
  c=system(command.c_str());


  //exit(1);

  command="gmm-rescore-lattice exp/model/final.mdl \"ark:gunzip -c ";
  command+=tempDir;
  command+="/pre.lat.gz|\" \"ark,s,cs:apply-cmvn --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk scp:";
  command+=rootDir;
  command+="/cmvn.scp scp:";
  command+=rootDir;
  command+="/feats.scp ark:- | splice-feats --left-context=3 "
  "--right-context=3 ark:- ark:- | transform-feats exp/model/final.mat "
  "ark:- ark:- | transform-feats --utt2spk=ark:";
  command+=rootDir;
  command+="/utt2spk ark:";
  command+=tempDir;
  command+="/trans ark:- ark:- |\" ark:- | "
  "lattice-determinize-pruned --acoustic-scale=0.083333 "
  "--beam=20.0 ark:- \"ark:|gzip -c > ";
  //beam=6.0
  command+=latticePath;
  command+=".gz\" && rm ";
  command+=tempDir;
  command+="/pre.lat.gz";
/*  command+=" > exp/out/log/";
  command+=uttId;
  command+="_decode4";

  //cout<<command<<endl;
  //getchar();*/
  c=system(command.c_str());
//  */
cout<<"-----------------------------------------------------------"<<endl;
  

  command="gunzip -c ";
  command+=latticePath;
  command+=".gz > ";
  command+=latticePath;

  //cout<<command<<endl;
  //getchar();
  c=system(command.c_str());
 cout<<"-----------------------------------------------------------"<<endl; 

/*
  string command1="gmm-latgen-faster --max-active=7000 --beam=13.0"
  " --lattice-beam=6.0 --acoustic-scale=0.083333"
  " --determinize-lattice=false --allow-partial=true"
  " --word-symbol-table=exp/model/words.txt"
  " exp/model/final.alimdl ";
  string command10=HCLG;
  string command11=" \"ark,s,cs:apply-cmvn --utt2spk=ark:";
  string command2=rootDir;
  string command3="/utt2spk scp:";
  string command4=rootDir;
  string command5="/cmvn.scp scp:";
  string command6=rootDir;
  string command7="/feats.scp ark:- | splice-feats --left-context=3"
  " --right-context=3 ark:- ark:- |"
  " transform-feats exp/model/final.mat"
  " ark:- ark:- |\" ark:";
  string command8=latticePath;

  string command=command1+command10+command11+command2+command3+command4+command5+command6
    +command7+command8;
*/
  //cout<<endl<<command<<endl;
  //getchar();
  //int c=system(command.c_str());
  //cout<<c<<endl;
}

void Score(string latticePath, string testDir)
{

    string comm1="cat ";
    comm1+=testDir;
    comm1+="/text | sed 's:<NOISE>::g'";
    comm1+=" | sed 's:<SPOKEN_NOISE>::g' > ";
    comm1+=latticePath;
    comm1+="/scoring/test_filt.txt";

    //cout<<comm1<<endl;
    //getchar();
    int p=system(comm1.c_str());

    


    int min_lmwt=1;
    int max_lmwt=20;
    for(int i=min_lmwt;i<=max_lmwt;i++)
    {
      string command1="lattice-scale --inv-acoustic-scale=1 \"ark:gunzip -c ";
      string command2=latticePath;
      command2+="/output.lat.gz";
      string command3=" |\" ark:- | lattice-add-penalty --word-ins-penalty=0 ark:- ark:- |"
      " lattice-best-path --word-symbol-table=exp/graph/words.txt ark:-"
      " ark,t:exp/out/evaluate/scoring/";
      string command4=ConvertToString(i);
      string command5=".tra "; 
      string command=command1+command2+command3+command4+command5;


      //cout<<command<<endl;
      //getchar();
      int c=system(command.c_str());

    }

    for(int i=min_lmwt;i<=max_lmwt;i++)
    {
      string command1="cat ";
      command1+=latticePath;
      command1+="/scoring/";
      string command2=ConvertToString(i);
     string command3=".tra | ../utils/int2sym.pl -f 2- exp/graph/words.txt | sed 's|\bfb_.......\b||g;s|sil||g;"
     "s|\bfb_......\b||g;s|\bfb_.....\b||g;s|\bfb_....\b||g;"
     "s|\bfb_...\b||g;s|\bfb_..\b||g;s|<s>||g;s|</s>||g;"
     "s|<pau>||g;s|<aah>||g;s|<hmm>||g;s|<hm>||g;s|<laugh>||g;"
     "s|<vn>||g;s|<babble>||g;s|<horn>||g;s|<bang>||g;"
     "s|<bn>||g;s|sil||g;' | compute-wer --text --mode=present"
     " ark:";
     string command4=latticePath;
     command4+="/scoring/test_filt.txt"
     " ark,p:- &> exp/out/evaluate/wer_";
     string command5=ConvertToString(i); 

     string command=command1+command2+command3+command4+command5;

     //cout<<command<<endl;
     //getchar();
     int c=system(command.c_str());
  
    }
    
}







/*int main()
{
    Score("exp/out/evaluate", "data/test");
}*/
