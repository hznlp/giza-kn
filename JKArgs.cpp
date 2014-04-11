#include "JKArgs.h"

JKArgs::
JKArgs(int argc,char *argv[])
{
	init(argc,argv);
}

JKArgs::
JKArgs(const char * filename)
{
	init(filename);
}

void
JKArgs::
init(int argc,char *argv[])
{
    for(int i=1;i<argc;i++)
    {
		process_one_arg(argv[i]);
    }
}

void
JKArgs::
init(const char *filename)
{
	ifstream file;
	file.open(filename);
	if(!file.good())
	{
		cerr<<"open "<<filename<<" failed"<<endl;
		file.close();
		return;
	}
	else 
		cerr<<"open "<<filename<<" succeed"<<endl;

	string arg="";
	for(;!file.eof();)
    {
		arg="";
		getline(file,arg);
		//cerr<<arg<<endl;
		if(arg!="")
			process_one_arg(arg);
/*		if(file.badbit||file.failbit)
		{
			file.clear();
			file.close();
			file.open(filename);
		}
*/
    }
	file.close();
}

void 
JKArgs::
process_one_arg(string arg)
{
	string value;
	size_t pos;
	//cerr<<"processing:"<<arg<<endl;
	//cerr<<arg<<endl;
	while(arg.size()>0&&arg[0]==' ')
		arg.erase(0,1);
	if(arg=="")return;
	while(!isgraph(arg[arg.length()-1]))
	{
		//cerr<<"erase "<<arg[arg.length()-1]<<":"<<endl;
		arg.erase(arg.length()-1,1);
		if(arg.length()==0)return;
	}

	if(arg[0]=='-'||arg[0]=='/')
    {
		arg.erase(0,1);
    }
	else return;
    pos=arg.find("=");
    if(pos==string::npos)
    {
		value="";
    }
    else
    {
        value=arg.substr(pos+1,arg.size()-pos-1);
        arg.erase(pos,arg.size()-pos);
    }
	 //cerr<<arg<<" = "<<value<<endl;
    argsPool[arg]=value;
}

JKArgs::
~JKArgs()
{
	argsPool.clear();
}
bool
JKArgs:: 
is_set(string arg)
{
	if(argsPool.find(arg)==argsPool.end()) 
		return false;
	else return true;
}
string
JKArgs::
value(string arg)
{
	if(argsPool.find(arg)==argsPool.end()) 
		return "";
	else return argsPool[arg];
}

void 
JKArgs::
print(ostream& os)const
{
	map<string,string>::const_iterator citer=argsPool.begin();
	for(;citer!=argsPool.end();citer++)
	{
		os<<"-"<<citer->first<<"="<<citer->second<<endl;
	}
}
