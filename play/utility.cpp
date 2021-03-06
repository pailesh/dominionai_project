//
//  utility.cpp
//  parseptron
//
//  Created by Yuki Murakami on 2014/05/21.
//  Copyright (c) 2014年 yuki. All rights reserved.
//

#include "utility.h"
#include "card.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;

double getInnerProduct(const vector<double> &a, const vector<double> &b) {
    if(a.size() != b.size()) {
        cout << "error: size of vector don't match" << endl;
        exit(0);
    }
    double sum = 0;
    for(int i=0;i<a.size();i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

void showVector(const vector<double> &a) {
    for(int i=0;i<a.size();i++) {
        cout << a[i] << ",";
    }
    cout << endl;
}

vector<double> addVector(const vector<double> &a,const vector<double> &b) {
    if(a.size() != b.size()) {
        cout << "error: size of vector don't match" << endl;
        exit(0);
    }
    vector<double> c;
    for(int i=0;i<a.size();i++) {
        c.push_back(a[i]+b[i]);
    }
    return c;
}

vector<double> mulVector(const vector<double> &a,double b) {
    vector<double> c;
    for(int i=0;i<a.size();i++) {
        c.push_back(a[i] * b);
    }
    return c;
}


int showMaxValuePlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand,int ordinal) {
    
    int nHand = hand.size();
    if(ordinal > nHand) {
        ordinal = nHand;
    }
    
    vector<double> values;
    for(int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]-1],feature);
        values.push_back(value);
    }
    
    int maxHand = 0;
    
    vector<int> already;
    for(int i=0;i<ordinal;i++) {
        double maxValue = -99999999;
        int index = -1;
        for(int i=0;i<values.size();i++) {
            bool isAlreadyIndex = false;
            for(int j=0;j<already.size();j++) {
                if(already[j] == i) {
                    isAlreadyIndex = true;
                    break;
                }
            }
            if(isAlreadyIndex) continue;
            if(values[i] > maxValue) {
                maxValue = values[i];
                index = i;
            }
        }
        
        if(index != -1) {
            if(i == 0) {
                maxHand = hand[index];
                if(maxValue < 0) maxHand = 0;
            }
            already.push_back(index);
            if(index >= hand.size()) {
                cout << i+1 << "位 :( ) " << maxValue << endl;
            } else {
                cout << i+1 << "位 :(" << getString(hand[index]) << ") " << maxValue << endl;
            }
        }
    }
    
    return maxHand;
}

int showMaxValuePlayCardWithDummy(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand,int ordinal) {
    
    int nHand = hand.size() + 1;
    if(ordinal > nHand) {
        ordinal = nHand;
    }
    
    vector<double> values;
    for(int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]],feature);
        values.push_back(value);
    }
    values.push_back(getInnerProduct(weight[0],feature));
    
    int maxHand = 0;
    
    vector<int> already;
    for(int i=0;i<ordinal;i++) {
        double maxValue = -99999999;
        int index = -1;
        for(int i=0;i<values.size();i++) {
            bool isAlreadyIndex = false;
            for(int j=0;j<already.size();j++) {
                if(already[j] == i) {
                    isAlreadyIndex = true;
                    break;
                }
            }
            if(isAlreadyIndex) continue;
            if(values[i] > maxValue) {
                maxValue = values[i];
                index = i;
            }
        }
        
        if(index != -1) {
            if(i == 0) {
                if(index >= hand.size()) {
                    maxHand = 0;
                } else {
                    maxHand = hand[index];
                }
            }
            already.push_back(index);
            if(index >= hand.size()) {
                cout << i+1 << "位 :( ) " << maxValue << endl;
            } else {
                cout << i+1 << "位 :(" << getString(hand[index]) << ") " << maxValue << endl;
            }
        }
    }
    
    return maxHand;
}

int getMaxValuePlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand) {
    
    vector<double> values;
    for(unsigned int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]-1],feature);
        values.push_back(value);
    }
    
    if (values.size() <= 0) return 0;
    
    double maxValue = values[0];
    int index = -1;
    for(unsigned int i=0;i<values.size();i++) {
        if(values[i] >= maxValue) {
            maxValue = values[i];
            index = i;
        }
    }
    if(index == -1) {
        cout << "error: selected index = -1 @getMaxValuePlayCardWithMinus" << endl;
        exit(0);
    }
    
    if(maxValue < 0) return 0;
    
    return hand[index];
}

int getMaxValuePlayCardWithDummy(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand) {
    
    vector<double> values;
    for(unsigned int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]],feature);
        values.push_back(value);
    }
    values.push_back(getInnerProduct(weight[0],feature));
    
    if (values.size() <= 0) return 0;
    
    double maxValue = values[0];
    int index = -1;
    for(unsigned int i=0;i<values.size();i++) {
        if(values[i] >= maxValue) {
            maxValue = values[i];
            index = i;
        }
    }
    if(index == -1) {
        cout << "error: selected index = -1 @getMaxValuePlayCardWithMinus" << endl;
        exit(0);
    }
    
    if(index >= hand.size()) return 0;
    
    return hand[index];
}

int getMaxValueMustPlayCard(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand) {
    
    vector<double> values;
    for(unsigned int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]-1],feature);
        values.push_back(value);
    }
 //   values.push_back(getInnerProduct(weight[0],feature));
    
    if (values.size() <= 0) return 0;
    
    double maxValue = values[0];
    int index = -1;
    for(unsigned int i=0;i<values.size();i++) {
        if(values[i] >= maxValue) {
            maxValue = values[i];
            index = i;
        }
    }
    if(index == -1) {
        cout << "error: selected index = -1 @getMaxValuePlayCardWithMinus" << endl;
        exit(0);
    }
    
    
    return hand[index];
}

int getMaxValuePlayCardWithMinus(const vector< vector<double> > &weight, const vector<double> &feature, vector<int> &hand) {
    
    vector<double> values;
    for(unsigned int i=0;i<hand.size();i++) {
        double value = getInnerProduct(weight[hand[i]-1],feature);
        values.push_back(value);
    }
    
    if (values.size() <= 0) return 0;
    
    double maxValue = values[0];
    int index = -1;
    for(unsigned int i=0;i<values.size();i++) {
        if(values[i] >= maxValue) {
            maxValue = values[i];
            index = i;
        }
    }
    if(index == -1) {
        cout << "error: selected index = -1 @getMaxValuePlayCardWithMinus" << endl;
        exit(0);
    }
    
    
    return hand[index];
}


vector<int> getTrashCardsByChapel(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand) {

    vector<int> gotSelectCards;
    vector<double> feature;
    copy(_feature.begin(),_feature.end(),back_inserter(feature));
    vector<int> hand;
    copy(_hand.begin(),_hand.end(),back_inserter(hand));
    
    int limitCount = 0;
    while(true) {
        limitCount++;
        int gotSelectCard = getMaxValuePlayCard(_weight,feature,hand);
        if(gotSelectCard != 0 && limitCount <= 4) {
            gotSelectCards.push_back(gotSelectCard);
        } else {
            break;
        }
        for(unsigned int i=0;i<hand.size();i++) {
            if(hand[i] == gotSelectCard) {
                hand.erase(hand.begin()+i);
                break;
            }
        }
        //礼拝堂廃棄なので対象カードを手札から削除
        feature[(CARD_MAX+1) + gotSelectCard]--;
        continue;
    }
    return gotSelectCards;
}

vector<int> getDiscardCardsByCellar(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand) {
    
    vector<int> gotSelectCards;
    vector<double> feature;
    copy(_feature.begin(),_feature.end(),back_inserter(feature));
    vector<int> hand;
    copy(_hand.begin(),_hand.end(),back_inserter(hand));
    
    while(true) {
        int gotSelectCard = getMaxValuePlayCard(_weight,feature,hand);
        if(gotSelectCard != 0) {
            gotSelectCards.push_back(gotSelectCard);
        } else {
            break;
        }
        for(unsigned int i=0;i<hand.size();i++) {
            if(hand[i] == gotSelectCard) {
                hand.erase(hand.begin()+i);
                break;
            }
        }
        //ちかちょは、対象カードが手札から捨て札に移り、何枚目かの特徴量をインクリメント
        feature[(CARD_MAX+1) + gotSelectCard]--;//手札から削除
        feature[(CARD_MAX+1)*2 + gotSelectCard]++;//捨て札に追加
        feature[feature.size()-1]++;//何枚目のdiscardか、をインクリメント
        continue;
    }
    return gotSelectCards;
}

vector<int> getDiscardCardsByMilitia(const vector< vector<double> > &_weight, const vector<double> &_feature, vector<int> &_hand) {
    
    vector<int> hand;
    copy(_hand.begin(),_hand.end(),back_inserter(hand));
    
    vector<int> discardCards;
    while(hand.size() > 3) {
        int gotSelectCard = getMaxValuePlayCardWithMinus(_weight,_feature,hand);
        discardCards.push_back(gotSelectCard);
        for(unsigned int j=0;j<hand.size();j++) {
            if(hand[j] == gotSelectCard) {
                hand.erase(hand.begin()+j);
                break;
            }
        }
    }
    
    return discardCards;
}

vector<int> getMaxValueGain( vector< vector<double> > weight, vector<double> feature,vector<int> supply,int coin,int buy,int ordinal) {
    
    map<int,double> cardValues;
    for(int i=0;i<supply.size();i++) {
        double value = getInnerProduct(weight[i],feature);
        cardValues.insert(map<int,double>::value_type(i+1,value));
    }
    
    vector< vector<int> > gainList = getGainList(coin, buy, supply);
    
    vector<int> already;
    vector<int> maxGain;
    vector<int> resultGain;
    
    for(int k=0;k<ordinal;k++) {
    
        double maxValue = -99999999;
        int maxindex = 0;
        for(int i=0;i<gainList.size();i++) {
            double sumValue = 0.0;
            for(int j=0;j<gainList[i].size();j++) {
                sumValue += cardValues[gainList[i][j]];
            }
            //showGain(gainList[i]); cout << "Value:" << sumValue << endl;
            if(maxValue < sumValue) {
                int flag = 0;
                for(int n=0;n<already.size();n++) {
                    if(already[n] == i) {
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0) {
                    maxValue = sumValue;
                    maxindex = i;
                }
            }
        }
    
        maxGain = gainList[maxindex];
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
        if(k==0) resultGain = maxGain;
        
        cout << k+1 << "位 :" << maxValue << endl;
        showGain(maxGain);
        already.push_back(maxindex);
    }
    
    //showGain(maxGain); cout << "maxValue:" << maxValue << endl;
    
    return resultGain;
}

vector<int> getMaxValueMustGain( vector< vector<double> > weight, vector<double> feature,vector<int> supply,int coin,int buy,int ordinal) {
    
    map<int,double> cardValues;
    for(int i=0;i<supply.size();i++) {
        double value = getInnerProduct(weight[i],feature);
        cardValues.insert(map<int,double>::value_type(i+1,value));
    }
    
    vector< vector<int> > gainList = getGainList(coin, buy, supply);
    
    vector<int> already;
    vector<int> maxGain;
    vector<int> resultGain;
    
    vector< vector<int> > ordinalList;
    
    for(int k=0;k<ordinal;k++) {
        
        double maxValue = -99999999;
        int maxindex = 0;
        for(int i=0;i<gainList.size();i++) {
            double sumValue = 0.0;
            for(int j=0;j<gainList[i].size();j++) {
                sumValue += cardValues[gainList[i][j]];
            }
            //showGain(gainList[i]); cout << "Value:" << sumValue << endl;
            if(maxValue < sumValue) {
                int flag = 0;
                for(int n=0;n<already.size();n++) {
                    if(already[n] == i) {
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0) {
                    maxValue = sumValue;
                    maxindex = i;
                }
            }
        }
        
        maxGain = gainList[maxindex];
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
        if(k==0) resultGain = maxGain;
        ordinalList.push_back(maxGain);
        
        cout << k+1 << "位 :" << maxValue << endl;
        showGain(maxGain);
        already.push_back(maxindex);
    }
    
    //showGain(maxGain); cout << "maxValue:" << maxValue << endl;
    
    for(unsigned int i=0;i<ordinalList.size();i++) {
        vector<int> list = ordinalList[i];
        if(list.size() > 0 && !(list.size() == 1 && list[0] == 0)) {
            resultGain = ordinalList[i];
            break;
        }
    }
    
    if(resultGain.size() <= 0 || (resultGain.size() == 1 && resultGain[0] == 0)) {
        vector<int> a;
        a.push_back(CARD_COPPER);
        resultGain = a;
    }
    
    return resultGain;
}

bool getIsDiscard( vector<double> weight, vector<double> feature) {
    
    bool flag = false;
    double value = getInnerProduct(weight,feature);
     
    if(value < 0) {
        flag = false;
        cout << "NO (" << value << ")" << endl;
    } else {
        flag = true;
        cout << "YES (" << value << ")" << endl;
    }
    
    return flag;
}


vector<string> SpritString(string src,string delim) {
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

vector< vector<double> > readWeightVector(string weightfile,int nWeight,int dimensionOfFeature) {
    vector< vector<double> > weight;
    ifstream ifs(weightfile);
    if(!ifs) {
        cout << "error: not found weightFile" << endl;
        exit(0);
    }
    string buf;
    while(ifs && getline(ifs,buf)) {
        vector<string> output = SpritString(buf,",");
        vector<double> tmpVector;
        for(int i=0;i<output.size();i++) {
            double val = atof(output[i].c_str());
            tmpVector.push_back(val);
        }
        weight.push_back(tmpVector);
    }
    
    if(weight.size() != nWeight) {
        cout << "error: the number of weightVectors don't match" << endl;
        exit(0);
    }
    if(weight[0].size() != dimensionOfFeature) {
        cout << "error: the number of dimension don't match" << endl;
        exit(0);
    }
    
    return weight;
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

void showOutVector(vector<int> a) {
    if(a.size() <= 0) {
        cout << "0" << endl;
    } else {
        for(unsigned int i=0;i<a.size();i++) {
            cout << a[i];
            if(i != a.size()-1) {
                cout << ",";
            }
        }
        cout << endl;
    }
}

void showOutCard(int a) {
    cout << a << endl;
}

