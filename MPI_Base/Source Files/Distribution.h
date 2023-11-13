#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <random>

class Distribution
{
public:
	Distribution();
	virtual double GetRV() = 0;
protected:
	static std::default_random_engine generator;
};

class Exponential : public Distribution
{
public:
	Exponential(double mean);
	double GetRV();
private:
	std::exponential_distribution<double> *_distr;
};

class Uniform : public Distribution
{
public:
	Uniform(double min, double max);
	double GetRV();
private:
	std::uniform_real_distribution<double> *_distr;
};

class Triangular : public Distribution
{
public:
	Triangular(double min, double expected, double max);
	double GetRV();
private:
	std::uniform_real_distribution<double> *_distr;
	double a, b, c, fc, term1, term2;
};

class Normal : public Distribution
{
public:
	Normal(double mean, double stdev);
	double GetRV();
private:
	std::normal_distribution<double> *_distr;
};

class Poisson : public Distribution
{
public:
	Poisson(double mean);
	double GetRV();
private:
	std::poisson_distribution<int> *_distr;
};

class Constant : public Distribution
{
public:
	Constant(double mean);
	double GetRV();
private:
	double _mean;
};

class Weibull : public Distribution
{
public:
	Weibull(double scale, double shape);
	double GetRV();
private:
	std::weibull_distribution<double> *_distr;
};

#endif
