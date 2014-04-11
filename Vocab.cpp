#include "Vocab.h"
Vocab::Vocab()
{
	_dic.clear();
	/*add(unk);
	add(sent_begin);
	add(sent_end);*/
}

int 
Vocab::
add(const string& wrd){
	if(_dic.find(wrd)!=_dic.end())
		return _dic[wrd];
	int ind=(int)_dic.size();
	_dic[wrd]=(int)ind;
	_int2str.push_back(wrd);
	return ind;
}

int 
Vocab::
lookup(const string& wrd, bool forced)
{
	if(forced)
		return add(wrd);
	else if(_dic.find(wrd)==_dic.end())
		return _dic[unk];
	else return _dic[wrd];
}

void 
Vocab::
remove(int id)
{
	if(id>=_int2str.size())
		return; 
	string str=_int2str[id];
	//_int2str.erase(_int2str.begin()+id);
	_dic.erase(_dic.find(str));
}

int 
Vocab::
lookup(string* pWrd, vector<int>& context, int clen, bool forced)
{
	for(;clen>0;clen--,pWrd++)
	{
		context.push_back(lookup(*pWrd,forced));
	}
	return (int)context.size();
}

void 
Vocab::
reverse(vector<int>& context)
{
	reverse(&context[0],(int)context.size());
}

void 
Vocab::
reverse(int* context, int len)
{
	int hlen=len/2;
	for(int i=0;i<hlen;i++)
		swap(context[i],context[len-1-i]);
}
