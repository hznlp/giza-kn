#ifndef LOGPROB_H
#define LOGPROB_H
#include <cmath>

const double MINLOG=-1E10;
const double MINDOUBLE=-700;
class LogProb;

LogProb operator*(const LogProb& p1, const LogProb& p2);


class LogProb{
public:
	LogProb(int x){
		if(x<=0)log_=MINLOG;
		else log_=log((double)x);
	}
	LogProb(size_t x){
		if(x<=0)log_=MINLOG;
		else log_=log((double)x);
	}
	LogProb(double x){
		if(x<=0)log_=MINLOG;
		else log_=log(x);
	}
	LogProb():log_(MINLOG){}

	LogProb operator+(const LogProb& p2){
		LogProb result;
		if(log_==MINLOG)return p2;
		else if(p2.log_==MINLOG)return *this;
		else if(log_<p2.log_){
			result.log_=p2.log_+log1p(exp(log_-p2.log_));
		}else{
			result.log_=log_+log1p(exp(p2.log_-log_));
		}
		return result;
	}

	LogProb operator/(const LogProb& p2){
		LogProb result;
		result.log_=log_-p2.log_;
		return result;
	}

	LogProb operator+=(const LogProb& p2){
		*this=*this+p2;
		return *this;
	}

	LogProb operator*=(const LogProb& p2){
		*this=*this*p2;
		return *this;
	}

	LogProb operator/=(const LogProb& p2){
		*this=*this/p2;
		return *this;
	}

	inline double todouble()const{
		if(log_<MINDOUBLE)return 0;
		else if(log_>-MINDOUBLE)return exp(-MINDOUBLE);
		else return exp(log_);
	}

	bool operator<(const LogProb& p2){return log_<p2.log_;}
    bool operator>(const LogProb& p2){return log_>p2.log_;}
    bool operator==(const LogProb& p2){return log_==p2.log_;}
    bool operator<=(const LogProb& p2){return *this<p2||*this==p2;}
    bool operator>=(const LogProb& p2){return *this>p2||*this==p2;}

	operator double() const    // converts logr to (double) b**logr
    {
      return todouble();
    }

	double log_;
};


inline double log(const LogProb& p){
	return p.log_;
}

inline std::ostream& operator<<(std::ostream& os, const LogProb& p){
	os<<p.todouble();
	return os;
}

inline std::istream& operator>>(std::istream& is, LogProb& p){
	double number=0;
	is>>number;
	p=number;
	return is;
}

inline	LogProb operator*(const LogProb& p1, const LogProb& p2){
		LogProb result;
		result.log_=p1.log_+p2.log_;
		return result;
	}

#endif
