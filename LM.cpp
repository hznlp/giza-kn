#include "LM.h"
int _debug=0;

Node*
LM::
setNode(int* context, int clen)
{
	Node* pNode=&_root;
	for(;clen>0;clen--,context++)
	{
		int wrd=*context;
		pNode=&(pNode->childs[wrd]);
	}
	return pNode;
}

void 
Node::
normalize()
{
	double total=0;
	for(ZMap<int, double>::iterator iter=probs.begin();iter!=probs.end();iter++)
		total+=iter->second;
	for(ZMap<int, double>::iterator iter=probs.begin();iter!=probs.end();iter++)
		iter->second=log10(iter->second/total);
	for(ZMap<int, Node>::iterator iter=childs.begin();iter!=childs.end();iter++)
		iter->second.normalize();
}

void 
Node::
prune(double threshold)
{
	for(ZMap<int, double>::iterator iter=probs.begin();iter!=probs.end();)
	{
		ZMap<int, double>::iterator curIter=iter;
		iter++;
		if(curIter->second<=threshold)
		{
			int wrd=curIter->first;
			probs.erase(probs.find(wrd));
			types.erase(types.find(wrd));
            //cout<<"prune "<<curIter->second<<" "<<threshold<<endl;
		}
	}
	//if(probs.size()==0)childs.clear();
}

void 
Node::
erase(int wrd)
{
	ZMap<int, double>::iterator iter=probs.find(wrd);
	if(iter!=probs.end())probs.erase(iter);
	ZMap<int, FracType>::iterator titer=types.find(wrd);
	if(titer!=types.end())types.erase(titer);
}

Node* 
LM::lookupNode(int* context, int clen)
{
	Node* pNode=&_root;
	for(;clen>0;clen--,context++)
	{
		int wrd=*context;
		if(pNode->childs.find(wrd)!=pNode->childs.end())
			pNode=&(pNode->childs[wrd]);
		else return NULL;
	}
	return pNode;
}

double& 
LM::setProb(int wrd, int* context, int clen)
{
	Node* pNode=setNode(context,clen);
	if(pNode->probs.find(wrd)==pNode->probs.end())
		pNode->probs[wrd]=0;
	return pNode->probs[wrd];
}

Node* 
LM::
addFracCount(int wrd, int* context, int clen, double fcount, double ftype)
{
	Node* pNode=setNode(context,clen);
	if(pNode->probs.find(wrd)==pNode->probs.end())
		pNode->probs[wrd]=0;
	pNode->probs[wrd]+=fcount;
	pNode->types[wrd].update(ftype);

    return pNode;
}

Node*
LM::
addFracCount(int wrd, int* context, int clen, double fcount, FracType& ftype){
    Node* pNode=setNode(context,clen);
	pNode->probs[wrd]=fcount;
	pNode->types[wrd]=ftype;
    return pNode;
}

void 
LM::
wbEstimate(Node& node)
{
	double ntype=0;
	double ntoken=0;
	ZMap<int,double>::iterator iter;
	iter=node.probs.begin();
	for(;iter!=node.probs.end();iter++)
		ntoken+=iter->second;
	
	ZMap<int,FracType>::iterator tIter;
	tIter=node.types.begin();
	for(;tIter!=node.types.end();tIter++)
		ntype-=expm1(tIter->second[0]);

	//	ntype+=(1-iter->second);
	//cerr<<"ntype:"<<ntype<<endl;
	if(ntype==0){cerr<<"error, ntype is 0"; exit(1);}
	node.bow=log10(ntype)-log10(ntype+ntoken);
	
	iter=node.probs.begin();
	for(;iter!=node.probs.end();iter++)
	{
		if(iter->second==0){cerr<<"error, nfrac is 0"; exit(1);}
		iter->second=log10(iter->second)-log10(ntype+ntoken);
	}
	node.types.clear();
	
	ZMap<int,Node>::iterator childIter=node.childs.begin();
	for(;childIter!=node.childs.end();childIter++)
		wbEstimate(childIter->second);
}

void
LM::
mlEstimate(Node& node)
{
	double ntoken=0;
	ZMap<int,double>::iterator iter;
	iter=node.probs.begin();
	for(;iter!=node.probs.end();iter++)
		ntoken+=iter->second;
	node.bow=-99;

	iter=node.probs.begin();
	for(;iter!=node.probs.end();iter++)
	{
		if(iter->second==0){cerr<<"error, nfrac is 0"; exit(1);}
		iter->second=log10(iter->second)-log10(ntoken);
	}
	node.types.clear();

    for(auto& child : node.childs)
		mlEstimate(child.second);
}


void 
LM::
computeBOW(bool interpolate)
{
	vector<int> context; 
	for(int i=0;i<order();i++)
		computeBOWByLayer(_root,context,i+1, interpolate);
}

void
LM::
setSomeDetails()
{
   	_root.probs[_vocab.add(unk)]=_root.bow;
	_root.probs[_vocab.add(sent_begin)]=-99;
	calcNumOfGrams();
}

double& 
LM::setBackoff(int* context, int clen)
{
	Node* pNode=setNode(context,clen);
	return pNode->bow;
}

double 
LM::
probBO(int wrd, int* context, int clen)
{
	double prob=0;
	double bow=0;
	Node* pNode=&_root;
	for(;clen>=0;context++,clen--)
	{
		if(pNode->probs.find(wrd)!=pNode->probs.end())
		{
			prob=pNode->probs[wrd];
			bow=0;
		}
		if(clen==0)	break;
		int cwrd=*context;
		if(pNode->childs.find(cwrd)==pNode->childs.end()) break;
		pNode=&(pNode->childs[cwrd]);
		bow+=pNode->bow;
	}
	double penalty=0; //in case clen > 0
	return prob+bow+penalty*clen;
}

bool
LM::
checkEntry(int wrd, int* context, int clen)
{
	Node* pNode=&_root;
	for(;clen>=0;context++,clen--)
	{
		if(pNode->probs.find(wrd)==pNode->probs.end())return false;
		if(clen==0)	break;
		int cwrd=*context;
		if(pNode->childs.find(cwrd)==pNode->childs.end()) return false;
		pNode=&(pNode->childs[cwrd]);
	}
	return true;
}

void 
LM::
stat(int wrd, int* context, int clen, LMStat& lmstat)
{
	int real_clen=clen+1;
	Node* pNode=&_root;
	for(;clen>=0;context++,clen--)
	{
		if(clen==0)	break;
		int cwrd=*context;
		if(pNode->childs.find(cwrd)==pNode->childs.end()) break;
		pNode=&(pNode->childs[cwrd]);
	}
	real_clen-=clen;
	if(wrd==_vocab.add(unk))real_clen=0;
	lmstat.ngrams[real_clen]++;
	return;
}

void 
LM::
read(string filename)
{
	ifstream is(filename.c_str());
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline=="\\data\\")
		{
			//cout<<endl<<curline<<endl;
			readNumofGrams(is);
		}
		else if(curline.find("-grams:")!=string::npos)
		{
			//cout<<curline<<endl;
			curline=curline.substr(curline.find("\\")+1);
			curline=curline.substr(0,curline.find("-"));
			int order=atoi(curline.c_str());
			readNgram(is,order);
		}
		//else if(curline=="\\end\\")	cout<<curline<<endl;
	}
}

int 
LM::
readNumofGrams(ifstream& is)
{
	string curline="";
	while(!is.eof())
	{
		getline(is,curline);
		//cout<<curline<<endl;
		if(curline.find("ngram")!=0)break;
		curline.erase(0,curline.find("=")+1);
		_numofGrams.push_back(atoi(curline.c_str()));
	}
	return (int)_numofGrams.size();
}

void 
LM::
readNgram(ifstream& is, int order)
{
	string curline="";
	while(!is.eof())
	{
		getline(is,curline);
		if(curline=="")
		{
			//cout<<curline<<endl;
			return;
		}
		vector<string> wrds;
		stringToVector(curline,wrds);
		double probRead=atof(wrds[0].c_str());
		vector<int> context;
		_vocab.lookup(&wrds[1],context,order,true);
		_vocab.reverse(context);
		double& prob=setProb(context[0],&context[0]+1,order-1);
		prob=probRead;
		//cout<<probRead;
		//for(int i=0;i<order;i++)cout<<" "<<wrds[i+1];

		if((int)wrds.size()==order+2)
		{
			double bowRead=atof(wrds.back().c_str());
			//if(order>1)cerr<<"setting bow "<<bowRead<<endl;
			double& bow=setBackoff(&context[0],order);
			bow=bowRead;
			//cout<<" "<<bowRead;
		}
		//cout<<endl;
	}
}

void 
LM::
clearCountsByLayer(Node& node, int order, int exception)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			if(iter->first!=exception)
			clearCountsByLayer(iter->second,order-1,exception);
		}
	}
	else
	{
		node.probs.clear();
		node.types.clear();
	}
}

void 
LM::
setKNcountsByLayer(Node& node, vector<int>& context, int order)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			context.push_back(iter->first);
			setKNcountsByLayer(iter->second,context,order-1);
			context.pop_back();
		}
	}
	else
	{
		int clen=(int)context.size()-1;
		if(clen<0)return;
		//cerr<<"lostType size:"<<node.types.size()<<endl;
		for(map<int,FracType>::iterator typeIter=node.types.begin();typeIter!=node.types.end();typeIter++)
			addFracCount(typeIter->first,&context[0],clen,1-typeIter->second[0],1-typeIter->second[0]);
	}
}

void
LM::
setKNcountsByLayerTam(Node& node, vector<int>& context, int order, double threshold)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			context.push_back(iter->first);
			setKNcountsByLayerTam(iter->second,context,order-1,threshold);
			context.pop_back();
		}
	}
	else
	{
		int clen=(int)context.size()-1;
		if(clen<0)return;
		//cerr<<"lostType size:"<<node.types.size()<<endl;
		for(map<int,double>::iterator probIter=node.probs.begin();probIter!=node.probs.end();probIter++){
            double count=probIter->second;
            if(count>threshold)count=1;
                addFracCount(probIter->first,&context[0],clen,count,count);
        }
	}
}

void 
LM::
setMissKNcountsByLayer(Node& node, vector<int>& context, int order)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			context.push_back(iter->first);
			setMissKNcountsByLayer(iter->second,context,order-1);
			context.pop_back();
		}
	}
	else
	{
		for(int clen=(int)context.size()-1;clen>=(int)context.size()-1;clen--)
		{
			if(clen<0)return;
			Node* pnode=lookupNode(&context[1],clen);
			if(pnode==NULL)setNode(&context[1],clen);
			if(pnode->probs.find(context[0])==pnode->probs.end())
			{
				double totalProb=0;
				for(map<int,double>::iterator probIter=node.probs.begin();probIter!=node.probs.end();probIter++)
					totalProb+=probIter->second;
				if(totalProb!=0)
					addFracCount(context[0],&context[1],clen,totalProb,totalProb);
			}
		}
		//cerr<<"lostType size:"<<node.types.size()<<endl;
	}
}

void 
LM::
cleanBadEntriesByLayer(Node& node, vector<int>& context, int order)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();)
		{
			map<int,Node>::iterator curIter=iter;
			iter++;
			context.push_back(curIter->first);
			cleanBadEntriesByLayer(curIter->second,context,order-1);
			if(curIter->second.probs.size()==0)node.childs.erase(curIter);
			context.pop_back();
		}
	}
	else
	{
		int clen=(int)context.size()-1;
		if(context[0]!=_vocab.lookup(sent_begin,false))
		if(!checkEntry(context[0],&context[0]+1,clen)){node.probs.clear();node.types.clear();node.childs.clear();}
	}
}


void 
LM::
calculateDiscounts(vector<vector<double> >& discounts, bool logrized)
{
	for(int i=0;i<order();i++)
	{
		vector<double> discount;
		discounts.push_back(discount);
		vector<double> coc(4,0);
		collectCountofCount(_root,coc,i+1,logrized);
		calculateDiscount(coc,discounts[i]);
		for(int j=0;j<(int)coc.size();j++)
			cerr<<"n"<<j+1<<": "<<coc[j]<<endl;
		for(int j=0;j<(int)discounts[i].size()-1;j++)
			cerr<<"D"<<j+1<<": "<<discounts[i][j]<<endl;
	}
}

void
calculateDiscount(vector<double>& coc, vector<double>& discount)
{
	discount.clear();
	double n1=coc[0];
	double n2=coc[1];
	double n3=coc[2];
	double n4=coc[3];
	double Y=n1/(n1+2*n2);
	discount.push_back(1-2*Y*n2/n1);
	discount.push_back(2-3*Y*n3/n2);
	discount.push_back(3-4*Y*n4/n3);
	discount.push_back(3-4*Y*n4/n3);
        for(size_t i=1;i<discount.size();i++)
	{
		if(discount[i]<0){
			cerr<<"discount "<<i<<":"<<discount[i]<<endl;
			discount[i]=discount[i-1];
		}
	}
}

int discountSlot(double count)
{
	for(int i=1;i<4;i++)
		if(count<=double(i))
			return i-1;
	return 3;
}

void 
LM::
calcNumOfGrams(Node& node, int order)
{
	while((int)_numofGrams.size()<=order)_numofGrams.push_back(0);
	_numofGrams[order]+=(int)node.probs.size();
	for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		calcNumOfGrams(iter->second,order+1);
}

void
LM::
reComputeLowerCount()
{
	for(int order=(int)(_numofGrams.size());order>1;order--)
	{
		vector<int> context;
		reComputeLowerCount(_root,context,order);
	}
}

void 
LM::
reComputeLowerCount(Node& node, vector<int>& context, int order)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			context.push_back(iter->first);
			reComputeLowerCount(iter->second,context,order-1);
			context.pop_back();
		}
	}
	else
	{
		_vocab.reverse(context);
		for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
		{
			addFracCount(iter->first,&context[0]+1,(int)context.size()-1,iter->second,1-node.types[iter->first][0]);
		}
		_vocab.reverse(context);
	}
}

void 
LM::
computeBOWByLayer(Node& node, vector<int>& context, int order, bool interpolate)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			context.push_back(iter->first);
			computeBOWByLayer(iter->second,context,order-1,interpolate);
			context.pop_back();
		}
	}
	else
	{
		double numerator=1.0;
		double denumerator=1.0;
		//cerr<<"lower weight: "<<exp10(node.bow)<<endl;
		//double sumu=0;
		for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
		{
			double lowLogProb=0;
			if(context.size()>0)
			{
				lowLogProb=probBO(iter->first,&context[0],(int)context.size()-1);
				denumerator-=exp10(lowLogProb);
			}
			else
				lowLogProb=-log10((double)node.probs.size()+1);
			iter->second=exp10(iter->second);
			if(interpolate)iter->second+=exp10(node.bow+lowLogProb);

			numerator-=iter->second;
			iter->second=log10(iter->second);
		}
		//cerr<<"numerator:"<<numerator<<", sumu:"<<sumu<<endl;
		if(context.size()>0)
			node.bow=log10(numerator)-log10(denumerator);
		else
			node.bow-=log10((double)node.probs.size()+1);
	}
}

void
LM::
collectCountofCount(Node& node, vector<double>& coc, int order, bool logrized)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			collectCountofCount(iter->second,coc,order-1, logrized);
		}
	}
	else
	{
		for(map<int,FracType>::iterator typeIter=node.types.begin();typeIter!=node.types.end();typeIter++)
		{
			FracType& ftype=typeIter->second;
			for(int i=1;i<5;i++)
			{
                coc[i-1]+=ftype[i];
			}
		}
	}
}

void 
LM::
resetVocab()
{
	vector<int> ids(_vocab.size(),0);
	for(map<int,double>::iterator iter=_root.probs.begin();iter!=_root.probs.end();iter++)
	{
		ids[iter->first]=1;
	}
	ids[_vocab.lookup(sent_begin,false)]=1;
	ids[_vocab.lookup(sent_end,false)]=1;
	ids[_vocab.lookup(unk,false)]=1;
	for(size_t id=1;id<ids.size();id++)
	{
		if(ids[id]==0)_vocab.remove((int)id);
	}
}

void 
LM::
knEstimate(bool isBackWard, bool interpolate, DiscountType dt, double * tamcutoff)
{
	vector<vector<double> > discounts;
	if(tamcutoff!=NULL)
	{
        setKNcountsTam(*tamcutoff);
        vector<double> cutoffs(order()+1,*tamcutoff);
        prune(&cutoffs[0]);
        for(int i=0;i<order();i++){
            discounts.push_back(vector<double>(4,*tamcutoff));
            for(int j=0;j<(int)discounts[i].size()-1;j++)
                cerr<<"D"<<j+1<<": "<<discounts[i][j]<<endl;
        }
    }
    else{
        setKNcounts(isBackWard);
        calculateDiscounts(discounts);
    }
	if(dt==D_AD)
	{
        cerr<<"start ad"<<endl;
       	adEstimate(_root, discounts, 0);
	}
    else
		wbEstimate(_root);
	cerr<<"start compute bow"<<endl;
    computeBOW(interpolate);
    cerr<<"start resetVocab"<<endl;
	//resetVocab();
    cerr<<"start set Some details"<<endl;
}

void 
LM::
normalize()
{
	_root.normalize();
}

void 
LM::
prune(Node& node, double* thresholds, int order)
{
	double threshold=0;
    if(thresholds!=NULL)threshold=thresholds[order];
	node.prune(threshold);
	for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
	{
		prune(iter->second,thresholds,order+1);
	}
}

void 
LM::
adEstimate(Node& node, vector<vector<double> >& discounts, int order)
{
	vector<double> discount=discounts[order];
	double subtract=0;
	double total=0;
	if(node.probs.size()>0)
	{
		for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
		{
			total+=iter->second;
			FracType& ft=node.types[iter->first];
			double dMass=discountMass(ft,discount);
			iter->second-=dMass;
			subtract+=dMass;
		}

		if(isBadNumber(log10(subtract)))
		{
			cerr<<"subtract is "<<subtract<<endl;
			for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
			{
				FracType& ft=node.types[iter->first];
				double mass=0;
				double n3=1-ft[0]-ft[1]-ft[2];
				if(n3<0)n3=ft[3];
				ft.print(cerr);
				mass=discountMass(ft,discount);
				cerr<<mass<<"="<<discount[0]<<"*"<<ft[1]<<"+"<<discount[1]<<"*"<<ft[2]<<"+"<<discount[2]<<"*"<<n3<<endl;
				cerr<<iter->second<<"-"<<mass<<endl;
			}
            cerr<<"bad number"<<endl;
			exit(1);
		}
		node.bow=log10(subtract)-log10(total);
	}
	for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
	{
		//cout<<_vocab.int2str(iter->first)<<", count:"<<iter->second<<", total:"<<total<<endl;
		iter->second=log10(iter->second)-log10(total);
	}

	for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
	{
		adEstimate(iter->second,discounts,order+1);
	}
}

void 
LM::
setKNcounts(bool isBackWard)
{
	for(int i=2;i<=order();i++)
	{
		int layer=i;
		//if(isBackWard)layer=order()-layer+2;
		vector<int> context;
		clearCountsByLayer(_root,layer-1,_vocab.add(sent_begin));
		setKNcountsByLayer(_root,context,layer);
	}
}

void
LM::
setKNcountsTam(double threshold)
{
	for(int i=2;i<=order();i++)
	{
		int layer=i;
		vector<int> context;
		clearCountsByLayer(_root,layer-1,_vocab.add(sent_begin));
		setKNcountsByLayerTam(_root,context,layer,threshold);
	}
}

void 
LM::
print(ostream& os, Node& node, string sufix, vector<int>& context, int order, bool countOnly)
{
	if(order>1)
	{
		for(map<int,Node>::iterator iter=node.childs.begin();iter!=node.childs.end();iter++)
		{
			string newsufix=_vocab.int2str(iter->first)+" "+sufix;
			context.push_back(iter->first);
			print(os,iter->second,newsufix,context,order-1,countOnly);
			context.pop_back();
		}
	}
	else
	{
		for(map<int,double>::iterator iter=node.probs.begin();iter!=node.probs.end();iter++)
		{
			string str=_vocab.int2str(iter->first);
			if(sufix!="")
			str=sufix+str;
			string probStr=doubleToString(iter->second);

			if(countOnly)
			{
				//for(int i=0;i<(int)context.size();i++)os<<context[i]<<" ";
				//os<<iter->first<<" "<<iter->second<<endl;
				os<<str+"\t"+probStr<<"\ttype: "<<node.types[iter->first]<<endl;
				continue;
			}
			else
			{
				str=probStr+"\t"+str;
				os<<str;
				//lookup BOW
				vector<int> bowContext;
				bowContext.push_back(iter->first);
				bowContext.insert(bowContext.end(),context.begin(),context.end());
			
				Node* tmpNode=lookupNode(&bowContext[0],(int)bowContext.size());
				if(tmpNode!=NULL)
					os<<"\t"<<tmpNode->bow<<endl;
				else
					os<<"\t"<<0<<endl;
			}
		}
	}
}

void 
LM::
write(string filename)
{
	ofstream os(filename.c_str());
	vector<int> context;

	os<<"\n\\data\\"<<endl;

	for(int i=1;i<=(int)_numofGrams.size();i++)
	{
		os<<"ngram "<<i<<"="<<_numofGrams[i-1]<<endl;
	}
	os<<endl;

	for(int i=1;i<=(int)_numofGrams.size();i++)
	{
		os<<"\\"<<i<<"-grams:"<<endl;
		print(os,_root,"",context,i,false);
		os<<endl;
	}
	os<<"\\end\\"<<endl;
	os.close();
}

void 
LM::
writeCount(string filename)
{
	ofstream os(filename.c_str());
	vector<int> context;

	for(int i=1;i<=(int)_numofGrams.size();i++)
	{
		print(os,_root,"",context,i,true);
	}
	os.close();
}

double 
LM::
sentLogProb(string& sentence, int ord, LMStat* pStat)
{
	string curline="<s> "+sentence+" </s>";
	vector<string> wrds;
	stringToVector(curline,wrds);
	if(wrds.size()==0)return 0;
	vector<int> context;
	_vocab.lookup(&wrds[0],context,(int)wrds.size(),false);
	if(pStat!=NULL)pStat->nwrds=(int)wrds.size();
	return sentLogProb(&context[0],(int)context.size(),ord,pStat);
}

double 
LM::
sentLogProb(int* context, int slen, int order, LMStat* pStat)
{
	double prob=0;
	_vocab.reverse(context,slen);
	for(int i=0;i<slen-1;i++)
	{
		int clen=slen-i-1>order-1?order-1:slen-i-1;
		double curProb=probBO(context[i],context+i+1,clen);
		if(_debug==2)
		cout<<curProb<<": "<<_vocab.int2str(context[i])<<" | "<<_vocab.int2str(context[i+1])<<endl;
		prob+=curProb;
		if(pStat!=NULL)
			stat(context[i],context+i+1,clen,*pStat);
	}
	_vocab.reverse(context,slen);
	return prob;
}

double 
LM::
ppl(string filename, LMStat* pStat, string probFilename,bool average)
{
	ifstream is(filename.c_str());
	double prob=0;
	int nwrds=0;
	ofstream os;
	if(probFilename!="")
		os.open(probFilename.c_str());
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		curline="<s> "+curline+" </s>";
		vector<string> wrds;
		stringToVector(curline,wrds);
		if(wrds.size()==2)continue;
		vector<int> context;
		_vocab.lookup(&wrds[0],context,(int)wrds.size(),false);
		double slp=sentLogProb(&context[0],(int)context.size(),order(),pStat);
		prob+=slp;
		nwrds+=(int)(context.size()-1);
		if(probFilename!="")
		{
			if(average)
				os<<slp/(double)(context.size()-1)<<" "<<context.size()-1<<endl;
			else
				os<<slp<<" "<<context.size()-1<<endl;
		}
	}
	if(pStat!=NULL)	pStat->nwrds=nwrds;
	if(probFilename!="")os.close();
	return prob;
}

void 
LM::
readCountFile(string filename)
{
	ifstream is(filename.c_str());
	int order=0;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		vector<string> wrds;
		stringToVector(curline,wrds);
		if((int)wrds.size()<2)continue;
		vector<int> context;
		double count=atof(wrds.back().c_str());
		for(int i=0;i<(int)wrds.size()-1;i++)
		{
			context.push_back(_vocab.add(wrds[i]));
			//cerr<<wrds[i]<<" ";
		}
		//cerr<<count<<endl;
		_vocab.reverse(context);

		addFracCount(context[0],&context[0]+1,(int)context.size()-1,count,count);
		order=max(order,(int)context.size());
	}
	for(int i=0;i<order;i++)
	{
		if((int)_numofGrams.size()<=i)
			_numofGrams.push_back(0);
	}
	is.close();
}


bool
LM::
addOneLine(string& curline, int order, double*& p_weight, double threshold, double floor)
{
    double fraccount=1;
	if(p_weight!=NULL){fraccount=*p_weight;p_weight++;}
	if(fraccount<=threshold)return false;
	if(fraccount<floor)fraccount=floor;

	curline="<s> "+curline+" </s>";
	vector<string> wrds;
	stringToVector(curline,wrds);
	vector<int> context;
	
	for(int i=0;i<(int)wrds.size();i++)
	{
		//int ind=_vocab.add(wrds[i]);
		context.push_back(_vocab.add(wrds[i]));
	}
	_vocab.reverse(context);

	for(int i=0;i<(int)context.size()-1;i++)
	{
		int maxDepth=min((int)context.size()-i-1,order-1);
		for(int depth=0;depth<=maxDepth;depth++)
		{
			addFracCount(context[i],&context[i]+1,depth,fraccount,fraccount);
		}
	}
	return true;
}

void 
LM::
setOrder(int order)
{
	for(int i=0;i<order;i++)
	{
		if((int)_numofGrams.size()<=i)
			_numofGrams.push_back(0);
	}
}

void
LM::
estimate(Smoothing sm, bool interpolate, double* tamcutoff){
    if(sm==KN)knEstimate(interpolate,tamcutoff);
    else if(sm==WB)wbEstimate(interpolate,tamcutoff);
    else if(sm==NO)mlEstimate(_root);
    setSomeDetails();
}

void
LM::
wbEstimate(bool interpolate, double* cutoff){
    //prune(cutoff);
    wbEstimate(_root);
    computeBOW(interpolate);
}

void
LM::
knEstimate(bool interpolate, double* tamcutoff){
    knEstimate(false,interpolate,D_AD,tamcutoff);
}

int
LM::
readPlainFile(string filename, int order, double* p_weight, double threshold, double floor, int nSentence)
{
	ifstream is(filename.c_str());
    if(!is.good())return 0;
	init(order);

	int sentCount=0;
    int usedCount=0;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")continue;
		sentCount++;
		if(sentCount>nSentence&&nSentence>0)break;
		usedCount+=addOneLine(curline,order,p_weight,threshold,floor);
	}
	is.close();
	return usedCount;
}

//void readCountFile(string filename, unordered_map<string,FracType>& ngrams);

void readPlainFile(string filename, int order, double*& p_weight, int nSentence,
                   unordered_map<string,pair<FracType,double>>& ngrams){
    ifstream is(filename.c_str());
    int sentCount=0;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")continue;
		sentCount++;
		if(sentCount>nSentence&&nSentence>0)break;
        double fcount=1;
        if(p_weight!=NULL){fcount=*p_weight++;}
        vector<string> wrds;
        curline="<s> "+curline+" </s>";
        split(wrds,curline,is_any_of(" \t"));

        string str="";
        for(int len=0;len<order-1;len++){
            str+=wrds[len];
            if(ngrams.find(str)==ngrams.end())ngrams[str].second=0;
            pair<FracType,double>& p=ngrams[str];
            p.first.update(fcount);
            p.second+=fcount;
            str+=" ";
        }

		for(int i=0;i<(int)wrds.size()-order+1;i++){
            string str="";
            int len=0;
            for(;len<order-1;len++)str+=wrds[i+len]+" ";
            str+=wrds[i+len];
            if(ngrams.find(str)==ngrams.end())ngrams[str].second=0;
            pair<FracType,double>& p=ngrams[str];
            p.first.update(fcount);
            p.second+=fcount;
        }
	}
    if(_debug)
        for(auto& item : ngrams)
            cout<<item.first<<" ||| count:"
                <<item.second.second<<" type:"<<item.second.first<<endl;
}

void
LM::
addNgrams(unordered_map<string,pair<FracType,double>>& ngrams){
    int order=0;
    _vocab.add(unk);
    for(auto& item: ngrams){
		vector<string> wrds;
		split(wrds,item.first,is_any_of(" \t"));
		if((int)wrds.size()<2)continue;
		vector<int> context;
		for(int i=0;i<(int)wrds.size();i++)
			context.push_back(_vocab.add(wrds[i]));
		_vocab.reverse(context);

		addFracCount(context[0],&context[0]+1,(int)context.size()-1,
                     item.second.second,item.second.first);
        for(int depth=0;depth<(int)context.size()-1;depth++){
            addFracCount(context[0],&context[0]+1,
                         depth,item.second.second,item.second.second);
        }
		order=max(order,(int)context.size());
	}

	for(int i=0;i<order;i++){
		if((int)_numofGrams.size()<=i)
			_numofGrams.push_back(0);
	}
}
