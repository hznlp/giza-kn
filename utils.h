#ifndef UTILS_H
#define UTILS_H

#ifdef WIN32
#define isNaN(x) _isnan(x)
#define isFinite(x) _finite(x)
#define isInf(x) !_finite(x)
#define isBadNumber(x) (isNaN(x)||isInf(x))
#else
#define isNaN(x) ::isnan(x)
#define isFinite(x) !::isinf(x)
#define isInf(x) ::isinf(x)
#define isBadNumber(x) (isNaN(x)||isInf(x))
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <math.h>
#include <cmath>
#include <time.h>
#include <float.h>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#ifndef M_LN10
#define M_LN10	2.30258509299404568402
#endif
using namespace std;
using namespace boost;
#ifdef WIN32
double log1p(double x);
double expm1(double x);
#endif

string intToString(int in);
string doubleToString(double in);
string& replaceSubStr(string& input, string oldSub, string newSub);
string& removeSpace(string& input);
int countSubStr(const string& in, const string& sub);
bool endsWith(char * src, char* tail);
inline void error(string str){cerr<<str<<endl;exit(1);}
inline double exp10(double x){return exp(x*M_LN10);}
vector<string>& stringToVector(const string& sent, vector<string>& result);
string vectorToString(vector<string>& result);
bool extractNgram(const string& istr, vector<string>& ngrams, int order);
double logAdd(double logx, double logy);
double log10Add(double logx, double logy);
double log10p(double x);
double expm10(double x);

#endif
