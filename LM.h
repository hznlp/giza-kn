#ifndef LM_H
#define LM_H
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <math.h>
#include "ZMap.h"
#include <unordered_map>
#include "Vocab.h"
#include "utils.h"
#include "FracType.h"

#define MAX_DISCOUNT_ORDER 5
enum Smoothing {KN,WB,KW,NO};
extern int _debug;

class Node {
public:
	void clear(){bow=0;probs.clear();childs.clear();}
	Node(){bow=0;probs.clear();childs.clear();}
	~Node(){probs.clear();types.clear();childs.clear();}
	void erase(int wrd);
	void prune(double threshold);
	void normalize();
	double bow;
	ZMap<int, double> probs;
	ZMap<int, FracType> types;
	ZMap<int, Node> childs;
};

class LMStat {
public:
	LMStat(int order){ngrams.clear();nwrds=0;for(int i=0;i<=order;i++)ngrams.push_back(0);}
	void print(ostream& os){for(int i=0;i<(int)ngrams.size();i++)os<<i<<"-gram:"<<ngrams[i]<<endl;}
	vector<int> ngrams;
	int nwrds;
};

enum DiscountType{D_AD,D_WB};

void calculateDiscount(vector<double>& coc, vector<double>& discount);

class LM
{
public:
	Node* setNode(int* context, int clen);
	Node* lookupNode(int* context, int clen);
	double& setProb(int wrd, int* context, int clen);
	void setOrder(int order);
	void init(int order){setOrder(order);_vocab.add(unk);}
	void readCountFile(string filename);
	int readPlainFile(string filename, int order, double* p_weight, double threshold=0, double floor=0, int nSentence=-1);
    void readCountFile(string filename, unordered_map<string,FracType>& ngrams);
    void readPlainFile(string filename, double*& p_weight, unordered_map<string,FracType>& ngrams);
	bool addOneLine(string& curline, int order, double*& p_weight, double threshold, double floor);
    void addNgrams(unordered_map<string,pair<FracType,double>>& ngrams);

	void reComputeLowerCount();
	void computeBOW(bool interpolate);
	void setSomeDetails();
	
	void estimate(Smoothing sm, bool interpolate, double* cutoff=NULL);

	void wbEstimate(bool interpolate, double* cutoff=NULL);
    void mlEstimate(Node& node);
	void knEstimate(bool interpolate, double* cutoff=NULL);
	void knEstimate(bool isBackward, bool interpolate, DiscountType dt, double* cutoff);
	void normalize();

	double& setBackoff(int* context, int clen);
	double probBO(int wrd, int* context, int clen);
	bool checkEntry(int wrd, int* context, int clen);
	void stat(int wrd, int* context, int clen, LMStat& lmstat);

	double sentLogProb(int* context, int slen, int order, LMStat* pStat);
	double sentLogProb(string& curline, int ord, LMStat* pStat=NULL);
	void read(string filename);
	void write(string filename);
	void writeCount(string filename);
	double ppl(string filename, LMStat* pStat, string probFilename="", bool average=true);
	void setKNcounts(bool isBackWard);
    void setKNcountsTam(double threshold);
	void calculateDiscounts(vector<vector<double> >& discounts, bool logrized=false);
	void adEstimate(Node& node, vector<vector<double> >& discounts, int order);
	void prune(Node& node, double* thresholds, int order);
	void prune(double* thresholds){if(thresholds!=NULL)prune(_root,thresholds,0);}
	int order(){return (int)_numofGrams.size();}
	void clear(){_numofGrams.clear();_root.clear();_vocab.clear();}
	void cleanBadEntries(){vector<double> t; for(int i=0;i<order();i++)t.push_back(0);prune(&t[0]);}
	void setForAlignment(){_numofGrams.clear();_numofGrams.push_back(1);_numofGrams.push_back(1);}
	void addBigram(int wrd, int context, double count){if(count>1E-10)addFracCount(wrd, &context, 1, count, count);}
	double bigramProb(int wrd, int context){return exp10(probBO(wrd,&context,1));}
	void resetVocab();
    
private:
	void calcNumOfGrams(){_numofGrams.clear();calcNumOfGrams(_root,0);}
	Node* addFracCount(int wrd, int* context, int clen, double fcount, double ftype);
    Node* addFracCount(int wrd, int* context, int clen, double fcount, FracType& ftype);


	void computeBOWByLayer(Node& node, vector<int>& context, int order, bool interpolate);
	void calcNumOfGrams(Node& node, int order);
	int readNumofGrams(ifstream& is);
	void readNgram(ifstream& is, int order);
	void print(ostream& os, Node& node, string sufix, vector<int>& context, int order, bool countOnly);
	void wbEstimate(Node& node);
	void reComputeLowerCount(Node& node, vector<int>& context, int order);
	void clearCountsByLayer(Node& node, int order, int exception);
	void collectCountofCount(Node& node, vector<double>& coc, int order,bool logrized);
	void setKNcountsByLayer(Node& node, vector<int>& context, int order);
    void setKNcountsByLayerTam(Node& node, vector<int>& context, int order, double threshold);
	void setMissKNcountsByLayer(Node& node, vector<int>& context, int order);
	void cleanBadEntriesByLayer(Node& node, vector<int>& context, int order);
	
	vector<int> _numofGrams;
	Node _root;
	Vocab _vocab;
};
void readPlainFile(string filename, int order, double*& p_weight, int nSentence,
                   unordered_map<string,pair<FracType,double>>& ngrams);

#endif
