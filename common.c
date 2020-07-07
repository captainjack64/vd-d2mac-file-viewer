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

#include <math.h>
#include "common.h"

/* Cut and rotate function (c)fsphil */
void rotate(Pixel32 *s, int x1, int x2, int xc, const FilterActivation *fa)
{
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *) fa->filter_data;

	int x;

	/* Temporary line store */
	Pixel32 *line = (Pixel32 *) malloc(1296 * sizeof(Pixel32));

	xc -= 2;
	
	for(x = x1 - 2; x <= x2 + 2; x++)
	{
		line[xc++] = s[x];
		if(xc > x2) xc = x1;
	}
	
	for(x = x1 - 2; x <= x2 + 2; x++)
	{
		s[x] = line[x];
	}

	free(line);
}

/* Conversion table courtesy of Emiliano Ferrari */
void yuv2rgb(int Y,int U,int V,int &r,int &g,int &b)
{
	/* input:  Y,U,V [0..255] */
	/* output: r,g,b [0..255] */
	int YK= (int)(1.164*65536.0*219.0/255.0);
	int k1= (int)(1.596*65536.0*223.0/255.0);
	int k2= (int)(0.813*65536.0*223.0/255.0);
	int k3= (int)(2.018*65536.0*223.0/255.0);
	int k4= (int)(0.391*65536.0*223.0/255.0);

	Y *= YK;
	U -= 128;
	V -= 128;

	r = (Y + k1 * U + 32768) >> 16 ;
	g = (Y - k2 * U -k4 * V + 32768) >> 16;
	b = (Y + k3 * V + 32768) >> 16;

	if ((unsigned)r>255) r=(~r>>31)&0xff;
	if ((unsigned)g>255) g=(~g>>31)&0xff;
	if ((unsigned)b>255) b=(~b>>31)&0xff;
}
