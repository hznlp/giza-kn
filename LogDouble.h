#ifndef LOGPROB_H
#define LOGPROB_H
#include <cmath>
#include <limits>  
#include <iostream>
using namespace std;
const double MINLOG=-std::numeric_limits<double>::max();
const double NINF=-std::numeric_limits<double>::infinity();
class LogDouble;

class LogDouble{
public:
	LogDouble(double x);
	LogDouble();

	LogDouble operator+(const LogDouble& p2);
	LogDouble operator*(const LogDouble& p2);
    LogDouble operator/(const LogDouble& p2);
    LogDouble operator-(const LogDouble& p2);

	LogDouble operator+=(const LogDouble& p2);
	LogDouble operator*=(const LogDouble& p2);
	LogDouble operator/=(const LogDouble& p2);
    LogDouble operator-=(const LogDouble& p2);

	bool operator<(const LogDouble& p2);
    bool operator>(const LogDouble& p2);
    bool operator==(const LogDouble& p2);
    bool operator!=(const LogDouble& p2);
    bool operator<=(const LogDouble& p2);
    bool operator>=(const LogDouble& p2);
	operator double() const;
    void print(std::ostream& od);
    bool sign_;
	double log_;
};

std::ostream& operator<<(std::ostream& os, const LogDouble& p);
std::istream& operator>>(std::istream& is, LogDouble& p);
#endif
