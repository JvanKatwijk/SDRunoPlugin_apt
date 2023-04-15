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

#ifndef __FIR_FILTERS_H
#define __FIR_FILTERS_H

#include	<complex>
#include	<vector>
#include	<stdint.h>

class	Basic_FIR {
public:
	int16_t		filterSize;
	std::vector<std::complex<float>> filterKernel;
	std::vector<std::complex<float>> Buffer;
	int16_t		ip;
	int32_t		sampleRate;

	Basic_FIR(int16_t size) {
		int16_t		i;
		filterSize = size;
		filterKernel.resize(filterSize);
		Buffer.resize(filterSize);
		ip = 0;

		for (i = 0; i < filterSize; i++) {
			filterKernel[i] = 0;
			Buffer[i] = 0;
		}
	}

	~Basic_FIR() {
	}
	//
	//	we process the samples backwards rather than reversing
	//	the kernel
	std::complex<float>	Pass(std::complex<float> z) {
		int16_t	i;
		std::complex<float>	tmp = 0;

		Buffer[ip] = z;
		for (i = 0; i < filterSize; i++) {
			int16_t index = ip - i;
			if (index < 0)
				index += filterSize;
			tmp += Buffer[index] * filterKernel[i];
		}

		ip = (ip + 1) % filterSize;
		return tmp;
	}

	float	Pass (float v) {
		int16_t		i;
		float	tmp = 0;

		Buffer[ip] = std::complex<float>(v, 0);
		for (i = 0; i < filterSize; i++) {
			int16_t index = ip - i;
			if (index < 0)
				index += filterSize;
			tmp += real(Buffer[index]) * real(filterKernel[i]);
		}

		ip = (ip + 1) % filterSize;
		return tmp;
	}
};

class	lowpassFIR : public Basic_FIR {
public:
			lowpassFIR (int16_t,	// order
	                            int32_t, 	// cutoff frequency
	                            int32_t	// samplerate
	                           );
			~lowpassFIR	();
	void		newKernel	(int32_t);	// cutoff
};
//
class	highpassFIR: public Basic_FIR {
public:
			highpassFIR	(int16_t, int32_t, int32_t);
			~highpassFIR	();
	void		newKernel	(int32_t);
};

class	bandpassFIR : public Basic_FIR {
public:
	bandpassFIR(int16_t, int32_t, int32_t, int32_t);
	~bandpassFIR();
	void		newKernel(int32_t, int32_t);
};

//	Both for lowpass band bandpass, we provide:
class	decimatingFIR: public Basic_FIR {
public:
		         decimatingFIR	(int16_t, int32_t, int32_t, int16_t);
	                 decimatingFIR	(int16_t, int32_t, int32_t,
	                                                   int32_t, int16_t);
			~decimatingFIR	(void);
	void		newKernel	(int32_t);
	void		newKernel	(int32_t, int32_t);
	bool		Pass	(std::complex<float>, std::complex<float> *);
	bool		Pass	(float, float *);
private:
	int16_t	decimationFactor;
	int16_t	decimationCounter;
};

#endif

