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

#include <stdio.h>
#include "common.h"
#include "main.h"

int get_bit(int b)
{
	int bit;
	
	if(b <= DUOBIN_LOW) bit = -1;
	if(b > DUOBIN_LOW && b <= DUOBIN_HIGH) bit = 0;
	if(b > DUOBIN_HIGH) bit = 1;
	
	return (!bit ? bit : 1);
}

int RunProcDMACFilter(const FilterActivation *fa, const FilterFunctions *ff)
{
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *)fa->filter_data;
	
	/* Initialise source and destination buffers */
	Pixel32 *src_buffer  = fa->src.data;
	Pixel32 *dest_buffer = fa->dst.data;

	int x, y;

	/* Read line 625 */
	int frame;
	int pos;
	
	/* Get VSAM flag */
	if(mfd->vsam < 0)
	{
		for(x = 0, mfd->vsam = 0, pos = 268; x < 3; x++, pos += 2)
		{
			/*Get position bit of sample in the frame */
			mfd->vsam |= (get_bit(src_buffer[1 + pos] & 0xFF) & 1) << x;
		}

		if(mfd->vsam & 4) MessageBox(0, "Conditional access flag detected - descrambling will fail.", "Info", 0);
	}
	
	/* Unscramble, if needed */
	if((mfd->vsam & 1) == 0)
	{
		uint16_t cut;

		/* Get current frame number */
		for(x = 0, frame = 0, pos = 352; x < 8; x++, pos += 2)
		{	
			frame |= (get_bit(src_buffer[1 + pos] & 0xFF) & 1) << x;
		}

		/* Initialise the PRBS generator with the free-access code word */
		_prbs2_reset(&mfd->prbs2, CW_FA, frame - 1);

		for (y = 0; y < mfd->src_height; y++)
		{
			cut = (uint16_t) _prbs2_update(&mfd->prbs2);
			
			if(mfd->vsam & 2)
			{
				/* Single cut */
				rotate(src_buffer + mfd->src_pitch * (mfd->src_height - y), 230, 1285, 282 + ((cut & 0xFF00) >> 8), fa);
			}
			else
			{
				/* Double cut */
				rotate(src_buffer + mfd->src_pitch * (mfd->src_height - y), 229,  580, 282 + ((cut & 0xFF00) >> 8), fa); /* Colour-diff */
				rotate(src_buffer + mfd->src_pitch * (mfd->src_height - y), 586, 1285, 682 + ((cut & 0x00FF) << 1), fa); /* Luminance */
			}
		 }
	}

	/* Weave two fields together */

	for (y = 0; y < mfd->src_height; y++)
	{
		/* Calculate pixel offset in source frame - a bit long winded */
		int offset = y & 1 ? mfd->src_pitch * (y / 2) : mfd->src_pitch * (mfd->src_height >> 1) + mfd->src_pitch * (y / 2);

		for (x = 0; x < mfd->src_width; x++)
		{
			dest_buffer[x + mfd->dest_pitch * y] = src_buffer[x + offset];
		}
	 }

	/* Copy frame to work from */
	memcpy(src_buffer, dest_buffer, mfd->src_line_size * mfd->src_height);
	
	int v_offset[4] = { 2, 1, 1, 0 };
	int u_offset[4] = { 0, 0,-1, 2 };

	/* Skip bottom 4 lines */
	src_buffer += mfd->src_pitch * 4;

	/* Combine components into one full frame */

	for (y = 0; y < mfd->dest_height; y++)
	{
		for (x = 0; x < mfd->src_width; x++)
		{
			/* Y component */
			dest_buffer[x + mfd->dest_pitch * y] = src_buffer[x + mfd->luma_start] & Y_MASK;
			
			/* U component */
			dest_buffer[x + mfd->dest_pitch * y] |= src_buffer[(x >> 1) + mfd->chroma_start + (mfd->src_pitch * u_offset[y % 4])] & U_MASK;

			/* V component */
			dest_buffer[x + mfd->dest_pitch * y] |= src_buffer[(x >> 1) + mfd->chroma_start + (mfd->src_pitch * v_offset[y % 4])] & V_MASK;
		}

		/* Next line */
		src_buffer  += mfd->src_pitch;
	 }

	/* Convert combined frame to RGB */
	int R, G, B, Y, U, V;
	 for (y = -4; y < mfd->dest_height; y++)
	 {
		for (x = 0; x < mfd->luma_width + mfd->luma_start; x++)
		{
			if(y < 0)
			{
				/* Mask bottom 4 lines */
				*dest_buffer++ = 0;
			}
			else
			{
				Y = (*dest_buffer >>  8) & 0xFF;
				U = (*dest_buffer >> 16) & 0xFF;
				V = (*dest_buffer)       & 0xFF;
				yuv2rgb(Y, U, V, R, G, B);
			
				*dest_buffer++ = R << 16 | G << 8 | B;
			}
		 }
	 } 

	return 0;
}

long ParamProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff)
{
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *)fa->filter_data;
	
	/* Get line length */
	mfd->dest_line_size = fa->dst.w * sizeof(Pixel32) + fa->dst.modulo;
	mfd->src_line_size  = fa->src.w * sizeof(Pixel32) + fa->src.modulo;

	/* Get pitch sizes */
	mfd->src_pitch  = fa->src.pitch >> 2;
	mfd->dest_pitch = fa->dst.pitch >> 2;

	/* Ideally 1296 */
	mfd->src_width  = fa->src.w;

	/* Ideally 625 (ish) */
	mfd->src_height = fa->src.h & ~1;

	/* Define target width - 702 */
	fa->dst.w = (PixCoord) (mfd->src_width * (702.0 / 1296.0));

	/* Define target height - 576 */
	fa->dst.h = (PixCoord) ((mfd->src_height + 1) * (576.0 / 625.0)) & ~ 1;
	
	/* 576 */
	mfd->dest_height = fa->dst.h; 

	/* Component coordinates */
	/* 234 */
	mfd->chroma_start = (int) ((float) fa->dst.w * (1.0/3.0));

	/*351 */
	mfd->chroma_width = fa->dst.w / 2;

	/* 586 */
	mfd->luma_start = mfd->chroma_start + mfd->chroma_width + 1;

	/* 702 */
	mfd->luma_width = fa->dst.w;

	return FILTERPARAM_SWAP_BUFFERS;
}

int InitProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff) 
{
	
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *)fa->filter_data;

	/* Default mode */
	mfd->vsam = -1;
	mfd->start_status = 0;

	return 0;
}

int EndProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff) 
{
	
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *)fa->filter_data;

	/* Default mode */
	mfd->vsam = -1;
	mfd->start_status = 0;

	return 0;
}

int StartProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff)
{
	/* Pointer to MFD */
	MACFilterData *mfd = (MACFilterData *)fa->filter_data;

	if((fa->src.w != 1296 || fa->src.h != 625) && mfd->start_status == 0)
	{
		MessageBox(0, "Non-standard video resolution - will not attempt descrambling", "Info", 0);
		mfd->vsam = 1;
		mfd->start_status = 1;
	} 

	return 0;
}

/* General routines to register filters */

extern "C" int __declspec(dllexport) __cdecl VirtualdubFilterModuleInit2(FilterModule *fm, const FilterFunctions *ff, int& vdfd_ver, int& vdfd_compat);
extern "C" void __declspec(dllexport) __cdecl VirtualdubFilterModuleDeinit(FilterModule *fm, const FilterFunctions *ff);

int __declspec(dllexport) __cdecl VirtualdubFilterModuleInit2(FilterModule *fm, const FilterFunctions *ff, int& vdfd_ver, int& vdfd_compat)
{
	if (!(fd_DMACFilter = ff->addFilter(fm, &filterDef_DMACFilter, sizeof(FilterDefinition))))
        return 1;

	vdfd_ver = VIRTUALDUB_FILTERDEF_VERSION;
	vdfd_compat = VIRTUALDUB_FILTERDEF_COMPATIBLE;

    return 0;
}

void __declspec(dllexport) __cdecl VirtualdubFilterModuleDeinit(FilterModule *fm, const FilterFunctions *ff)
{
	ff->removeFilter(fd_DMACFilter);
}