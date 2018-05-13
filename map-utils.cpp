//g++ --std=c++11 -I/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/include -I/home/abhinavj/Documents/seminarAsr/src -g -O3 -o ./maputils map-utils.cpp /home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs/libfst.so /home/abhinavj/Documents/seminarAsr/src/lib/*.so -ldl -lpthread
//export LD_LIBRARY_PATH=/home/abhinavj/Documents/seminarAsr/src/lib:/home/abhinavj/Documents/seminarAsr/tools/openfst-1.3.4/src/lib/.libs:$LD_LIBRARY_PATH


#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cstdlib>

#include "utils.cpp"

using namespace std;

typedef map<pair<string, string >, string > M;

void GenerateMapsForFeatureGeneration(const char *fileName,
            map<string, string>& phoneToState,
           vector<float>& weights,
 					M& inputToWeight, M& stateToState,
 				map<string, string>& finalStateWeights)
{
    ifstream myFile(fileName);
    string delimiter="\t";
    string token;
    int pos=0;
    string line;
    if(myFile.is_open())
    {
    	vector<string> lineVector;
    	int x=0;
      	while(getline(myFile, line))
      	{
          	lineVector.clear();
          	while((pos=line.find(delimiter))!=string::npos)
          	{
            	token=line.substr(0,pos);
            	lineVector.push_back(token);
            	line.erase(0,pos+delimiter.length());
          	}
          	lineVector.push_back(line);

          	if(lineVector.size()<5)
          	{
          		finalStateWeights[lineVector[0]]=lineVector[1];
          		continue;
          	}
          //if(stateToState.find(make_pair(lineVector[0], lineVector[1]))!=stateToState.end())
            stateToState[make_pair(lineVector[0], lineVector[2])]=lineVector[1];
            phoneToState[lineVector[2]]=lineVector[1];
          //if(inputToWeight[make_pair(lineVector[0], lineVector[2])]!=stateToState.end())
            inputToWeight[make_pair(lineVector[0], lineVector[2])]=lineVector[4];
      }
    }

    for(M::iterator it=inputToWeight.begin(); it!=inputToWeight.end(); ++it)
    {
        weights.push_back(atof((it->second).c_str())); 
    }

     /*for(auto const &val:inputToWeight)
    {
      	weights.push_back(stof(val.second));
    }*/ 
    myFile.close();
}


void GetFeatureVector(string strUntrimmed, M& stateToState, int backoffState,
					 vector<int>& feature)
{
	bool bigramFound=true;
	bool monogramFound=true;
	//feature.resize(stateToState.size());
  	string str=trim(strUntrimmed);
	string delimiter=" ";
	string currentState="1";
	int pos=0;
	string token="";
	while((pos=str.find(delimiter))!=string::npos)
  	{
    	token=str.substr(0,pos);
    	//PrintMessage("Phone:"+token);
    	int indexBigram=distance(stateToState.begin(),
    	 					  stateToState.find(make_pair(currentState, token)));
    	int indexMonogram=distance(stateToState.begin(),
    	 					  stateToState.find(make_pair("0", token)));

    	if(indexBigram==feature.size())
    	{
    		bigramFound=false;
    	}
    	if(indexMonogram==feature.size())
    	{
    		monogramFound=false;
    	}


    	if(bigramFound)
      {
      	//cout<<"BigramIndex:"<<indexBigram<<endl;
      	feature[indexBigram]=1;
    	currentState=stateToState[make_pair(currentState, token)];
      }
      if(monogramFound)
      {
      	//cout<<"MonogramIndex:"<<indexMonogram<<endl;
      	feature[indexMonogram]=1;
      }

      if(!(monogramFound || bigramFound))
      {
      	PrintMessage("Feature not found, going to state 0 for the next phone..");
      	currentState=ConvertToString(backoffState);
      }

		bigramFound=true;
    	monogramFound=true;
    	str.erase(0,pos+delimiter.length());

      /*if(dist==feature.size())
      {
      	if(bigramFound)
      	{
      		bigramFound=false;
      		currentState=ConvertToString(backoffState);
        	continue;	
      	}
      	else
      	{
      		monogramFound=false;
      		cout<<"Monogram also not found"<<endl;
      		currentState=ConvertToString(backoffState);
      	}*/
        
      }
      //PrintMessage("Phone:"+str);
      int indexBigram=distance(stateToState.begin(),
    	 					  stateToState.find(make_pair(currentState, str)));
    	int indexMonogram=distance(stateToState.begin(),
    	 					  stateToState.find(make_pair("0", str)));

    	if(indexBigram==feature.size())
    	{
    		bigramFound=false;
    	}
    	if(indexMonogram==feature.size())
    	{
    		monogramFound=false;
    	}


    	if(bigramFound)
      {
      	//cout<<"BigramIndex:"<<indexBigram<<endl;
      	feature[indexBigram]=1;
      }
      if(monogramFound)
      {
      	//cout<<"MonogramIndex:"<<indexMonogram<<endl;
      	feature[indexMonogram]=1;
      }

      if(!(monogramFound || bigramFound))
      {
      	PrintMessage("Feature not found, going to state 0 for the next phone..");
      }
    	
    	
      //cout<<str<<endl;
  	
  	/*
  	int dist=distance(stateToState.begin(),
    	 				  stateToState.find(make_pair(currentState, str)));
    if(dist==feature.size())
    {
      currentState=ConvertToString(backoffState);
      dist=distance(stateToState.begin(),
                stateToState.find(make_pair(currentState, str)));
    }
    //cout<<dist<<endl;

    //Do something here


	if(dist==feature.size())
    {
      feature[dist]=1;
    }*/
   	
}

void UpdateWeightVector(vector<int>& feature, bool whatToDo, int importanceScale, vector<float>& weight)
{
	if(whatToDo)
	{
		for(int i=0;i<feature.size();i++)
		{
			weight[i]+=importanceScale*feature[i];
		}
	}
	else
	{
		for(int i=0;i<feature.size();i++)
		{
			weight[i]-=importanceScale*feature[i];
		}	
	}
}


void MapToTextFst(M& inputToWeight, M& stateToState,
					 map<string, string >& finalStateWeights,
					 const char *fileName)
{
	ofstream myFile;
	myFile.open(fileName);
    string delimiter="\t";
    string lineSeparator="\n";
    int x=0;
	for(pair<M::iterator,M::iterator> it(inputToWeight.begin(), stateToState.begin());
		it.first != inputToWeight.end();
		++it.first,++it.second)
	{
		//cout<<"Input To weight Map:"<<endl;
		//cout<<"<"<<((it.first)->first).first<<","<<((it.first)->first).second<<">,<"<<(it.first)->second<<">"<<endl;
		//cout<<"State to State Map:"<<endl;
		//cout<<"<"<<((it.second)->first).first<<","<<((it.second)->first).second<<">,<"<<(it.second)->second<<">"<<endl;
		myFile << (((it.second)->first).first) << delimiter << ((it.second)->second);
		myFile << delimiter << (((it.first)->first).second) << delimiter;
		myFile << (((it.first)->first).second) << delimiter << ((it.first)->second);
		myFile << lineSeparator;

	}

  for(map<string, string >::iterator it=finalStateWeights.begin(); it!=finalStateWeights.end(); ++it)
  {
    myFile << it->first << delimiter << it->second << lineSeparator;
  }


	/*for(auto const &val:finalStateWeights)
	{
		myFile << val.first << delimiter << val.second << lineSeparator;
	}*/
	myFile.close();
	//getchar();
}



void GetStringFromTranscriptFile(string fileName, string& decoded)
{
    const char *file = fileName.c_str();
	 ifstream myFile(file);
    string delimiter=" ";
    string token;
    int pos=0;
    string line;
    getline(myFile, line);
    pos=line.find(delimiter);
    line.erase(0,pos+delimiter.length());
    decoded=line;
    myFile.close();
}

/*

int main(int argc, char *argv[])
{
  
	vector<int> feature;
	//string input="10 1 160 40 1 1 40 10";
  string input="1 220 169 328 61 56 322 58 278 294 142 289 1 288 330 62 238 102 169 276 62 293 276 254 134 58 250 62 273 228 62 162 169 10 1 288 314 226 169 220 58 206 61 40";
	char *inputFile="temp.txt";
	//char *outputFile="composed_minimized_re.txt";
	map<pair<string, string >, string > stateToState;
    map<pair<string, string >, string > inputToWeight;
    map<string, string > finalStateWeights;
    vector<float> weights;
	GenerateMapsForFeatureGeneration(inputFile, weights, inputToWeight, stateToState, finalStateWeights);
  cout<<"Done creating the maps"<<endl;
  cout<<"THe weight vector after:"<<endl;
  PrintVector(weights);
	//MapToTextFst(inputToWeight, stateToState, finalStateWeights, outputFile);
	GetFeatureVector(input, stateToState, feature);
	cout<<"Done creating the feature vector"<<endl;
	UpdateWeightVector(feature, true, 1, weights);
  cout<<"THe faeture vector after:"<<endl;
	PrintVector(feature);
  cout<<"THe weight vector after:"<<endl;
  PrintVector(weights);
  
}
*/