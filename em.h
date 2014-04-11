#ifndef EM_H
#define EM_H
#include "LM.h"
#include "utils.h"
#include "Vocab.h"
#include "JKArgs.h"
#include <cmath>
#include <thread> 
void usage();
void em(JKArgs& args);
extern double fracFloor;
double ppl(double logprob, double nwrds);

#endif
