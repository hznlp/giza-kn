#ifndef FRACTYPE_H
#define FRACTYPE_H
#include <iostream>
#include <array>
#include <cmath>
#include <vector>
//#include "logprob.h"
#include "LogDouble.h"
//#define LogProb LogDouble
#define MAX_DISCOUNT_ORDER 5
extern int _debug;
using namespace std;
using std::array;

class FracType : public std::array<LogDouble,5>
{
public:
    FracType(){reset();}
	~FracType(){}
	void reset(){fill(0.0);(*this)[0]=1;}

	void print(ostream& os)const{for(int i=0;i<5;i++)os<<(*this)[i]<<"\t";}
	void update(double p);
	int compare(FracType& f);
};
inline ostream& operator <<(ostream&os, const FracType& ft){ft.print(os);return os;}

double discountMass(FracType& ft, vector<double>& discount);

#endif
