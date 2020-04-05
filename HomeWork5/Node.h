//
//  Node.h
//  CSCI-246 HW5
//
//  Created by Andrew Valenzuela on 11/5/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
//
//
//
//
#ifndef Node_h
#define Node_h

#define LOAD 35
#define $S1 17
#define SET 2
#define TAG 4
#define WORD 32

#include <iomanip>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <sstream>
#include <memory>

using namespace std;
string d2B(int num, int bits);

struct Cache{
    vector<bool> valid;
    vector<pair<int,int>> set;
    Cache(){
        valid.resize(4);
        set.resize(4);
    }
};
struct CPU{
    int S1, S2;
    Cache cache;
};
struct Node{
    bool isAffected;
    string name;
    int nodeNum;
    vector<CPU> cpus;
    vector<vector<bool>> dir;
    vector<int> mem;
    Node(int);
    void printNode();
};
Node::Node(int n){
    this->name = "Node_" + to_string(n);
    nodeNum = n;
    isAffected = false;
    cpus.resize(2);
    mem.resize(16,0);
    dir.resize(16);
    for (int i = 0; i < cpus.size(); i++)
        for (int i = 0; i < mem.size(); i++){
            mem.at(i) = (n * 16) + i + 5;
            dir.at(i).resize(6,false);
        }
}
void Node::printNode(){
    cout << endl
    << name<<"\n"
    << "CPU_0 --------------------------------             CPU_1 --------------------------------\n"
    << "    ----------------------------------                 ----------------------------------\n"
    << "$S1 |" << d2B(cpus[0].S1, WORD) << "|             $S1 |" << d2B(cpus[1].S1, WORD) << "|\n"
    << "    ----------------------------------                 ----------------------------------\n"
    << "$S2 |" << d2B(cpus[0].S2, WORD) << "|             $S2 |" << d2B(cpus[1].S2, WORD) << "|\n"
    << "    ----------------------------------                 ----------------------------------\n"
         << "Cache_0                                      Cache_1" << endl;
    cout << "   -----------------------------------------    -----------------------------------------\n";
    for (int i = 0; i < cpus[0].cache.set.size(); i++){
        cout << d2B(i, SET) << " |" << cpus[0].cache.valid[i] << "|" << d2B(cpus[0].cache.set[i].first, TAG) << "|" << d2B(cpus[0].cache.set[i].second, WORD)<<
        "| " << d2B(i, SET) << " |" << cpus[1].cache.valid[i] << "|" << d2B(cpus[1].cache.set[i].first, TAG) << "|" << d2B(cpus[1].cache.set[i].second, WORD) << "|\n";
        cout << "   -----------------------------------------    -----------------------------------------\n";
    }
    cout << "\t\tMem:                                    Dir:\n";
    cout << "\t\t   ----------------------------------   ------------\n";
    for (int i = 0; i < mem.size(); i++){
        cout << "\t\t";
        cout << setw(2) << right << i + (nodeNum * 16) << " |" << d2B(mem[i], WORD) << "|   ";
        int j = 0;
        for (auto d : dir[i]){
            if (j++ != 1)
                cout << "|";
            cout << d;
        }
        cout << "|\n";
        cout << "\t\t   ----------------------------------   ------------\n";
    }
}
string d2B(int num, int bits){
    string s;
    int mask = 1;
    mask <<= bits - 1;
    for (int i = 0; i < bits; i++){
        if (!(num & mask))
            s.push_back('0');
        else
            s.push_back('1');
        num <<= 1;
    }
    return s;
}
#endif /* Node_h */
