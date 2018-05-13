//run path.sh before runing this
/*
export LD_LIBRARY_PATH=/usr/lib/atlas-base:/net/voxel10/misc/extra/data/software/kaldi/src/lib:/net/voxel10/misc/extra/data/software/kaldi/tools/openfst-1.6.2/src/lib/.libs:$LD_LIBRARY_PATH
*/
/*Compile on Linux
g++ --std=c++11 -I/net/voxel10/misc/extra/data/software/kaldi/tools/openfst-1.6.2/src/include -I/net/voxel10/misc/extra/data/software/kaldi/src -I/net/voxel10/misc/extra/data/software/kaldi/tools/CLAPACK/ -g -O3 final.cpp -o ./final /net/voxel10/misc/extra/data/software/kaldi/tools/openfst-1.6.2/src/lib/.libs/libfst.so* /net/voxel10/misc/extra/data/software/kaldi/src/lib/*.so -ldl -lpthread
*/


/*

Made all the RemoveEpsilonAndDeterminize to use log,
that means the input argument is used the the next steps
Removed monophone update weights
*/


#include "fst-utils.cpp"
#include "map-utils.cpp"
#include "kaldi-utils.cpp"
#include "utils.cpp"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fst/fstlib.h>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand


using namespace std;
using namespace fst;

typedef map<pair<string, string >, string > M;

int main(int argc, char** argv)
{

	srand ( unsigned ( time(0) ) );

	//take arguments from command line instead of this..
	string XPath="exp/requiredFiles/X_with_markers_all_data.fst";
	string LPath="exp/requiredFiles/L_disambig.fst";
	string GPath="exp/requiredFiles/G.fst";
	string dataPath="data/train_without_disambig";
	//string dataPath="data/train_with_disambig";

	string phoneSetFile="exp/graph/phones.txt";

	//program constants
	const int numOfEpochs=2;
	const float importanceScale=5.0;
	const int minibatchSize=1;
	const int backoffState=0;
	const int dataReductionFactor=10;
	int dataPoint;

	//maps to store some things
	map<int, int> stateToPhone;
	map<int, string> indexToPhone;
	map<int, int> totalPhoneCountFromTrain;

	//variables used in the program for storing different fsts
	static const int arr[] = {333,334,335,336,337};
	vector<int> disambigs(arr, arr + sizeof(arr)/sizeof(arr[0]));
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

	//Read the FSTs into variables
	PrintMessage("Reading all the fsts");
	StdVectorFst* X = StdVectorFst::Read(XPath);
	StdVectorFst* XAveraged = StdVectorFst::Read(XPath);
	StdVectorFst* L = StdVectorFst::Read(LPath);
	StdVectorFst* G = StdVectorFst::Read(GPath);

	//Set the weights of X to 0
	//Function is defined fst-utils.cpp
	PrintMessage("Setting X weights to Zeros");
	AddSelfLoopsToX(X,disambigs);
	SetFstWeightsToZero(X);
	AddSelfLoopsToX(XAveraged,disambigs);
	SetFstWeightsToZero(XAveraged);



	string file="exp/X/X_initial.txt";
	const char* fileName=file.c_str();
	PrintFstWeights(X, fileName);


	/*
	Know the previous phone based on the current state as current state corresponds to a particular previous phone.
	GetStateToPhoneMap defined in fstutils.cpp
	*/
	GetStateToPhoneMap(X, stateToPhone);

	/*string fileName="exp/StateIndexToPhone";
	const char* cfileName1=fileName.c_str();
	ofstream myFile;
	myFile.open(cfileName1);
	for(map<int, int>::iterator it=stateToPhone.begin(); it!=stateToPhone.end(); ++it )
	{
		myFile<<"State:"<<it->first<<" Phone:"<<it->second<<endl;
	}
	myFile.close();
	getchar();*/
	

	/*
	Get the phone symbol using the phone index...
	Used for Logging purposes
	*/	
	GetPhoneMap(phoneSetFile, indexToPhone);
	/*fileName="exp/PhoneIndexToPhone";
	const char* cfileName=fileName.c_str();
	myFile.open(cfileName);
	for(map<int, string>::iterator it=indexToPhone.begin(); it!=indexToPhone.end(); ++it )
	{
		myFile<<"Index:"<<it->first<<" Phone:"<<it->second<<endl;
	}
	myFile.close();
	getchar();
	exit(1);*/

	//Initialize the totalPhoneCountFromTrain
	for(map<int, string>::iterator it=indexToPhone.begin(); it!=indexToPhone.end(); ++it )
	{
		totalPhoneCountFromTrain[it->first]=0;
	}

	/*
	Read the training data in vector
	DataPoint class is defined in utils.cpp
	ReadDataDir is defined in utils.cpp
	*/
	PrintMessage("Reading Data Objects into Memory");
	ReadDataDir(dataPath, data);
	
	/*
	Composing L and G, Determinizing, minimizing and sorting the result
	ComposeTwoFst defined in fst-utils.cpp
	RemoveEpsilonAndDeterminize defined in fst-utils.cpp
	MinimizeFst defined in fst-utils.cpp
	*/ 
	PrintMessage("Composition and Minimization of L & G");
	ComposeTwoFsts(L, G, &LG);
	LG.Write("exp/out/LG.fst");
	//RemoveEpsilonAndDeterminize makes changes in the  argument itself
	RemoveEpsilonAndDeterminize(true, &LG, &LGDet);
	PrintMessage("Minimization of LG");
	MinimizeFst(&LG);
	//MinimizeFst(&LGDet);
	//ArcSort(&LG, StdILabelCompare());
	//LGDet.Write("exp/out/LGDet.fst");
	delete L;
	delete G;
	cout<<"==========================================================="<<endl;
	string mess="Will run for " + ConvertToString(numOfEpochs) + " Epochs with randomly shuffled " + ConvertToString(data.size()/dataReductionFactor) + " utterances";
	PrintMessage(mess);
	cout<<"==========================================================="<<endl;


	for(int epoch_count=0;epoch_count<numOfEpochs;++epoch_count)
	{
		int multiplier=data.size()/dataReductionFactor;
		random_shuffle(data.begin(), data.end(), myrandom);
		cout<<"==========================================================="<<endl;
		string mess="Starting Epoch : " + ConvertToString(epoch_count+1);
		PrintMessage(mess);
		cout<<"==========================================================="<<endl;
		for(dataPoint=1;dataPoint<=data.size()/dataReductionFactor;++dataPoint)
		{
			//break;
			/*if(data[dataPoint].uttId!="M13MH33A3219C07900" && data[dataPoint].uttId!="F13MH12A1134C18307" && data[dataPoint].uttId!="F13MH31A3034C31703")
				continue;*/

			cout<<"-----------------------------------------------------------"<<endl;
			mess="Doing for Utterance : " + ConvertToString(dataPoint);
			mess+=" : ";
			mess+=data[dataPoint].uttId;
			PrintMessage(mess);
			cout<<"-----------------------------------------------------------"<<endl;


			string rootDir="exp/data";
			data[dataPoint].CreateDataFolder(rootDir);

			/*
			Composing X and LG, determinizing, minimizing and sorting the result
			ComposeToFst defined in fst-utils.cpp
			RemoveEpsilonAndDeterminize defined in fst-utils.cpp
			MinimizeFst defined in fst-utils.cpp
			*/ 
			PrintMessage("Composition and Determinization of X & LG");
			ComposeTwoFsts(X, &LG, &XLG);
			XLG.Write("exp/out/XLG.fst");
			RemoveEpsilonAndDeterminize(true, &XLG, &XLGDet);
			MinimizeFst(&XLG);
			//ArcSort(&XLG, StdILabelCompare());
			XLG.Write("exp/out/XLG.fst");

			/*
			Composing C and XLG
			LeftComposingWithC is defined in fst-utils.cpp
			*/ 
			string disambigOutFilename="exp/lang/tmp/ilabels_3_1";
			PrintMessage("Composition of C & XLG");
			LeftComposeWithC(&XLG, disambigOutFilename, &CXLG);
			CXLG.Write("exp/out/CXLG.fst");
			//LeftComposeWithC(&XLG, disambigOutFilename, &CXLG);

			/*
			Making the H transducer
			MakeHTransducer is defied in kaldi-utils.cpp
			*/
			PrintMessage("Making H transducer");
			string disambigSymsFile="exp/graph/disambig_tid.int";
			string treePath="exp/model/tree";
			string modelPath="exp/model/final.mdl";
			H=MakeHTransducer(disambigSymsFile, disambigOutFilename, treePath, modelPath);
			//H->Write("exp/out/H.fst");
			
			/*
			Composing H and CXLG, determinizing, minimizing and sorting the result
			ComposeToFst defined in fst-utils.cpp
			RemoveEpsilonAndDeterminize defined in fst-utils.cpp
			RemoveDisambigSymbols is defined in fst-utils.cpp
			RemoveLocalEpsilons is defined in fst-utils.cpp
			MinimizeFst defined in fst-utils.cpp
			AddSelfLoopsToGraph is defined in kaldi-utils.cpp
			*/ 
			PrintMessage("Making HCXLG Graph");
			//ArcSort(H, StdOLabelCompare());
			ComposeTwoFsts(H, &CXLG, &HCXLG);
			RemoveEpsilonAndDeterminize(true, &HCXLG, &HCXLGDet);
			RemoveDisambigSymbols(disambigSymsFile, &HCXLG);
			RemoveLocalEpsilons(&HCXLG);
			MinimizeFst(&HCXLG);
			AddSelfLoopsToGraph(modelPath, &HCXLG);

			string HCLGPath="exp/graph/HCXLG.fst";
			string latticePath="exp/out/";
			latticePath+=data[dataPoint].uttId;
			latticePath+=".lat";
			string tempDir="exp/out/temp";
			HCXLG.Write(HCLGPath);
			//cout<<"Done"<<endl;

			//getchar();

			/*
			Decoding the utterance and generating the lattice
			DecodeUtterance defined in kaldi-utils.cpp
			*/ 
			PrintMessage("Lattice Creation");
			DecodeUtterance(HCLGPath, rootDir, latticePath, tempDir, data[dataPoint].uttId);

			if (FileExists(latticePath))
			{
				/*
				Converting the lattice into phone lattice and getting the phone sequence
				LatticeOneBest defined in kaldi-utils.cpp
				NBestToLinear defined in kaldi-utils.cpp
				AliToPhones defined in kaldi-utils.cpp
				*/ 
				PrintMessage("Getting Phone Sequence");
				string aliModelPath="exp/model/final.alimdl";
				string bestPathLattice="exp/out/";
				bestPathLattice+=data[dataPoint].uttId;
				bestPathLattice+="_bestPath.lat";
				string alignmentPath="exp/out/";
				alignmentPath+=data[dataPoint].uttId;
				alignmentPath+=".ali";
				string transcriptPath="exp/out/";
				transcriptPath+=data[dataPoint].uttId;
				transcriptPath+=".trans";
				LatticeOneBest(latticePath, bestPathLattice);
				cout<<endl;
				NBestToLinear(bestPathLattice, alignmentPath);
				cout<<endl;
				AliToPhones(aliModelPath, alignmentPath, transcriptPath);
				cout<<endl;

				/*
				Getting the reference and decoded phone sequences
				GetStringFromTranscriptFile is defined in map-utils.cpp
				*/ 
				PrintMessage("Getting the string for decoded..");
				string decoded="";
				GetStringFromTranscriptFile(transcriptPath, decoded);
				PrintMessage(decoded);
				PrintMessage("Getting the string for reference..");
				string reference="";
				string path=rootDir+"/text";
				GetStringFromTranscriptFile(path, reference);
				PrintMessage(reference);

				//getchar();

				//Get total Count of phones appeared in training till now
				CountTotalPhones(reference, totalPhoneCountFromTrain);

				/*
				Updating the weights of X
				UpdateFstWeights defined in fst-utils
				GetAveragedFst defined in fst-utils
				*/

				UpdateFstWeights(reference, true, importanceScale, backoffState, X, epoch_count, dataPoint, stateToPhone, indexToPhone);
				UpdateFstWeights(decoded, false, importanceScale, backoffState, X, epoch_count, dataPoint, stateToPhone, indexToPhone);
				GetAveragedFst((float)((epoch_count*multiplier)+(dataPoint-1.0)), X, XAveraged, dataPoint);

				/*
				Writing the updated X and averaged X to files
				PrintFstWeights is defined in fst-utils.cpp
				*/
				if(dataPoint%10==0)
				{
					string file="exp/X/X_";
					file+=ConvertToString(epoch_count+1);
					file+="_";
					file+=ConvertToString(dataPoint);
					file+=".txt";
					const char* fileName=file.c_str();
					PrintFstWeights(X, fileName);


					file="exp/X_averaged/X_";
					file+=ConvertToString(epoch_count+1);
					file+="_";
					file+=ConvertToString(dataPoint);
					file+=".txt";
					const char* fileNameAveraged=file.c_str();
					PrintFstWeights(XAveraged, fileNameAveraged);

					file="exp/total_phone_count/total_";
					file+=ConvertToString(epoch_count+1);
					file+="_";
					file+=ConvertToString(dataPoint);
					file+=".txt";
					const char* totalCountFileName=file.c_str();
					PrintTotalCountOfPhones(totalPhoneCountFromTrain, indexToPhone, totalCountFileName);

				}
				

				cout<<"-----------------------------------------------------------"<<endl;
				mess="Done for Utterance : " + ConvertToString(dataPoint);
				PrintMessage(mess);
				cout<<"-----------------------------------------------------------"<<endl;
			}
			else
			{
				mess="Lattice creation unsuccessful for : " + ConvertToString(dataPoint);
				PrintMessage(mess);
			}
			//getchar();
		}

		string comm="rm exp/out/*.lat";
		int p=system(comm.c_str());
		comm="rm exp/out/*.lat.gz";
		p=system(comm.c_str());
		comm="rm exp/out/*.trans";
		p=system(comm.c_str());
		comm="rm exp/out/*.ali";
		p=system(comm.c_str());

		cout<<"==========================================================="<<endl;
		mess="Epoch Completed: " + ConvertToString(epoch_count+1);
		PrintMessage(mess);
		cout<<"==========================================================="<<endl;

		PrintMessage("Evaluating on the test set with the averaged weights:");

		
		PrintMessage("Composition and Determinization of X & LG");
		ComposeTwoFsts(XAveraged, &LG, &XLG);
		RemoveEpsilonAndDeterminize(true, &XLG, &XLGDet);
		MinimizeFst(&XLG);
		ArcSort(&XLG, StdILabelCompare());

		string disambigOutFilename="exp/lang/tmp/ilabels_3_1";
		PrintMessage("Composition of C & XLG");
		

		LeftComposeWithC(&XLG, disambigOutFilename, &CXLG);

		PrintMessage("Making H transducer");
		string disambigSymsFile="exp/graph/disambig_tid.int";
		string treePath="exp/model/tree";
		string modelPath="exp/model/final.mdl";
		H=MakeHTransducer(disambigSymsFile, disambigOutFilename, treePath, modelPath);

		PrintMessage("Making HCXLG Graph");
		ArcSort(H, StdOLabelCompare());
		ComposeTwoFsts(H, &CXLG, &HCXLG);
		RemoveEpsilonAndDeterminize(true, &HCXLG, &HCXLGDet);
		RemoveDisambigSymbols(disambigSymsFile, &HCXLG);
		RemoveLocalEpsilons(&HCXLG);
		MinimizeFst(&HCXLG);
		AddSelfLoopsToGraph(modelPath, &HCXLG);

		string evaluatePath="exp/out/evaluate";
		string evaluateLatticePath=evaluatePath+"/output.lat";
		string HCLGPath=evaluatePath+"/HCXLG.fst";
		string testDir="data/test";
		string tempDir="exp/out/temp";
		HCXLGDet.Write(HCLGPath);
		PrintMessage("Lattice Creation");
		string uttId="evaluate"+ConvertToString(epoch_count);
		DecodeUtterance(HCLGPath, testDir, evaluateLatticePath, tempDir, uttId);
	    Score(evaluatePath, testDir);
	    //getchar();
	}
}