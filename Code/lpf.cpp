#include "lpf.h"

LowPassFilter::LowPassFilter():
	output(0),
	ePow(1){}

double LowPassFilter::next(double input){
	return output += (input - output) * ePow;
}

void LowPassFilter::set_parameters(double sampleFrequency, double cutoffFrequency){
	if (cutoffFrequency < 0) cutoffFrequency = 0;
	double deltaTime = 1/sampleFrequency;
	ePow = 1-exp(-deltaTime * 2 * M_PI * cutoffFrequency);
}