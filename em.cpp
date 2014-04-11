#include "em.h"
extern bool avgLogProb;
extern double log2_10;
enum UpdateType{Shrinkage,Viterbi};

double calculateFrac(double logp1,double logp2, double lambda)
{
	double frac=lambda/(lambda+(1-lambda)*exp10(logp2-logp1));
	//cout<<logp2<<"-"<<logp1<<"="<<logp2-logp1<<"\t"<<frac<<endl;
	return frac;
}

double expectedFrac(double logp1,double logp2)
{
    //a/(a-b)-ab/(a-b)^2*ln(a/b)
	if(abs(logp1-logp2)<1E-5)return 0.5;
    double frac=
        1/(1-exp10(logp2-logp1))-(logp1-logp2)*log(10)/(exp10(logp1-logp2)-2+exp10(logp2-logp1));
	return frac;
}

void getFracs(string filename, LM& lm1, LM& lm2, vector<double>& f1, vector<double>& f2, double& lambda, double threshold, UpdateType utype, int n_ind_sent, int iter)
{
	ifstream is(filename.c_str());
	int order1=lm1.order();
	int order2=lm2.order();
	int ind=0;
    double in_counts=0;
    double out_counts=0;
    vector<int> sentlens;
    vector<double> prob1s,prob2s;
	while(is.good())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")continue;
		double prob1=lm1.sentLogProb(curline,order1);
		double prob2=lm2.sentLogProb(curline,order2);
		if((int)f1.size()<=ind)
			f1.push_back(0);
		if((int)f2.size()<=ind)
			f2.push_back(0);

        vector<string> wrds;
        stringToVector(curline,wrds);
        int length=(int)wrds.size()+1;
        prob1/=length;
        prob2/=length;
        sentlens.push_back(length);
        prob1s.push_back(prob1);
        prob2s.push_back(prob2);

		f1[ind]=calculateFrac(prob1,prob2,lambda);
        if(iter<0)
            f1[ind]=expectedFrac(prob1,prob2);
        if(ind<n_ind_sent)
            f1[ind]=1;
        f2[ind]=1-f1[ind];
        in_counts+=f1[ind]*length;
        out_counts+=f2[ind]*length;
        ++ind;
	}
    lambda=in_counts/(in_counts+out_counts);

    cout<<"starting lambda: "<<lambda<<endl;
    for(int i=1;i<iter;i++){
        in_counts=out_counts=0;
        for(int ind=0;ind<f1.size();ind++){
            int length=sentlens[ind];
            if(ind<n_ind_sent)
                f1[ind]=1;
            else{
                f1[ind]=calculateFrac(prob1s[ind],prob2s[ind],lambda);
            }
            f2[ind]=1-f1[ind];
            in_counts+=f1[ind]*length;
            out_counts+=f2[ind]*length;
        }
        lambda=in_counts/(in_counts+out_counts);
        cout<<"inner iter: "<<i<<", lambda: "<<lambda<<endl;
    }
}

void getProbs(string filename, LM& lm1, LM& lm2, vector<double>& v1, vector<double>& v2, bool average)
{
	ifstream is(filename.c_str());
	int order1=lm1.order();
	int order2=lm2.order();
	int ind=0;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")continue;
		double prob1=lm1.sentLogProb(curline,order1);
		double prob2=lm2.sentLogProb(curline,order2);
		if((int)v1.size()<=ind)
			v1.push_back(0);
		if((int)v2.size()<=ind)
			v2.push_back(0);
		if(average)
		{
			vector<string> wrds;
			stringToVector(curline,wrds);
			int length=(int)wrds.size()+1;
			prob1/=length;
			prob2/=length;
		}
		v1[ind]=prob1;
		v2[ind]=prob2;
		++ind;
	}
}

void getProbs(string filename, LM& lm1, vector<double>& v1, bool average)
{
	ifstream is(filename.c_str());
	int order1=lm1.order();
	int ind=0;
	while(!is.eof())
	{
		string curline="";
		getline(is,curline);
		if(curline=="")continue;
		double prob1=lm1.sentLogProb(curline,order1);
		if((int)v1.size()<=ind)
			v1.push_back(0);
		if(average)
		{
			vector<string> wrds;
			stringToVector(curline,wrds);
			int length=(int)wrds.size()+1;
			prob1/=length;
		}
		v1[ind]=prob1;
		++ind;
	}
}

bool getFrac(vector<double>& v1, vector<double>& v2, double& lambda, vector<double>& f1, vector<double>& f2)
{
	f1.clear();f2.clear();
	if(v1.size()!=v2.size())return false;
	for(int i=0;i<(int)v1.size();i++)
	{
		double p1=v1[i];
		double p2=v2[i];
		double frac=calculateFrac(p1,p2, lambda);
		f1.push_back(frac);
		f2.push_back(1-frac);
	}
	return true;
}

bool updateLambda(vector<double>& v1, vector<double>& v2, double& lambda, int maxIter, double minGap, int priorCount)
{
	if(v1.size()!=v2.size())return false;
	for(int iter=0;iter<maxIter;iter++)
	{
		double oldLambda=lambda;
		double frac=0;
		for(int ind=0;ind<(int)v1.size();ind++)
		{
			double f1=v1[ind];
			double f2=v2[ind];
			frac+=calculateFrac(f1,f2,lambda);
		}
		lambda=(frac+priorCount)/((double)v1.size()+priorCount);
		if(abs(lambda-oldLambda)<minGap)break;
		cout<<"\tinIter "<<iter<<" lambda "<<lambda<<endl;
	}
	return true;
}

void selectSentencesByFrac(string filename, vector<double>& fracs, double threshold, string selectFilename, string filteredFilename)
{
	ifstream is(filename.c_str());
	ofstream os(selectFilename.c_str());
	string weightFilename=selectFilename+".weight";
	ofstream os_frac(weightFilename.c_str());

	ofstream os_filtered(filteredFilename.c_str());
	string filteredWeightFilename=filteredFilename+".weight";
	ofstream os_filtered_frac(filteredWeightFilename.c_str());
	
	for(int i=0;i<(int)fracs.size()&&!is.eof();i++)
	{
		string curline="";
		getline(is,curline);
		if(fracs[i]>threshold)
		{
			os<<curline<<endl;
			os_frac<<fracs[i]<<endl;
		}
		else
		{
			os_filtered<<curline<<endl;
			os_filtered_frac<<fracs[i]<<endl;
		}

	}
	is.close();
	os.close();
	os_filtered.close();
	os_frac.close();
	os_filtered_frac.close();
}

int getlines(string filename)
{
	int count=0;
	ifstream is(filename.c_str());
	while(is.good())
	{
		count++;
		string curline="";
		getline(is,curline);
	}
	is.close();
	return count;
}

void readFracs(string filename, vector<double>& frac)
{
	frac.clear();
	ifstream is(filename.c_str());
	while(!is.eof())
	{
		double score=0;
		is>>score;
		frac.push_back(score);
	}
	is.close();
}

void combineFiles(string file1, string file2, string resultfile)
{
	ifstream is1(file1.c_str());
	ifstream is2(file2.c_str());
	ofstream os(resultfile.c_str());
	while(!is1.eof()&&is1.good())
	{
		string curline="";
		getline(is1,curline);
		if(curline=="")continue;
		os<<curline<<endl;		
	}
	while(!is2.eof()&&is2.good())
	{
		string curline="";
		getline(is2,curline);
		if(curline=="")continue;
		os<<curline<<endl;
	}
	os.flush();
	cerr<<"here "<<file1<<","<<file2<<":"<<resultfile<<endl;
	is1.close();
	is2.close();
	os.close();
}

void normalize(vector<double>& frac)
{
	double maxvalue=0;
	for(size_t i=0;i<frac.size();i++)
		if(frac[i]>maxvalue)maxvalue=frac[i];
	for(size_t i=0;i<frac.size();i++)
		frac[i]/=maxvalue;
}

double ppl(double logprob, double nwrds)
{
	return pow(2,-(logprob*log2_10/nwrds));
}


void validation(string outDomainFile, string devFile, int order, Smoothing sm,
                double* frac, double threshold, double floor,
                double minline, double* result, int* line){
    LM lm1;
    *line=lm1.readPlainFile(outDomainFile,order,frac,threshold,floor);
    if(*line<minline)return;
    if(sm==NO)
        lm1.estimate(KN,true);
    else
        lm1.estimate(sm,true);
    LMStat lmStat(order);
    double logppl=lm1.ppl(devFile,&lmStat);
    *result=ppl(logppl,lmStat.nwrds);
}

void thread_validation(string outDomainFile, string devFile, int order, Smoothing sm,
                double* frac, vector<double>& thresholds, double floor,
                double minline, ofstream& os,
                int nthread, vector<double>& ppls, vector<int>& lines){
    vector<thread> threads;
    for(int i=0;i<thresholds.size();)
    {
        int start=i;
        for(;i-start<nthread&&i<thresholds.size();i++){
            double threshold=thresholds[i];
            threads.push_back(thread(
                                     validation,outDomainFile,devFile,order,sm,
                                     frac,threshold,floor,minline,&ppls[i],&lines[i]));
        }
        for(;start<i;start++){
            threads[start].join();
        }
    }
}

void getThresholds(vector<double> frac, vector<double>& thresholds){
    thresholds.clear();
    sort(frac.begin(),frac.end());

    for(double i=0;i<0.9;i+=0.1){
        thresholds.push_back(min(frac[i*frac.size()],0.99));
    }
    for(double i=0.9;i<1;i+=0.02){
        thresholds.push_back(min(frac[i*frac.size()],0.99));
    }
}
void em(JKArgs& args)
{
	//lm -em -cid=corpus-in-domain -cod=corpus-out-domain -iter=Iteraion -dev=devFile -lm1=lm1 -lm2=lm2 [-smoothing=kn|wb] [-lm1_init=arpalm] [-lm2_init=arpalm] [-order=lmOrder]
	if(!args.is_set("cid")||!args.is_set("cod")||!args.is_set("iter")||!args.is_set("dev"))
		usage();
	LM lm1;
	LM lm2;
	string inDomainFile=args.value("cid");
	string outDomainFile=args.value("cod");
	string devFile=args.value("dev");
	bool interpolate=true;
	double fracThreshold=0;
    bool justWeighting=args.value("justWeighting")=="true";
    double floor=0;
    Smoothing sm=KN;
    int maxOutIter=5;
    bool noThresholdInEM=args.value("noThresholdInEM")=="true";
	if(args.is_set("iter"))
		maxOutIter=atoi(args.value("iter").c_str());
    int lambdaIter=0;
    if(args.is_set("lambdaIter"))
		lambdaIter=atoi(args.value("lambdaIter").c_str());

    UpdateType utype=Shrinkage;
    if(args.value("utype")=="Viterbi")utype=Viterbi;
    if(utype==Viterbi)floor=1;
    if(args.is_set("fracThreshold"))fracThreshold=atof(args.value("fracThreshold").c_str());
	if(args.is_set("smoothing")){
		if(args.value("smoothing")=="wb")sm=WB;
		else if(args.value("smoothing")=="kn")sm=KN;
        else if(args.value("smoothing")=="no")sm=NO;
	}
	int order=3;
	if(args.is_set("order"))
		order=atoi(args.value("order").c_str());
	int maxInIter=1;
	if(args.is_set("inIter"))
		maxInIter=atoi(args.value("inIter").c_str());
	double minGap=0.01;
	if(args.is_set("minGap"))
		minGap=atof(args.value("minGap").c_str());
	string lm1Name="lm1";
	if(args.is_set("lm1"))lm1Name=args.value("lm1");
	string lm2Name="lm2";
	if(args.is_set("lm2"))lm2Name=args.value("lm2");

	//bool keepInLM=false;
	//if(args.value("keepInLM")=="true")keepInLM=true;
	
	vector<double> lambdas;
	//bool tuneByDev=false;
	//if(args.is_set("tuneByDev"))tuneByDev=true;
	if(args.is_set("lambdas"))
	{
		string lds=args.value("lambdas");
		replaceSubStr(lds,","," ");
		vector<string> ldsv;
		stringToVector(lds,ldsv);
		for(int i=0;i<(int)ldsv.size();i++)lambdas.push_back(atof(ldsv[i].c_str()));
	}
	int n_ind_sent=0;
	if(args.is_set("lm1_init"))
	{
		lm1.read(args.value("lm1_init"));
		n_ind_sent=getlines(inDomainFile);
	}
	else
	{
		n_ind_sent=lm1.readPlainFile(inDomainFile,order,NULL);
        if(n_ind_sent>0){
            if(sm==NO)lm1.estimate(KN,interpolate);
            else lm1.estimate(sm,interpolate);
        }
	}
	if(args.is_set("lm2_init"))
	{
		lm2.read(args.value("lm2_init"));
	}
	else
	{
		lm2.readPlainFile(outDomainFile,order,NULL);
		if(sm==NO)lm2.estimate(KN,interpolate);
		else lm2.estimate(sm,interpolate);
	}
    ofstream olog(lm1Name+".log");

	LMStat lmStat(lm1.order());
	
	olog<<"start em"<<endl;
    lm1.ppl(devFile,&lmStat);

	olog<<"lm1 ppl of dev: "<<ppl(lm1.ppl(devFile,NULL),lmStat.nwrds)<<endl;
    olog<<"lm2 ppl of dev: "<<ppl(lm2.ppl(devFile,NULL),lmStat.nwrds)<<endl;
	double lambda=0.5;
	if(lambdas.size()>0)lambda=lambdas[0];

    vector<double> thresholds={0.01,//0.02,0.03,0.04,0.06,0.08,
        0.11,0.21,0.31,0.41,0.51,0.61,0.71,0.81,0.91};
    if(args.is_set("thresholds")){
        thresholds.clear();
        ifstream is(args.value("thresholds").c_str());
        while(true){
            double d=0;
            is>>d;
            if(!is.good())break;
            thresholds.push_back(d);
        }
    }

	for(int oIter=0;oIter<maxOutIter;oIter++)
	{
		vector<double> frac1,frac2;

		getFracs(outDomainFile,lm1,lm2,frac1,frac2,lambda,0,utype,n_ind_sent,lambdaIter);
        getThresholds(frac1,thresholds);
        lm1.clear();
        lm2.clear();
        if(sm==NO){
            for(int i=0;i<n_ind_sent;i++){
                frac1[i]=0.999;
                frac2[i]=0.001;
            }
        }
        if(args.is_set("lambda"))
            lambda=atof(args.value("lambda").c_str());
		olog<<"iter"<<oIter<<", lambda "<<lambda<<endl;
        cout<<"iter"<<oIter<<", lambda "<<lambda<<endl;

		string fracFilename=lm1Name+".frac.iter"+intToString(oIter);
		ofstream fracFile(fracFilename.c_str());
		for(int i=0;i<(int)frac1.size();i++)fracFile<<frac1[i]<<endl;
		fracFile.close();
        if(justWeighting)return;
        double bestthreshold=0.1;
        double bestppl=1E10;
        int minline=100;
        int nthread=5;
        if(args.is_set("nthread"))nthread=atoi(args.value("nthread").c_str());

        vector<thread> threads;
        vector<double> ppls(thresholds.size());
        vector<int> lines(thresholds.size());
        //select threshold for lm1
        thread_validation(outDomainFile,devFile,order,sm,&frac1[0],thresholds,
                          floor,minline,olog,nthread,ppls,lines);

        for(int i=0;i<thresholds.size();i++){
            double threshold=thresholds[i];
            olog<<"threshold: "<<threshold<<", lm1 ppl of dev: "<<ppls[i]<<", line: "<<lines[i]<<endl;
            if(ppls[i]<bestppl){
                bestppl=ppls[i];
                bestthreshold=threshold;
            }
        }
        olog<<"best threshold: "<<bestthreshold<<endl;

        if(sm==NO||noThresholdInEM)bestthreshold=0;
        lm1.readPlainFile(outDomainFile,order,&frac1[0],bestthreshold,0);
        lm1.estimate(sm,interpolate);

        //select threshold for lm2
        bestppl=0;
        thread_validation(outDomainFile,devFile,order,sm,&frac2[0],thresholds,
                          floor,minline,olog,nthread,ppls,lines);
        for(int i=0;i<thresholds.size();i++){
            double threshold=thresholds[i];
            olog<<"threshold: "<<threshold<<", lm2 ppl of dev: "<<ppls[i]<<", line: "<<lines[i]<<endl;
            if(ppls[i]>bestppl){
                bestppl=ppls[i];
                bestthreshold=threshold;
            }
        }
        olog<<"best threshold: "<<bestthreshold<<endl;
        threads.clear();

        if(sm==NO||noThresholdInEM)bestthreshold=0;
        //        else bestthreshold=1-bestthreshold;
        lm2.readPlainFile(outDomainFile,order,&frac2[0],bestthreshold,0);
        lm2.estimate(sm,interpolate);
    }
    olog.close();
	//lm1.write(lm1Name);
	//lm2.write(lm2Name);
}
