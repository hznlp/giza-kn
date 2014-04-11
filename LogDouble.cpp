#include "LogDouble.h"

LogDouble::
LogDouble(double x){
    sign_=x<0?false:true;
    log_=x==0?MINLOG:log(abs(x));
}

LogDouble::
LogDouble():sign_(true),log_(MINLOG){}

LogDouble
LogDouble::
operator+(const LogDouble& p){
    LogDouble result;
    const LogDouble* p1=this;
    const LogDouble* p2=&p;

    if(log_<p.log_){
        p1=&p;
        p2=this;
    }
    double logdif=p2->log_-p1->log_;
    result.sign_=p1->sign_;
    result.log_=
        (p1->sign_==p2->sign_)?
        p1->log_+log1p(exp(logdif)):
        p1->log_+log1p(-exp(logdif));
    if(result.log_==NINF)
        result.log_=MINLOG;

    return result;
}

LogDouble
LogDouble::
operator*(const LogDouble& p2){
    LogDouble result;
    result.sign_=(sign_==p2.sign_);
    result.log_=log_+p2.log_;
    if(result.log_==NINF)
        result.log_=MINLOG;
    return result;
}

LogDouble
LogDouble::
operator-(const LogDouble& p){
    LogDouble np=p;
    np.sign_=!np.sign_;
    return (*this)+np;
}

LogDouble
LogDouble::
operator/(const LogDouble& p2){
    LogDouble np=p2;
    np.log_=-np.log_;
    return (*this)*np;
}

LogDouble
LogDouble::
operator+=(const LogDouble& p2){
    *this=*this+p2;
    return *this;
}

LogDouble
LogDouble::
operator-=(const LogDouble& p2){
    *this=*this-p2;
    return *this;
}

LogDouble
LogDouble::
operator*=(const LogDouble& p2){
    *this=*this*p2;
    return *this;
}

LogDouble
LogDouble::
operator/=(const LogDouble& p2){
    *this=*this/p2;
    return *this;
}

bool
LogDouble::
operator==(const LogDouble& p2){
    return ((sign_==p2.sign_)&&(log_==p2.log_))||
            (log_==MINLOG&&p2.log_==MINLOG);
}


bool
LogDouble::
operator!=(const LogDouble& p2){
    return !(*this==p2);
}

bool
LogDouble::
operator<(const LogDouble& p2){

    if(sign_&&p2.sign_)
        return log_<p2.log_;
    else if(!sign_&&!p2.sign_)
        return log_>p2.log_;
    else if(sign_&&!p2.sign_)
        return false;
    else return true;
}

bool
LogDouble::
operator>(const LogDouble& p2){
    return *this!=p2&&!(*this<p2);
}

bool
LogDouble::
operator<=(const LogDouble& p2){
    return *this<p2||*this==p2;
}

bool
LogDouble::
operator>=(const LogDouble& p2){
    return *this>p2||*this==p2;
}

LogDouble::
operator double() const    // converts logr to (double) b**logr
{
    return sign_?exp(log_):-exp(log_);
}


void
LogDouble::
print(std::ostream& os){
    os<<" log:"<<sign_<<" "<<log_<<" ";
}

std::ostream& operator<<(std::ostream& os, const LogDouble& p){
	os<<(double)(p);
	return os;
}

std::istream& operator>>(std::istream& is, LogDouble& p){
	double number=0;
	is>>number;
	p=number;
	return is;
}
