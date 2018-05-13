#pragma once

#include <sstream>
#include <vector>
#include <fstream>

typedef map<pair<string, string >, string > M;
using namespace std;


class DataPoint
{
  public:
    string uttId;
    string spkId;
    string wavPath;
    string cmvnPath;
    string featsPath;
    string text;

    DataPoint()
    {
      uttId="";
      spkId="";
      wavPath="";
      cmvnPath="";
      featsPath="";    
      text="";
    }

    DataPoint(string utt, string spk, string wav,
      string cmvn, string feats, string tex) 
    {
      uttId=utt;
      spkId=spk;
      wavPath=wav;
      cmvnPath=cmvn;
      featsPath=feats; 
      text=tex;
    }


    void CreateDataFolder(string root)
  {
    //create 4 files

    string file;
    string delimiter=" ";
    //wav.scp
    file=root+"/wav.scp";
    const char * fileName1 = file.c_str();
    ofstream myFile;
    myFile.open(fileName1);

    myFile << uttId << delimiter << wavPath << endl;

    myFile.close();

    //text
    file=root+"/text";
    const char *fileName2 = file.c_str();
    myFile.open(fileName2);
    myFile << uttId << delimiter << text << endl;

    myFile.close();

    //cmvn.scp
    file=root+"/cmvn.scp";
    const char *fileName3 = file.c_str();
    myFile.open(fileName3);

    myFile << spkId << delimiter << cmvnPath << endl;

    myFile.close();

    //feats.scp
    file=root+"/feats.scp";
    const char *fileName4 = file.c_str();
    myFile.open(fileName4);

  myFile << uttId << delimiter << featsPath << endl;

    myFile.close();

    //utt2spk
    file=root+"/utt2spk";
    const char *fileName5 = file.c_str();
    myFile.open(fileName5);
  myFile << uttId << delimiter << spkId << endl;

    myFile.close();

    //spk2utt
    file=root+"/spk2utt";
    const char *fileName6 = file.c_str();
    myFile.open(fileName6);

  myFile << spkId << delimiter << uttId << endl;

    myFile.close();
  }
};

int myrandom (int i) 
{
  return std::rand()%i;
}


template <class T>
string ConvertToString(T x)
{
  stringstream p;
  p << x;
  return p.str();
}

template <class T>
void PrintVector(vector<T> vec)
{
  for(int i=0;i<vec.size();i++)
  {
    cout<<vec[i]<<"\t";
  }
  cout<<endl;
}

string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

inline void PrintMessage(string mess)
{
  cout<<"------------"<<mess<<"--------------"<<endl;
}


void WaitForInput(string mess)

{  cout<<mess<<endl;
  PrintMessage("Press any key to move to the next step...");
    getchar();
}


inline bool FileExists (const std::string& name) 
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }   
}

void GetPhoneMap(string file, map<int, string>& indexToPhone)
{
  const char* fileName=file.c_str();
  ifstream myFile(fileName);
  string line;
  string delimiter=" ";
  int pos=0;
  string phone;
  while(getline(myFile, line))
  {
      while((pos=line.find(delimiter))!=string::npos)
      {
        phone=line.substr(0,pos);
        line.erase(0,pos+delimiter.length());
      }
      int index=atoi((trim(line)).c_str());
      indexToPhone[index]=phone;
  }
  myFile.close();
}

void CountTotalPhones(string str, map<int, int>& totalPhoneCountFromTrain)
{
    string delimiter=" ";
    string token;
    int pos=0;
    int x=0;
    while((pos=str.find(delimiter))!=string::npos)
    {
      token=str.substr(0,pos);
      x=atoi(token.c_str());
      totalPhoneCountFromTrain[x]++;
      str.erase(0,pos+delimiter.length());
    }
    x=atoi(str.c_str());
    totalPhoneCountFromTrain[x]++;
}

void PrintTotalCountOfPhones(map<int, int>& totalPhoneCountFromTrain, map<int, string>& indexToPhone, const char* fileName)
{
  ofstream myFile;
  myFile.open(fileName);
  string delimiter=" ";
  
  for(map<int, int>::iterator it=totalPhoneCountFromTrain.begin(); it!=totalPhoneCountFromTrain.end(); ++it)
  {
    myFile << indexToPhone[it->first] << delimiter << it->second << endl;
  }

   myFile.close();
}


/*
void GetFeatureMap(M& myMap)
{
     int largestState=-1;
     int largestPhoneIndex=-1;
     int totalCount=0;

     for(M::iterator it=myMap.begin(); it!=myMap.end(); ++it)
        {
          if(atoi(((it->first).first).c_str()) > largestState)
          {
            largestState=atoi(((it->first).first).c_str());
          }
          if(atoi(((it->first).second).c_str()) > largestPhoneIndex)
          {
           largestPhoneIndex=atoi(((it->first).second).c_str()); 
          }
        }

        cout<<largestState<<" "<<largestPhoneIndex<<endl;

        vector<int> arr(largestState+1,-1);

        for(int stateIndex=0;stateIndex<=largestState;stateIndex++)
        {
          for(int phoneIndex=0;phoneIndex<=largestPhoneIndex;phoneIndex++)
          {
            if(stateIndex==0)
            {
              if(myMap.find(make_pair(ConvertToString(stateIndex), ConvertToString(phoneIndex))) != myMap.end()) 
              {
                //cout<<"Here"<<endl;
                int index=atoi(myMap[make_pair(ConvertToString(stateIndex), ConvertToString(phoneIndex))].c_str());
                cout<<index<<endl;
               //getchar();
                arr[index]=phoneIndex;
              }   
              ++totalCount;   
              cout<<"Bigram is: "<<"<eps>|"<<phoneIndex<<endl;       
            }
            else
            {
              if(myMap.find(make_pair(ConvertToString(stateIndex), ConvertToString(phoneIndex))) != myMap.end()) 
              {
                if(arr[stateIndex]!=-1)
                {
                  cout<<"Bigram is: "<<arr[stateIndex]<<"|"<<phoneIndex<<endl;
                  ++totalCount;
                }
              }
              
              
            }
          }
        }

        cout<<totalCount<<endl;

        exit(1);
}


*/


void ReadDataDir(string dataPath, vector<DataPoint>& data)
{
  string t1=dataPath+"/wav.scp";
  const char *wavFilePath = t1.c_str();
  string t2=dataPath+"/text";
  const char *textFilePath = t2.c_str();
  string t3 = dataPath+"/cmvn.scp";
  const char *cmvnFilePath = t3.c_str();
  string t4=dataPath+"/feats.scp";
  const char *featsFilePath = t4.c_str();
  string t5=dataPath+"/utt2spk";
  const char *utt2spkFilePath = t5.c_str();
   ifstream wavFile(wavFilePath),
            textFile(textFilePath),
            cmvnFile(cmvnFilePath),
            featsFile(featsFilePath),
            utt2spkFile(utt2spkFilePath);

    //get map from the cmvn files
    map<string, string> myMap;
    string delimiter=" ";
    string line;
    int pos=0;
    //cout<<dataPath<<endl;
      while(getline(cmvnFile, line))
    {
      //cout<<line<<endl;
      //getchar();
      pos=line.find(delimiter);
      string id=line.substr(0,pos);
      line.erase(0,pos+delimiter.length());
      myMap[id]=line;
      //cout<<id<<":"<<line<<endl;
      //getchar();
    }  
    
  cmvnFile.close();
  /*  cout<<"CMVN MAP:"<<endl;
    for(map<string, string>::iterator it=myMap.begin();
                      it!=myMap.end();++it)
    {
      cout<<it->first<<":"<<it->second<<endl;
      //getchar();
    }
    getchar();*/

  string wavLine, textLine, featsLine, utt2spkLine;
  while(getline(wavFile, wavLine))
  {
    //cout<<"Here"<<endl;
   getline(textFile, textLine);
   getline(featsFile, featsLine);
   getline(utt2spkFile, utt2spkLine);

   //for uttId and wavPath
   delimiter="\t";
   int pos1=wavLine.find(delimiter);
   string uttId=wavLine.substr(0,pos1);
   wavLine.erase(0,pos1+delimiter.length());

   //for text
   delimiter=" ";
   int pos2=textLine.find(delimiter);
   textLine.erase(0,pos2+delimiter.length());

   //for spkId
   delimiter="\t";
   int pos3=utt2spkLine.find(delimiter);
   utt2spkLine.erase(0,pos3+delimiter.length());

   //for feats
   delimiter=" ";
   int pos4=featsLine.find(delimiter);
   featsLine.erase(0,pos4+delimiter.length());

  ///DataPoint(string utt, string spk, string wav,
     /// string cmvn, string feats, string tex) 
     DataPoint d(uttId, utt2spkLine, wavLine, myMap[utt2spkLine],
                  featsLine, textLine );


  /*   cout<<"uttId:"<<d.uttId<<endl;
     cout<<"spkId:"<<d.spkId<<endl;
     cout<<"wavPath:"<<d.wavPath<<endl;
     cout<<"featsPath:"<<d.featsPath<<endl;
     cout<<"cmvnPath:"<<d.cmvnPath<<endl;
     cout<<"text:"<<d.text<<endl;*/

  //getchar();


   data.push_back(d);


  }

  wavFile.close();
            textFile.close();
            cmvnFile.close();
            featsFile.close();
            utt2spkFile.close();

}



