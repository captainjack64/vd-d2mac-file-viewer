/* VirtualDub plugin for viewing MAC basedband files from hacktv         */
/*=======================================================================*/
/* Copyright 2018 Philip Heron <phil@sanslogic.co.uk>                    */
/* Copyright 2020 Alex James                                             */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "prbs.h"

/* Return first x LSBs in b in reversed order */
uint64_t rev(uint64_t b, int x)
{
	uint64_t r = 0;
	
	while(x--)
	{
		r = (r << 1) | (b & 1);
		b >>= 1;
	}
	
	return(r);
}

/* MAC/Packet PRBS generator #2 */

/* Generate IW for CA PRBS for video scrambling */
uint64_t _prbs_generate_iw(uint64_t cw, uint8_t fcnt)
{
	uint64_t iw;
	
	/* FCNT is repeated 8 times, each time inverted */
	iw  = ((fcnt ^ 0xFF) << 8) | fcnt;
	iw |= (iw << 16) | (iw << 32) | (iw << 48);
	
	return ((iw ^ cw) & CW_MASK);
}

void _prbs2_reset(_prbs2_t *prbs2, uint64_t cw, uint8_t fcnt)
{
	uint64_t iw = _prbs_generate_iw(cw, fcnt);
	
	/* Both shift registers shift to the right, LSB is the end */
	prbs2->sr3 = iw & SR3_MASK;
	prbs2->sr4 = (iw >> 31) & SR4_MASK;
	
	prbs2->c = 0;
}

uint64_t _prbs2_update(_prbs2_t *prbs2)
{
	int i;
	
	for(i = 0; i < 16; i++)
	{
		uint64_t a;
		
		/* Load the multiplexer address */
		a = rev(prbs2->sr4, 29) & 0x1F;
		if(a == 31) a = 30;
		
		/* Shift into result register */
		prbs2->c = (prbs2->c >> 1) | (((rev(prbs2->sr3, 31) >> a) & 1) << 15);
		
		/* Update shift registers */
		prbs2->sr3 = (prbs2->sr3 >> 1) ^ (prbs2->sr3 & 1 ? 0x7BB88888UL : 0);
		prbs2->sr4 = (prbs2->sr4 >> 1) ^ (prbs2->sr4 & 1 ? 0x17A2C100UL : 0);
	}
	
	return(prbs2->c);
}
