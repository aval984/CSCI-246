//
//  main.cpp
//  CSCI-246 Booths
//
//  Created by Andrew Valenzuela on 11/21/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
//
//  This program simulates a 16 bit multiplier
//  it takes two strings from either the console or passed in from a file
//  transforms them into bitsets and then multiplies them together using a
//  one bit alu adder other functions include a custom bit shifter and 2's
//  complement converter
//  the program can be exited at anytime by entering -99 or sending EOF(^d)
//  accepts input in the form of two binary numbers without anything else added.
//  compiles with g++ -std=c++17 main.cpp or with the Makefile inputs are stored in inputs.txt
//  im using the given test cases as well as a few more for further testing
//  and the output was taken from the console with the script command

#include <iostream>
#include <bitset>
#include <fstream>
#include <iomanip>

#define SIZE 16
using namespace std;

const bool _DEBUG = true;
//const bool _DEBUG = false;

bool alu(bool, bool, bool&, bool = false, bool = false);
bitset<2*SIZE> multiplyer(string, string);
template <size_t T>
bitset<T> bitShift(const bitset<T>&, bool = true, int = 1);
template <size_t T>
void print(const bitset<T>&, const bitset<T>&, const bitset<T>&, bool, int);
template <size_t T>
bitset<T> twosComp(bitset<T>);
template <size_t T>
void operator+=(bitset<T>&, bitset<T>);

int main(int argc, const char *argv[]) {
    string s, t;
    ifstream ins;
    if (argc == 2){
        ins.open(argv[1]);
        if (!ins) cout << "Error opening file!\n";
    }
    istream &in = (ins.is_open() ? ins : cin);
    cout << "Enter two binary numbers to multiply: ";
    while(in >> s){
        if (!in or s == "-99") break;
        in >> t;
        if (!in or t == "-99") break;
        if (s.size() < SIZE)
            s.insert(s.begin(), SIZE - s.size(),s.front());
        if (t.size() < SIZE)
                  t.insert(t.begin(), SIZE - t.size(),t.front());
        bitset<2*SIZE> d = multiplyer(s, t);
        cout << d << endl;
        
        if constexpr (_DEBUG){
            bitset<16> b(s);
            bitset<16> c(t);
            b[b.size() - 1] ? cout << "-"<< twosComp(b).to_ulong() << " * ": cout << b.to_ulong() << " * ";
            c[c.size() - 1] ? cout << "-"<< twosComp(c).to_ulong() : cout << c.to_ulong();
            d[d.size() - 1] ? cout << " = -" << twosComp(d).to_ulong() << endl : cout << " = " << d.to_ulong() << endl;
        }
        cout << "Enter two binary numbers to multiply: ";
    }
    return 0;
}
// simulates a one bit adder; can subtract but not used
// subtaction is handled by using 2's comp and adding
bool alu(bool a, bool b, bool &c_out, bool c_in, bool op){
    // for subtraction
    if (op){
        c_in = true;
        b = !b;
    }
    // add two bits together
    c_out = (a & b) | (c_in & (a ^ b));
    return a ^ b ^ c_in;
}
// overloaded operator for +=
template <size_t T>
void operator+=(bitset<T> &lhs, bitset<T> rhs){
    bitset<T> result;
    bool c_in = false, c_out = false;
    for (int i = 0; i < T; i++){
        result[i] = alu(lhs[0],rhs[0],c_out, c_in);
        lhs = bitShift(lhs);
        rhs = bitShift(rhs);
        c_in = c_out;
    }
    lhs = result;
}
//this function takes a bitset and returns the 2's complement of the number
template <size_t T>
bitset<T> twosComp(bitset<T> tC){
    tC.flip();
    for (int i = 0; i < T; i++)
        tC[i] ? tC[i] = 0 : (tC[i] = 1, i = T);
    return tC;
}
// print fuction: prints the cycle and all registers, only prints header once
template <size_t T>
void print(const bitset<T> &MD, const bitset<T> &MQ, const bitset<T> &AC, bool Q0, int pos){
    bitset<5> count(pos);
    string s(7,' '), t(4,' ');
    if (pos == T)
        cout << endl << "Cycle" << "   " << s << "MD" << s << t << s << "AC" << s << t << s << "MQ" << s << t << "Q0\n" << string(70,'-') << endl;
    cout << setw(8) << left << count << MD << t << AC << t << MQ << t << setw(2) << right << Q0 << endl;
}
// bit shifter: shifts bits either left or right any number of times
// followed the logic of a hw shifter i found online
// default setting is to shift right one bit
template <size_t T>
bitset<T> bitShift(const bitset<T> &orig, bool right, int bits){
    bitset<T> s;
    if (T == 1 or bits >= T) return s;
    for (int j = 0; j < bits; j++){
        for (int i = 0; i < T; i++){
            if (i - 1 < 0 && i + 1 < T)
                s[i] = orig[i + 1] & right;
            else if (i + 1 == T && i - 1 >= 0)
                s[i] = orig[i - 1] & !right;
            else
                s[i] = (orig[i - 1] & !right) | (orig[i + 1] & right);
        }
    }
    return s;
}
// the multiplyer function, returns a bitset twice the size as the operands
// only needs a multiplyer and a multipicand does not need an overflow as
// two signed 16 bit numbers cannot overflow a 32 bit register
bitset<2 * SIZE> multiplyer(string MD, string MQ){
    bitset<SIZE> M(MD), Q(MQ), negM, acc;
    negM = twosComp(M);
    bool q0 = false;
    int N = SIZE, L = N - 1;
    while (N){
        print(M,Q,acc,q0,N);
        if (Q[0] and !q0)
            acc += negM;
        else if (!Q[0] and q0)
            acc += M;
        bool keep = acc[L], carry = acc[0];
        q0 = Q[0];
        acc = bitShift(acc);
        Q = bitShift(Q);
        acc[L] = keep;
        Q[L] = carry;
        N--;
    }
    print(M,Q,acc,q0,N);
    return bitset<2*SIZE>(acc.to_string() + Q.to_string());
}
