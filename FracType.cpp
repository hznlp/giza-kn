#include "FracType.h"

double logAdd(double logx, double logy)
{
	if(abs(logx)>308)return logy;
	if(abs(logy)>308)return logx;
	return logx+log1p(exp(logy-logx));
}

double discountMass(FracType& ft, vector<double>& discount)
{
	double mass=0;
	double n3=1-ft[0]-ft[1]-ft[2];
	if(n3<0)n3=ft[3];
	mass=(LogDouble)discount[0]*ft[1]+(LogDouble)discount[1]*ft[2]+
                (LogDouble)discount[2]*(LogDouble)n3;
	return mass;
}

int 
FracType::
compare(FracType& f)
{
	for(int i=0;i<5;i++)
	{
		if(abs((*this)[i]-f[i])>1E-10)
		{
			cerr<<(*this)[i]<<"!="<<f[i]<<endl;
			return i;
		}
	}
	return -1;
}

void
FracType::
update(double p)
{
	FracType& t=*this;
	while(p>1.01)
	{
		update(1);
		p-=1;
	}
	if(p==0)return;
	for(int i=4;i>0;i--)
        t[i]=t[i-1]*(LogDouble)p+t[i]*(LogDouble)(1-p);
    t[0]*=(LogDouble)(1-p);
    //print(cerr);
    //cerr<<endl;
}
