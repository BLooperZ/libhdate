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

/*
 * See Amos Shapir hdate_README file attached for 
 * Copyright notice for functions based on Amos's code
 */

#include <time.h>
#include <stdio.h>

#include "hdate.h"
#include "support.h"

#define HOUR 1080
#define DAY  (24*HOUR)
#define WEEK (7*DAY)
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))	/* Tikun for regular month */

/**
 @brief Days since bet (?) Tishrey 3744
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005 
 
 @param hebrew_year The Hebrew year
 @return Number of days since 3,1,3744
*/
int
hdate_days_from_3744 (int hebrew_year)
{
	int years_from_3744;
	int molad_3744;
	int leap_months;
	int leap_left;
	int months;
	int parts;
	int days;
	int parts_left_in_week;
	int parts_left_in_day;
	int week_day;

	/* Start point for calculation is Molad new year 3744 (16BC) */
	years_from_3744 = hebrew_year - 3744;
	molad_3744 = M (1 + 6, 779);	/* Molad 3744 + 6 hours in parts */

	/* Time in months */
	leap_months = (years_from_3744 * 7 + 1) / 19;	/* Number of leap months */
	leap_left = (years_from_3744 * 7 + 1) % 19;	/* Months left of leap cycle */
	months = years_from_3744 * 12 + leap_months;	/* Total Number of months */

	/* Time in parts and days */
	parts = months * MONTH + molad_3744;	/* Molad This year + Molad 3744 - corections */
	days = months * 28 + parts / DAY - 2;	/* 28 days in month + corections */

	/* Time left for round date in corections */
	parts_left_in_week = parts % WEEK;	/* 28 % 7 = 0 so only corections counts */
	parts_left_in_day = parts % DAY;
	week_day = parts_left_in_week / DAY;

	/* Special cases of Molad Zaken */
	if ((leap_left < 12 && week_day == 3
	     && parts_left_in_day >= M (9 + 6, 204)) ||
	    (leap_left < 7 && week_day == 2
	     && parts_left_in_day >= M (15 + 6, 589)))
	{
		days++, week_day++;
	}

	/* ADU */
	if (week_day == 1 || week_day == 4 || week_day == 6)
	{
		days++;
	}

	return days;
}

/**
 @brief Size of Hebrew year in days
 
 @param hebrew_year The Hebrew year
 @return Size of Hebrew year
*/
int
hdate_size_of_hebrew_year (int hebrew_year)
{
	return hdate_days_from_3744 (hebrew_year + 1) -
		hdate_days_from_3744 (hebrew_year);
}

/**
 @brief Return Hebrew year type based on size and first week day of year.

 @param size_of_year Length of year in days
 @param new_year_dw First week day of year
 @return A number for year type (1..24)
*/
int
hdate_get_year_type (int size_of_year, int new_year_dw)
{
	int year_type;
	int offset;
	
	/* 2,3,5,7 -> 1,2,3,4 */
	year_type = new_year_dw - 1;
	if (year_type > 2) year_type = year_type - 1;
	if (year_type > 3) year_type = year_type - 1;	
	
	/* 353, 354, 355, 383, 384, 385 -> 0, 1, 2, 3, 4, 5 */
	offset = size_of_year % 10 - 3;
	if (size_of_year > 355) offset = offset + 3;
		
	/* Combine year_type and offset */
	year_type = offset * 4 + year_type;
	
	return year_type;
}

/**
 @brief Compute Julian day from Gregorian day, month and year
 Algorithm from 'Julian and Gregorian Day Numbers' by Peter Meyer

 @author Yaacov Zamir ( algorithm from Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param day Day of month 1..31
 @param month Month 1..12
 @param year Year in 4 digits e.g. 2001
 @return The julian day number
 */
int
hdate_gdate_to_jd (int day, int month, int year)
{
	int jd;
	jd = (1461 * (year + 4800 + (month - 14) / 12)) / 4 +
		(367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 -
		(3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4 + day -
		32075;
	return jd;
}

/**
 @brief Compute Julian day from Hebrew day, month and year
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005

 @param day Day of month 1..31
 @param month Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Hebrew year in 4 digits e.g. 5753
 @return The julian day number
 */
int
hdate_hdate_to_jd (int day, int month, int year)
{
	int length_of_year;
	int jd;

	/* length of year */
	length_of_year = hdate_size_of_hebrew_year (year);

	/* Adjust for leap year */
	if (month == 13)
	{
		month = 6;
	}
	if (month == 14)
	{
		month = 6;
		day += 30;
	}

	/* Calculate days since 1,1,3744 */
	day = hdate_days_from_3744 (year) + (59 * (month - 1) + 1) / 2 + day;

	/* Special cases for this year */
	if (length_of_year % 10 > 4 && month > 2)	/* long Heshvan */
		day++;
	if (length_of_year % 10 < 4 && month > 3)	/* short Kislev */
		day--;
	if (length_of_year > 365 && month > 6)	/* leap year */
		day += 30;

	/* adjust to julian */
	jd = day + 1715118;

	return jd;
}

/**
 @brief Converting from the Julian day to the Gregorian day
 Algorithm from 'Julian and Gregorian Day Numbers' by Peter Meyer 

 @author Yaacov Zamir ( Algorithm, Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param jd Julian day
 @param d Return Day of month 1..31
 @param m Return Month 1..12
 @param y Return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_gdate (int jd, int *d, int *m, int *y)
{
	int l, n, i, j;
	l = jd + 68569;
	n = (4 * l) / 146097;
	l = l - (146097 * n + 3) / 4;
	i = (4000 * (l + 1)) / 1461001;	/* that's 1,461,001 */
	l = l - (1461 * i) / 4 + 31;
	j = (80 * l) / 2447;
	*d = l - (2447 * j) / 80;
	l = j / 11;
	*m = j + 2 - (12 * l);
	*y = 100 * (n - 49) + i + l;	/* that's a lower-case L */

	return;
}

/**
 @brief Converting from the Julian day to the Hebrew day
 
 @author Yaacov Zamir 2005

 @param jd Julian day
 @param day Return Day of month 1..31
 @param month Return Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_hdate (int jd, int *day, int *month, int *year)
{
	int julian;

	/* calculate Gregorian date */
	hdate_jd_to_gdate (jd, day, month, year);

	/* Guess Hebrew year is Gregorian year + 3760 */
	*year = *year + 3760 + 1;

	/* Check if computed year was overestimated */
	julian = hdate_hdate_to_jd (1, 1, *year);
	while (julian > jd)
	{
		*year = *year - 1;
		julian = hdate_hdate_to_jd (1, 1, *year);
	}

	/* days into this year */
	*day = jd - julian;

	/* Guess month is days in year / 28 */
	*month = *day / 28 + 1;

	/* Check for leap year */
	if (hdate_size_of_hebrew_year (*year) > 365)
	{
		if (*month == 6)
			*month = 14;
		if (*month > 6)
			*month = *month - 1;
		if (*month == 6)
			*month = 14;
	}

	/* Check if computed month was overestimated */
	julian = hdate_hdate_to_jd (1, *month, *year);
	while (julian > jd)
	{
		/* Check for leap year */
		if (*month == 13)
			*month = 5;
		else
			*month = *month - 1;

		julian = hdate_hdate_to_jd (1, *month, *year);
	}

	/* Get the day */
	*day = jd - julian + 1;

	return;
}

/********************************************************************************/
/********************************************************************************/

/**
 @brief compute date structure from the Gregorian date

 @param d Day of month 1..31
 @param m Month 1..12 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 2001
 */
hdate_struct *
hdate_hdate (int d, int m, int y)
{
	static hdate_struct h;
	int jd, jd_tishrey1;
	
	/* check for null dates (kobi) */
	if ((d == 0) || (m == 0))
	{
		struct tm *tm;
		long t;
		/* FIXME: day start at 6:00 or 12:00 like in Gregorian cal. ? */
		t = time (0);
		tm = localtime (&t);
		d = tm->tm_mday;
		m = tm->tm_mon + 1;
		y = tm->tm_year + 1900;
	}
	else
	{
		/* sanity checks */
		if (!(m >= 1 && m <= 12) ||
		    !((d >= 1)
		      && ((y >= 3000) && (m == 6) && (d <= 59)
			  || (d <= 31))) || !(y > 0))
			return NULL;
	}
	/* end of cheking */

	h.gd_day = d;
	h.gd_mon = m;
	h.gd_year = y;
	
	jd = hdate_gdate_to_jd (d, m, y);
	hdate_jd_to_hdate (jd, &(h.hd_day), &(h.hd_mon), &(h.hd_year));
	jd_tishrey1 = hdate_hdate_to_jd (1,1,h.hd_year);
	
	h.hd_dw = (jd + 1) % 7 + 1;
	h.hd_size_of_year = hdate_size_of_hebrew_year (h.hd_year);
	h.hd_new_year_dw = (jd_tishrey1 + 1) % 7 + 1;
	h.hd_year_type = hdate_get_year_type (h.hd_size_of_year , h.hd_new_year_dw);
	h.hd_jd = jd;
	h.hd_days = jd - jd_tishrey1 + 1;
	h.hd_weeks = ((h.hd_days - 1) + (h.hd_new_year_dw - 1)) % 7 + 1;
	
	return (&h);
}

/**
 @brief compute date structure from the Hebrew date

 @param d Day of month 1..31
 @param m Month 1..14 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 5731
 */
hdate_struct *
hdate_gdate (int d, int m, int y)
{
	static hdate_struct h;
	int jd, jd_tishrey1;
	
	h.hd_day = d;
	h.hd_mon = m;
	h.hd_year = y;
	
	jd = hdate_hdate_to_jd (d, m, y);
	jd_tishrey1 = hdate_hdate_to_jd (1,1,h.hd_year);
	hdate_jd_to_gdate (jd, &(h.gd_day), &(h.gd_mon), &(h.gd_year));
		
	h.hd_dw = (jd + 1) % 7 + 1;
	h.hd_size_of_year = hdate_size_of_hebrew_year (h.hd_year);
	h.hd_new_year_dw = (jd_tishrey1 + 1) % 7 + 1;
	h.hd_year_type = hdate_get_year_type (h.hd_size_of_year , h.hd_new_year_dw);
	h.hd_jd = jd;
	h.hd_days = jd - jd_tishrey1 + 1;
	h.hd_weeks = ((h.hd_days - 1) + (h.hd_new_year_dw - 1)) % 7 + 1;
	
	return (&h);
}
