//export LD_LIBRARY_PATH=/usr/lib/atlas-base:/home/abhinavj/Documents/seminarAsr/src/lib:/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs:$LD_LIBRARY_PATH
//Compile on Linux
//g++ -I/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/include -I/home/abhinavj/Documents/seminarAsr/src -I/home/abhinavj/Documents/seminarAsr/tools/CLAPACK/ -g -O3 main.cpp -o ./run /home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs/libfst.so /home/abhinavj/Documents/seminarAsr/src/lib/*.so -ldl -lpthread


#include "fst-utils.cpp"
#include "map-utils.cpp"
#include "kaldi-utils.cpp"
#include "utils.cpp"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fst/fstlib.h>
using namespace std;
using namespace fst;

typedef map<pair<string, string >, string > M;





//MAIN FUNCTION - GET THE H AS ARGUMENT, GET THE X AS ARGUMENT,
//GET THE L AS ARGUMENT, TAKE THE G AS ARGUMENT, GET THE REFERENCE PHONE SEQUENCES , 
//GET THE DATA LOCATION

int main(int argc, char** argv)
{

  string HPath="exp/graph/Ha.fst";
  string XPath="exp/X_with_markers_new_v2.fst";
  string LPath="exp/L.fst";
  string GPath="exp/G.fst";
  string dataPath="data/train";
  
  int numOfEpochs=2;
  int importanceScale=1;
  int minibatchSize=1;
  int dataPoint;
  int totalDataPoints=13512;
  int backoffState=0;


  /*string HPath=argv[1];
  string XPath=argv[2];
  string LPath=argv[3];
  string GPath=argv[4];
  string datapath=argv[5];
*/


  //StdVectorFst* H = StdVectorFst::Read(HPath);
  //PrintMessage("H is read");
  StdVectorFst* X = StdVectorFst::Read(XPath);
  StdVectorFst* XAveraged = StdVectorFst::Read(XPath);
  PrintMessage("X is read");
  StdVectorFst* L = StdVectorFst::Read(LPath);
  PrintMessage("L is read");
  StdVectorFst* G = StdVectorFst::Read(GPath);
  PrintMessage("G is read");

  vector<DataPoint> data;
  StdVectorFst LG;
  StdVectorFst LGDet;
  StdVectorFst XLG;
  StdVectorFst XLGDet;
  StdVectorFst CXLG;
  StdVectorFst CXLGDet;
  StdVectorFst* H;
  StdVectorFst HCXLG;
  StdVectorFst HCXLGDet;
	//STORE THE BINARY FSTS WHICH COMES AS ARGUMENTS
	/*for (int i = 1; i != argc-1; ++i)
	{
	    StdVectorFst* fst = StdVectorFst::Read(argv[i]);
	    ArcSort(fst, StdOLabelCompare());
	    if (fst == NULL)
	      LOG(FATAL) << "Failed to read fst from : " << argv[i];
	    fsts.push_back(fst);
  	}
*/
  	//WaitForInput("Composition and Determinization of LG");
//X = StdVectorFst::Read(dest);

  PrintMessage("Getting Data objects..");
  	ReadDataDir(dataPath, data);

  	//cout<<data.size()<<endl;
  	//getchar();

    //COMPOSE LG and Determinize
    //ArcSort(L, StdOLabelCompare());
    //ComposeFst<StdArc>* LG;

    //Compose L and G, store it in LG, determinize LG and store in LGDet
    //Minimize LGDet and store it in LGDetMin
    //ArcSort for further composition
    
    PrintMessage("Composition of L & G");
    ComposeTwoFsts(L, G, LG);
    //PrintMessage("Composition of L & G - DONE");
    
    PrintMessage("Determinization of LG");
    RemoveEpsilonAndDeterminize(&LG, LGDet);
    //PrintMessage("Determinization of LG - DONE");

    //MutableFst<Arc> LGDetMin_mut;
    PrintMessage("Minimization of LG");
    MinimizeFst(LGDet);
    ArcSort(&LGDet, StdILabelCompare());
    //LGDet.Write("exp/out/LG.fst");
    //StdVectorFst LGDetMin=(StdVectorFst)LGDetMin_mut;
    //ArcSort(&LGDetMin, StdILabelCompare());
    

  	//COMPOSE HC SO THAT IT NEED NOT BE COMPOSED EVERYTIME.
  	//OUTPUT SORT HC

    //Need a way to get C

    
  	//WaitForInput("Print X and Generating Maps");

    //PRINT X.FST INTO X.TXT
  //GET THE WEIGHTS OF THE FST INTO A VECTOR
  	//PRINT X.FST TO X.TXT, GET THE FEATURE VECTOR FROM THIS TXT SOMEHOW AND CREATE A MAP OF THE FEATURE VECTOR
  
////THIS HAS BEEN COMMENTED RECENTLY#######################################################################################

    /*const char* fileName="exp/X/old1/1/X_3.txt";
    PrintMessage("Printing X");
    PrintFstText(X, fileName);
    //PrintMessage("Printing X - DONE");
    map<pair<string, string >, string > stateToState;
    map<pair<string, string >, string > inputToWeight;
    map<string, string > finalStateWeights;
    vector<float> weights;
    map<string, string> phoneToState;
    PrintMessage("Generating Various Maps from X");
    GenerateMapsForFeatureGeneration(fileName, phoneToState, weights, inputToWeight, stateToState, finalStateWeights);
	
    vector<float> averageWeights(weights.begin(), weights.end());*/
/*


////THIS HAS BEEN COMMENTED RECENTLY##########################################################################################

    for(map<string,string>::iterator it=phoneToState.begin();it!=phoneToState.end();++it)
    {
    	cout<<it->first<<":"<<it->second<<endl;
    }

    getchar();*/

   /* for(int i=0;i<averageWeights.size();i++)
	    {
	    	cout << averageWeights[i] <<endl; 
	    }
	    getchar();*/
    //GetFeatureMap(stateToState);

	//PrintMessage("Generating Various Maps - DONE");
/*
cout<<"The input to weight map is:"<<endl;
    for(M::iterator it=inputToWeight.begin(); it!=inputToWeight.end(); ++it)
	      {
	      	cout << "<" << (it->first).first << "," << (it->first).second << ">:<" << it->second << ">" << endl;
	       
	      }
	      getchar();
cout<<"The state to state map is:"<<endl;
for(M::iterator it=stateToState.begin(); it!=stateToState.end(); ++it)
	      {
	      cout << "<" << (it->first).first << "," << (it->first).second << ">:<" << it->second << ">" << endl;
	      }	      

	      //exit(1);

	//WaitForInput("Start of training");
getchar();
*/

    //DO THIS FOR EACH UTTERANCE 
    for(int epoch_count=0;epoch_count<numOfEpochs;++epoch_count)
    {

    	random_shuffle(data.begin(), data.end());


    	cout<<"==========================================================="<<endl;
    	string mess="Starting Epoch : " + ConvertToString(epoch_count+1);
    	PrintMessage(mess);
    	cout<<"==========================================================="<<endl;
    	for(dataPoint=1;dataPoint<=data.size();++dataPoint)
    	{

    		break;
    		
	        //COMPOSE HCXLG.FST
	      //ComposeFst<StdArc>* XLG;
    		cout<<"-----------------------------------------------------------"<<endl;
    		mess="Doing for Utterance : " + ConvertToString(dataPoint);
    		mess+=" : ";
    		mess+=data[dataPoint].uttId;
    		PrintMessage(mess);
    		cout<<"-----------------------------------------------------------"<<endl;
	     

    		string rootDir="exp/data";
    		data[dataPoint].CreateDataFolder(rootDir);
    		//MakeDataFolder(data[dataPoint], rootDir);

    		//getchar();

	      //Compose X with LGDetMin
	      //determinize XLG and store in XLGDet
	      //Minimize XLGDet and store it in XLGDetMin
	      //ArcSort XLGDetMin for further composition 
	      
	      //ComposeTwoFsts(X, &LGDetMin, XLG);
	      PrintMessage("Composition of X & LG");
	      ComposeTwoFsts(X, &LGDet, XLG);
	      //PrintMessage("Composition of X & LG - DONE");
	      
	      PrintMessage("Determinization of XLG");
	      RemoveEpsilonAndDeterminize(&XLG, XLGDet);
	      //PrintMessage("Determinization of XLG - DONE");
	      //StdVectorFst XLGDetMin;
	      PrintMessage("Minimization of XLG");
	      MinimizeFst(XLGDet);
	      ArcSort(&XLGDet, StdILabelCompare());
	      //XLGDet.Write("exp/out/XLG.fst");
	      //ArcSort(&XLGDetMin, StdILabelCompare());

	      //WaitForInput("Composition of C and XLG, Determinization of CXLG");

	      //Left compose C with XLGDetMin and store in CXLG.
	      //Determinize CXLG and store in CXLGDet
	      //Minimize CXLGDet and store it in CXLGDetMin 
	      string disambigOutFilename="exp/lang/tmp/ilabels_3_1";
	      
	      PrintMessage("Composition of C & XLG");
	      LeftComposeWithC(&XLGDet, disambigOutFilename, CXLG);
	      //PrintMessage("Composition of C & XLG - DONE");
	      //LeftComposeWithC(&XLGDetMin, disambigInFilename, CXLG);
	      
	      PrintMessage("Determinization of CXLG");
	      RemoveEpsilonAndDeterminize(&CXLG, CXLGDet);
	      //PrintMessage("Determinization of CXLG - DONE");
	      //StdVectorFst CXLGDetMin;
	      PrintMessage("Minimization of CXLG");
	      MinimizeFst(CXLGDet);
	      //CXLGDet.Write("exp/out/CXLG.fst");

	      PrintMessage("Making H transducer");
	      string disambigSymsOut="exp/graph/disambig_tid.int";
	      string treePath="exp/model/tree";
	      string modelPath="exp/model/final.mdl";
	      H=MakeHTransducer(disambigSymsOut, disambigOutFilename, treePath, modelPath);
	      //PrintMessage("Making H transducer - DONE");
	      //WaitForInput("Composition of H and CXLG, Determinization of HCXLG");
	      //ArcSort H for composition
	      //Compose H with CXLGDetMin and store in HCXLG
	      //Determinize HCXLG and store it in HCXLGDet
	      //Minimize HCXLGDet and store it in HCXLGDetMin.
	      ArcSort(H, StdOLabelCompare());
	      
	      PrintMessage("Composition of H & CXLG");
	      ComposeTwoFsts(H, &CXLGDet, HCXLG);
	      //PrintMessage("Composition of H & CXLG - DONE");
	      //ComposeTwoFsts(H, &CXLGDetMin, HCXLG);

	      
		  PrintMessage("Determinization of HCXLG");
	      RemoveEpsilonAndDeterminize(&HCXLG, HCXLGDet);
	      //PrintMessage("Determinization of HCXLG - DONE");



	      PrintMessage("Removing disambiguation symbols");
	      string disambigFileName="exp/graph/disambig_tid.int";
	      RemoveDisambigSymbols(disambigFileName, HCXLG);
		  //PrintMessage("Removing disambiguation symbols - DONE");

			//RemoveLocalEpsilons(&HCXLG_Det);
			//RemoveLocalEpsilons(&HCXLG);


	      PrintMessage("Minimizing graph");
	      MinimizeFst(HCXLG);
	      //PrintMessage("Minimizing graph - DONE");


		  PrintMessage("Adding self loops to graph");
	      
	      AddSelfLoopsToGraph(modelPath, HCXLG);
	      //PrintMessage("Adding self loops to graph - DONE");

	       //StdVectorFst HCXLGDet;
	      

	      //StdVectorFst HCXLGDetMin;
	      //Minimize(&HCXLGDet, HCXLGDetMin);
	      //HCXLG.Write("exp/out/HCXLG.fst");


	      string HCLGPath="exp/graph/HCXLG.fst";
	      string latticePath="exp/out/";
	      latticePath+=data[dataPoint].uttId;
	      latticePath+=".lat";
	      string tempDir="exp/out/temp";
	      //HCXLGDet.Write(HCLGPath);
	      HCXLG.Write(HCLGPath);
	      //HCXLGDetMin.Write(HCLGPath);

	      PrintMessage("HCXLG written to file");

	      //GENERATE WORD LATTICE OF THE UTTERANCE
	      PrintMessage("Lattice Creation");
	      DecodeUtterance(HCLGPath, rootDir, latticePath, tempDir, data[dataPoint].uttId);
	      //PrintMessage("Lattice Creation - DONE");

	      //CONVERT WORD LATTICE INTO PHONE LATTICE FST
	      modelPath="exp/model/final.alimdl";
	      string bestPathLattice="exp/out/";
	      bestPathLattice+=data[dataPoint].uttId;
	      bestPathLattice+="_bestPath.lat";
	      string alignmentPath="exp/out/";
	      alignmentPath+=data[dataPoint].uttId;
	      alignmentPath+=".ali";
	      string transcriptPath="exp/out/";
	      transcriptPath+=data[dataPoint].uttId;
	      transcriptPath+=".trans";
	      //ConvertLatticeToPhoneLattice(modelPath, latticePath, phoneLatticePath);

	      PrintMessage("Converting Lattice to One Best");
	      LatticeOneBest(latticePath, bestPathLattice);
	      cout<<endl;
	      PrintMessage("Linearizing..");
	      NBestToLinear(bestPathLattice, alignmentPath);
	      cout<<endl;
	      PrintMessage("Getting Alignment");
	      AliToPhones(modelPath, alignmentPath, transcriptPath);
	      cout<<endl;				
	      //#####################################################################
	      //READ THE TRANSCRIPT FILE TO GET THE DECODED AND REFERENCE STRING

	      PrintMessage("Getting the string for decoded..");
	      string decoded="";
	      GetStringFromTranscriptFile(transcriptPath, decoded);

	      PrintMessage(decoded);

	      PrintMessage("Getting the string for reference..");
	      string reference="";
	      string path=rootDir+"/text";
	      cout<<endl<<path<<endl;
	      GetStringFromTranscriptFile(path, reference);

	      PrintMessage(reference);
	      //WaitForInput("After this, feature vector is created...");
	      //#####################################################################


	      UpdateFstWeights(reference, true, importanceScale, backoffState, X);
	      UpdateFstWeights(decoded, false, importanceScale, backoffState, X);


	      GetAveragedFst(dataPoint, X, XAveraged);


////THIS HAS BEEN COMMENTED RECENTLY#######################################################################################
/*

	      //GENERATE THE FEATURE VECTOR FOR THIS BEST PATH
	      PrintMessage("Getting Feature Vectors for transcriptions..");
	      //cout<<stateToState.size()<<endl;
	      vector<int> decodedFeature(inputToWeight.size());
	      //decodedFeature.resize(inputToWeight.size());
	      GetFeatureVector(decoded, stateToState, backoffState, decodedFeature);
	      //	cout<<stateToState.size()<<endl;

			PrintMessage("Decoded Done..");
	     // WaitForInput("Decoded Feature Vector is done...");

	      //GET THE FEATURE VECTOR OF THE REFERENCE PHONE SEQUENCE(THIS IS THE SEQUENCE WE GOT FROM THE DECODING OF THE PHONE LATTICE FROM HCLG)      
	      vector<int> referenceFeature(inputToWeight.size());
	      //referenceFeature.resize(inputToWeight.size());
	      GetFeatureVector(reference, stateToState, backoffState, referenceFeature);

	      //WaitForInput("Feature Vector is done...");

	      //INCREASE THE WEIGHT OF THE FEATURES WHICH ARE PRESENT IN THE REFERENCE FEATURE VECTOR
	      //DECREASE THE WEIGHT OPF THE FEATURES WHICH ARE PRESENT IN THE DECODED FEATURE VECTOR

	      PrintMessage("Updating Weight Vector....");
	      //incude a minibatch size
	      

	      UpdateWeightVector(referenceFeature, true, importanceScale, weights);
	      UpdateWeightVector(decodedFeature, false, importanceScale, weights);


	      for(int i=0;i<weights.size();i++)
	      {
	      	averageWeights[i]=((averageWeights[i]*dataPoint)+weights[i])/(dataPoint+1);
	      }

	      /*  for(int i=0;i<averageWeights.size();i++)
	    {
	    	cout << averageWeights[i] <<endl; 
	    }
	    //getchar();

	      //INCORPORATE THESE WEIGHTS IN X.TXT
	      int i=0;
	      for(M::iterator it=inputToWeight.begin(); it!=inputToWeight.end(); ++it)
	      {
	        it->second=ConvertToString(weights[i]);
	          i++;
	      }


	      PrintMessage("Writing the X fst to text");
	      //COMPILE THIS TXT TO GENERATE THE MODIFIED X.FST
	      string file="exp/X/";
	      file+=ConvertToString(epoch_count+1);
	      file+="/X_";
	      file+=ConvertToString(dataPoint);
	      file+=".txt";
	      const char* fileName=file.c_str();
	      //remove(fileName);
	      //getchar();
	      MapToTextFst(inputToWeight, stateToState, finalStateWeights, fileName);
	      
			PrintMessage("Waiting for some time...");
	      sleep(1);

	      PrintMessage("Compiling the X fst from text");
	      //CompileFstFromText(fileName, dest);

	      

	      string dest="X.fst";
	      remove(dest.c_str());
	      //getchar();
	      TempCompileFromFst(file, dest);
	      PrintMessage("Waiting for some time...");
	      sleep(1);
	      PrintMessage("Compiling the X fst from text - Done");
	      
	      
	      X=StdVectorFst::Read(dest);
	      PrintMessage("Reading the X fst - Done");
*/
////THIS HAS BEEN COMMENTED RECENTLY#######################################################################################

	      string file="exp/X/";
	      file+=ConvertToString(epoch_count+1);
	      file+="/X_";
	      file+=ConvertToString(dataPoint);
	      file+=".txt";
	      const char* fileName=file.c_str();
	      PrintFstWeights(X, fileName);


	      file="exp/X/X_averaged";;
	      file+="/X_";
	      file+=ConvertToString(dataPoint);
	      file+=".txt";
	      const char* fileNameAveraged=file.c_str();
	      PrintFstWeights(X, fileNameAveraged);

	      getchar();

	      cout<<"-----------------------------------------------------------"<<endl;
    		mess="Done for Utterance : " + ConvertToString(dataPoint);
    		//WaitForInput(mess);
    		cout<<"-----------------------------------------------------------"<<endl;
    		//getchar();
    		break;

	    }
/*
	    string f="exp/out/";
	    f+=ConvertToString(epoch_count);
	    const char *fi=f.c_str();
	    ofstream t;
	    t.open(fi);
	    for(int i=0;i<averageWeights.size();i++)
	    {
	    	t << averageWeights[i] <<endl; 
	    }
*/


	    cout<<"==========================================================="<<endl;
    	mess="Epoch Completed: " + ConvertToString(epoch_count+1);
    	PrintMessage(mess);
    	cout<<"==========================================================="<<endl;


    	PrintMessage("Evaluating on the test set with the averaged weights:");




    	//incorporate changes in the input to weight map
/*
    		int i=0;
	      for(M::iterator it=inputToWeight.begin(); it!=inputToWeight.end(); ++it)
	      {
	        it->second=ConvertToString(weights[i]);
	          i++;
	      }


	      PrintMessage("Writing the X fst to text");
	      //COMPILE THIS TXT TO GENERATE THE MODIFIED X.FST
	      string file="exp/out/evaluate";
	      file+="/X_";
	      file+=ConvertToString(epoch_count);
	      file+=".txt";
	      const char* fileName=file.c_str();
	      //remove(fileName);
	      //getchar();
	      MapToTextFst(inputToWeight, stateToState, finalStateWeights, fileName);
	      
			PrintMessage("Waiting for some time...");
	      sleep(2);

	      PrintMessage("Compiling the X fst from text");
	      //CompileFstFromText(fileName, dest);

	      

	      string dest="exp/out/evaluate/X.fst";
	      //remove(dest.c_str());
	      //getchar();
	      TempCompileFromFst(file, dest);
	      PrintMessage("Waiting for some time...");
	      sleep(1);
	      PrintMessage("Compiling the X fst from text - Done");
	      
	      
	      X=StdVectorFst::Read(dest);
	      PrintMessage("Reading the X fst - Done");



*/







	      //ComposeTwoFsts(X, &LGDetMin, XLG);
	      PrintMessage("Composition of X & LG");
	      ComposeTwoFsts(XAveraged, &LGDet, XLG);
	      //PrintMessage("Composition of X & LG - DONE");
	      
	      PrintMessage("Determinization of XLG");
	      RemoveEpsilonAndDeterminize(&XLG, XLGDet);
	      //PrintMessage("Determinization of XLG - DONE");
	      //StdVectorFst XLGDetMin;
	      PrintMessage("Minimization of XLG");
	      MinimizeFst(XLGDet);
	      ArcSort(&XLGDet, StdILabelCompare());
	      //XLGDet.Write("exp/out/XLG.fst");
	      //ArcSort(&XLGDetMin, StdILabelCompare());

	      //WaitForInput("Composition of C and XLG, Determinization of CXLG");

	      //Left compose C with XLGDetMin and store in CXLG.
	      //Determinize CXLG and store in CXLGDet
	      //Minimize CXLGDet and store it in CXLGDetMin 
	      string disambigOutFilename="exp/lang/tmp/ilabels_3_1";
	      
	      PrintMessage("Composition of C & XLG");
	      LeftComposeWithC(&XLGDet, disambigOutFilename, CXLG);
	      //PrintMessage("Composition of C & XLG - DONE");
	      //LeftComposeWithC(&XLGDetMin, disambigInFilename, CXLG);
	      
	      PrintMessage("Determinization of CXLG");
	      RemoveEpsilonAndDeterminize(&CXLG, CXLGDet);
	      //PrintMessage("Determinization of CXLG - DONE");
	      //StdVectorFst CXLGDetMin;
	      PrintMessage("Minimization of CXLG");
	      MinimizeFst(CXLGDet);
	      //CXLGDet.Write("exp/out/CXLG.fst");

	      PrintMessage("Making H transducer");
	      string disambigSymsOut="exp/graph/disambig_tid.int";
	      string treePath="exp/model/tree";
	      string modelPath="exp/model/final.mdl";
	      H=MakeHTransducer(disambigSymsOut, disambigOutFilename, treePath, modelPath);
	      //PrintMessage("Making H transducer - DONE");
	      //WaitForInput("Composition of H and CXLG, Determinization of HCXLG");
	      //ArcSort H for composition
	      //Compose H with CXLGDetMin and store in HCXLG
	      //Determinize HCXLG and store it in HCXLGDet
	      //Minimize HCXLGDet and store it in HCXLGDetMin.
	      ArcSort(H, StdOLabelCompare());
	      
	      PrintMessage("Composition of H & CXLG");
	      ComposeTwoFsts(H, &CXLGDet, HCXLG);
	      //PrintMessage("Composition of H & CXLG - DONE");
	      //ComposeTwoFsts(H, &CXLGDetMin, HCXLG);

	      
		  PrintMessage("Determinization of HCXLG");
	      RemoveEpsilonAndDeterminize(&HCXLG, HCXLGDet);
	      //PrintMessage("Determinization of HCXLG - DONE");



	      PrintMessage("Removing disambiguation symbols");
	      string disambigFileName="exp/graph/disambig_tid.int";
	      RemoveDisambigSymbols(disambigFileName, HCXLG);
		  //PrintMessage("Removing disambiguation symbols - DONE");

			//RemoveLocalEpsilons(&HCXLG_Det);
			//RemoveLocalEpsilons(&HCXLG);


	      PrintMessage("Minimizing graph");
	      MinimizeFst(HCXLG);
	      //PrintMessage("Minimizing graph - DONE");


		  PrintMessage("Adding self loops to graph");
	      
	      AddSelfLoopsToGraph(modelPath, HCXLG);
	      //PrintMessage("Adding self loops to graph - DONE");

	       //StdVectorFst HCXLGDet;
	      

	      //StdVectorFst HCXLGDetMin;
	      //Minimize(&HCXLGDet, HCXLGDetMin);
	      //HCXLG.Write("exp/out/HCXLG.fst");
	      string evaluatePath="exp/out/evaluate";
	      string evaluateLatticePath=evaluatePath+"/output.lat";
	      string HCLGPath=evaluatePath+"/HCXLG.fst";
	      string testDir="data/test";
	      string tempDir="exp/out/temp";
	      //HCXLGDet.Write(HCLGPath);
	      HCXLG.Write(HCLGPath);
	      //HCXLGDetMin.Write(HCLGPath);

	      PrintMessage("HCXLG written to file");

	      //GENERATE WORD LATTICE OF THE UTTERANCE
	      PrintMessage("Lattice Creation");
	      string uttId="evaluate"+ConvertToString(epoch_count);
	      DecodeUtterance(HCLGPath, testDir, evaluateLatticePath, tempDir, uttId);
	      Score(evaluatePath, testDir);

    	//Do Evaluation on test set


    }
    

}

