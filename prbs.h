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

/* Missing stdint defs for VS2005 */

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#define CW_FA    ((uint64_t) (1 << 60) - 1)
#define CW_MASK  ((uint64_t) (1 << 60) - 1)
#define SR3_MASK ((uint64_t) (1UL << 31) - 1)
#define SR4_MASK ((uint64_t) (1UL << 29) - 1)

typedef struct {
	uint64_t sr3;  /* Shift register 3 */
	uint64_t sr4;  /* Shift register 4 */
	uint64_t c;    /* Result shift register */
} _prbs2_t;

extern uint64_t _prbs2_update(_prbs2_t *prbs2);
extern void _prbs2_reset(_prbs2_t *prbs2, uint64_t cw, uint8_t fcnt);
