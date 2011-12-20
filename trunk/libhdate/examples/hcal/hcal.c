/* hcal.c
 * Hebrew calendar (part of package libhdate)
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hcal.c -o hcal
 * 
 * Copyright:  2011 (c) Boruch Baum, 2004-2010 (c) Yaacov Zamir 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE		// for mempcpy
#include <stdio.h>		// For printf
//include <hdate.h>		// For hebrew date
#include "../../src/hdate.h"
#include <stdlib.h>		// For atoi, malloc
#include <locale.h>		// For setlocale
#include <getopt.h>		// For getopt_long
#include <string.h>		// For strlen, mempcpy
#include <time.h>		// For time, tzset
//#include <wchar.h>		// for unicode character operations

#define FALSE 0
#define TRUE -1
#define SHABBAT 7

// for hdate_get_int_string_
#define NO_COMPRESS 0
#define COMPRESS 1

// copied from support.h in src dir
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


// for colorization
#define CODE_REVERSE_VIDEO "%c[7m", 27
#define CODE_RESTORE_VIDEO "%c[m", 27
#define CODE_BLACK         "%c[30m", 27
#define CODE_LIGHT_RED     "%c[31m", 27
#define CODE_LIGHT_GREEN   "%c[32m", 27
#define CODE_LIGHT_BROWN   "%c[33m", 27
#define CODE_DARK_BLUE     "%c[34m", 27
#define CODE_LIGHT_PURPLE  "%c[35m", 27
#define CODE_LIGHT_AQUA    "%c[36m", 27
#define CODE_LIGHT_GREY    "%c[37m", 27
#define CODE_BOLD_GREY     "%c[1;30m", 27
#define CODE_BOLD_RED      "%c[1;31m", 27
#define CODE_BOLD_GREEN    "%c[1;32m", 27
#define CODE_BOLD_YELLOW   "%c[1;33m", 27
#define CODE_BOLD_BLUE     "%c[1;34m", 27
#define CODE_BOLD_PURPLE   "%c[1;35m", 27
#define CODE_BOLD_AQUA     "%c[1;36m", 27
#define CODE_BOLD_WHITE    "%c[1;37m", 27
#define ELEMENT_WEEKDAY_G      1
#define ELEMENT_WEEKDAY_H      2
#define ELEMENT_SHABBAT_DAY    3
#define ELEMENT_HOLIDAY_DAY    4
#define ELEMENT_HOLIDAY_FLAG   5
#define ELEMENT_SHABBAT_NAME   6
#define ELEMENT_WEEKDAY_NAMES  7
#define ELEMENT_MONTH_G        8
#define ELEMENT_MONTH_H        9
#define ELEMENT_SHABBAT_TIMES 10
#define ELEMENT_PARASHA       11
#define ELEMENT_THIS_SHABBAT_TIMES 12
#define ELEMENT_THIS_PARASHA  13
#define ELEMENT_HOLIDAY_NAME  14
#define ELEMENT_TODAY_HOLIDAY_DAY  15
#define ELEMENT_TODAY_HOLIDAY_NAME 16

/**************************************************
*   functions to support hcal and hdate
**************************************************/
#include "./local_functions.c"		// for revstr



static char holiday_flag[] = { '/', '+', '*', '~', '!', '@', '#', '$', '%', '^' };
/*  Holiday types: (reference hdate_holyday.c)
	/ - 0 - Regular day
	+ - 1 - Yom tov (plus yom kippor)
	* - 2 - Erev yom kippur
	~ - 3 - Hol hamoed
	! - 4 - Hanuka and purim
	@ - 5 - Tzomot
	# - 6 - Independance day and Yom yerushalaim
	$ - 7 - Lag baomer ,Tu beav, Tu beshvat
	% - 8 - Tzahal and Holocaust memorial days
	^ - 9 - National days
*/



typedef struct {
			int html;
			int external_css;
			int diaspora;
			int parasha;
			int shabbat;
			int no_reverse;
			int three_month;
			int colorize;
			int footnote;
			int jd_today_g;
			int jd_today_h;
			int force_hebrew;
			int force_israel;
			int not_sunset_aware;
			int quiet_alerts;
			int bidi;
			double lat;
			double lon;
			int tz;
				} option_list;

typedef struct {
			int g_month_1;
			int g_year_1;
			int g_month_2;
			int g_year_2;
			int h_month_1;
			int h_year_1;
			int h_month_2;
			int h_year_2;
				} header_info;


const char* hcal_config_file_text = "\
# configuration file for hcal - Hebrew calendar program\n\
# part of package libhdate\n\
#\n# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hcal; hcal will automatically\n\
# regenerate the default content.\n#\n\
# Your system administrator can set system-wide defaults for hcal by\n\
# modifying file <not yet implemented>.\n\
# You may override all defaults by changing the contents of this file.\n\
#\n\
# Location awareness\n\
# hcal wants to accurately highlight the current Hebrew day, including\n\
# during the hours between sunset and secular midnight. If you don't\n\
# provide it with latitude, longitude, and time zone information, hcal\n\
# will try to guess the information, based upon your system timezone,\n\
# and its (limited, and maybe biased) of the dominant Jewish community\n\
# in that timezone. When hcal is forced to guess, it alerts the user\n\
# with a message that includes the guessed location.\n\
# hcal's guesses will also affect its default behaviour for ouput of\n\
# Shabbat times, parshiot, and choice of Israel/diaspora hoidays\n\
#SUNSET_AWARE=TRUE\n\
# LATITUDE and LONGITUDE may be in decimal format or in the form\n\
# degrees[:minutes[:seconds]] with the characters :'\" as possible\n\
# delimiters.\n\
#LATITUDE=\n\
#LONGITUDE=\n\
# TIMEZONE may may be in decimal format or in the form degrees[:minutes]\n\
# with the characters :'\" as possible delimiters.\n\
#TIMEZONE=\n\n\
# Israel and the diaspora\n\
# If hcal guesses that you're not in Israel, the DIASPORA option will be\n\
# set true. This will affect holiday and parasha output.  FORCE_ISRAEL\n\
# forces hcal to display calendar information for Israel, using hcal's\n\
# default coordinates foe Israel, or coordinates that you provide that\n\
# seem legitmately within Israel.\n\
# Thus, if you are living in Sao Paolo, and will be visiting Israel for\n\
# Sukkot, set BOTH flags true in order to see holiday information for\n\
# a non-resident vistor to Israel. The command line options for these\n\
# features are --israel, -I, --diaspora, -d.\n\
#FORCE_DIASPORA=FALSE;\n\
#FORCE_ISRAEL=FALSE;\n\n\
# Shabbat related\n\
# setting SHABBAT_INFO true will output parshiot and Shabbat times. The\n\
# command line options for these features are --parasha, -p, --shabbat -s.\n\
#PARASHA_NAMES=FALSE\n\
#SHABBAT_INFO=FALSE\n\n\
# Holiday identification\n\
# hcal flags holidays by replacing the gregorian/Hebrew date separator\n\
# with assorted unhelpful cryptic symbols. Setting FOOTNOTES to true\n\
# will have hcal output after the month's calendar, a list of the month's\n\
# holidays along with the days on which they occur.\n\
#FOOTNOTE=FALSE\n\n\
# Output in hebrew characters\n\
# hcal defaults to output all information in your default language, so\n\
# if your default language is Hebrew, you're all set. Otherwise, you can\n\
# set FORCE_HEBREW to true to output Hebrew information in Hebrew, while\n\
# still outputting gregorian information in your default language. To\n\
# output ALL information in Hebrew, run something like this:\n\
#    LC_TEMP=LC_ALL; LC_ALL=\"he_IL.UTF-8\"; hcal; LC_ALL=$LC_TEMP\n\
# If setting FORCE_HEBREW to true results in 'garbage' or non-Hebrew\n\
# output, you need to install a terminal font that includes the Hebrew\n\
# character set (hint: unicode).\n\
# The command line option for FORCE_HEBREW is either --hebrew or -H\n\
#FORCE_HEBREW=FALSE\n\n\
# The FORCE_HEBREW option outputs data that is 'correct' and 'logical'.\n\
# Unfortunately, the world can not be depended upon to be either. Most\n\
# Xwindow applications will display the data fine with FORCE_HEBREW; most\n\
# xterm implementations will not. (in fact, the only xterm clone I know\n\
# of that comes close is mlterm). If using FORCE_HEBREW results in\n\
# Hebrew characters being displayed in reverse, set OUTPUT_BIDI to true.\n\
# This will reverse the order of the Hebrew characters, so they will\n\
# display 'visual'ly correct; however, such output will not be suitable\n\
# for piping or pasting to many other applications. Setting OUTPUT_BIDI\n\
# automatically forces hebrew.\n\
# The command line option for OUTPUT_BIDI is either --bidi, --visual, or -b\n\
#OUTPUT_BIDI=FALSE\n\n\
# Display enhancements\n\
# hcal defaults to display the current day in reverse video\n\
# The command line option for this feature is --no-reverse\n\
#SUPPRESS_REVERSE_VIDEO=FALSE;\n\
# hcal can display its output \"calming, muted tones\". Note that piping\n\
# colorized output may yield unexpected results.\n\
#COLORIZE=FALSE\n\n\
# HTML OUTPUT\n\
#OUTPUT_HTML=FALSE\n\
#USE_EXTERNAL_CSS_FILE=\"pathto/foo/bar\"\n\n\
# Suppress alerts and warnings\n\
# hcal alerts the user via STDERR when it guesses the user's location.\n\
#QUIET_ALERTS=FALSE\n\n\
# Three month display\n\
# hcal can display a previous, current and next month side-by-side. hcal\n\
# can also display a calendar for an entire year in four rows of three\n\
# months each. Note that this will display properly only if your console\n\
# is set for at least 127 columns. Note also that setting this option to\n\
# will cause options FOOTNOTES, SHABBAT, and PARASHA_NAMES to false.\n\
#THREE_MONTH=FALSE\n";


/**************************************************
*  print version
*************************************************/
int print_version ()
{
	printf ("hcal - Hebrew calendar\nversion 2\n");
	return 0;
}

/**************************************************
*  print usage
*************************************************/
void print_usage ()
{
	printf ("\
Usage: hcal [options] [coordinates timezone] ] [[month] year]\n\
       coordinates: -l xx[.xxx] -L yy[.yyy]\n\
             -l xx[:mm[:ss]] -L yy[:mm[:ss]]\n\
       timezone:    -z nn[( .nn | :mm )]\n");
}

/**************************************************
*  print help
*************************************************/
void print_help ()
{
	print_usage();
	printf ("Hebrew calendar\nOPTIONS:\n\
   -3 --three-month   displays previous/next months\n\
                      side by side. requires 127 columns\n\
   -b --bidi          prints hebrew in reverse (visual)\n\
      --visual\n\
   -c --colorize      displays in calming, muted tones\n\
   -d --diaspora      use diaspora reading and holidays.\n\
   -f --footnote      print descriptive notes of holidays\n\
   -h --html          output in html format to stdout\n\
   -H --hebrew        print hebrew information in Hebrew\n\
   -i                 use external css file \"./hcal.css\".\n\
   -I --israel        override a diaspora default\n\n\
      --no-reverse    do not highlight today's date\n\
   -p --parasha       print week's parasha on each calendar row\n\
   -q --quiet-alerts  suppress warning messages\n\
   -s --shabbat       print Shabbat times and parshiot\n\
   -z --timezone nn   timezone, +/-UTC\n\
   -l --latitude xx   latitude xx degrees. Negative values are South\n\
   -L --longitude yy  longitude yy degrees. Negative values are West\n");
}


/**************************************************
*  HTML - print css section
*************************************************/
int print_css (int external_css)
{
	if (external_css) printf ("\n\t@import \"hcal.css\";\n");
	else
	{
		printf ("\nbody.hebrew { direction: rtl;}\n\
img { margin:0; padding: 0;	vertical-align: middle;	border: 0;}\n\
p {}\n\
table.main { width: 90%%; table-layout: fixed; font-size: 14pt; border: solid #aaaaaa; }\n\
table.day {	width: 100%%; height: 100%%; table-layout: fixed; font-size: 14pt; }\n\
th.gmonth { font-size: 30pt; font-weight: bold }\n\
th.gyear { font-size: 30pt; font-weight: bold}\n\
th.hmonth { font-size: 30pt; font-weight: bold}\n\
th.hyear {font-size: 30pt; font-weight: bold }\n\
th.week { background-color: #aaaaaa;	text-align: center; }\n\
td.gday { font-size: 30pt; vertical-align: top; }\n\
td.hday { font-size: 12pt; vertical-align: top; }\n\
td.sat { border: solid #777777; }\n\
td.regular { border: solid #aaaaaa; }\n\
td.holiday { color: #990000; border: solid #888888; }\n\
td.out_of_month { color: #dddddd; border: solid #dddddd; }\n\
tr.line1 { height: 30%%; }\n\
tr.line2 { height: 50%%; font-size: 30pt; }\n\
tr.line3 { height: 20%%; font-size: 12pt; vertical-align: bottom; }\n\
span.today { }\n\
span.gmonth { font-size: 24pt; }\n\
span.gyear { font-size: 24pt; }\n\
span.hmonth { font-size: 24pt; }\n\
span.hyear { font-size: 24pt; }\n\
span.holiday_name { }\n");
	}

	return 0;
}



/**************************************************
*  HTML - print header
*************************************************/
int print_header_html (int external_css)
{
	printf ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\"http://www.w3.org/TR/html4/strict.dtd\">\n\
<html>\n\
<head>\n\
<meta name=\"generator\" content=\"hcal (libhdate)\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n\
<style title=\"Normal\" type=\"text/css\" media=\"all\">");

	print_css (external_css);

	printf ("</style>\n");

	/* some alternative css styles */
	printf ("\n\
<link rel=\"alternate stylesheet\" title=\"High contrast\" type=\"text/css\" media=\"screen\" href=\"high_contrast.css\">\n\
<link rel=\"alternate stylesheet\" title=\"Colorful\" type=\"text/css\" media=\"screen\" href=\"colorful.css\">\n\
<link rel=\"alternate stylesheet\" title=\"Print\" type=\"text/css\" media=\"all\" href=\"print.css\">\n");

	if (hdate_is_hebrew_locale())
		printf ("<title>Hebrew Calendar</title>\n</head>\n<body class=\"hebrew\"><center>\n");
	else
		printf ("<title>Hebrew Calendar</title>\n</head>\n<body><center>\n");

	return 0;
}



/**************************************************
 *  HTML - print footer
 *************************************************/
int print_html_footer ()
{
	printf ("<!-- <p>\n\
<a href=\"http://validator.w3.org/check/referer\">\n<img \
src=\"http://www.w3.org/Icons/valid-html401\"\n\
alt=\"Valid HTML 4.01!\" height=\"31\" width=\"88\" />\n</a>\n");
	printf ("<a href=\"http://jigsaw.w3.org/css-validator\">\n\
<img src=\"http://www.w3.org/Icons/valid-css\" alt=\"Valid CSS!\">\n\
</a>\n</p> -->\n");

	printf ("</body></html>\n");

	return 0;
}


/**************************************************
*  Print HTML header - month line
*************************************************/
void print_header_month_line_html( header_info header)
{
	
	// TODO - fix bug ellul xxxx - tishrei xxxx+1
//	char *h_year_1;
//	h_year_1 = malloc(HEBREW_NUMBER_BUFFER_SIZE);
	char h_year_1[HEBREW_NUMBER_BUFFER_SIZE + 1];

	/**************************************************
	*  Print Gregorian month and year
	**************************************************/
	printf ("<table width=90%%><tr><td align=left><span class=\"gmonth\">%s <span class=\"gyear\">%d</span></td>\n",
			hdate_get_month_string (header.g_month_1, FALSE),
			header.g_year_1);

	/**************************************************
	*  Print Hebrew month and year
	**************************************************/
	printf ("<td align=right><span class=\"hmonth\">");
	if (header.h_month_1 != header.h_month_2)
	{
		printf ("%s-",
			hdate_get_hebrew_month_string (header.h_month_1, FALSE));
	}
	printf ("%s ", hdate_get_hebrew_month_string (header.h_month_2, FALSE));
	printf ("</span>\n");
	printf ("<span class=\"hyear\">%s</span></td></tr></table>\n", hdate_get_int_string_ (h_year_1, header.h_year_1, NO_COMPRESS));

	printf ("<span class=\"month_table\">\n<table class=main>\n");


//	free(h_year_1);
}


/**************************************************
*  HTML - print column headings for days of weeks
**************************************************/
void print_header_dow_line_html()
{
	int j;
	
	for (j = 1; j < 8; j++)
		printf ("<th class=\"week\">%3s</th>", hdate_get_day_string (j, TRUE));
}



/**************************************************
 *  end HTML functions
 *************************************************/


/*************************************************
*  Display data in a more pleasing visual manner
*************************************************/
void colorize_element ( int element )
{
	switch (element) {
	case ELEMENT_WEEKDAY_G: printf(CODE_LIGHT_GREY); break;
	case ELEMENT_WEEKDAY_H: printf(CODE_LIGHT_BROWN); break;
	case ELEMENT_MONTH_G: printf(CODE_LIGHT_GREY); break;
	case ELEMENT_MONTH_H: printf(CODE_LIGHT_BROWN); break;
	case ELEMENT_WEEKDAY_NAMES: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_SHABBAT_NAME: printf(CODE_LIGHT_AQUA); break;
	case ELEMENT_SHABBAT_DAY: printf(CODE_LIGHT_AQUA); break;
	case ELEMENT_HOLIDAY_DAY: printf(CODE_LIGHT_AQUA); break;
	case ELEMENT_SHABBAT_TIMES: printf(CODE_LIGHT_PURPLE); break;
	case ELEMENT_PARASHA: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_THIS_SHABBAT_TIMES: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_THIS_PARASHA: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_HOLIDAY_NAME: printf(CODE_LIGHT_GREY); break;
	case ELEMENT_TODAY_HOLIDAY_DAY: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_TODAY_HOLIDAY_NAME: printf(CODE_LIGHT_GREEN); break;
	}
}



/**************************************************
 *  print header - month line to stdout
 *************************************************/
void print_header_month_line_stdout( header_info header, int colorize, int force_hebrew, int visual_bidi )
{
	char *g_month, *h1_month, *h1_year, *h2_month, *h2_year;
	size_t g_month_len, g_year_len, h1_month_len, h2_month_len, h1_year_len, h2_year_len;

	char *hebrew_buffer, *hebrew_buffer_next;
	size_t hebrew_buffer_len, literals_len;

	int j;
	size_t padding = 0;

	// for forcing Hebrew printing of Hebrew data
	char *language;


	/**************************************************
	*  Print Gregorian month and year
	*************************************************/
	g_month = hdate_get_month_string (header.g_month_1, FALSE);
	if (colorize) colorize_element(ELEMENT_MONTH_G);
	printf ("%s %d", g_month, header.g_year_1);
	if (colorize) printf(CODE_RESTORE_VIDEO);

	/**************************************************
	*  Print Hebrew month and year
	*
	*  because we want this printed right-justified on
	*  the same line, we need to calculate how many
	*  spaces to pad.
	* 
	*  because of the possible user option --bidi, we
	*  need to buffer the entire Hebrew text in order
	*  reverse its sequence for 'visual' display.
	* 
	*  This all requires collecting the data first
	*  and determining each's length.
	*************************************************/

	#define CALENDAR_WIDTH 41
	#define DASH_WIDTH      3 // includes surrounding spaces
	#define SPACE_WIDTH		1


	/*************************************************
	*  padding info for gregorian date
	*************************************************/
	g_month_len = strlen(g_month);
	g_year_len = 4;
	switch (header.g_year_1)
	{
	case 1-9:		g_year_len  = 1; break;
	case 10-99:		g_year_len  = 2; break;
	case 100-999:	g_year_len  = 3; break;
	}
	
	/*************************************************
	*  Hebrew date entry - force-hebrew setup
	*************************************************/
	if (force_hebrew)
	{
		// BUG - this isn't thread-safe
		language = getenv("LANGUAGE");
		setenv("LANGUAGE", "he_IL.UTF-8", 1);
	}

	/**************************************************
	*  collect padding info for Hebrew date
	*************************************************/
	h1_year = malloc(HEBREW_NUMBER_BUFFER_SIZE+1);
	h1_month = hdate_get_hebrew_month_string (header.h_month_1, FALSE);
	h1_year = hdate_get_int_string_ (h1_year, header.h_year_1, NO_COMPRESS);
	h1_month_len = strlen(h1_month);
	h1_year_len = strlen(h1_year);
//	DEBUG - printf("\nh1 year value = %d, h1 year string = %s, string length = %d\n",header.h_year_1, h1_year, h1_year_len);
//	DEBUG - int ii; for (ii=0; ii<h1_year_len; ii++) printf("%x  ",h1_year[ii]);

	if (header.h_year_1 != header.h_year_2)
	{
		h2_year = malloc(HEBREW_NUMBER_BUFFER_SIZE+1);

		h2_month = hdate_get_hebrew_month_string (header.h_month_2, FALSE);
		h2_year = hdate_get_int_string_ (h2_year, header.h_year_2, NO_COMPRESS);

		h2_month_len = strlen(h2_month);
		h2_year_len = strlen(h2_year);
		literals_len = SPACE_WIDTH + DASH_WIDTH + SPACE_WIDTH;

		hebrew_buffer_len = h1_month_len + h1_year_len + h2_month_len
							+ h2_year_len + literals_len;
							
//		printf("\nlength values = %d %d %d %d %d %d\n",h1_month_len , h1_year_len , DASH_WIDTH, h2_month_len, SPACE_WIDTH , h2_year_len);
		hebrew_buffer = malloc(hebrew_buffer_len+1);
		hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " - ", DASH_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_month, h2_month_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_year, h2_year_len);
		hebrew_buffer[hebrew_buffer_len]='\0';

//		printf("\nbuffer length = %d\nbuffer contents = %s\n",hebrew_buffer_len, hebrew_buffer); exit(0);
//		free(hebrew_buffer);
	}
	else if (header.h_month_1 != header.h_month_2)
	{
		h2_month = hdate_get_hebrew_month_string (header.h_month_2, FALSE);

		h2_month_len = strlen(h2_month);
		literals_len = DASH_WIDTH + SPACE_WIDTH;

		hebrew_buffer_len = h1_month_len + h2_month_len + h1_year_len + literals_len;
		hebrew_buffer = malloc(hebrew_buffer_len+1);
		hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " - ", DASH_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_month, h2_month_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
		hebrew_buffer[hebrew_buffer_len]='\0';
	}
	else 
	{
		h2_month_len = 0;
		h2_year_len = 0;
		literals_len = SPACE_WIDTH;

		hebrew_buffer_len = h1_month_len + h1_year_len + literals_len;

		hebrew_buffer = malloc(hebrew_buffer_len+1);
		hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
		hebrew_buffer[hebrew_buffer_len]='\0';
	}

	if (!force_hebrew)
		padding = CALENDAR_WIDTH - g_month_len - g_year_len - hebrew_buffer_len ;
	else
		padding = CALENDAR_WIDTH - g_month_len - g_year_len - (hebrew_buffer_len/2) - literals_len +1;


	
	/**************************************************
	*  print padding
	*************************************************/
	for (j = 1; j < padding; j++) printf(" ");



	/**************************************************
	*  Print Hebrew month and year
	*************************************************/
	if (colorize) colorize_element(ELEMENT_MONTH_H);
	if (visual_bidi) revstr(hebrew_buffer, hebrew_buffer_len);
	printf ("%s", hebrew_buffer);
	if (colorize) printf(CODE_RESTORE_VIDEO);


	/**************************************************
	*  clean up force of Hebrew
	*************************************************/
	if (force_hebrew) setenv("LANGUAGE", language, 1);

	/**************************************************
	*  free allocated memory
	*************************************************/
	free(h1_year);
	if (header.h_year_1 != header.h_year_2)	free(h2_year);
	free(hebrew_buffer);
}


/**************************************************
*  print column headings for days of weeks
**************************************************/
void print_header_dow_line_stdout( int colorize )
{
	int column;

	void print_dow_column( int column )
	{
		if (hdate_is_hebrew_locale())
		{		/* Hebrew heading is a single charcter per day */
			printf ("%s%s%s", "  ", hdate_get_day_string (column, TRUE)," ");
		}
		else
		{		/* presume three character heading */
			printf ("%s%3s", " ", hdate_get_day_string (column, TRUE));
		}
	
		if (column != 7) printf ("  ");
	}
	
	if (colorize) colorize_element(ELEMENT_WEEKDAY_NAMES);
	for (column = 1; column < 7; column++) print_dow_column(column);
	if (colorize) colorize_element(ELEMENT_SHABBAT_NAME);
	print_dow_column(7);
	if (colorize) printf(CODE_RESTORE_VIDEO);
}



/**************************************************
*  calculate start/end month/year for a calendar
**************************************************/
header_info calculate_calendar_start_end (int month, int year)
{
	hdate_struct h;
	header_info header;

	hdate_set_gdate (&h, 1, month, year);
	header.g_month_1 = h.gd_mon;
	header.g_year_1 = h.gd_year;
	header.h_month_1 = h.hd_mon;
	header.h_year_1 = h.hd_year;


	hdate_set_jd ( &h, hdate_gdate_to_jd ( 1, month % 12 + 1, ((month==12) ? year+1 : year) ) - 1  );
//	DEBUG - hdate_set_jd ( &h, 3313455  );
//	DEBUG - printf("\nfunction calculate_calendar_start: for julian day 3313455,  month #2 = %d\n",h.hd_mon);
	header.g_month_2 = h.gd_mon;
	header.g_year_2 = h.gd_year;
	header.h_month_2 = h.hd_mon;
	header.h_year_2 = h.hd_year;

	return header;
}


/**************************************************
 *  print header - year and month
 *************************************************/
int print_header (int month, int year, option_list opt)
{
	header_info previous_month, current_month, next_month;


	/**********************************************************
	*  Preliminary - set dates for begining and end of calendar
	**********************************************************/
	current_month = calculate_calendar_start_end(month, year);
	if (opt.three_month)
	{
		previous_month = calculate_calendar_start_end(
					(month==1 ? 12 : month-1), (month==1 ? year-1 : year) );
		next_month = calculate_calendar_start_end(
					(month==12 ? 1 : month+1), (month==12 ? year+1: year) );
	}
	
	/**************************************************
	*  HTML output - month header line
	*************************************************/
	if (opt.html) print_header_month_line_html(current_month);


	/**************************************************
	*  STDOUT output - month header line
	*************************************************/
	else
	{
		if (opt.three_month)
		{
			print_header_month_line_stdout(previous_month, opt.colorize, opt.force_hebrew, opt.bidi);
			printf("  ");
		}

		print_header_month_line_stdout(current_month, opt.colorize, opt.force_hebrew, opt.bidi);

		if (opt.three_month)
		{
			printf("  ");
			print_header_month_line_stdout(next_month, opt.colorize, opt.force_hebrew, opt.bidi);
		}
	}

	/**************************************************
	*  terminate line
	**************************************************/
	if (opt.html) printf ("</tr>\n");
	else printf ("\n");


	/**************************************************
	*  print column headings for days of weeks
	**************************************************/
	if (opt.html) print_header_dow_line_html();
	else
	{
		if (opt.three_month)
		{
			print_header_dow_line_stdout(opt.colorize);
			printf("   ");
		}
		print_header_dow_line_stdout(opt.colorize);
		if (opt.three_month)
		{
			printf("   ");
			print_header_dow_line_stdout(opt.colorize);
		}
	}

	/**************************************************
	*  terminate line
	**************************************************/
	if (opt.html) printf ("</tr>\n");
	else printf ("\n");

	return 0;
}


/**************************************************
*  print HTML calendar entry (ie. a single day)
*************************************************/
void print_day_html ( hdate_struct h, int month, option_list opt)
{

	int holiday_type;
//	char *hd_day;
//	hd_day = malloc(HEBREW_NUMBER_BUFFER_SIZE);
	char hd_day_buffer[HEBREW_NUMBER_BUFFER_SIZE + 1];

	
	holiday_type = hdate_get_holyday_type(hdate_get_holyday(&h, opt.diaspora));


	if (h.gd_mon != month) printf ("<td class=\"out_of_month\">");
	else if (h.hd_dw == SHABBAT) printf ("<td class=\"sat\">");
	else if (holiday_type) printf ("<td class=\"holiday\">");
	else printf ("<td class=\"regular\">");

	/* Print a day */
	printf ("<table class=day><tr class=line1><td class=\"gday\">%2d</td>\n\
<td class=\"hday\">%3s&nbsp;%s</td></tr><tr class=line2><td>&nbsp;</td></tr>",
		h.gd_day,
		hdate_get_int_string_ (hd_day_buffer, h.hd_day, NO_COMPRESS),
		hdate_get_hebrew_month_string (h.hd_mon, FALSE));

	if (holiday_type) printf ("<tr class=line3><td class=\"holiday_name\">%s</td></tr>",
			hdate_get_holyday_string (hdate_get_holyday (&h, opt.diaspora), FALSE));
	else printf ("<tr class=line3><td class=\"holiday_name\">&nbsp;</td></tr>");

	printf ("</table></td>\n");
	
//	free(hd_day);
}


/*************************************************
*  print stdout calendar entry (ie. a single day)
*************************************************/
void print_day ( hdate_struct h, int month, option_list opt)
{

	// for forcing Hebrew printing of Hebrew data
	char *language;

	char hd_day_buffer[HEBREW_NUMBER_BUFFER_SIZE + 1];
	// hd_day_buffer = malloc(HEBREW_NUMBER_BUFFER_SIZE + 1);
	
	size_t hd_day_buffer_str_len;

	int holiday_type;
	holiday_type = hdate_get_holyday_type(hdate_get_holyday(&h, opt.diaspora));

	/**************************************************
	*  out of month - needs padding
	*************************************************/
	if (h.gd_mon != month) printf("     ");


	/**************************************************
	*  in month - print the data
	*************************************************/
	else
	{
		/*************************************************
		*  Gregorian date entry - color prefix
		*************************************************/
//		unnecessary because opt.jd_today_g/h == 0 if opt.no_reverse 
//		if ((h.hd_jd == opt.jd_today_g) && (! opt.no_reverse))
		if (h.hd_jd == opt.jd_today_g)
				printf (CODE_REVERSE_VIDEO);
		else if (opt.colorize)
		{
			if (h.hd_dw==7) colorize_element(ELEMENT_SHABBAT_DAY);
			else if (holiday_type) colorize_element(ELEMENT_HOLIDAY_DAY);
			else            colorize_element(ELEMENT_WEEKDAY_G);
		}
		
		/*************************************************
		*  Gregorian date entry - the day of the month
		*************************************************/
		// this next line is necessary to align numbers
		// correctly with bidi (tested using mlterm)
		if ( (h.gd_day < 10) && (hdate_is_hebrew_locale()) )
			 printf ("%d ", h.gd_day);
		else printf ("%2d", h.gd_day);
		
		/*************************************************
		*  Gregorian date entry - color cleanup
		*************************************************/
		if ((h.hd_jd == opt.jd_today_g) || (opt.colorize))
			printf (CODE_RESTORE_VIDEO);


		/*************************************************
		*  holiday flag
		*************************************************/
		printf ("%c", holiday_flag[holiday_type]);


		/*************************************************
		*  Hebrew date entry - color prefix
		*************************************************/
//		unnecessary because opt.jd_today_g/h == 0 if opt.no_reverse 
//		if ((h.hd_jd == opt.jd_today_h) && (! opt.no_reverse))
		if (h.hd_jd == opt.jd_today_h)
				printf (CODE_REVERSE_VIDEO);
		else if (opt.colorize)
		{
			if (h.hd_dw==7) colorize_element(ELEMENT_SHABBAT_DAY);
			else if (holiday_type) colorize_element(ELEMENT_HOLIDAY_DAY);
			else            colorize_element(ELEMENT_WEEKDAY_H);
		}

		/*************************************************
		*  Hebrew date entry - force-hebrew setup
		*************************************************/
		if (opt.force_hebrew)
		{
			// BUG - this isn't thread-safe
			language=getenv("LANGUAGE");
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
		}

		/*************************************************
		*  Hebrew date entry - day of the month
		*************************************************/
//		printf("\nDEBUG: h.hd_day = %d\n",h.hd_day);
		hdate_get_int_string_(hd_day_buffer, h.hd_day,COMPRESS);
		if (opt.bidi)
		{
			hd_day_buffer_str_len = (size_t) strlen(hd_day_buffer);
			if ( hd_day_buffer_str_len > HEBREW_NUMBER_BUFFER_SIZE)
				hd_day_buffer_str_len = HEBREW_NUMBER_BUFFER_SIZE - 1;
			revstr(hd_day_buffer, hd_day_buffer_str_len);
		}
		if  (hdate_is_hebrew_locale() &&
			( (h.hd_day < 11) || (h.hd_day == 20) || (h.hd_day == 30) ) )
		{	// need to pad Hebrew dates 1-10, 20, 30
			printf ("%s%s"," ",hd_day_buffer);
		}
		else printf ("%2s", hd_day_buffer);

		/*************************************************
		*  Hebrew date entry - color cleanup
		*************************************************/
		if ((h.hd_jd == opt.jd_today_h) || (opt.colorize))
			printf (CODE_RESTORE_VIDEO);

		/*************************************************
		*  Hebrew date entry - force-hebrew cleanup
		*************************************************/
		if (opt.force_hebrew) setenv("LANGUAGE", language, 1);
	}

	//free(hd_day_buffer);
}


/*************************************************
*  print a calendar week's entry (ie. seven columns)
*************************************************/
void print_week( int jd, int month, option_list opt)
{
#define long_parasha_name 0

	hdate_struct h;

	int calendar_column;

	// for opt.shabbat
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;
	int this_week;
	hdate_struct yom_shishi;

	// for opt.parasha
	int shabbat_name;
	char *shabbat_name_str, *shabbat_name_buffer;
	size_t shabbat_name_str_len;


	// for forcing Hebrew printing of Hebrew data
	char *language;
	
	
	/**************************************************
	*  for each column of calendar
	*************************************************/
	for (calendar_column = 0; calendar_column < 7; calendar_column++)
	{

		/**************************************************
		* Get this day's information
		*************************************************/
		hdate_set_jd (&h, jd);
		if ( ((opt.shabbat) || (opt.parasha)) && (calendar_column == 5) )
			yom_shishi = h;


		/**************************************************
		*  HTML calendar option
		*************************************************/
		if (opt.html) print_day_html ( h, month, opt );



		/**************************************************
		*  non-HTML calendar option
		*************************************************/
		else print_day ( h, month, opt);


		/**************************************************
		*  space between days of week
		*************************************************/
		if (calendar_column != 6)	printf (" ");
		
		/**************************************************
		*  advance to next day of week
		*************************************************/
		jd++;
	}

	/**************************************************
	*  print end of calendar line
	*************************************************/
	if ((!opt.html) && ( (h.gd_mon == month) || (h.gd_day < SHABBAT) ) )
	{

		/********************************************************
		*  determine whether this line gets special highlighting
		********************************************************/
		if ((opt.colorize)
			&& ((opt.shabbat) || (opt.parasha))
			&& ((jd - 1) > opt.jd_today_h)
			&& (((jd - 1) - opt.jd_today_h) < 7) )
				this_week = TRUE;
		else this_week = FALSE;

		/*************************************************
		*  print shabbat times
		*************************************************/
		if (opt.shabbat)
		{
			// candlelighting times
			hdate_get_utc_sun_time (yom_shishi.gd_day, yom_shishi.gd_mon, yom_shishi.gd_year,
									opt.lat, opt.lon, &sunrise, &sunset);
			// FIXME - allow for minhag variation
			sunset = sunset + opt.tz * 60 - 20;
			if (opt.colorize)
			{
				if (this_week) colorize_element(ELEMENT_THIS_SHABBAT_TIMES);
				else colorize_element(ELEMENT_SHABBAT_TIMES);
			}
			printf ("  %02d:%02d", sunset / 60, sunset % 60);
			if (opt.colorize) printf (CODE_RESTORE_VIDEO);
	
			printf(" - ");
	
			// motzay shabat time
			hdate_get_utc_sun_time_full (h.gd_day, h.gd_mon, h.gd_year, opt.lat,
										 opt.lon, &sun_hour, &first_light, &talit,
										 &sunrise, &midday, &sunset,
										 &first_stars, &three_stars);
			// FIXME - allow for minhag variation
			three_stars = three_stars + opt.tz * 60;
			if (opt.colorize)
			{
				if (this_week) colorize_element(ELEMENT_THIS_SHABBAT_TIMES);
				else colorize_element(ELEMENT_SHABBAT_TIMES);
			}
			printf ("%02d:%02d", three_stars / 60, three_stars % 60);
			if (opt.colorize) printf (CODE_RESTORE_VIDEO);
		}


		/*************************************************
		*  print shabbat name
		*************************************************/
		if (opt.parasha)
		{

			/*************************************************
			*  print shabbat name - force-hebrew setup
			*************************************************/
			// BUG - this isn't thread-safe
			if (opt.force_hebrew)
			{
				language=getenv("LANGUAGE");
				setenv("LANGUAGE", "he_IL.UTF-8", 1);
			}


			shabbat_name = hdate_get_parasha (&h, opt.diaspora);
			if (shabbat_name) shabbat_name_str = hdate_get_parasha_string (shabbat_name, long_parasha_name);
			else
			{
				shabbat_name = hdate_get_holyday(&h, opt.diaspora);
				if (shabbat_name) shabbat_name_str = hdate_get_holyday_string (shabbat_name, FALSE);
			}
			if (shabbat_name)
			{
				if (opt.colorize)
				{
					if (this_week) colorize_element(ELEMENT_THIS_PARASHA);
					else colorize_element(ELEMENT_PARASHA);
				}
				if (opt.bidi)
				{
					shabbat_name_str_len = strlen(shabbat_name_str);
					shabbat_name_buffer = malloc(shabbat_name_str_len+1);
					memcpy(shabbat_name_buffer, shabbat_name_str, shabbat_name_str_len);
					shabbat_name_buffer[shabbat_name_str_len] = '\0';
					revstr(shabbat_name_buffer, shabbat_name_str_len);

					// padding - FIXME - spaces are single-byte, while
					// the Hebrew characters are two bytes
					const int margin_max = 15;
					printf("%*s%s", (margin_max - shabbat_name_str_len/2)," ", shabbat_name_buffer);

					free(shabbat_name_buffer);
				}
				else printf("  %s", shabbat_name_str);
				if (opt.force_hebrew) setenv("LANGUAGE", language, 1);
				if (opt.colorize) printf (CODE_RESTORE_VIDEO);
			}
		}
	}
}




/**************************************************
*  print month table
*************************************************/
int print_calendar (int current_month, int current_year, option_list opt)
{
	hdate_struct h;
	int calendar_line;
	int max_calendar_lines = 4;
	int previous_month, next_month;
	int previous_year, next_year;
	int jd_current_month, jd_previous_month, jd_next_month;
	
	void how_many_calendar_lines( int month, int start_dow )
	{
		switch (month)
		{
		case  4:
		case  6:
		case  9:
		case 11:	if (start_dow == 7) max_calendar_lines = 6;
					else if (max_calendar_lines == 4) max_calendar_lines = 5;
					break;
		case  1:
		case  3:
		case  5:
		case  7:
		case  8:
		case 10:
		case 12:	if (start_dow > 5) max_calendar_lines = 6;
					else if (max_calendar_lines == 4) max_calendar_lines = 5;
					break;
		}
	}

	/*********************************************************
	*  Preliminaries:
	*  - Find the first sunday(s) of each calendar
	*  - Find number of calendar lines
	*********************************************************/
	hdate_set_gdate (&h, 1, current_month, current_year);
	jd_current_month = h.hd_jd - h.hd_dw + 1;
	how_many_calendar_lines( h.gd_mon, h.hd_dw );

	/*********************************************************
	*  three months, side-by-side
	*********************************************************/
	if (opt.three_month)
	{
		/*********************************************************
		*  previous month
		*********************************************************/
		if (current_month == 1)
		{
			previous_month = 12;
			previous_year = current_year - 1;
		}
		else
		{
			previous_month = current_month - 1;
			previous_year = current_year;
		}
		hdate_set_gdate (&h, 1, previous_month, previous_year);
		jd_previous_month = h.hd_jd - h.hd_dw + 1;
		how_many_calendar_lines( h.gd_mon, h.hd_dw );
		
		/*********************************************************
		*  next month
		*********************************************************/
		if (current_month == 12)
		{
			next_month = 1;
			next_year = current_year + 1;
		}
		else
		{
			next_month = current_month + 1;
			next_year = current_year;
		}
		hdate_set_gdate (&h, 1, next_month, next_year);
		jd_next_month = h.hd_jd - h.hd_dw + 1;
		how_many_calendar_lines( h.gd_mon, h.hd_dw );
	}
		
	

	/**************************************************
	*  maximum six lines of calendar
	**************************************************/
	for (calendar_line = 0; calendar_line < max_calendar_lines; calendar_line++)
	{
		if (opt.html) printf ("<tr>\n");

		if (opt.three_month)
		{
			print_week(jd_previous_month, previous_month, opt);
			jd_previous_month = jd_previous_month + 7;
			printf("  ");
		}

		print_week(jd_current_month, current_month, opt);
		jd_current_month = jd_current_month + 7;
		
		
		if (opt.three_month)
		{
			printf("  ");
			print_week(jd_next_month, next_month, opt);
			jd_next_month = jd_next_month + 7;
		}
		
		if (opt.html) printf ("</tr>\n");
		else printf ("\n");
	}


	/**************************************************
	*  print end of calendar
	*************************************************/
	if (opt.html) printf ("</table>\n</span>");

	return 0;
}






/****************************************************
* print month header, month table, month footnotes
****************************************************/
int print_month (int month, int year, option_list opt)
{
	hdate_struct h;

	// following are for opt.footnote
	int jd_counter, holiday;
	char *language; // for forcing Hebrew printing of footnote


	/* check if hebrew year (year > 3000) */
	if (year > 3000) hdate_set_hdate (&h, 1, month, year);
	else			 hdate_set_gdate (&h, 1, month, year);


	print_header (h.gd_mon, h.gd_year, opt);
	print_calendar (h.gd_mon, h.gd_year, opt);
	printf("\n");

	
	/****************************************************
	* print footnotes
	****************************************************/
	if (opt.footnote)
	{
		jd_counter = h.hd_jd;
		month = h.gd_mon;
		while ( month == h.gd_mon )
		{
			holiday = hdate_get_holyday(&h, opt.diaspora);
			if (holiday)
			{
				print_day ( h, month, opt);
				if (opt.force_hebrew)
				{
					// BUG - this isn't thread-safe
					language=getenv("LANGUAGE");
					setenv("LANGUAGE", "he_IL.UTF-8", 1);
				}
				if (opt.colorize)
				{
					if (opt.jd_today_h == h.hd_jd) colorize_element(ELEMENT_TODAY_HOLIDAY_NAME);
					else colorize_element(ELEMENT_HOLIDAY_NAME);
				}

				if (opt.bidi)
				{
					char *holiday_str, *holiday_buffer;
					size_t holiday_str_len;
					holiday_str = hdate_get_holyday_string (holiday, TRUE);
					holiday_str_len = strlen(holiday_str);
					holiday_buffer = malloc(holiday_str_len+1);
					memcpy(holiday_buffer, holiday_str, holiday_str_len);
					holiday_buffer[holiday_str_len] = '\0';
					revstr(holiday_buffer, holiday_str_len);
					
					// padding - FIXME - spaces are single-byte, while
					// the Hebrew characters are two bytes
					const int margin_max = 16; // 15 fails because of jabotinsky in july
					printf("%*s%s\n", (margin_max - holiday_str_len/2)," ", holiday_buffer);

					free(holiday_buffer);
				}
				else printf ("  %s\n", hdate_get_holyday_string (holiday, FALSE));

				if (opt.colorize) printf (CODE_RESTORE_VIDEO);
				if (opt.force_hebrew) setenv("LANGUAGE", language, 1);
			}
			jd_counter++;
			hdate_set_jd (&h, jd_counter);
		}
	}

	return 0;
}


/****************************************************
* read and parse config file
****************************************************/
void read_config_file( FILE *config_file, option_list *opt )
{
	char	*input_string;
	size_t	input_str_len = 50;
	char	*input_key = "";
	char	*input_value = "";
	int		line_count = 0;
	int		match_count;
	int		end_of_input_file = FALSE;
	int		i;
	const int	num_of_keys = 17;
	const char*	key_list[] = {	"SUNSET_AWARE",
								"LATITUDE",
								"LONGITUDE",
								"TIMEZONE",
								"DIASPORA",
								"FORCE_ISRAEL",
								"PARASHA_NAMES",
								"SHABBAT_INFO",
								"FOOTNOTE",
								"FORCE_HEBREW",
								"OUTPUT_BIDI",
								"SUPPRESS_REVERSE_VIDEO",
								"COLORIZE",
								"OUTPUT_HTML",
								"USE_EXTERNAL_CSS_FILE",
								"QUIET_ALERTS",
								"THREE_MONTH"};
	


	input_string = malloc(input_str_len+1);
	input_key    = malloc(input_str_len+1);
	input_value  = malloc(input_str_len+1);
	while ( end_of_input_file!=TRUE )
	{
		end_of_input_file = getline(&input_string, &input_str_len, config_file);
		if ( end_of_input_file!=TRUE )
		{
			errno = 0;
			match_count = sscanf(input_string,"%[A-Z_]=%sa",input_key,input_value);
			line_count++;
			if (errno != 0) error(0,errno,"scan error at line %d", line_count);
// DEBUG - 	printf("line number = %d, matches made = %d, key = %s, value = %s, string = %s",
//					line_count, match_count, input_key, input_value, input_string);
			if (match_count ==2)
			{
				for (i=0; i<num_of_keys; i++)
				{
					if (strcmp(input_key, key_list[i]) == 0)
					{
// DEBUG -				printf("match found!, %s = %s, index = %d\n",input_key,key_list[i],i);
						switch(i)
						{

		case  0:if      (strcmp(input_value,"FALSE") == 0) opt->not_sunset_aware = 1;
				else if (strcmp(input_value,"TRUE") == 0) opt->not_sunset_aware = 0;
				break;
		case  1:break;		//LATITUDE
		case  2:break;		//Longitude
		case  3:break;		//time zone
		case  4:if      (strcmp(input_value,"FALSE") == 0) opt->diaspora = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->diaspora = 1;
				break;
		case  5:if      (strcmp(input_value,"FALSE") == 0) opt->force_israel = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->force_israel = 1;
				break;
		case  6:if      (strcmp(input_value,"FALSE") == 0) opt->parasha = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->parasha = 1;
				break;
		case  7:if      (strcmp(input_value,"FALSE") == 0) opt->shabbat = 0;
				else if (strcmp(input_value,"TRUE") == 0)
				{
					opt->shabbat = 1;
					opt->parasha = 1;
				}
				break;
		case  8:if      (strcmp(input_value,"FALSE") == 0) opt->footnote = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->footnote = 1;
				break;
		case  9:if      (strcmp(input_value,"FALSE") == 0) opt->force_hebrew = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->force_hebrew = 1;
				break;
		case 10:if      (strcmp(input_value,"FALSE") == 0) opt->bidi = 0;
				else if (strcmp(input_value,"TRUE") == 0)
						{
							opt->bidi = 1;
							opt->force_hebrew = 1;
						}
				break;
		case 11:if      (strcmp(input_value,"FALSE") == 0) opt->no_reverse = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->no_reverse = 1;
				break;
		case 12:if      (strcmp(input_value,"FALSE") == 0) opt->colorize = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->colorize = 1;
				break;
		case 13:if      (strcmp(input_value,"FALSE") == 0) opt->html = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->html = 1;
				break;
		case 14:if      (strcmp(input_value,"FALSE") == 0) opt->external_css = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->external_css = 1;
				break;
		case 15:if      (strcmp(input_value,"FALSE") == 0) opt->quiet_alerts = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->quiet_alerts = 1;
				break;
		case 16:if      (strcmp(input_value,"FALSE") == 0) opt->three_month = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->three_month = 1;
				break;

						}
						break;
					}
				}
			}
		}
	}
	free(input_string);
	free(input_key);
	free(input_value);
	return;	
}



/**************************************************
***************************************************
***************************************************
* main
***************************************************
***************************************************
**************************************************/
int main (int argc, char *argv[])
{

	/* date */
	int month, year;

	double lat = BAD_COORDINATE;	/* set to this value for error handling */
	double lon = BAD_COORDINATE;	/* set to this value for error handling */
	int tz = BAD_TIMEZONE;			/* -z option Time Zone, default to system local time */

	int opt_latitude = 0;			/* -l option latitude */
	int opt_Longitude = 0;			/* -L option longitude */
	
	int error_detected = FALSE;		/* exit after reporting ALL bad parms */


	option_list opt;
	opt.bidi = 0;			// visual bidi, implies --force-hebrew
	opt.html = 0;			// -h html format flag
	opt.diaspora = 0;		// -d Diaspora holidays
	opt.external_css = 0;	// -i External css file
	opt.parasha = 0;		// -p print parasha alongside calendar
	opt.shabbat = 0;		// -c print candle-lighting alongside calendar
	opt.no_reverse = 0;		// don't highlight today in reverse video
	opt.three_month = 0;	// print previous and next months also
	opt.colorize = 0;		// display calendar in muted, more pleasing tones
	opt.footnote = 0;		// display description of month's holidays
	opt.force_hebrew = 0;	// force display of Hebrew data in Hebrew
	opt.force_israel = 0;	// override diaspora-awareness
	opt.not_sunset_aware = 0;	// override sunset-awareness
	opt.quiet_alerts = 0;
	opt.lat = BAD_COORDINATE;
	opt.lon = BAD_COORDINATE;
	opt.tz = BAD_TIMEZONE;


	// support for getopt short options
	static char * short_options = "bhHdiIpq3cfsl:L:z:";
	
	/* support for long options */
	int option_index = 0;
	int c;
	static struct option long_options[] = {
		{"version", 0, 0, 0},
		{"help", 0, 0, 0},
		{"no-reverse", 0, 0, 0},
		{"html", 0, 0, 0},
		{"parasha", 0, 0, 0},
		{"shabbat", 0, 0, 0},
		{"three-month", 0, 0, 0},
		{"colorize", 0, 0, 0},
		{"footnote",0,0,0},
		{"hebrew",0,0,0},
		{"israel",0,0,0},
		{"latitude", 0, 0, 0},
		{"longitude", 0, 0, 0},
		{"timezone", 0, 0, 0},
		{"not-sunset-aware", 0, 0, 0},
		{"quiet-alerts",0,0,0},
		{"bidi",0,0,0},
		{"visual",0,0,0},
		{0, 0, 0, 0}
		};

	/* hdate struct */
	hdate_struct h;

	/* init locale */
	/* WHY ??*/
	setlocale (LC_ALL, "");


	/************************************************************
	* parse config file
	************************************************************/
	FILE *config_file = get_config_file("/hcal", "/hcalrc", hcal_config_file_text);
	if (config_file != NULL)
	{
		read_config_file(config_file, &opt);
		fclose(config_file);
	}



	/************************************************************
	* parse command line
	************************************************************/
 	while ((c = getopt_long(argc, argv, short_options,
						long_options, &option_index)) != -1)

	{
		switch (c)
		{
		case 0: /* long options */
			switch (option_index)
			{
			case 0:	print_version (); exit (0); break;
			case 1:	print_help (); exit (0); break;
			case 2:	opt.no_reverse = 1; break;
			case 3: opt.html = 1; break;
			case 4: opt.parasha = 1; break;
			case 5: opt.shabbat = 1; opt.parasha = 1; break;
			case 6: opt.three_month = 1; break;
			case 7: opt.colorize = 1; break;
			case 8: opt.footnote = 1; break;
			case 9: opt.force_hebrew = 1; break;
			case 10:opt.force_israel = 1; break;
			case 11: error_detected = parse_coordinate(1, &lat, &opt_latitude);	break;
			case 12: error_detected = parse_coordinate(2, &lon, &opt_Longitude);	break;
			case 13: error_detected = parse_timezone(&tz);	break;
			case 14: opt.not_sunset_aware = 1;	break;
			case 15: opt.quiet_alerts = 1; break;
			case 16:
			case 17: opt.bidi = 1; opt.force_hebrew = 1; break;
			}
			break;
		case '3': opt.three_month = 1; break;
		case 'b': opt.bidi = 1; opt.force_hebrew = 1; break;
		case 'c': opt.colorize = 1; break;
		case 'd': opt.diaspora = 1; break;
		case 'f': opt.footnote = 1; break;
		case 'h': opt.html = 1; break;
		case 'H': opt.force_hebrew = 1; break;
		case 'I': opt.force_israel = 1; break;
		case 'i': opt.external_css = 1; break;
		case 'p': opt.parasha = 1; break;
		case 'q': opt.quiet_alerts = 1; break;
		case 'l': error_detected = parse_coordinate(1, &lat, &opt_latitude);	break;
		case 'L': error_detected = parse_coordinate(2, &lon, &opt_Longitude); break;
		case 's': opt.shabbat = 1; opt.parasha = 1; break;
		case 'z': parse_timezone(&tz);	break;
		case '?':
			if (strchr(short_options,optopt)!=NULL)
				error(0,0,"option %c missing parameter",optopt);
			// variable error_detected is defined in location.include.c
			error_detected = TRUE;
			break;
		default: print_usage (); exit (0); break;
		}
	}


	/**************************************************
	* sanity check - options compatability
	*************************************************/
	if	( (opt.three_month) &&
		  ((opt.parasha) || (opt.shabbat) || (opt.footnote) ) )
	{
		error(0,0,"%s", N_("options --parasha, --shabbat, --footnote are not supported in 'three-month' mode"));
		// variable error_detected is defined in location.include.c
		error_detected = TRUE;
	}

	/************************************************************
	* function validate_location is defined in the include file
	* ./location.include.c
	* It issues an exit(EXIT_CODE_BAD_PARMS) [==1]
	* if it discovers, um, bad parameters 
	************************************************************/
	validate_location(opt_latitude, opt_Longitude, &lat, &lon, &tz, opt.quiet_alerts, error_detected);
	opt.lat = lat;
	opt.lon = lon;
	opt.tz = tz;

	/************************************************************
	* determine what day to highlight (ie. what is today)
	************************************************************/
	if ((opt.no_reverse) || (opt.html))
	{
		// user doesn't want any highlighting
		opt.jd_today_g = 0;
		opt.jd_today_h = 0;
	}
	else
	{
		hdate_set_gdate (&h, 0, 0, 0);
		opt.jd_today_g = h.hd_jd;
		if	((!opt.not_sunset_aware) &&
			 (check_for_sunset(&h, lat, lon, tz)) )
					opt.jd_today_h = h.hd_jd + 1;
		else 		opt.jd_today_h = h.hd_jd;
	}

/**************************************************
* parse and vaildate date parameters
**************************************************/


	/**************************************************
	* no date parameter provided - use current mm yyyy
	* and no need to validate parameters
	**************************************************/
	if (argc == (optind))
	{
		if ((opt.not_sunset_aware) || (opt.no_reverse) || (opt.html))
			// call function only if not already called
			// for sunset awareness, above.
			hdate_set_gdate (&h, 0, 0, 0);	// today
		month = h.gd_mon;
		year = h.gd_year;
	}
	else
	{
		/********************************************************
		* additional parameters provided - get them and validate
		********************************************************/
		if (argc == (optind + 2))
		{
			month = atoi (argv[optind]);
			year = atoi (argv[optind + 1]);
		}
		else if (argc == (optind + 1))
		{
			month = 0;
			year = atoi (argv[optind]);
		}
		else
		{
			error(0,0,"%s: %s", N_("error"), N_("too many parameters received. expected  [[mm] [yyyy]"));
			exit (0);
		}

		/********************************************************
		* parameter validation - year
		********************************************************/
		if (!validate_hdate(CHECK_YEAR_PARM, 0, month, year))
		{
			print_parm_error(N_("year"));
			exit (0);
		}
		/********************************************************
		* parameter validation - month
		********************************************************/
		if ((month!=0) && (!validate_hdate(CHECK_MONTH_PARM, 0, month, year)))
		{
			print_parm_error(N_("month"));
			exit (0);
		}
	}


/************************************************************
* begin processing the user request
************************************************************/

	/************************************************************
	* diaspora awareness
	************************************************************/
	if (opt.force_israel) opt.diaspora = 0;
	else
	{
		tzset();
		// system timezone is denominated in seconds
		if ( (timezone/-3600) != 2) opt.diaspora = 1;
	}

	/************************************************************
	* print HTML header
	************************************************************/
	if (opt.html) print_header_html (opt.external_css);


	/************************************************************
	* print one year
	************************************************************/
	if (month == 0)
	{
		if (opt.three_month)
			for (month=2; month<13; month=month+3) print_month (month, year, opt);
		else for (month=1; month<13; month++)
		{
			print_month (month, year, opt);
 			if ((opt.footnote) && month<12) printf("\n");
		}
	}

	
	/************************************************************
	* print only this month 
	************************************************************/
	else print_month (month, year, opt);


	/************************************************************
	* print HTML footer
	************************************************************/
	if (opt.html) print_html_footer ();

	return 0;
}
