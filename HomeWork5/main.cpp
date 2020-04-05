//
//  main.cpp
//  CSCI-246 HW5
//
//  Created by Andrew Valenzuela on 11/2/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
//**********************************************************
//  this program simulates the directory-based cache coherence protocol
//  used in the cc-NUMA(DASH) machine.
//  ive used three structs to represent a node, a cpu, and the cache.
//  all nodes, cpus, cache, and main memory are accessed using either
//  the quot or rem or both of one of the instructions.
//  im using the input provided from the HW:
/*
 000: 10001100000100010000000001101100
 001: 10001100000100100000000001101100
 000: 10101100000100010000000001001000
 010: 10001100000100010000000001101100
 100: 10001100000100010000000001101100
 010: 10001100000100010000000011100100
 010: 10101100000100010000000001101100
 110: 10001100000100010000000001101100
 */
//  each instruction is broken down to its compoinents: node,cpu,op,rs,rt,offset
//  the affected nodes are printed to the screen after the instructino is run
//  this program can be compiled using 'g++ -std=c++17 main.cc' or using the make file 'make'
//  input has been saved in a file, 'test.txt' and outputed to the console
//  sent to an output file using the terminal: './a.out < test.txt > output.txt'
//  the program runs in an infinite loop but can be halted if sent EOF from keyboard: '^d'

#include "Node.h"

//const bool debug = true;
const bool debug = false;

vector<Node*> nodeVec;

string d2B(int,int);
int b2D(const string&);
int getValue(int, int, int);
void setDir(int, int, int, int = INT_MIN);
bool isShared(int, int);
bool isValid(int, int, int);
bool isTag(int, int, int, int);
bool isUncached(int, int);
void writeHit(int, int, int, int, int, int, int);
void inValidate(int, int, int, int);
void makeShared(int, int);
bool isDirty(int, int);
void setReg(int, int, int, int, int = INT_MIN);
void setNode(int, int, int, int, int, int);
vector<int> translate(const string&);

int main(int argc, char **argv){
    if constexpr (debug) cerr << "IN DEBUG MODE\n";
    for (int i = 0; i < 4; i++)
        nodeVec.push_back(new Node(i));
    
  /*  if constexpr (debug)
        for (auto i : nodeVec)
            i -> printNode();
*/
    int totalCost = 0;
    while (true){
        string code = "";
        cout << "Enter a Command: ";
        cin >> ws;
        getline(cin,code);
        if(cin.eof()) break;
        // 0: Node/CPU 1: OP code 2: RS 3: RT 4: OFFSET
        vector<int> instr;
        if constexpr (debug) cerr << code << endl;
        instr = translate(code);
        
        if constexpr (debug)
            for (auto v : instr)
                cerr << v << endl;
        
        int opCode = instr[1], rs = instr[2], rt = instr[3], wordAddr = instr[4]/4;
        div_t nc = div(instr[0],2), addrs = div(wordAddr, 4), mem = div(wordAddr,16);
        
        int node = nc.quot, cpu = nc.rem, set = addrs.rem, tag = addrs.quot, memNode = mem.quot, memI = mem.rem;
        
        if constexpr (debug) cerr << "Node: " << node << " cpu: " << cpu << "\nOpCode: " << opCode << "\nRS: " << rs << " RT: " << rt << "\nWord address: " << wordAddr << "\nSet: " << set << " Tag: " << tag << endl;
        
        int cost = 0;
        if (opCode == LOAD){
            if ((void)(cost += 1), isValid(node, cpu, set)){
                if (isTag(node, cpu, set, tag)){
                    nodeVec[node] -> isAffected = true;
                    setReg(node, cpu, set, rt);
                }
            }else if (int c = (cpu+1)%2;(void)(cost += 30), isValid(node,c, set)){
                if (isTag(node,c, set, tag)){
                    nodeVec[node] -> isAffected = true;
                    setNode(node, cpu, set, tag, rt, getValue(node, c, set));
                }
            }else {
                cost += 100;
                if (!isDirty(memNode, memI)){ //uncached or shared
                    setDir(memNode, memI, node);
                    int value = nodeVec.at(memNode) -> mem.at(memI);
                    setNode(node, cpu, set, tag, rt, value);
                    nodeVec[node] -> isAffected = true;
                    nodeVec[memNode] -> isAffected = true;
                }else {//dirty dir
                    nodeVec[node] -> isAffected = true;
                    cost += 135;
                    for (int i = 2; i < 6; i++){//find first node with a shared copy
                        //Node_(i - 2)
                        if (nodeVec.at(memNode) -> dir.at(memI).at(i)){//found a node
                            //cpu_0
                            if (isValid(i - 2, 0, set)){//check if set is valid
                                if (isTag(i - 2, 0, set, tag)){//if valid check that tags match
                                    int value = getValue(i - 2, 0, set);//get value stored in the cache
                                    setNode(node, cpu, set, tag, rt, value);//update the cache and sets the register to value
                                    setDir(memNode, memI, node);//update the dir
                                    break;
                                }else {} //tags dont match
                            }else if (isValid(i - 2, 1, set)){ //not valid check other cpu
                                //cpu_1
                                if (isTag(i - 2, 1, set, tag)){
                                    int value = getValue(i - 2, 1, set);
                                    setNode(node, cpu, set, tag, rt, value);
                                    setDir(memNode, memI, node);
                                    break;
                                }else {}//tags dont match
                            }//end check of cpu 1
                        }//not marked shared
                    }//end of for loop
                }//end of dirty block
            }//end check of main mem
        }else{ //store instruction
            int value = (rt == $S1) ? nodeVec[node] -> cpus[cpu].S1 : nodeVec[node] -> cpus[cpu].S2;
            nodeVec[node] -> isAffected = true;
            nodeVec[memNode] -> isAffected = true;
            if ((void)(cost += 1), isValid(node, cpu, set)){
                if (isTag(node, cpu, set, tag)){
                    if (!isUncached(memNode,memI))
                        inValidate(memNode, memI, set, tag);
                    writeHit(memNode, memI, value, node, cpu, set, tag);
                }
            }else {//write miss
                cost += 100;
                inValidate(memNode, memI, set, tag);
                nodeVec[memNode] -> mem[memI] = value;
                if (!isUncached(memNode, memI))
                    makeShared(memNode, memI);
            }
        }
        if constexpr (debug) cerr << "Cost: " << cost << endl;
        cout << "\n";
        cout << "                -------------------------------------\n";
        cout << "Instruction set |" << d2B(opCode,6) << "|" << d2B(rs,5) << "|" << d2B(rt,5) << "|" << d2B(wordAddr * 4,16) << "|\n";
        cout << "                -------------------------------------\n";

        for (auto i : nodeVec)
            if (i -> isAffected)
                i -> printNode();
        cout << "Cost: " << cost << endl;
        totalCost += cost;
    }
    printf("Total Cost: %i\n",totalCost);
}
void makeShared(int memNode, int memI){//sets the dir at a mem loc to shared
    nodeVec[memNode] -> dir[memI][0] = false;
    nodeVec[memNode] -> dir[memI][1] = true;
}
void writeHit(int memNode, int memI, int value, int node, int cpu, int set, int tag){
    for (int i = 0; i < 6; i++)
        nodeVec[memNode] -> dir[memI][i] = false;
    nodeVec[memNode] -> dir[memI][0] = true;
    nodeVec[memNode] -> dir[memI][node + 2] = true;
    nodeVec[memNode] -> mem[memI] = value;
    nodeVec[node] -> cpus[cpu].cache.valid[set] = true;
    nodeVec[node] -> cpus[cpu].cache.set[set].first = tag;
    nodeVec[node] -> cpus[cpu].cache.set[set].second = value;
}
bool isUncached(int memNode, int memI){
    return !(nodeVec[memNode] -> dir[memI][0]) and !(nodeVec[memNode] -> dir[memI][1]);
}
bool isDirty(int memNode, int memI){
    return nodeVec[memNode] -> dir[memI][0] and !(nodeVec[memNode] -> dir[memI][1]);
}
bool isShared(int memNode, int memI){
    return !(nodeVec[memNode] -> dir[memI][0]) and nodeVec[memNode] -> dir[memI][1];
}
void inValidate(int memNode, int memI, int set, int tag){
    for (int i = 2; i < 6; i++){
        if (nodeVec[memNode] -> dir[memI][i]){
            if (isValid(i - 2, 0, set)){
                if (isTag(i - 2, 0, set, tag))
                    nodeVec[i - 2] -> cpus[0].cache.valid[set] = false;
            }else if (isValid(i - 2, 1, set))
                if (isTag(i - 2, 1, set, tag))
                    nodeVec[i - 2] -> cpus[1].cache.valid[set] = false;
        }
    }
}
bool isTag(int node, int cpu, int set, int tag){
    return nodeVec.at(node) -> cpus.at(cpu).cache.set.at(set).first == tag;
}
int getValue(int node, int cpu, int set){
    return nodeVec.at(node) -> cpus.at(cpu).cache.set.at(set).second;
}
bool isValid(int node, int cpu, int set){
    return nodeVec.at(node) -> cpus.at(cpu).cache.valid.at(set);
}
void setReg(int node, int cpu, int set, int rt, int value){
    if (value == INT_MIN)
        value = getValue(node, cpu, set);
    
    (rt == $S1) ? nodeVec.at(node) -> cpus.at(cpu).S1 = value : nodeVec.at(node) -> cpus.at(cpu).S2 = value;
}
//sets the dir to shared and marks the node that has the copy, optionally sets the value of the mem
void setDir(int memNode, int memI, int node, int value){
    if (value != INT_MIN)
        nodeVec[memNode] -> mem[memI] = value;
    nodeVec.at(memNode) -> dir.at(memI).at(0) = false;
    nodeVec.at(memNode) -> dir.at(memI).at(1) = true;
    nodeVec.at(memNode) -> dir.at(memI).at(node + 2) = true;
}
void setNode(int node, int cpu, int set, int tag, int rt, int value){
    setReg(node, cpu, set, rt, value);
    nodeVec.at(node) -> cpus.at(cpu).cache.valid.at(set) = true;
    nodeVec.at(node) -> cpus.at(cpu).cache.set.at(set).first = tag;
    nodeVec.at(node) -> cpus.at(cpu).cache.set.at(set).second = value;
}
int b2D(const string &s){
    int decimal = 0;
    for (int i = 0; i < s.size(); i++){
        char c = s.at(s.size()-1 - i);
        decimal = (c == '1') ? decimal + pow(2,i) : decimal;
    }
    return decimal;
}
vector<int> translate(const string &s){
    string t;
    vector<int> vec;
    for (char c : s)
        if (c != ' ' and c != ':')
            t.push_back(c);
    if constexpr (debug)
        cerr << t << endl << t.substr(0,3) << endl << t.substr(3,6) << endl
        << t.substr(9,5) << endl << t.substr(14,5) << endl << t.substr(19,16) << endl;
    vec.push_back(b2D(t.substr(0,3)));
    vec.push_back(b2D(t.substr(3,6)));
    vec.push_back(b2D(t.substr(9,5)));
    vec.push_back(b2D(t.substr(14,5)));
    vec.push_back(b2D(t.substr(19,16)));
    return vec;
}
