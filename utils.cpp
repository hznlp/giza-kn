#include "utils.h"
double log2_10=log(10.0)/log(2.0);
string
intToString(int i)
{
  stringstream ss;
  ss<<i; 
  string ans;
  ss>>ans;
  return ans;
}
string doubleToString(double i)
{
  stringstream ss;
  //ss.precision(20);
  if(i==0)
	  return "0.0";
  ss<<i; 
  string ans;
  ss>>ans;
  return ans;
}
string& replaceSubStr(string& in, string oldSub, string newSub)
{
	size_t pos=0;
	while(true)
	{
		pos=in.find(oldSub,pos);
		if(pos!=string::npos)
		{
			in.replace(pos,oldSub.length(),newSub);
			pos+=newSub.length();
		}
		else break;
	}
	return in;
}
string& removeSpace(string& input)
{
	string result="";
	for(size_t i=0;i<input.length();i++)
	{
		if(isgraph(input[i]))result+=input[i];
		//cout<<(int)input[i]<<"="<<(char)input[i]<<" ";
	}
	//cout<<endl;
	input=result;
	return input;
}

int countSubStr(const string& in, const string& sub)
{
	int count=0;
	size_t pos=0;
	while(true)
	{
		pos=in.find(sub,pos);
		if(pos!=string::npos)
		{
			count++;
			pos+=sub.length();
		}
		else break;
	}
	return count;
}

bool endsWith(char * src, char* tail)
{
	size_t srclen=strlen(src);
	size_t taillen=strlen(tail);
	return !strcmp(src+(srclen-taillen),tail);
}

vector<string>& stringToVector(const string& sent, vector<string>& result)
{
	stringstream ss(sent.c_str());
	result.clear();
	while(!ss.eof())
	{
		string word="";
		ss>>word;
		if(word=="")break;
		result.push_back(word);
	}
	return result;
}

bool isSpace(char ch)
{
	return ch==' '||ch=='\t'||ch=='\n'||ch=='\r';
}
string& trim(const string& in, string& out)
{
	string result="";
	int start=0,stop=0;
	for(start=0;start<static_cast<int>(in.length());start++)
	{
		if(isSpace(in[start]))
			continue;
		else 
			break;
	}
	for(stop=static_cast<int>(in.length())-1;stop>=0;stop--)
	{
		if(isSpace(in[stop]))
			continue;
		else
			break;
	}
	for(int i=start;i<=stop;i++)
	{
		result+=in[i];
	}
	out=result;
	return out;
}

bool lineWithNoRealWords(string line)
{
	stringstream tmp_ss(line.c_str());
	string wrd="";tmp_ss>>wrd;
	return (wrd=="");
}	

string& lowercase(const string& in, string& out)
{
	string result=in;
	for(size_t i=0;i<result.length();i++)
	{
		if(result[i]>='A'&&result[i]<='Z')
			result[i]+='a'-'A';
	}
	out=result;
	return out;
}

string vectorToString(vector<string>& words)
{
	string result="";
	int len=(int)words.size();
	for(int i=0;i<len;i++)
	{
		result+=words[i];
		if(i!=len-1)
			result+=" ";
	}
	return result;
}

bool extractNgram(const string& istr, vector<string>& ngrams, int order)
{
	//cout<<istr<<endl;
	vector<string> sent;
	stringstream ss(istr);
	while(ss.good())
	{
		string str="";
		ss>>str;
		if(str=="")break;
		sent.push_back(str);
	}
	for(int i=0; i<static_cast<int>(sent.size())-order+1;i++)
	{
		string str="";
		int len=0;
		for(;len<order-1;len++)
		{
			str+=sent[i+len]+" ";
		}
		str+=sent[i+len];
		ngrams.push_back(str);
	}
	return true;
}

#ifdef WIN32
double log1p(double x)
{
    if (x <= -1.0)
    {
		cerr<<"x less than -1"<<endl;
        exit(1);
    }

    if (fabs(x) > 1e-4)
    {
        // x is large enough that the obvious evaluation is OK
        return log(1.0 + x);
    }

    // Use Taylor approx. log(1 + x) = x - x^2/2 with error roughly x^3/3
    // Since |x| < 10^-4, |x|^3 < 10^-12, relative error less than 10^-8

    return (-0.5*x + 1.0)*x;
}

double expm1(double x)
{
        if (fabs(x) < 1e-5)
                return x + 0.5*x*x;
        else
                return exp(x) - 1.0;
}
#endif
