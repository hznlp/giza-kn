#ifndef JKARGS_H
#define JKARGS_H
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
using namespace std;
class JKArgs{
   public:
	JKArgs(){argsPool.clear();};
	JKArgs(int argc,char *argv[]);
	JKArgs(const char * filename);
	void init(int argc,char *argv[]);
	void init(const char * filename);
	~JKArgs(void);
	bool is_set(string arg);
    bool count(const string& arg){return argsPool.find(arg)!=argsPool.end();}
	string value(string arg);
	size_t num_of_args(void) {return argsPool.size();}
	void print(ostream& os)const;
	void erase(string key){if(argsPool.find(key)!=argsPool.end())argsPool.erase(argsPool.find(key));}
    string operator[](const string& str){return argsPool[str];}
   private:
	map<string, string> argsPool;
	void process_one_arg(string arg);
};
#endif
