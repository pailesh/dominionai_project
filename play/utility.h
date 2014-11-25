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
#include "sample.h"

using namespace std;

double getInnerProduct(const vector<double> &a, const vector<double> &b);

vector<double> addVector(const vector<double> &a,const vector<double> &b);

vector<double> mulVector(const vector<double> &a,double b);

void showVector(vector<double> a);

void showMaxValuePlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand,int ordinal);

vector<int> getMaxValueGain( vector< vector<double> > weight,  vector<double> feature, vector<int> supply,int coin,int buy,int ordinal);


vector<string> SpritString(string src, string delim);

void writeWeightVector(vector< vector<double> > weight , string filename);

vector<int> getRandVec(int n);


#endif /* defined(__parseptron__utility__) */