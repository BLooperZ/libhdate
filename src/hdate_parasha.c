/*  libhdate
 *  Copyright  2004-2005  Yaacov Zamir <kzamir@walla.co.il>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "hdate.h"
#include "support.h"

/**
 @brief Return number of hebrew parasha.
 
 @author Yaacov Zamir 2003-2005, Reading tables by Zvi Har'El

 @param hebdate The hdate_struct of the date to use.
 @param diaspora if True give diaspora readings
 @return the name of parasha 1. Bereshit etc..
 (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*/
int
hdate_get_parasha (hdate_struct * h, int diaspora)
{
	static int join_flags[2][14][7] =
	{
		{
			{1, 1, 1, 1, 0, 1, 1}, /* 1 be erez israel */
			{1, 1, 1, 1, 0, 1, 0}, /* 2 */
			{1, 1, 1, 1, 0, 1, 1}, /* 3 */
			{1, 1, 1, 0, 0, 1, 0}, /* 4 */
			{1, 1, 1, 1, 0, 1, 1}, /* 5 */
			{0, 1, 1, 1, 0, 1, 0}, /* 6 */
			{1, 1, 1, 1, 0, 1, 1}, /* 7 */
			{0, 0, 0, 0, 0, 1, 1}, /* 8 */
			{0, 0, 0, 0, 0, 0, 0}, /* 9 */
			{0, 0, 0, 0, 0, 1, 1}, /* 10 */
			{0, 0, 0, 0, 0, 0, 0}, /* 11 */
			{0, 0, 0, 0, 0, 0, 0}, /* 12 */
			{0, 0, 0, 0, 0, 0, 1}, /* 13 */
			{0, 0, 0, 0, 0, 1, 1}  /* 14 */
		},
		{
			{1, 1, 1, 1, 0, 1, 1}, /* 1 in diaspora */
			{1, 1, 1, 1, 0, 1, 0}, /* 2 */
			{1, 1, 1, 1, 1, 1, 1}, /* 3 */
			{1, 1, 1, 1, 0, 1, 0}, /* 4 */
			{1, 1, 1, 1, 1, 1, 1}, /* 5 */
			{0, 1, 1, 1, 0, 1, 0}, /* 6 */
			{1, 1, 1, 1, 0, 1, 1}, /* 7 */
			{0, 0, 0, 0, 1, 1, 1}, /* 8 */
			{0, 0, 0, 0, 0, 0, 0}, /* 9 */
			{0, 0, 0, 0, 0, 1, 1}, /* 10 */
			{0, 0, 0, 0, 0, 1, 0}, /* 11 */
			{0, 0, 0, 0, 0, 1, 0}, /* 12 */
			{0, 0, 0, 0, 0, 0, 1}, /* 13 */
			{0, 0, 0, 0, 1, 1, 1}  /* 14 */
		}
	};
	
	int reading;
	
	
	/* if simhat tora return vezot habracha */
	if (h->hd_mon == 1 && h->hd_day == 22)
	{
		return 54;
	}
	
	/* if not shabat return none */
	if (h->hd_dw != 7)
	{
		return 0;
	}
	
	/* check for diaspora readings */
	diaspora = diaspora?1:0;
	
	switch (h->hd_weeks)
	{
	case  1:
		if (h->hd_new_year_dw == 7)
		{
			reading = 0;
			return reading;
		}
		else if ((h->hd_new_year_dw == 2) || (h->hd_new_year_dw == 3))
		{
			reading = 52;
			return reading;
		}
		else /* if (h->hd_new_year_dw == 5) */
		{
			reading = 53;
			return reading;
		}
		break;
	case  2:
		if (h->hd_new_year_dw == 5)
		{
			reading = 0;
			return reading;
		}
		else
		{
			reading = 53;
			return reading;
		}
		break;
	case  3:
		reading = 0;
		return reading;
		break;
	case  4:
		if (h->hd_new_year_dw == 7)
		{
			reading = 54;
			return reading;
		}
		else
		{
			reading = 1;
			return reading;
		}
		break;
	default:
		/* simhat tora on week 4 bereshit too */
		reading = h->hd_weeks - 3;
		
		/* was simhat tora on shabat ? */
		if (h->hd_new_year_dw == 7)
			reading = reading - 1;
		
		/* no joining */
		if (reading < 22)
		{
			return reading;
		}
		
		/* pesach */
		if ((h->hd_mon == 7) && (h->hd_day > 14) && (h->hd_day < 22))
		{
			reading = 0;
			return reading;
		}
		if (((h->hd_mon == 7) && (h->hd_day > 21)) || (h->hd_mon > 7 && h->hd_mon < 13))
		{
			reading--;
		}
		/* on diaspora, shavot may fall on shabat if next new year is on shabat */
		if (diaspora && 
			(h->hd_mon < 13) && 
			((h->hd_mon > 8) || (h->hd_mon == 8 && h->hd_day >= 6)) && 
			((h->hd_new_year_dw + h->hd_size_of_year) % 7) == 0)
		{
			if (h->hd_mon == 8 && h->hd_day == 6)
			{
				reading = 0;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		
		/* joining */
		if (join_flags[diaspora][h->hd_year_type - 1][0] && (reading >= 22))
		{
			if (reading == 22)
			{
				reading = 55;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flags[diaspora][h->hd_year_type - 1][1] && (reading >= 27))
		{
			if (reading == 27)
			{
				reading = 56;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flags[diaspora][h->hd_year_type - 1][2] && (reading >= 29))
		{
			if (reading == 29)
			{
				reading = 57;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flags[diaspora][h->hd_year_type - 1][3] && (reading >= 32))
		{
			if (reading == 32)
			{
				reading = 58;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		
		if (join_flags[diaspora][h->hd_year_type - 1][4] && (reading >= 39))
		{
			if (reading == 39)
			{
				reading = 59;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flags[diaspora][h->hd_year_type - 1][5] && (reading >= 42))
		{
			if (reading == 42)
			{
				reading = 60;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flags[diaspora][h->hd_year_type - 1][6] && (reading >= 51))
		{
			if (reading == 51)
			{
				reading = 61;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		break;
	}

	return reading;
}
