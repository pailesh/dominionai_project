//
//  utility.h
//  parseptron
//
//  Created by Yuki Murakami on 2014/05/21.
//  Copyright (c) 2014年 yuki. All rights reserved.
//

#ifndef __parseptron__utility__
#define __parseptron__utility__

#include <iostream>
#include <vector>
#include <map>


using namespace std;

double getInnerProduct(const vector<double> &a, const vector<double> &b);

vector<double> addVector(const vector<double> &a,const vector<double> &b);

vector<double> mulVector(const vector<double> &a,double b);

void showVector(vector<double> a);

int showMaxValuePlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand,int ordinal);
int showMaxValuePlayCardWithDummy(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand,int ordinal);

vector<int> getMaxValueGain( vector< vector<double> > weight,  vector<double> feature, vector<int> supply,int coin,int buy,int ordinal);
vector<int> getMaxValueMustGain( vector< vector<double> > weight, vector<double> feature,vector<int> supply,int coin,int buy,int ordinal);

int getMaxValuePlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand);
int getMaxValueMustPlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand);
int getMaxValuePlayCardWithDummy(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand);
int getMaxValuePlayCardWithMinus(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand);

vector<int> getDiscardCardsByMilitia(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand);
vector<int> getDiscardCardsByCellar(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand);
vector<int> getTrashCardsByChapel(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand);

bool getIsDiscard( vector<double> weight, vector<double> feature);


vector<string> SpritString(string src, string delim);

void writeWeightVector(vector< vector<double> > weight , string filename);
vector< vector<double> > readWeightVector(string weightfile,int nWeight,int dimensionOfFeature);

vector<int> getRandVec(int n);

void showOutVector(vector<int> a);

void showOutCard(int a);


#endif /* defined(__parseptron__utility__) */
