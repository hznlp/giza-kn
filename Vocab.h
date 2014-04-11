#ifndef VOCAB_H
#define VOCAB_H
#include <string>
#include <vector>
#include "ZMap.h"
const string unk = "<unk>";
const string sent_begin = "<s>";
const string sent_end = "</s>";

class Vocab
{
public:
	Vocab();
	int add(const string& wrd);
	string int2str(int ind){if(ind<0||ind>=(int)_int2str.size())return "";else return _int2str[ind];}
	int lookup(const string& wrd, bool forced);
	int lookup(string* pWrd, vector<int>& context, int clen, bool forced);
	void reverse(vector<int>& context);
	void reverse(int* context, int len);
	void clear(){_dic.clear();_int2str.clear();}
	size_t size(){return _dic.size();}
	void remove(int id);
	map<string,int>& dic(){return _dic;}
private:
	map<string,int> _dic;
	vector<string> _int2str;
};

#endif
