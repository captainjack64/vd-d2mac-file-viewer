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

#define DUOBIN_LOW 0x55
#define DUOBIN_HIGH 0xAA

int RunProcDMACFilter(const FilterActivation *fa, const FilterFunctions *ff);
long ParamProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff);
int StartProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff);
int EndProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff);
int InitProcDMACFilter(FilterActivation *fa, const FilterFunctions *ff);

FilterDefinition *fd_DMACFilter;

FilterDefinition filterDef_DMACFilter =
{
	NULL, NULL, NULL,			// next, prev, module
	"HackTV MAC Viewer",		// name
	"Converts MAC baseband output from HackTV to colour\nwith optional descrambler (free-access mode).",		// desc
	"http://filmnet.plus",		// maker
	NULL,						// private_data
	sizeof(MACFilterData),		// inst_data_size
	InitProcDMACFilter,			// initProc
	NULL,                       // deinitProc
	RunProcDMACFilter,			// runProc
	ParamProcDMACFilter,		// paramProc
	NULL,						// configProc
	NULL,						// stringProc
	StartProcDMACFilter,		// startProc
	EndProcDMACFilter,			// endProc
	NULL,						// script_obj
	NULL,						// fssProc
};