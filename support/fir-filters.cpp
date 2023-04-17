#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the apt plugin
 *
 *    apt plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    apt plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with apt plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"fir-filters.h"
#define  _USE_MATH_DEFINES
#include <math.h>

	lowpassFIR::lowpassFIR (int16_t firsize,
	                        int32_t Fc, int32_t fs):
	                                    Basic_FIR (firsize) {
	sampleRate	= fs;
	newKernel (Fc);
}

	lowpassFIR::~lowpassFIR () {
}

void	lowpassFIR::newKernel (int32_t Fc) {
int16_t	i;
std::vector<float>	tmp (filterSize);
float	f	= (float)Fc / sampleRate;
float	sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);
//
//	Blackman window
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
}


	highpassFIR::highpassFIR (int16_t firsize,
	                          int32_t Fc, int32_t fs):Basic_FIR (firsize) {
	sampleRate	= fs;
	newKernel (Fc);
}

	highpassFIR::~highpassFIR () {
}

void	highpassFIR::newKernel (int32_t Fc) {
int16_t	i;
std::vector<float> 	tmp (filterSize);
float f	= (float)Fc / sampleRate;
float sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);

	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   if (i == filterSize / 2)
	      filterKernel [i] = std::complex<float> (1.0 - tmp [i] / sum, 0);
	   else  
	      filterKernel [i] = std::complex<float> (- tmp [i] / sum, 0);
}



/*
 *	The bandfilter is for the complex domain. 
 *	We create a lowpass filter, which stretches over the
 *	positive and negative half, then shift this filter
 *	to the right position to form a nice bandfilter.
 */
	bandpassFIR::bandpassFIR (int16_t firSize,
	                          int32_t low, int32_t high,
	                          int32_t fs):Basic_FIR (firSize) {
	sampleRate	= fs;
	newKernel (low, high);
}
//
//	For the kernel we compute the size of the "equivalent"
//	low pass filter (lo), and the amount of the shift
void	bandpassFIR::newKernel (int32_t low, int32_t high) {
std::vector<float> tmp (filterSize);
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;
int16_t	i;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing, according to Blackman
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}
}

	bandpassFIR::~bandpassFIR () {
}

//
//====================================================================
//
//	decimationFilter.
//	Useful for e.g. reduction of the samplerate with dabsticks
//	Two tastes are within the same class: the lowpass and
//	the bandpass variant.

	decimatingFIR::decimatingFIR (int16_t firSize,
	                              int32_t low,
	                              int32_t fs,
	                              int16_t Dm):Basic_FIR (firSize) {

	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low);
}

void	decimatingFIR::newKernel (int32_t low) {
int16_t	i;
float	*tmp	= (float *)alloca (filterSize * sizeof (float));
float	f	= (float)low / sampleRate;
float	sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/ 2);

	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
}

	decimatingFIR::decimatingFIR (int16_t firSize,
	                              int32_t low,
	                              int32_t high,
	                              int32_t fs,
	                              int16_t Dm):Basic_FIR (firSize) {

	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low, high);
}

void	decimatingFIR::newKernel (int32_t low, int32_t high) {
float	*tmp	= (float *)alloca (filterSize * sizeof (float));
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;
int16_t	i;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing
	   tmp [i]  *= (0.42 -
		        0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		        0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}
//
//	and modulate the kernel
	for (i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}
}

	decimatingFIR::~decimatingFIR () {
}
//
//	The real cpu killer: this function is called once for every
//	sample that comes from the dongle. So, it really should be
//	optimized.
bool	decimatingFIR::Pass (std::complex<float> z,
	                              std::complex<float> *z_out) {
int16_t		i;
std::complex<float>	tmp	= 0;
int16_t		index;

	Buffer [ip] = z;
	if (++decimationCounter < decimationFactor) {
	   ip =  (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
//
//	we are working with a circular buffer, we take two steps
//	we move from ip - 1 .. 0 with i going from 0 .. ip -1
	for (i = 0; i <= ip; i ++) {
	   index =  ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}
//	and then we take the rest
	for (i = ip + 1; i < filterSize; i ++) {
	   index =  filterSize + ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	*z_out = tmp;
//	*z_out = cdiv (tmp, filterSize);
	return true;
}

bool	decimatingFIR::Pass (float z, float *z_out) {
std::complex<float> t = std::complex<float> (z, 0);
	if (Pass (t, &t)) {
	   *z_out = real (t);
	   return true;
	}
	return false;
}

