//
//  main.cpp
//  parseptron
//
//  Created by Yuki Murakami on 2014/05/16.
//  Copyright (c) 2014年 yuki. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <fstream>

#include <unistd.h>

#include "utility.h"
#include "sample.h"
#include "card.h"

#define KIND_OF_CARD 32

using namespace std;

int main(int argc, const char * argv[])
{
    cout << "choose playCard Learning" << endl;
    
    srand((unsigned)time(NULL));
    bool readFlag = false;
    bool action2Flag = false;
    bool card2Flag = false;
    
    if(argc == 2) {
        if(argv[1][0] == '2') {
            action2Flag = true;
        }
        if(argv[1][1] == '2') {
            card2Flag = true;
        }
        if(argv[1][0] == 'r') {
            readFlag = true;
            cout << "loading Mode" << endl;
        }
    }
    
    if(argc == 3) {
        if(argv[1][0] == '2') {
            action2Flag = true;
        }
        if(argv[1][1] == '2') {
            card2Flag = true;
        }
        if(argv[2][0] == 'r') {
            readFlag = true;
            cout << "loading Mode" << endl;
        }
    }
    
    if(action2Flag) {
        cout << "nAction >= 2 Mode" << endl;
    } else {
        cout << "nAction == 1 Mode" << endl;
    }
    if(card2Flag) {
        cout << "nCard >= 2 Mode" << endl;
    } else {
        cout << "nCard == 1 Mode" << endl;
    }
    
    if(!readFlag) {
        cout << "Warning!! :This mode will make new weight vector." << endl;
    }
    
    int dimensionOfFeature = 0;
    int nSample;   
    int roundlimit = 2000000000;//学習回数上限
    int roundtest = 100000;//テスト実施の間隔学習回数
    string studyfile = "learn.txt";//インプット教師データ
    string testfile = "test.txt";//インプットテストデータ
    
    string directoryString = "./";
    if(action2Flag) {
        directoryString += "nAction2/";
    }
    if(card2Flag) {
        directoryString += "twoCard/";
    }
    
    
    //--------------------------------------教師信号ベクトルの初期化--------
    cout << "load teacher data" << endl;
    ifstream ifs(studyfile.c_str());
    if(!ifs) {
        cout << "not found teacher data file" << endl;
        exit(0);
    }
    
    string buf;
    vector<Sample> teachers;
    int count = 0;
    while(getline(ifs, buf)) {
        vector<string> out = SpritString(buf,"/");
        int nAction = atoi(out[1].c_str());
        int nCard = (int)(SpritString(out[2],",").size());
        if((nAction == 1 && !action2Flag && nCard == 1 && !card2Flag) ||
           (nAction == 1 && !action2Flag && nCard >= 2 && card2Flag) ||
           (nAction >= 2 && action2Flag && nCard == 1 && !card2Flag) ||
           (nAction >= 2 && action2Flag && nCard >= 2 && card2Flag)) {
         //   fprintf(stderr,"loading teacher data:%d \r",count+1);
            Sample teacher(count++,buf);
            dimensionOfFeature = teacher.getDimensionOfFeature();
            teachers.push_back(teacher);
            bool flag = false;
            for(int i=0;i<teacher._hand.size();i++) {
                if(teacher._hand[i] == CARD_LABORATRY) {
                    flag = true;
                }
                /*
                if(teacher._hand[i] == CARD_THRONEROOM) {
                    flag = false;
                    break;
                }
                 */
            }
            if(flag) {
                teacher.show();
                cout << getString(teacher._answerPlayCard) << endl;
            }
        }
    }
    ifs.close();
    nSample = count;
    
    cout << teachers.size() << " teachers data                                        " << endl;
    
    
    
    //--------------------------------------テストデータベクトルの初期化--------
    cout << "load test data" << endl;
    ifstream ifs3(testfile.c_str());
    if(!ifs3) {
        cout << "not found test data file" << endl;
        exit(0);
    }
    
    vector<Sample> tests;
    count = 0;
    while(getline(ifs3, buf)) {
        vector<string> out = SpritString(buf,"/");
        int nAction = atoi(out[1].c_str());
        int nCard = (int)(SpritString(out[2],",").size());
        if((nAction == 1 && !action2Flag && nCard == 1 && !card2Flag) ||
           (nAction == 1 && !action2Flag && nCard >= 2 && card2Flag) ||
           (nAction >= 2 && action2Flag && nCard == 1 && !card2Flag) ||
           (nAction >= 2 && action2Flag && nCard >= 2 && card2Flag)) {
            fprintf(stderr,"loading test data:%d \r",count+1);
            Sample test(count++,buf);
            dimensionOfFeature = test.getDimensionOfFeature();
            tests.push_back(test);
        }
    }
    ifs3.close();
    
    cout << tests.size() << " tests data                                        " << endl;
    
    
    writeRound(teachers.size(),directoryString + "learnSize.txt");
    writeRound(tests.size(),directoryString + "testSize.txt");
    
    
    //--------------------------------------重みベクトルの初期化-------
    cout << "init weight vector                             " << endl;
    cout << "dimension of vector = " << dimensionOfFeature << endl;
    vector< vector<double> > weight;
    vector< vector<double> > averageWeight;
    for(int i=0;i<KIND_OF_CARD+1;i++) {
        vector<double> tmpVector1,tmpVector2;
        for(int j=0;j<dimensionOfFeature;j++) {
            tmpVector1.push_back(0.0);
            tmpVector2.push_back(0.0);
        }
        weight.push_back(tmpVector1);
        averageWeight.push_back(tmpVector2);
    }
    vector< vector<double> > testWeight;
    
    int round = 0;//ラウンド数
    
    if(readFlag) {
        cout << "load weight vector" << endl;
        weight = readWeightVector(directoryString + "w_weight.txt");
        averageWeight = readWeightVector(directoryString + "u_weight.txt");
        round = readRound(directoryString + "round.txt");
        //start = readWeightVector("weight.txt");
    }
    
    //---------------------------------------学習----------------------
    cout << "start learning" << endl;
    vector<int> indexs = getRandVec((int)teachers.size());
    
    double lastCorrectRate = 0.0;//前回の正解率
    double maxCorrectRate = 0.0;
    int outCount = 0;//前回の正解率を連続で上回らなかった回数
    int finishCount = 5;//outCountがfinishCountに達したら学習終了
    
    
    while(round < roundlimit) {
        
        showProgress(round%roundtest,roundtest,"learning");
        
        if(round%teachers.size() == 0) {
            indexs = getRandVec((int)teachers.size());
        }
        int sampleIndex = indexs[round%teachers.size()];
        round++;
        
        int gotPlayCard = getMaxValuePlayCard(weight,teachers[sampleIndex]._feature,teachers[sampleIndex]._notZero,teachers[sampleIndex]._hand);
        int answerPlayCard = teachers[sampleIndex]._answerPlayCard;
        
        if(gotPlayCard != answerPlayCard) {
            
            //間違えたものの重みを引く
           // if(gotPlayCard != 0) {
                int wid = gotPlayCard;
                weight[wid] = addVector(weight[wid], mulVector(teachers[sampleIndex]._feature , -1) );
                averageWeight[wid] = addVector(averageWeight[wid], mulVector(teachers[sampleIndex]._feature, round*-1));
           // }
           
            //正解の重みを足す
          //  if(answerPlayCard != 0) {
                wid = answerPlayCard;
                weight[wid] = addVector(weight[wid],teachers[sampleIndex]._feature );
                averageWeight[wid] = addVector(averageWeight[wid], mulVector(teachers[sampleIndex]._feature, round));
        //    }
        }
        
        if(round % roundtest == 0) {
            testWeight.clear();
            for(unsigned int i=0;i<averageWeight.size();i++) {
                testWeight.push_back( addVector(weight[i], mulVector(averageWeight[i], -1.0/(double)round)));
            }
            
            double correct = test(testWeight, tests,false);
            cout << "round:" << round << "/正解率：" << correct * 100 << "%" << endl;
            
            if(maxCorrectRate <= correct) {//最大正解率を更新!!
                maxCorrectRate = correct;
                writeWeightVector(testWeight,directoryString + "weight.txt");
                writeWeightVector(weight,directoryString + "w_weight.txt");
                writeWeightVector(averageWeight,directoryString + "u_weight.txt");
                writeRound(round,directoryString + "round.txt");
                writeRate(maxCorrectRate,directoryString + "maxRate.txt");
                if(correct >= 1) {//正解率1.0なら終了
                    break;
                }
            }
            
            if(lastCorrectRate < correct) {//前回の正解率を更新!!
                outCount = 0;
            } else {
                outCount++;
            }
            if(outCount >= finishCount) {
                break;
            }
            lastCorrectRate = correct;
        }
    }
    writeWeightVector(testWeight,directoryString + "weight.txt");
    test(testWeight, tests,true);
    
    cout << "number of sample:" << nSample << endl;
    cout << "number of dimension:" << dimensionOfFeature << endl;
    
    
    
    return 0;
}

