#include "Distribution.h"

Distribution::Distribution() {}

std::default_random_engine Distribution::generator;

Exponential::Exponential(double mean) : Distribution()
{
	_distr = new std::exponential_distribution<double>(1.0/mean);
}

double Exponential::GetRV()
{
	return _distr->operator()(generator);
}

Uniform::Uniform(double min, double max) : Distribution()
{
	_distr = new std::uniform_real_distribution<double>(min, max);
}

double Uniform::GetRV()
{
	return _distr->operator()(generator);
}

Triangular::Triangular(double min, double expected, double max) : Distribution()
{
	_distr = new std::uniform_real_distribution<double>(0.0, 1.0);
	a = min;
	c = expected;
	b = max;
	fc = (c - a) / (b - a);
	term1 = (b - a)*(c - a);
	term2 = (b - a)*(b - c);
}

double Triangular::GetRV()
{
	double u = _distr->operator()(generator);
	double x;
	if (u < fc)
		x = a + sqrt(u*term1);
	else
		x = b - sqrt((1 - u)*term2);
	return x;
}

Normal::Normal(double mean, double stdev)
{
	_distr = new std::normal_distribution<double>(mean, stdev);
}

double Normal::GetRV()
{
	return _distr->operator()(generator);
}

Poisson::Poisson(double mean)
{
	_distr = new std::poisson_distribution<int>(mean);
}

double Poisson::GetRV()
{
	return _distr->operator()(generator);
}

Constant::Constant(double mean)
{
	_mean = mean;
}

double Constant::GetRV()
{
	return _mean;
}

Weibull::Weibull(double scale, double shape)
{
	_distr = new std::weibull_distribution<double>(scale, shape);
}

double Weibull::GetRV()
{
	return _distr->operator()(generator);
}
