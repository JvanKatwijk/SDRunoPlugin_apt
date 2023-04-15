#
/*
 *    Copyright (C) 2010, 2011
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fmreceiver
 *
 *    fmreceiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fmreceiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with fmreceiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __PLL_CH_H
#define __PLL_CH_H
/*
 */

#include	<stdint.h>
#include	<complex>
class pllC {
private:
	int32_t		rate;
	int32_t		cf;   // center frequency
	float		NcoPhase;
	float	phaseIncr;
	float	NcoHLimit;
	float	NcoLLimit;
	float	Beta;
	std::complex<float>	pll_Delay;
	float	phaseError;
public:
			pllC (int32_t	rate,
			       float freq, float lofreq, float hifreq,
	                       float bandwidth);

			~pllC ();

	void		do_pll		(std::complex<float> signal);
	std::complex<float>	getDelay	();
	float	getPhaseIncr	();
	std::complex<float>	getNco		();
	float	getPhaseError	();
};

#endif
