//
//  utility.cpp
//  parseptron
//
//  Created by Yuki Murakami on 2014/05/21.
//  Copyright (c) 2014年 yuki. All rights reserved.
//

#include "utility.h"
#include "card.h"
#include "sample.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;

double getInnerProduct(const vector<double> &a,const vector<double> &b,const vector<int> &notZero) {
    if(a.size() != b.size()) {
        cout << "error: size of vector don't match innerProduct" << endl;
        exit(0);
    }
    double sum = 0;
    int size = notZero.size();
    for(int i=0;i<size;i++) {
        sum += a[notZero[i]] * b[notZero[i]];
    }
    return sum;
}

void showVector(vector<double> a) {
    for(int i=0;i<a.size();i++) {
        cout << a[i] << ",";
    }
    cout << endl;
}

vector<double> addVector(const vector<double> &a,const vector<double> &b) {
    //cout << "addVector" << endl;
    if(a.size() != b.size()) {
        cout << "error: size of vector don't match add" << endl;
        exit(0);
    }
    const int size = a.size();
    vector<double> c(size);
    for(int i=0;i<size;i++) {
        c[i] = a[i]+b[i];
    }
//    if(a.size() != c.size()) {
//        cout << "error: size of vector don't match add c" << endl;
//        cout << a.size() << " " << c.size() << endl;
//        exit(0);
//    }
    return c;
}

vector<double> mulVector(const vector<double> &a,double b) {
    
    const int size = a.size();
    vector<double> c(size);
    for(int i=0;i<size;i++) {
        c[i] = a[i] * b;
    }
//    if(a.size() != c.size()) {
//        cout << "error: size of vector don't match mul c" << endl;
//        cout << a.size() << " " << c.size() << endl;
//        exit(0);
//    }
    return c;
}


vector<int> getMaxValueGain(const vector< vector<double> > &weight, const vector<double> &feature,const vector<int> &notZero, vector<int> supply,int coin,int buy) {
    
    map<int,double> cardValues;
    int sSize = supply.size();
    for(int i=0;i<sSize;i++) {
        double value = getInnerProduct(weight[i],feature,notZero);
        cardValues.insert(map<int,double>::value_type(i+1,value));
    }
    
    vector< vector<int> > gainList = getGainList(coin, buy, supply);
    double maxValue = -999999;
    int maxindex = 0;
    int gSize = gainList.size();
    for(int i=0;i<gSize;i++) {
        double sumValue = 0.0;
        int ggSize = gainList[i].size();
        for(int j=0;j<ggSize;j++) {
            sumValue += cardValues[gainList[i][j]];
        }
        //showGain(gainList[i]); cout << "Value:" << sumValue << endl;
        if(maxValue < sumValue) {
            maxValue = sumValue;
            maxindex = i;
        }
    }
    
    vector<int> maxGain = gainList[maxindex];
    if(cardValues[CARD_COPPER] > 0) {
        int gainCount = gainList[maxindex].size();
        if(gainList[maxindex].size() == 1 && gainList[maxindex][0] == 0) gainCount = 0;
        for(int i=0;i<buy - gainCount;i++) {
            if(maxGain[0] == CARD_DUMMY) {
                maxGain.clear();
            }
            maxGain.push_back(CARD_COPPER);
        }
    }
    
    //showGain(maxGain); cout << "maxValue:" << maxValue << endl;
    
    return maxGain;
}

double test(const vector< vector<double> > &weight, vector<Sample> testData,bool isOuput) {
    
    int count = 0;
    int correct = 0;
    
    if(isOuput) {
        cout << "-----can't fit data-----" << endl;
    }
    
    int tSize = testData.size();
    for(int i=0;i<tSize;i++) {
        showProgress(i,tSize,"test    ");
       
        if(isEqualGain(  getMaxValueGainFromSample(weight, testData[i])  , testData[i]._gain )) {
            count++;
            correct++;
        } else {
            count++;
            if(isOuput) {
                testData[i].show();
                cout << "supply:";
                vector<int>tmpsupply;
                int tsSize = testData[i]._supply.size();
                for(int j=0;j<tsSize;j++) {
                    if(testData[i]._supply[j] > 0) {
                        tmpsupply.push_back(j+1);
                    }
                }
                showGain(tmpsupply);
                cout << "coin:" << testData[i]._coin << endl;
                cout << "buy:" << testData[i]._buy << endl;
                cout << "AnsGain:";
                showGain(testData[i]._gain);
                cout << "gotGain:";
                showGain( getMaxValueGainFromSample(weight, testData[i]) );
            }
        }
    }
    
    return (double)correct / (double)count ;
    
}


vector<string> SpritString(const string &src,const string &delim) {
    string::size_type start = 0;
    vector<string> dest;
    while(true){
        string::size_type end = src.find(delim, start);
        if(end != string::npos){
            dest.push_back(src.substr(start, end - start));
        }
        else{
            dest.push_back(src.substr(start, src.length() - start));
            break;
        }
        start = end + delim.length();
    }
    return dest;
}


void writeWeightVector(vector< vector<double> > weight , string filename) {
    ofstream ofs(filename);
    for(int i=0;i<weight.size();i++) {
        for(int j=0;j<weight[i].size();j++) {
            if(j == weight[i].size()-1)  {
                ofs << weight[i][j] << endl;
            } else {
                ofs << weight[i][j] << ",";
            }
        }
    }
    ofs.close();
}

vector< vector<double> > readWeightVector(string filename) {
    vector< vector<double> > weight;
    ifstream ifs(filename);
    string buf;
    while(ifs && getline(ifs,buf)) {
        vector<double> tmp;
        vector<string> out = SpritString(buf,",");
        for(int i=0;i<out.size();i++) {
            double val = atof(out[i].c_str());
            tmp.push_back(val);
        }
        weight.push_back(tmp);
    }
    return weight;
}

void writeRound(int round ,string filename) {
    ofstream ofs(filename);
    ofs << round << endl;
    ofs.close();
}

void writeRate(double rate,string filename) {
    ofstream ofs(filename);
    ofs << rate << endl;
    ofs.close();
}

int readRound(string filename) {
    int round = 0;
    ifstream ifs(filename);
    string buf;
    while(ifs && getline(ifs,buf)) {
        round = atoi(buf.c_str());
    }
    return round;
}


vector<int> getRandVec(int n) {
    vector<int> v;
    
    for(int i=0;i<n;i++) {
        v.push_back(i);
    }
    
    for(int i=0;i<n*10;i++) {
        int f1 = rand()%n;
        int f2 = rand()%n;
        int a = v[f1];
        v[f1] = v[f2];
        v[f2] = a;
    }
    
    return v;
}


void showProgress(int a,int b,string str) {
    
    double progress = (double)a / (double) b * 100;
    int d = (int)(progress/2);
    string para;
    for(int i=0;i<50;i++) {
        if(i == d) {
            para += ">";
        } else {
            para += ".";
        }
    }
    fprintf(stderr,"%s:%s\r",str.c_str(),para.c_str());
    //fprintf(stderr,"%3.0f / 100\r",progress);
    if(a>=b) {
        fprintf(stderr,"                                                     \r");
    }
    
}




vector<int> getMaxValueGainFromSample(const vector< vector<double> > &weight, Sample teacher) {
    
    
    map<int,double> cardValues;
    int sSize = teacher._supply.size();
    for(int i=0;i<sSize;i++) {
        double value = getInnerProduct(weight[i],teacher._feature,teacher._notZero);
        cardValues.insert(map<int,double>::value_type(i+1,value));
    }
    
    double maxValue = -999999;
    int maxindex = 0;
    int gSize = teacher._gainlist.size();
    for(int i=0;i<gSize;i++) {
        double sumValue = 0.0;
        int ggSize = teacher._gainlist[i].size();
        for(int j=0;j<ggSize;j++) {
            sumValue += cardValues[teacher._gainlist[i][j]];
        }
        if(maxValue < sumValue) {
            maxValue = sumValue;
            maxindex = i;
        }
    }
    
    vector<int> maxGain = teacher._gainlist[maxindex];
    if(cardValues[CARD_COPPER] > 0) {
        int nloop = teacher._buy - teacher._gainlist[maxindex].size();
        for(int i=0;i<nloop;i++) {
            if(maxGain[0] == CARD_DUMMY) {
                maxGain.clear();
            }
            maxGain.push_back(CARD_COPPER);
        }
    }
    
    //showGain(maxGain); cout << "maxValue:" << maxValue << endl;
    
    return maxGain;
}
