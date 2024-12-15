#ifndef _LPF_H_
#define _LPF_H_

#include <cmath>

class LowPassFilter{
public:
	//constructor
	LowPassFilter();
	//functions
	double next(double input);
	//configure funtion
	void set_parameters(double sampleFrequency, double cutoffFrequency);
private:
	double output;
	double ePow;
};

#endif //_LPF_H_