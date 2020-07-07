#include "prbs.h"
#include "filter.h"
/* VirtualDub plugin for viewing MAC basedband files from hacktv         */
/*=======================================================================*/
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
/* Missing stdint defs for VS2005 */

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#define Y_MASK 0xFF00
#define U_MASK 0xFF
#define V_MASK 0xFF0000

/* D-MAC filter definition */
typedef struct MACFilterData {
	int chroma_start;
	int chroma_width;
	int luma_start;
	int luma_width;
	int dest_line_size;
	int src_line_size;
	int src_width;
	int src_height;
	int dest_height; 
	int src_pitch;
	int dest_pitch;
	int vsam;

	_prbs2_t prbs2;

	bool start_status;

} MACFilterData;

extern void yuv2rgb(int Y,int U,int V,int &r,int &g,int &b);
extern void rotate(Pixel32 *s, int x1, int x2, int xc, const FilterActivation *fa);
