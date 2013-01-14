/** hdate.c            http://libhdate.sourceforge.net
 * Hebrew date/times information (part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate.c -o hdate
 *
 * build:
 * gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -o "%e" "%f"
 * 
 * Copyright:  2011-2013 (c) Baruch Baum, 2004-2010 (c) Yaacov Zamir
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
//gcc -Wall -c -g -I "../../src" "%f"
//gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -efence -o "%e" "%f"

// in geany, build: gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -o "%e" "%f" local_functions.c custom_days.c


#include <hdate.h>		/// For hebrew date  (gcc -I ../../src)
#include <support.h>	/// libhdate general macros, including for gettext
#include <stdlib.h>		/// For atoi, getenv, setenv
#include <stdio.h>		/// For printf, FILE, asprintf
#include <locale.h>		/// For setlocale
#include <getopt.h>		/// For getopt_long
#include <string.h>		/// For strchr, mempcpy, asprintf
#include <fnmatch.h>	/// For fnmatch
#include <time.h>		/// For time
#include <sys/stat.h>	/// for stat
#include <error.h>		/// For error
#include <errno.h>		/// For errno
#include "local_functions.h" /// hcal,hdate common_functions
#include "custom_days.h" /// hcal,hdate common_functions
#include "zdump3.h"      /// zdump, zdumpinfo


#define DATA_WAS_NOT_PRINTED 0
#define DATA_WAS_PRINTED 1

/// for opt.menu[MAX_MENU_ITEMS]
#define MAX_MENU_ITEMS 10


/// quiet levels
#define QUIET_ALERTS         1 /// suppress only alert messages
#define QUIET_GREGORIAN      2 /// suppress also gregorian date
#define QUIET_DESCRIPTIONS   3 /// suppress also time labels
#define QUIET_HEBREW         4 /// suppress also Hebrew date


char *debug_var;				/// system environment variable

static char * day_text   = N_("day");
static char * month_text = N_("month");
static char * year_text  = N_("year");

static char * sunrise_text = N_("sunrise");
static char * sunset_text  = N_("sunset");
static char * first_light_text = N_("first_light");
static char * talit_text = N_("talit");
static char * shema_text = N_("end_Shema_(GR\"A)");
static char * magen_avraham_text = N_("end_Shema_(M\"A)");
static char * amidah_text = N_("end_amidah");
static char * midday_text = N_("mid-day");
static char * mincha_gedola_text = N_("mincha_gedola");
static char * mincha_ketana_text = N_("mincha_ketana");
static char * plag_hamincha_text = N_("plag_hamincha");
static char * first_stars_text = N_("first_stars");
static char * three_stars_text = N_("three_stars");
static char * sun_hour_text = N_("sun_hour");
static char * candles_text = N_("candle-lighting");
static char * havdalah_text = N_("havdalah");
static char * daf_yomi_text = N_("daf_yomi");
static char * omer_text = N_("omer");
static char * parasha_text = N_("parasha");
static char * holiday_text = N_("holiday");
static char * custom_day_text = N_("today is also");
static char * custom_day_tabular_text = N_("custom_day");
static char * creation_year_text = N_("Creation Year");

static char *sof_achilat_chametz_ma_text  = N_("end_eating_chometz_(M\"A)");
static char *sof_achilat_chametz_gra_text  = N_("end_eating_chometz_(GR\"A)");
static char *sof_biur_chametz_ma_text  = N_("end_owning_chometz_(M\"A)");
static char *sof_biur_chametz_gra_text  = N_("end_owning_chometz_(GR\"A)");


typedef struct  {
				int hebrew;
				int bidi;
				int yom;
				int leShabbat;
				int leSeder;
				int tablular_output;
				int not_sunset_aware;
				int print_tomorrow;
				int quiet;
				int sun_hour;
				int first_light;
				int talit;
				int sunrise;
				int magen_avraham;
				int shema;
				int amidah;
				int midday;
				int mincha_gedola;
				int mincha_ketana;
				int plag_hamincha;
				int sunset;
				int first_stars;
				int three_stars;
				int candles;
				int havdalah;
				int times;
				int short_format;
				int only_if_holiday;
				int holidays;
				int omer;
				int only_if_parasha;
				int parasha;
				int julian;
				int diaspora;
				int iCal;
				int daf_yomi;
				int afikomen;
				int	end_eating_chometz_ma;
				int end_eating_chometz_gra;
				int end_owning_chometz_ma;
				int end_owning_chometz_gra;
				int la_omer;
				int custom_days_cnt;
				int* jdn_list_ptr;		/// for custom_days
				char* string_list_ptr;	/// for custom_days
				int data_first;
				int menu;
				char* menu_item[MAX_MENU_ITEMS];
				int tzif_entries;
				int tzif_index;
				void* tzif_data;
				int tz_offset;
				double lat;
				double lon;
				time_t today_time;
				} option_list;


static const char* hdate_config_file_text = N_("\
# configuration file for hdate - Hebrew date information program\n\
# part of package libhdate\n\
#\n# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hdate; hdate will automatically\n\
# regenerate the default content.\n#\n\
# Your system administrator can set system-wide defaults for hcal by\n\
# modifying file <not yet implemented>.\n\
# You may override all defaults by changing the contents of this file.\n\
#\n\
# Version information\n\
# This may be used by updates to hcal to determine how to parse the file\n\
# and whether additional information and options should be appended to\n\
# the end of this file.\n\
VERSION=2.00\n\
#\n\
# Location awareness\n\
# hdate wants to accurately highlight the current Hebrew day, including\n\
# during the hours between sunset and secular midnight. If you don't\n\
# provide it with latitude, longitude, and time zone information, hdate\n\
# will try to guess the information, based upon your system timezone,\n\
# and its (limited, and maybe biased) of the dominant Jewish community\n\
# in that timezone. When hdate is forced to guess, it alerts the user\n\
# with a message that includes the guessed location.\n\
# hdate's guesses will also affect its default behaviour for ouput of\n\
# Shabbat times, parshiot, and choice of Israel/diaspora hoidays.\n\
#SUNSET_AWARE=TRUE\n\
# LATITUDE and LONGITUDE may be in decimal format or in the form\n\
# degrees[:minutes[:seconds]] with the characters :'\" as possible\n\
# delimiters. Use negative values to indicate South and West, or\n\
# use the abbreviated compass directions N, S, E, W.\n\
#LATITUDE=\n\
#LONGITUDE=\n\
# TIMEZONE may may be in decimal format or in the form degrees[:minutes]\n\
# with the characters :'\" as possible delimiters.\n\
#TIMEZONE=\n\n\
# Output in hebrew characters\n\
# hdate defaults to output all information in your default language, so\n\
# if your default language is Hebrew, you're all set. Otherwise, you can\n\
# set FORCE_HEBREW to true to output Hebrew information in Hebrew, while\n\
# still outputting gregorian information in your default language. To\n\
# output ALL information in Hebrew, run something like this:\n\
#    LC_TEMP=LC_ALL; LC_ALL=\"he_IL.UTF-8\"; hdate; LC_ALL=$LC_TEMP\n\
# If setting FORCE_HEBREW to true results in 'garbage' or non-Hebrew\n\
# output, you need to install a terminal font that includes the Hebrew\n\
# character set (hint: unicode).\n\
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
#OUTPUT_BIDI=FALSE\n\n\
# The Hebrew language output of Hebrew information can also be 'styled'\n\
# in the following ways:\n\
# option YOM ~> yom shishi, aleph tishrei ...\n\
# option LESHABBAT ~> yom sheni leshabbat miketz, kof kislev ...\n\
# option LESEDER ~> yom sheni leseder miketz, kof kislev ...\n\
#YOM=FALSE\n\
#LESHABBAT=FALSE\n\
#LESEDER=FALSE\n\
\n\
#SUN_RISE_SET=FALSE\n\
#TIMES_OF_DAY=FALSE\n\
#SHORT_FORMAT=FALSE\n\
#SEFIRAT_HAOMER=FALSE\n\
#DIASPORA=FALSE\n\
\n\
\n\
# Shabbat related\n\
# Setting SHABBAT_INFO true will output parshiot and Shabbat times.\n\
# The command line options for these features are -r (--parasha), and\n\
# -c. The CANDLE_LIGHTING field can accept a value of 18 - 90 (minutes\n\
# before sunset). The HAVDALAH field can accept a value of 20 - 90\n\
# (minutes after sunset).\n\
#PARASHA_NAMES=FALSE\n\
#ONLY_IF_PARASHA_IS_READ=FALSE\n\
#SHABBAT_INFO=FALSE\n\n\
#CANDLE_LIGHTING=FALSE\n\
#HAVDALAH=FALSE\n\n\
# Holiday related\n\
#HOLIDAYS=FALSE\n\
#ONLY_IF_HOLIDAY=FALSE\n\n\
# Tabular output\n\
# This option has hdate output the information you request in a single\n\
# comma-delimited line per day, suitable for piping or import to\n\
# spreadsheet formatting applications, etc. To belabor the obvious,\n\
# try running -   ./hdate 12 2011 -Rt --table |column -s, -t \n\
# The command line option for this feature is, um, --table\n\
#TABULAR=FALSE\n\n\
# iCal format\n\
# hdate can output its information in iCal-compatible format\n\
# ICAL=FALSE\n\
# Suppress alerts and warnings\n\
# hdate alerts the user via STDERR when it guesses the user's location.\n\
#QUIET_ALERTS=FALSE\n\n\
# Julian day number\n\
# The Julian day number is a .... See ... for more details.\n\
# setting the option JULIAN_DAY will have hdate output that number in\n\
# the format JDN-nnnnnnn at the beginning of its first line of output.\n\
#JULIAN_DAY=FALSE\n\n\
# User-defined menus\n\
# You may specify here command-line strings to optionally be parsed\n\
# by hcal at execution time. To do so, use the command line option -m\n\
# (--menu). hcal will process first the settings of this config file,\n\
# then the other settings of your command line, and then will prompt\n\
# you for which menu item you would like to select. hcal will process\n\
# your menu selection as if it were a new command line, further modifying\n\
# all the prior settings.\n\
# Only the first ten \"MENU=\" entries will be read. Each line will be\n\
# truncated at one hundred characters\n\
#MENU= -bd -l -23.55 -L -46.61 -z -3 # parents in Sao Paolo\n\
#MENU= -b -l 32 -L 34 -z 2           # son in bnei brak\n\
#MENU= -bd -l 43.71 -L -79.43 -z -5  # me in Toronto\n\
#MENU= -bd -l 22.26 -L 114.15 -z 8   # supplier in Hong Kong\n\
");


// TODO - move daf yomi and limud yomi into libhdate api
/// Daf Yomi cycle began 02 March 2005 = Julian day 2453432
#define DAF_YOMI_START 2453432
#define DAF_YOMI_LEN      2711

typedef struct {
	int start_day;
	int num_of_days;
	char* e_masechet;
	char* h_masechet; } limud_unit;

static const limud_unit daf_yomi[] = {
{   0,  63, N_("Berachot"), "ברכות" },
{  63, 156, N_("Shabbat"), "שבת" },
{ 219, 104, N_("Eiruvin"), "עירובין" },
{ 323, 120, N_("Pesachim"), "פסחים" },
{ 443,  21, N_("Shekalim"), "שקלים" },
{ 464,  87, N_("Yoma"), "יומא" },
{ 551,  55, N_("Sukkah"), "סוכה" },
{ 606,  39, N_("Beitzah"), "ביצה" },
{ 645,  34, N_("Rosh_HaShannah"), "ראש השנה" },
{ 679,  30, N_("Taanit"), "תענית" },
{ 709,  31, N_("Megillah"), "מגילה" },
{ 740,  28, N_("Moed_Katan"), "מועד קטן" },
{ 768,  26, N_("Chagigah"), "חגיגה" },
{ 794, 121, N_("Yevamot"), "יבמות" },
{ 915, 111, N_("Ketubot"), "כתובות" },
{1026,  90, N_("Nedarim"), "נדרים" },
{1116,  65, N_("Nazir"), "נזיר" },
{1181,  48, N_("Sotah"), "סוטה" },
{1229,  89, N_("Gittin"), "גיטין" },
{1318,  81, N_("Kiddushin"), "קידושין" },
{1399, 118, N_("Bava_Kamma"), "בבא קמא" },
{1517, 118, N_("Bava_Metzia"), "בבא מציעא" },
{1635, 175, N_("Bava_Batra"), "בבא בתרא" },
{1810, 112, N_("Sanhedrin"), "סנהדרין" },
{1922,  23, N_("Makkot"), "מכות" },
{1945,  48, N_("Shevuot"), "שבועות" },
{1993,  75, N_("Avodah_Zara"), "עבודה זרה" },
{2068,  13, N_("Horayot"), "הוריות" },
{2081, 119, N_("Zevachim"), "זבחים" },
{2200, 109, N_("Menachot"), "מנחות" },
{2309, 141, N_("Chullin"), "חולין" },
{2450,  60, N_("Bechorot"), "בכורות" },
{2510,  33, N_("Erchin"), "ערכין" },
{2543,  33, N_("Temurah"), "תמורה" },
{2576,  27, N_("Keritut"), "כריתות" },
{2603,  20, N_("Meilah"), "מעילה" },
{2623,   1, N_("Meilah-Kinnim"), "מעילה - קינים" },
{2625,   2, N_("Kinnim"), "קינים" },
{2626,   1, N_("Kinnim-Tamid"), "קינים - תמיד" },
{2627,   8, N_("Tamid"), "תמיד" },
{2635,   4, N_("Middot"), "מדות" },
{2639,  72, N_("Niddah"), "נדה" },
{2711,   0,    "",        ""} };


static const char* afikomen[9] = {
	N_("There are no easter eggs in this program. Go away."),
	N_("There is no Chanukah gelt in this program. Leave me alone."),
	N_("Nope, no Hamentashen either. Scram kid, your parents are calling you"),
	N_("Nada. No cheesecake, no apples with honey, no pomegranate seeds. Happy?"),
	N_("Afikomen? Afikomen! Ehrr... Huh?"),
	N_("(grumble...) You win... (moo)"),
	N_("Okay! enough already! MOO! MOO! MoooooH!\n\nSatisfied now?"),
	N_("Etymology: Pesach\n   Derived from the ancient Egyptian word \"Feh Tzach\" meaning \"purchase a new toothbrush\" --based upon a hieroglyphic steele dating from the 23rd dynasty, depicting a procession of slaves engaging in various forms of oral hygiene."),
	N_("You have reached Wit's End.")};



/************************************************************
* print version information
************************************************************/
int print_version ()
{
	printf ("hdate (libhdate) 1.6\n\
Copyright (C) 2011-2012 Boruch Baum, 2004-2010 Yaacov Zamir\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n");
	return 0;
}

/************************************************************
* print usage information
************************************************************/
void print_usage_hdate ()
{
	printf ("%s\n",
N_("Usage: hdate [options] [coordinates timezone] [[[day] month] year]\n\
       hdate [options] [coordinates timezone] [julian_day]\n\n\
       coordinates: -l [NS]yy[.yyy] -L [EW]xx[.xxx]\n\
                    -l [NS]yy[:mm[:ss]] -L [EW]xx[:mm[:ss]]\n\
       timezone:    -z nn[( .nn | :mm )]"));
}

/************************************************************
* print "try --help" message
************************************************************/
void print_try_help_hdate ()
{
	printf ("%s\n",
	N_("Try \'hdate --help\' for more information"));
}


/************************************************************
* print help information
************************************************************/
void print_help ()
{
	print_usage_hdate();

	printf ("%s\n", N_("hdate - display Hebrew date information\nOPTIONS:\n\
   -b --bidi          prints hebrew in reverse (visual)\n\
      --visual\n\
   -d --diaspora      force diaspora readings and holidays.\n\
      --israel        force Eretz Yisroel readings an holidays\n\
      --daf-yomi\n\
   -h --holidays      print holiday information.\n\
   -H                 print only if day is a holiday.\n\
   -i --ical          use iCal formated output.\n\
   -j --julian        print Julian day number.\n\
   -m --menu          prompt user-defined menu from config file\n\
   -o --omer          print Sefirat Ha-Omer, number of days only.\n\
                      -oo  \"today is n days in the omer\"\n\
                      -ooo the full text, with weeks and remainder days\n\
      --ba-omer       full omer text in Hebrew\n\
      --la-omer       full omer text in Hebrew\n\
   -q --quiet         suppresses optional output (four levels):\n\
      --quiet-alerts       -q    suppresses only warning messages\n\
      --quiet-gregorian    -qq   also suppresses gregorian date\n\
      --quiet-descritpions -qqq  also suppresses labels and descripions\n\
      --quiet_hebrew       -qqqq also suppresses Hebrew date\n\
   -r --parasha       print weekly reading if day is Shabbat.\n\
   -R                 print only if day is a Shabbat on which there\n\
                      is a weekly reading (ie. not a special holiday\n\
                      reading), and print that regular weekly reading.\n\
   -s --shabbat       print Shabbat start/end times.\n\
      --shabbat-times\n\
      --candles       modify default minhag of 20 minutes. (17<n<91)\n\
      --havdalah      modify default minhag of 3 stars. (19<n<91 minutes)\n\
      --sun           print sunrise/sunset times.\n\n\
   -S --short-format  print using short format.\n\
   -t --times         print day times (three preset verbosity levels):\n\
      --times-of-day  -t    first light, talit, sunrise, midday,\n\
      --day-times           sunset, first stars, three stars, sun hour.\n\
                      -tt   adds sof zman kriyat Shema, sof zman amidah,\n\
                            mincha gedolah, mincha ketana, plag hamincha\n\
                      -ttt  adds sof zman kriyat Shema per Magen Avraham\n\
                      instead of using the presets, customize, with:\n\
                      --first-light  --midday         --shekia\n\
                      --alot         --noon           --tzeit-hakochavim\n\
                      --talit        --chatzot        --first-stars\n\
                      --netz         --mincha-gedola  --three-stars\n\
                      --shema        --mincha-ketana  --magen-avraham\n\
                      --amidah       --plag-hamincha  --sun-hour\n\
                      \n\
                      --sunrise      --sunset         --candle-lighting\n\
                      --end-eating-chometz-ma   --end-eating-chometz-gra\n\
                      --end-owning-chometz-ma   --end-owning-chometz-gra\n\
	                  \n\
   -T --table         tabular output, comman delimited, and most suitable\n\
      --tabular       for piping or spreadsheets\n\n\
   -z --timezone nn   timezone, +/-UTC\n\
   -l --latitude yy   latitude yy degrees. Negative values are South\n\
   -L --longitude xx  longitude xx degrees. Negative values are West\n\n\
   --hebrew           forces Hebrew to print in Hebrew characters\n\
   --yom              force Hebrew prefix to Hebrew day of week\n\
   --leshabbat        insert parasha between day of week and day\n\
   --leseder          insert parasha between day of week and day\n\
   --not-sunset-aware don't display next day if after sunset\n\
   --data-first       display data, followed by it's description\n\
   --labels-first     display data descriptions before the data itself\n\
All options can be made default in the config file, or menu-ized for\n\
easy selection.\n\
Report bugs to: <http://sourceforge.net/tracker/?group_id=63109&atid=502872>\n\
libhdate home page: <http://libhdate.sourceforge.net>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\n"));
}


/************************************************************
* begin - error message functions
************************************************************/
void print_alert_sunset ()
{
	error(0,0,"%s", N_("ALERT: The information displayed is for today's Hebrew date.\n \
             Because it is now after sunset, that means the data is\n \
             for the Gregorian day beginning at midnight."));
}
/************************************************************
* end - error message functions
************************************************************/



/************************************************************
* generic print astronomical time
************************************************************/
int print_astronomical_time( const int quiet_level,
							char *description,
							const int timeval, const int tz,
							const int data_first)
{
	char  delim = '\0';
	char* descr = "";

	if (quiet_level < QUIET_DESCRIPTIONS)
	{
		delim =':';
		descr = description;
	}

	if (timeval < 0)
	{
		if (!data_first) printf("%s%c --:--\n", descr,delim);
		else printf("--:-- %s\n", descr);
		return DATA_WAS_NOT_PRINTED;
	}

	if (!data_first) printf("%s%c %02d:%02d\n", descr,delim,
							 (timeval+tz) / 60, (timeval+tz) % 60 );
	else printf("%02d:%02d %s\n", (timeval+tz) / 60, (timeval+tz) % 60, descr);
	return DATA_WAS_PRINTED;
}

/************************************************************
* generic print astronomical time for tablular output
************************************************************/
void print_astronomical_time_tabular( const int timeval, const int tz)
{
	if (timeval < 0) printf(",--:--");
	else printf(",%02d:%02d",
			(timeval+tz) / 60, (timeval+tz) % 60 );
	return;
}


/************************************************************
* get daf yomi information
************************************************************/
int daf_yomi_info( const int julian_day, int* daf, char** masechet, int force_hebrew)
{
	int index, i;

	if (julian_day < 2453432) return FALSE;

	index = (julian_day % DAF_YOMI_START) % DAF_YOMI_LEN;

	for ( i=0; daf_yomi[i+1].start_day <= index ; i++) ;

	/// These masechaot ketanot don't all start at 2
	if ( (index > 2622) && (index < 2639) ) *daf = index - 2601;
	else *daf = index - daf_yomi[i].start_day + 2;

	if (force_hebrew) *masechet = daf_yomi[i].h_masechet;
	else *masechet = daf_yomi[i].e_masechet;
	
	return TRUE;
}

/************************************************************
* print daf yomi information
************************************************************/
int print_daf_yomi( const int julian_day,
					const int force_hebrew, const int force_bidi,
					const int tabular_output, const int data_first )
{
	int daf;
	char* masechet;

	char *daf_str;
	char *bidi_buffer, *bidi_buffer2;
	int   bidi_buffer_len;

	if (!daf_yomi_info( julian_day, &daf, &masechet, force_hebrew))
		return DATA_WAS_NOT_PRINTED;

	if (!force_hebrew)
	{
		if (!tabular_output)
		{
			if (!data_first) printf("%s: %s %d\n", daf_yomi_text, masechet, daf);
			else printf("%s %d %s\n", masechet, daf, daf_yomi_text);
		}
		else  printf(",%s %d", masechet, daf);
	}
	else
	{
		daf_str = hdate_string(HDATE_STRING_INT, daf, HDATE_STRING_LONG, force_hebrew);
		if (!force_bidi)
		{
			if (!tabular_output)
			{
				if (!data_first) printf("%s: %s %s\n", daf_yomi_text, masechet, daf_str);
				else printf("%s %s %s\n", masechet, daf_str, daf_yomi_text);
			}
			else printf(",%s %d", masechet, daf);
		}
		else
		{
			bidi_buffer_len = asprintf(&bidi_buffer, "%s %s", masechet, daf_str);
			bidi_buffer2 = malloc(bidi_buffer_len+1);
			mempcpy(bidi_buffer2, bidi_buffer, bidi_buffer_len);
			bidi_buffer2[bidi_buffer_len]='\0';
			revstr(bidi_buffer2, bidi_buffer_len);
			if (!tabular_output)
			{
				if (!data_first) printf("%s: %s\n",daf_yomi_text, bidi_buffer2);
				else printf("%s %s\n", bidi_buffer2 ,daf_yomi_text);
			}
			else printf(",%s",bidi_buffer2);
			if (bidi_buffer  != NULL)  free(bidi_buffer);
			if (bidi_buffer2 != NULL)  free(bidi_buffer2);
		}
		if (daf_str  != NULL)  free(daf_str);
	}
	return DATA_WAS_PRINTED;
}


/************************************************************
* print iCal header
************************************************************/
int print_ical_header ()
{
	/// Print start of iCal format
	printf ("BEGIN:VCALENDAR\n");
	printf ("VERSION:2.0\n");
	printf ("CALSCALE:GREGORIAN\n");
	printf ("METHOD:PUBLISH\n");

	return 0;
}




/************************************************************
* print iCal footer
************************************************************/
int print_ical_footer ()
{
	/// Print end of iCal format
	printf ("END:VCALENDAR\n");

	return 0;
}





/************************************************************
* find Shabbat
*	this function is to become obsolete - I want a feature
*	for this standard in libhdate
************************************************************/
int find_shabbat (hdate_struct * h, int opt_d)
{
	hdate_struct coming_Shabbat;

	hdate_set_jd (&coming_Shabbat, h->hd_jd+(7-h->hd_dw));
	///	this return value is the reading number, used to print parshiot
	return hdate_get_parasha (&coming_Shabbat, opt_d);
	}



/************************************************************
* print one date - both Gregorian and Hebrew
************************************************************/
int print_date (hdate_struct* h, hdate_struct* tomorrow, const option_list* opt)
{
	char *motzash		= "";	/// prefix for Saturday night
	char *eve_before    = "";   /// prefix if after sunset
	char *for_day_of_g  = "";	/// Hebrew prefix for day of week
	char *apostrophe	= "";	/// Hebrew suffix for day of week
	char *bet_g 		= "";	/// Hebrew prefix for Gregorian month
	char *bet_h         = "";	/// Hebrew prefix for Hebrew month
	char *psik_mark     = "";	/// --force-yom
	char *for_day_of_h  = "";	/// --force-yom
	char *for_week_of   = "";	/// --force-leshabbat --force-leseder
	int  is_parasha_read;
	int h_dow_form		= HDATE_STRING_SHORT;

	char *hday_int_str, *hyear_int_str;
	hday_int_str = hdate_string(HDATE_STRING_INT, h->hd_day, HDATE_STRING_LONG, opt->hebrew);
	hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);

/*
#ifdef ENABLE_NLS
	locale = setlocale (LC_MESSAGES, NULL);
	language = getenv ("LANGUAGE");
#else
	locale = NULL;
	language = NULL;
#endif
*/

	/************************************************************
	* preliminary - if it's after sunset, it's tomorrow already
	* but we will only acknowledge this if printing in the long
	* format (which is the default)
	************************************************************/
	if (opt->print_tomorrow)
	{
		if (opt->hebrew)
		{
			if (h->hd_dw==7) motzash = "מוצ\"ש, ";
			if (h->hd_dw==6) eve_before = "ליל";
			else  eve_before = "אור ל";
			if (!opt->yom) for_day_of_h="-";
		}
		else eve_before = N_("eve of ");
	}


	/************************************************************
	* preliminary - prepare Hebrew prefixes if forcing any Hebrew
	************************************************************/
	if (opt->hebrew)
	{
		bet_h="ב";
		/// preliminary - prepare Yom prefix
		if (opt->yom)
		{
			if ( !((opt->print_tomorrow) && (h->hd_dw==6))) for_day_of_h="יום";
			if  ( ((h->hd_dw!=7) || ((h->hd_dw==7) && (opt->print_tomorrow))) &&
				( !((opt->print_tomorrow) && (h->hd_dw==6))) ) apostrophe = "'";
			if ((h->hd_dw==6) && (opt->print_tomorrow)) h_dow_form = HDATE_STRING_LONG;
			psik_mark=",";

			if (opt->leShabbat)
				{
				if (h->hd_dw==7)		for_week_of="פרשת";
				else					for_week_of="לשבת";
				}
			else if (opt->leSeder)	for_week_of="לסדר";
		}
	}

	/************************************************************
	*	preliminary - prepare the Hebrew prefixes
	*	for the Gregorian month and day of week
	************************************************************/
	if (hdate_is_hebrew_locale())
		{ bet_g="ב"; for_day_of_g="יום ";}



	/************************************************************
	* Three major print formats: iCal, short, long
	************************************************************/


	/************************************************************
	* iCal format
	************************************************************/
	if (opt->iCal)
	{
		printf ("%s%s %s%s ", for_day_of_g,	hday_int_str, bet_h,
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));

		printf ("%s", hyear_int_str);
	}


	/************************************************************
	* short format
	************************************************************/
	else if (opt->short_format)
	{
		printf ("%d.%d.%d  ", h->gd_day, h->gd_mon, h->gd_year);


		if (!hdate_is_hebrew_locale() && (!opt->hebrew))
		{
			printf ("%d", h->hd_day);
		}
		else
		{
			printf ("%s", hday_int_str);
		}

		printf (" %s %s\n",
			hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
			hyear_int_str);
	}


	/************************************************************
	* long (normal) format
	************************************************************/
	else
	{
		/************************************************************
		* Gregorian date - the easy part
		************************************************************/
		if (opt->quiet < QUIET_GREGORIAN)
			printf ("%s%s, %d %s%s %d, ",
				for_day_of_g,
				hdate_string( HDATE_STRING_DOW, h->hd_dw, opt->short_format, HDATE_STRING_LOCAL),
				h->gd_day,
				bet_g,
				hdate_string( HDATE_STRING_GMONTH, h->gd_mon, opt->short_format, HDATE_STRING_LOCAL),
				h->gd_year);


		/************************************************************
		* Start of the Hebrew date
		************************************************************/
		if (opt->print_tomorrow) *h = *tomorrow;


		/************************************************************
		* All these definitions were backports to enable bidi
		************************************************************/
		char *hebrew_buffer, *hebrew_buffer1, *hebrew_buffer2, *hebrew_buffer3, *hebrew_buffer4, *hebrew_buffer_next;
		size_t hebrew_buffer_len = 0;
		size_t hebrew_buffer1_len = 0;
		size_t hebrew_buffer2_len = 0;
		size_t hebrew_buffer3_len = 0;
		size_t hebrew_buffer4_len = 0;


		/************************************************************
		* prepare buffers with  Hebrew day of week, including
		* possibly Shabbat name
		************************************************************/
		if (opt->hebrew)
		{
			if (opt->yom)
				{
				hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s%s%s %s%s", motzash, eve_before, for_day_of_h,
					hdate_string( HDATE_STRING_DOW, h->hd_dw, h_dow_form, opt->hebrew), apostrophe);
				if ((opt->leShabbat) || (opt->leSeder))
				{
					is_parasha_read = find_shabbat(h, opt->diaspora);
					if (is_parasha_read)
							hebrew_buffer2_len = asprintf(&hebrew_buffer2, " %s %s", for_week_of,
							hdate_string( HDATE_STRING_PARASHA, is_parasha_read, opt->short_format, opt->hebrew)
							);
				}
				hebrew_buffer3_len = asprintf (&hebrew_buffer3, "%s ", psik_mark);
			}
		}
		else hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s", eve_before);


		hday_int_str  = hdate_string(HDATE_STRING_INT, h->hd_day,  HDATE_STRING_LONG, opt->hebrew);

		/// year 0 is the six-day long 'year' of creation
		if (h->hd_year == 0) hyear_int_str = creation_year_text;
		else hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);
		/************************************************************
		* prepare buffers with Hebrew dd mmmm yyyy
		************************************************************/
		hebrew_buffer4_len =
			asprintf (&hebrew_buffer4, "%s %s%s %s",
				hday_int_str,
				bet_h,
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
				hyear_int_str);

		/************************************************************
		* prepare another GrUB - grand unified buffer
		************************************************************/
		hebrew_buffer_len = hebrew_buffer1_len + hebrew_buffer2_len + hebrew_buffer3_len + hebrew_buffer4_len;
		hebrew_buffer = malloc(hebrew_buffer_len+1);

		hebrew_buffer_next = mempcpy(hebrew_buffer, hebrew_buffer1, hebrew_buffer1_len);
		if (hebrew_buffer2_len > 0) hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer2, hebrew_buffer2_len);
		if (hebrew_buffer3_len > 0) hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer3, hebrew_buffer3_len);
		hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer4, hebrew_buffer4_len);
		hebrew_buffer[hebrew_buffer_len]='\0';
		/************************************************************
		* bidi support (what all the buffering was for)
		************************************************************/
		if (opt->bidi) revstr(hebrew_buffer, hebrew_buffer_len);

		/************************************************************
		* Finally. print the information
		************************************************************/
		printf ("%s\n", hebrew_buffer);
		
		/************************************************************
		* CLEANUP - free allocated memory
		************************************************************/
		if (hebrew_buffer1_len > 0) free(hebrew_buffer1);
		if (hebrew_buffer2_len > 0) free(hebrew_buffer2);
		if (hebrew_buffer3_len > 0) free(hebrew_buffer3);
		free(hebrew_buffer4);
		free(hebrew_buffer);
	}


	/************************************************************
	* CLEANUP - free allocated memory
	************************************************************/
	if (hday_int_str  != NULL)  free(hday_int_str);
	if ((hyear_int_str != NULL) && (h->hd_year != 0))  free(hyear_int_str);

	return 0;
}


/************************************************************
* option 't' - day times
************************************************************/
int print_times ( hdate_struct * h, option_list* opt, const int holiday)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;
	int data_printed = 0;
	int ma_sun_hour;
	int place_holder;

// CONTINUE HERE
	/** Originally, we got times using this next single system call; however this function
	 *  returns values rounded (formerly truncated) to the minute, and I want better because
	 *  I want accuracy in sha'a zmanit in order to derive other times using it. Refer to the
	 *  source code of hdate_get_utc_sun_time_full for comparison.
	 * 
	hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								 &sun_hour, &first_light, &talit, &sunrise,
								 &midday, &sunset, &first_stars, &three_stars);
	*/
	hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 90.833, &sunrise, &sunset);
	hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 106.01, &first_light, &place_holder);
	hdate_get_utc_sun_time_deg (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 101.0, &talit, &place_holder);
	hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 96.0, &place_holder, &first_stars);
	hdate_get_utc_sun_time_deg (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 98.5, &place_holder, &three_stars);

	/// sha'a zmanit according to the GR"A, in seconds
	sun_hour = (sunset - sunrise) / 12;
	/// sha'a zmanit according to the Magen Avraham, in seconds
	ma_sun_hour = (first_stars - first_light)/12;

	/// mid-day, in minutes
	midday = (sunset + sunrise) / (2 * 60);

	/// now we can convert values to minutes
	sunrise = sunrise / 60;
	sunset  = sunset  / 60;	
	first_stars = first_stars / 60;
	first_light = first_light / 60;

//
//
// continue here - check anfd possibly modify tz for dst
//
//
// Change all occurences of tz to opt.tz_offset;
//
	if ((opt->tzif_entries) && (opt->tzif_index < (opt->tzif_entries -1)) )
	{
		zdumpinfo *zdinfo;
		// not sure if these next two lines can be combined
		zdinfo = opt->tzif_data;
		zdinfo = zdinfo + opt->tzif_index + 1;
		if (opt->today_time >= zdinfo->start)
		{
			opt->tzif_index = opt->tzif_index +1 ;;
			opt->tz_offset = zdinfo->utc_offset;
		}
	}

	if (opt->first_light) data_printed = data_printed | print_astronomical_time( opt->quiet, first_light_text, first_light, opt->tz_offset, opt->data_first);
	if (opt->talit)       data_printed = data_printed | print_astronomical_time( opt->quiet, talit_text, talit, opt->tz_offset, opt->data_first);
	if (opt->sunrise)     data_printed = data_printed | print_astronomical_time( opt->quiet, sunrise_text, sunrise, opt->tz_offset, opt->data_first);

	// FIXME - these times will be moved into the libhdate api
	/// sof zman kriyat Shema (verify that the Magen Avraham calculation is correct!)
	if (opt->magen_avraham)
						 data_printed = data_printed | print_astronomical_time( opt->quiet, magen_avraham_text, first_light + (3 * ma_sun_hour)/60, opt->tz_offset, opt->data_first);
	if (opt->shema)       data_printed = data_printed | print_astronomical_time( opt->quiet, shema_text, sunrise + (3 * sun_hour)/60, opt->tz_offset, opt->data_first);
	/// sof zman tefilah
	if (opt->amidah)      data_printed = data_printed | print_astronomical_time( opt->quiet, amidah_text, sunrise + (4 * sun_hour)/60, opt->tz_offset, opt->data_first);


	// TODO - if an erev pesach time was explicitly requested (ie. NOT by -t option),
	//		for a day that is not erev pesach, give the requested information, for the
	//		coming erev pesach, along with a (detailed) message	explaining that the
	//		requested day was not erev pesach. Also give the gregorian date of that
	//		erev pesach if the user wanted gregorian output.
	//		- however, if the explicit erev pesach time request was made with a command
	//		line specifying a month range that includes erev pesach, or a year range,
	//		then just use the code as is now (ie. ignore on days that are not erev pesach
	// TODO - erev pesach times for tabular output
	#define EREV_PESACH 38	// This is the 'holiday code' used in hdate_holyday.c and hdate_strings.c
	if (holiday == EREV_PESACH)
	{
		//	print_astronomical_time( opt->quiet, "Magen Avraham sun hour", ma_sun_hour, 0);
		if (opt->end_eating_chometz_ma)
			data_printed = data_printed | print_astronomical_time( opt->quiet, sof_achilat_chametz_ma_text, first_light + (4 * ma_sun_hour)/60, opt->tz_offset, opt->data_first);
		if (opt->end_eating_chometz_gra)
			data_printed = data_printed | print_astronomical_time( opt->quiet, sof_achilat_chametz_gra_text, sunrise + (4 * sun_hour)/60, opt->tz_offset, opt->data_first);
		if (opt->end_owning_chometz_ma)
			data_printed = data_printed | print_astronomical_time( opt->quiet, sof_biur_chametz_ma_text, first_light + (5 * ma_sun_hour)/60, opt->tz_offset, opt->data_first);
		if (opt->end_owning_chometz_gra)
			data_printed = data_printed | print_astronomical_time( opt->quiet, sof_biur_chametz_gra_text, sunrise + (5 * sun_hour)/60, opt->tz_offset, opt->data_first);
	}


	if (opt->midday)      data_printed = data_printed | print_astronomical_time( opt->quiet, midday_text, midday, opt->tz_offset, opt->data_first);


	// FIXME - these times will be moved into the libhdate api
	/// mincha gedola
	// TODO - There are two other shitot for this:
	//     shaot zmaniot, and shaot zmaniot lechumra
	if (opt->mincha_gedola) data_printed = data_printed | print_astronomical_time( opt->quiet, mincha_gedola_text, midday + 30, opt->tz_offset, opt->data_first);

	if (opt->mincha_ketana) data_printed = data_printed | print_astronomical_time( opt->quiet, mincha_ketana_text, sunrise + (9.5 * sun_hour)/60, opt->tz_offset, opt->data_first);
	if (opt->plag_hamincha) data_printed = data_printed | print_astronomical_time( opt->quiet, plag_hamincha_text, sunrise + (10.75 * sun_hour)/60, opt->tz_offset, opt->data_first);

	if (opt->sunset)       data_printed = data_printed | print_astronomical_time( opt->quiet, sunset_text, sunset, opt->tz_offset, opt->data_first);
	if (opt->first_stars)  data_printed = data_printed | print_astronomical_time( opt->quiet, first_stars_text, first_stars, opt->tz_offset, opt->data_first);
	if (opt->three_stars)  data_printed = data_printed | print_astronomical_time( opt->quiet, three_stars_text, three_stars, opt->tz_offset, opt->data_first);

	/// if (opt->sun_hour)     data_printed = data_printed | print_astronomical_time( opt->quiet, sun_hour_text, sun_hour/60, 0, opt->data_first);
	if (opt->sun_hour)
	{
		data_printed = TRUE;
		if (opt->quiet >= QUIET_DESCRIPTIONS)
			 printf("%02d:%02d:%02d\n", sun_hour/3600, (sun_hour%3600)/60, sun_hour%60 );
		else
		{
			if (!opt->data_first) printf("%s: %02d:%02d:%02d\n", sun_hour_text,
										sun_hour/3600, (sun_hour%3600)/60, sun_hour%60 );
			else printf("%02d:%02d:%02d %s\n", sun_hour/3600, (sun_hour%3600)/60, sun_hour%60,
										sun_hour_text);
		}
	}
	return data_printed;
}


/************************************************************
* option 'o' - sefirat ha'omer
************************************************************/
/// parameter hebrew_form is currently unnecessary, but retained
/// for future feature to force a fully hebrew string
int print_omer (hdate_struct * h, const option_list* opt)
{
	int omer_day;
	char* empty_text = "";
	char* days_text = N_("days");
	char* in_the_omer_text = N_("in the omer");
	char* today_is_day_text = N_("today is day");

	omer_day = hdate_get_omer_day(h);
	if (omer_day == 0) 	return DATA_WAS_NOT_PRINTED;

	if (opt->omer == 1) /// short format; just the numeric value
	{
		if (opt->quiet >= QUIET_DESCRIPTIONS) printf ("%d\n", omer_day);
		else
		{
			if (opt->data_first) printf ("   %d %s\n", omer_day, omer_text);
			else printf ("%s: %d\n", omer_text, omer_day);
		}
	}
	else if (opt->omer == 2) /// short text format
	{
		if (opt->data_first)
		{
			if (omer_day == 1) days_text = day_text;
			printf ("   %2d %s %s\n", omer_day, day_text, in_the_omer_text);
		}
		else printf ("%s %d %s \n", today_is_day_text, omer_day, in_the_omer_text);
	}
	else /// long text format
	{

		if (opt->hebrew)
		{
			/// Let's try to construct all the options
			/// for this Hebrew sentence as atoms
			/// for a single call to asprintf ....
			char* hayom = "היום ";
			char* yom = "";
			char* days = "";
			char* that_are = "שהם ";
			char* weeks = "שבועות";
			char* vav = " ו";
			char* days2 = "ימים, ";
			char* b_l_omer = "בעומר";
			
			char* n1 = NULL;
			char* n2 = NULL;
			char* n3 = NULL;
			
			int n2_needs_free = FALSE;
			int n3_needs_free = FALSE;
			
			char* bidi_buffer;
			int bidi_buffer_len;

			if (opt->la_omer) b_l_omer = "לעומר";

			n1 = hdate_string(HDATE_STRING_OMER, omer_day, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
			
			if (omer_day == 1) yom = "יום ";
			else
			{
				if (omer_day > 10) days = "יום, ";
				else if (omer_day > 6) days = "ימים, ";
				else days = "ימים ";
			}

			if  (omer_day < 7)
			{
				that_are = empty_text;
				weeks = empty_text;
				vav = empty_text;
				days2 = empty_text;
				n2 = empty_text;
				n3 = empty_text;
			}
			else
			{
				if (omer_day < 14)
				{
					weeks = "שבוע אחד";
					n2 = empty_text;
				}
				else
				{
					n2 = hdate_string(HDATE_STRING_OMER, omer_day/7, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
					n2_needs_free = TRUE;
				}

				if (omer_day%7 == 0)
				{
					vav = ", ";
					days2 = empty_text;
					n3 = empty_text;
				}
				else
				{
					if (omer_day%7 == 1)
					{
						vav = " ויום אחד, ";
						days2 = empty_text;
						n3 = empty_text;
					}
					else
					{
						n3 = hdate_string(HDATE_STRING_OMER, omer_day%7, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
						n3_needs_free = TRUE;
					}
				}
			}
			
			bidi_buffer_len = asprintf(&bidi_buffer, "%s%s%s%s%s%s%s%s%s%s%s",
				hayom, yom,	n1,	days, that_are,	n2,	weeks, vav,	n3,	days2, b_l_omer);
			
			if (opt->bidi) revstr(bidi_buffer, bidi_buffer_len);
			
			printf("%s\n", bidi_buffer);
			
			if (n1 != NULL) free(n1);
			if (n2_needs_free && (n2 != NULL)) free(n2);
			if (n3_needs_free && (n3 != NULL)) free(n3);
			if (bidi_buffer_len != -1) free(bidi_buffer);
		}
		else /// !opt->hebrew
		{
			if (omer_day == 1) printf ("%s ", today_is_day_text);
			else printf ("%s ", N_("today is"));

			printf("%d",omer_day);

			if (omer_day > 1) printf(" %s", days_text);

			if (omer_day > 6)
			{
				printf("%s %d ", N_(", which is"),omer_day/7);

				if (omer_day < 14) printf ("%s", N_("week"));
				else printf("%s", N_("weeks"));
	
				if (omer_day%7 != 0)
				{
					printf (" %s %d", N_("and"), omer_day%7);
					if (omer_day%7 != 1) printf (" %s",	days_text);
					else printf (" %s",	day_text);
				}
	
				printf("%s", N_(","));
			}

		printf(" %s\n", in_the_omer_text);
		}
	}
	return DATA_WAS_PRINTED;
}



/************************************************************
* option 'r' - parashat hashavua
************************************************************/
// now part of function print_day
// this function name may be restored when/if bidi/force hebrew
// becomes supported for option -r
//
/* int print_reading (hdate_struct * h, int opt.diaspora, int short_format, int opt_i)
{
	int reading;

	reading = hdate_get_parasha (h, opt.diaspora);
	if (reading)
	{
		printf ("%s  %s\n", N_("Parashat"), hdate_string (reading, short_format));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}
*/


/************************************************************
* option 'c' - candle-lighting time; tzeit Shabbat
************************************************************/
int print_candles (hdate_struct * h, const option_list* opt)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	/// check for friday - print knisat shabat
	if ( (h->hd_dw == 6) && (opt->candles) )
	{
		/// get times
		hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon,
								&sunrise, &sunset);

		// FIXME - allow for further minhag variation
		if (opt->candles != 1) sunset = sunset - opt->candles;
		else sunset = sunset - DEFAULT_CANDLES_MINUTES;

		/// print candlelighting times
		print_astronomical_time( opt->quiet, candles_text, sunset, opt->tz_offset, opt->data_first);
		return DATA_WAS_PRINTED;
	}

	/// check for saturday - print motzay shabat
	else if ( (h->hd_dw == 7) && (opt->havdalah) )
	{
		/// get times
		hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, opt->lat,
									 opt->lon, &sun_hour, &first_light, &talit,
									 &sunrise, &midday, &sunset,
									 &first_stars, &three_stars);

		// FIXME - allow for further minhag variation
		if (opt->havdalah != 1) three_stars = sunset + opt->havdalah;

		/// print motzay shabat
		print_astronomical_time( opt->quiet, havdalah_text, three_stars, opt->tz_offset, opt->data_first);
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}


/************************************************************
* print tabular header
************************************************************/
void print_tabular_header( const option_list* opt)
{
	if (opt->quiet >= QUIET_DESCRIPTIONS) return;

	if (opt->quiet < QUIET_GREGORIAN) printf("%s,",N_("Gregorian date"));
	printf("%s", N_("Hebrew Date"));
	
	if (opt->first_light) printf(",%s",first_light_text);
	if (opt->talit) printf(",%s",talit_text);
	if (opt->sunrise) printf(",%s",sunrise_text);
	if (opt->magen_avraham) printf(",%s",magen_avraham_text);
	if (opt->shema) printf(",%s",shema_text);
	if (opt->amidah) printf(",%s",amidah_text);
	if (opt->midday) printf(",%s",midday_text);
	if (opt->mincha_gedola) printf(",%s",mincha_gedola_text);
	if (opt->mincha_ketana) printf(",%s",mincha_ketana_text);
	if (opt->plag_hamincha) printf(",%s",plag_hamincha_text);
	if (opt->sunset) printf(",%s",sunset_text);
	if (opt->first_stars) printf(",%s",first_stars_text);
	if (opt->three_stars) printf(",%s",three_stars_text);
	if (opt->sun_hour) printf(",%s",sun_hour_text);
	if (opt->daf_yomi) printf(",%s",daf_yomi_text);
	if (opt->candles) printf(",%s", candles_text);
	if (opt->havdalah) printf(",%s", havdalah_text);
	if (opt->holidays) printf(",%s", holiday_text);
	if ((opt->holidays) && (opt->custom_days_cnt)) printf(";%s", custom_day_tabular_text);
	if (opt->omer) printf(",%s",omer_text);
	if (opt->parasha) printf(",%s", parasha_text);
	printf("\n");
return;
}

/************************************************************
* print one day - tabular output *
************************************************************/
int print_tabular_day (hdate_struct * h, const option_list* opt)
{
	int sun_hour = -1;
	int first_light = -1;
	int talit = -1;
	int sunrise = -1;
	int midday = -1;
	int sunset = -1;
	int first_stars = -1;
	int three_stars = -1;

	hdate_struct tomorrow;
	int data_printed = 0;

	int parasha = 0;
	int holiday = 0;
	int omer_day = 0;

	char *hday_str  = NULL;
	char *hyear_str = NULL;

	char *hebrew_buffer = NULL;		/// for bidi (revstr)
	size_t hebrew_buffer_len = 0;	/// for bidi (revstr)

	/************************************************************
	* options -R, -H are restrictive filters, so if there is no
	* parasha reading / holiday, print nothing.
	************************************************************/
	// parasha = hdate_get_parasha (h, opt->diaspora);
	// holiday = hdate_get_holyday (h, opt->diaspora);
	// if (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)
	//	 return 0;
	// if (opt->only_if_parasha && !opt->only_if_holiday && !parasha)
	//	 return 0;
	// if (opt->only_if_holiday && !opt->only_if_parasha && !holiday)
	//	 return 0;
	if ((opt->parasha) || (opt->only_if_parasha))
		parasha = hdate_get_parasha (h, opt->diaspora);

	if ((opt->holidays) || (opt->only_if_holiday))
		holiday = hdate_get_holyday (h, opt->diaspora);

	if ( (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)	|| /// eg. Shabbat Chanukah
		 (opt->only_if_parasha && !opt->only_if_holiday && !parasha)				|| /// eg. regular Shabbat
		 (opt->only_if_holiday && !opt->only_if_parasha && !holiday)				)  /// eg. Holidays
		return 0;

	// TODO - decide how to handle custom_days in the context of
	//        opt->only_if_holiday. Possibly add options: custom_day
	//        only_if_custom_day, and suppress_custom_day

	// TODO - possibly add options: custom_day, suppress_custom_day


	/************************************************************
	* print Gregorian date
	************************************************************/
	if (opt->quiet < QUIET_GREGORIAN)
		printf ("%d.%d.%d,", h->gd_day, h->gd_mon, h->gd_year);


	/************************************************************
	* begin - print Hebrew date
	************************************************************/
	if (opt->quiet < QUIET_HEBREW)
	{
		// BUG - looks like a bug - why sunset awareness in tabular output?
		if (opt->print_tomorrow)	hdate_set_jd (&tomorrow, (h->hd_jd)+1);
	
		if (opt->bidi)
		{
			hday_str  = hdate_string(HDATE_STRING_INT, h->hd_day,  HDATE_STRING_LONG, opt->hebrew);
			hyear_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);
			hebrew_buffer_len =
				asprintf (&hebrew_buffer, "%s %s %s",
					hday_str,
					hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
					hyear_str);
			revstr(hebrew_buffer, hebrew_buffer_len);
			printf ("%s", hebrew_buffer);
			if (hebrew_buffer != NULL) free(hebrew_buffer);
		}
		else
		{
			// review sometime whether both checks are still necessary (see below for hyear)
			//if ((!hdate_is_hebrew_locale()) && (!opt->hebrew))
			//{	/* non hebrew numbers */
			//	printf ("%d", h->hd_day);
			//}
			//else /* Hebrew */
			//{
				hday_str = hdate_string(HDATE_STRING_INT, h->hd_day, HDATE_STRING_LONG,opt->hebrew);
				printf ("%s", hday_str);
			//}
		
			printf (" %s ",
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, HDATE_STRING_LONG, opt->hebrew));
		
			hyear_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG,opt->hebrew);
			printf ("%s", hyear_str);
		}
		if (hday_str  != NULL) free(hday_str);
		if (hyear_str != NULL) free(hyear_str);
	}
	/************************************************************
	* end - print Hebrew date
	************************************************************/


	/************************************************************
	* begin - print times of day
	************************************************************/
	/// get astronomical times
	hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon,
								 &sun_hour, &first_light, &talit, &sunrise,
								 &midday, &sunset, &first_stars, &three_stars);

	/// print astronomical times
	if (opt->first_light) print_astronomical_time_tabular( first_light, opt->tz_offset);
	if (opt->talit) print_astronomical_time_tabular( talit, opt->tz_offset);
	if (opt->sunrise) print_astronomical_time_tabular( sunrise, opt->tz_offset);


	// FIXME - these times will be moved into the libhdate api
	/// sof zman kriyat Shema (verify that the Magen Avraham calculation is correct!)
	if (opt->magen_avraham) print_astronomical_time_tabular( first_light + (3 * ((first_stars - first_light)/12) ), opt->tz_offset);
	if (opt->shema) print_astronomical_time_tabular( sunrise + (3 * sun_hour), opt->tz_offset);
	/// sof zman tefilah
	if (opt->amidah) print_astronomical_time_tabular( sunrise + (4 * sun_hour), opt->tz_offset);

	// TODO - erev pesach times (don't forget to also do tabular header line!)

	if (opt->midday) print_astronomical_time_tabular( midday, opt->tz_offset);


	// FIXME - these times will be moved into the libhdate api
	/// mincha gedola
	// TODO - There are two other shitot for this:
	//             shaot zmaniot, and shaot zmaniot lechumra
	if (opt->mincha_gedola) print_astronomical_time_tabular( midday + 30, opt->tz_offset);
	/// mincha ketana
	if (opt->mincha_ketana) print_astronomical_time_tabular( sunrise + (9.5 * sun_hour), opt->tz_offset);
	/// plag hamincha
	if (opt->plag_hamincha) print_astronomical_time_tabular( sunrise + (10.75 * sun_hour), opt->tz_offset);


	if (opt->sunset) print_astronomical_time_tabular( sunset, opt->tz_offset);
	if (opt->first_stars) print_astronomical_time_tabular( first_stars, opt->tz_offset);
	if (opt->three_stars) print_astronomical_time_tabular( three_stars, opt->tz_offset);
	if (opt->sun_hour) print_astronomical_time_tabular(sun_hour, 0);

	if (opt->daf_yomi) print_daf_yomi(h->hd_jd, opt->hebrew, opt->bidi, TRUE);

	if (opt->candles)
	{
		if ( (h->hd_dw != 6) && (!opt->only_if_parasha) ) sunset = -1; //printf(",");
		else
		{
			if (sunset == -1) hdate_get_utc_sun_time (h->gd_day, h->gd_mon,
									h->gd_year, opt->lat, opt->lon, &sunrise, &sunset);
			if (opt->candles != 1) sunset = sunset - opt->candles;
			else sunset = sunset- DEFAULT_CANDLES_MINUTES;
		}
		print_astronomical_time_tabular( sunset, opt->tz_offset);
	}
	
	if (opt->havdalah)
	{
		if ( (h->hd_dw != 7)  && (!opt->only_if_parasha) ) three_stars = -1; //printf(",");
		else
		{
			if (opt->havdalah != 1)
			{
				if (sunset == -1) hdate_get_utc_sun_time (h->gd_day, h->gd_mon,
									h->gd_year, opt->lat, opt->lon, &sunrise, &sunset);
				three_stars = sunset + opt->havdalah;
			}
			else
			{
				if(three_stars == -1) hdate_get_utc_sun_time_full (
									h->gd_day, h->gd_mon, h->gd_year, opt->lat,
									opt->lon, &sun_hour, &first_light, &talit,
									&sunrise, &midday, &sunset,
									&first_stars, &three_stars);
			}
		}
		print_astronomical_time_tabular( three_stars, opt->tz_offset);
	}
	/************************************************************
	* end - print times of day
	************************************************************/

	if (opt->omer)
	{
		omer_day = hdate_get_omer_day(h);
		if (omer_day != 0) printf (",%d", omer_day);
		else printf(",");
	}

	if (opt->parasha)
	{
		if (parasha)
		{
			if (!opt->bidi) printf (",%s",
						hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
			else
			{
				hebrew_buffer_len =	asprintf (&hebrew_buffer, "%s",
						hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
				revstr(hebrew_buffer, hebrew_buffer_len);
				printf (",%s", hebrew_buffer);
				if (hebrew_buffer != NULL) free(hebrew_buffer);
			}
		}
		else printf(",");
	}

	if (opt->holidays)
	// TODO - bidi ?? check out logic for opt->parasha, above
	{
		if (holiday) printf(",%s", hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
		else printf (",");

		// TODO - I could shrink the jdn_list array upon finding a match. This would require a
		//        a second array with the index into the custom_day text string array, or having
		//        the jdn_list array in pairs (jdn & 'index into custom_day text string array'
		if (opt->custom_days_cnt)
		{
			int i;
			int* jdn_list_ptr = opt->jdn_list_ptr;
			for (i=0; i<opt->custom_days_cnt; i++)
			{
				if (h->hd_jd == *jdn_list_ptr)
					printf(";%s", get_custom_day_ptr(i, opt->string_list_ptr));
				else jdn_list_ptr = jdn_list_ptr + 1;
			}
		}
	}

	printf("\n");

	if ((opt->print_tomorrow) && (data_printed) && (!opt->quiet)) print_alert_sunset();

	return 0;
}




/************************************************************
* print one day - regular output
************************************************************/
int print_day (hdate_struct * h, option_list* opt)
{

	time_t t;
	hdate_struct tomorrow;

	int iCal_uid_counter = 0;
	int data_printed = 0;
	int parasha = 0;
	int holiday = 0;

	/************************************************************
	* options -R, -H are restrictive filters, so if there is no
	* parasha reading / holiday, print nothing.
	************************************************************/
	// parasha = hdate_get_parasha (h, opt->diaspora);
	// holiday = hdate_get_holyday (h, opt->diaspora);
	// if (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)
	//	 return 0;
	// if (opt->only_if_parasha && !opt->only_if_holiday && !parasha)
	//	 return 0;
	// if (opt->only_if_holiday && !opt->only_if_parasha && !holiday)
	//	 return 0;
	if ((opt->parasha) || (opt->only_if_parasha))
		parasha = hdate_get_parasha (h, opt->diaspora);

	if ((opt->holidays) || (opt->only_if_holiday))
		holiday = hdate_get_holyday (h, opt->diaspora);

	if ( (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)	|| /// eg. Shabbat Chanukah
		 (opt->only_if_parasha && !opt->only_if_holiday && !parasha)				|| /// eg. regular Shabbat
		 (opt->only_if_holiday && !opt->only_if_parasha && !holiday)				)  /// eg. Holidays
		return 0;
	// TODO - decide how to handle custom_days in the context of
	//        opt->only_if_holiday. Possibly add options: custom_day
	//        only_if_custom_day, and suppress_custom_day

	// TODO - possibly add options: custom_day, suppress_custom_day

	/************************************************************
	* print the iCal event header
	************************************************************/
	if (opt->iCal)
	{
		printf ("BEGIN:VEVENT\n");
		printf ("UID:hdate-%ld-%d\n", time(&t), ++iCal_uid_counter);
		printf ("DTSTART;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
				h->gd_mon, h->gd_day);
		printf ("SUMMARY:");
	}


	/************************************************************
	* print the Julian Day Number
	************************************************************/
	if (opt->julian) printf ("JDN-%d ", h->hd_jd);


	/************************************************************
	* print the date
	************************************************************/
	if (opt->print_tomorrow)	hdate_set_jd (&tomorrow, (h->hd_jd)+1);
	if (opt->quiet < QUIET_HEBREW) print_date (h, &tomorrow, opt);



	/************************************************************
	* begin - print additional information for day
	************************************************************/
	if (opt->print_tomorrow) *h = tomorrow;
	opt->today_time = opt->today_time + SECONDS_PER_DAY;
	data_printed = data_printed | print_times (h, opt, holiday);
	if (opt->candles || opt->havdalah) data_printed = data_printed | print_candles (h, opt);
	if (opt->holidays)
	{
		if (holiday)
		{
			if (opt->quiet < QUIET_DESCRIPTIONS) printf ("%s: ", N_("holiday_today"));
//			TODO - allow option to force hebrew here; This is not trivial as it entails bidi
//					when the locale for the label is LTR.
//			printf ("%s\n",	hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
			printf ("%s\n",	hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, 0));
			data_printed = DATA_WAS_PRINTED;
		}
		if (opt->custom_days_cnt)
		{
			int i;
			int* jdn_list_ptr = opt->jdn_list_ptr;
			for (i=0; i<opt->custom_days_cnt; i++)
			{
				if (h->hd_jd == *jdn_list_ptr)
					printf("%s: %s\n", custom_day_text, get_custom_day_ptr(i, opt->string_list_ptr));
				jdn_list_ptr = jdn_list_ptr + 1;
			}
		}
	}
	if (opt->omer) data_printed = data_printed | print_omer (h, opt);
	if (opt->parasha && parasha)
	{
		if (opt->quiet < QUIET_DESCRIPTIONS) printf ("%s: ", N_("parasha"));
//		TODO - allow option to force hebrew here; This is not trivial as it entails bidi
//				when the locale for the label is LTR.
//		printf("%s\n", hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
		printf("%s\n", hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, 0));
		data_printed = DATA_WAS_PRINTED;
	}
	if (opt->daf_yomi) data_printed = data_printed | print_daf_yomi(h->hd_jd, opt->hebrew, opt->bidi, FALSE);

	if ((opt->print_tomorrow) && (data_printed) && (!opt->quiet)) print_alert_sunset();
	
	/************************************************************
	* end - print additional information for day
	************************************************************/




	/************************************************************
	* print the iCal event footer
	************************************************************/
	if (opt->iCal)
	{
		printf ("\nCLASS:PUBLIC\n");
		printf ("DTEND;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
				h->gd_mon, h->gd_day);
		printf ("CATEGORIES:Holidays\n");
		printf ("END:VEVENT\n");
	}
	else printf("\n");

	return 0;
}



/************************************************************
* print one Gregorian month - tabular output
************************************************************/
int print_tabular_gmonth( const option_list* opt, const int month, const int year)
{
	hdate_struct h;
	int jd;

	/// get date of month start
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/// print month days
	while (h.gd_mon == month)
	{
		print_tabular_day (&h, opt);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Gregorian month - regular output
************************************************************/
int print_gmonth ( option_list* opt, int month, int year)
{
	hdate_struct h;
	int jd;

	/// get date of month start
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/// print month header
	if (!opt->iCal && !opt->short_format)
		printf ("\n%s:\n",
			hdate_string( HDATE_STRING_GMONTH, h.gd_mon, opt->short_format, HDATE_STRING_LOCAL));

	/// print month days
	while (h.gd_mon == month)
	{
		print_day (&h, opt);

		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Hebrew month - tabular output *
************************************************************/
int print_tabular_hmonth
	(	const option_list* opt, const int month, const int year)
{
	hdate_struct h;
	int jd;

	/// get date of month start
	hdate_set_hdate (&h, 1, month, year);
	jd = h.hd_jd;

	/// print month days
	while (h.hd_mon == month)
	{
		print_tabular_header( opt );
		print_tabular_day (&h, opt);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}


/************************************************************
* print one Hebrew month - regular output
************************************************************/
int print_hmonth (hdate_struct * h, option_list* opt,
					const int month, const int year)
{
	int jd;
	int bidi_buffer_len;
	char* bidi_buffer;

	/// get date of month start
	jd = h->hd_jd;

	/// print month header
	if (!opt->iCal && !opt->short_format)
	{
		if (opt->bidi)
		{
			bidi_buffer_len = asprintf(&bidi_buffer, "%s",
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));
			revstr(bidi_buffer, bidi_buffer_len);
			if (bidi_buffer_len != -1)
			{
				printf("\n%s:\n", bidi_buffer);
				if (bidi_buffer != NULL) free(bidi_buffer);
			}
		}
		else printf ("\n%s:\n",
			hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));
	}

	/// print month days
	while (h->hd_mon == month)
	{
		print_day (h, opt);

		jd++;
		hdate_set_jd (h, jd);
	}
	return 0;
}



/************************************************************
* print one Gregorian year - tabular output *
************************************************************/
int print_tabular_gyear
	( const option_list* opt, const int year)
{
	int month = 1;
	while (month < 13)
	{
		print_tabular_gmonth(opt, month, year);
		month++;
	}
	return 0;
}


/************************************************************
* print one Hebrew year - tabular output *
************************************************************/
int print_tabular_hyear
	( const option_list* opt, const int year)
{
	hdate_struct h;
	int month = 1;

	/// print year months
	while (month < 13)
	{
		/// get date of month start
		hdate_set_hdate (&h, 1, month, year);

		/// if leap year, print both Adar months
		if (h.hd_size_of_year > 365 && month == 6)
		{
			hdate_set_hdate (&h, 1, 13, year);
			print_tabular_hmonth(opt, 13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_tabular_hmonth(opt, 14, year);
		}
		else
		{
			print_tabular_hmonth(opt, month, year);
		}
		month++;
	}
	return 0;
}



/************************************************************
* print one Gregorian year - regular output
************************************************************/
int print_gyear ( option_list* opt, const int year)
{

	int month = 1;

	/// print year header
	if (!opt->iCal && !opt->short_format)
		printf ("%d:\n", year);

	/// print year months
	while (month < 13)
	{
		print_gmonth ( opt, month, year);
		month++;
	}

	return 0;
}



/************************************************************
* print one hebrew year - regular output
************************************************************/
int print_hyear ( option_list* opt, const int year)
{
	hdate_struct h;
	int month = 1;

	char *h_int_str;
	h_int_str = hdate_string(HDATE_STRING_INT, year,HDATE_STRING_LONG,opt->hebrew);
	// FIXME - error check for NULL return value

	/// print year header
	if (!opt->iCal && !opt->short_format)	printf ("%s:\n", h_int_str);

	/// print year months
	while (month < 13)
	{
		/// get date of month start
		hdate_set_hdate (&h, 1, month, year);

		/// if leap year, print both Adar months
		if (h.hd_size_of_year > 365 && month == 6)
		{
			hdate_set_hdate (&h, 1, 13, year);
			print_hmonth (&h, opt, 13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_hmonth (&h, opt, 14, year);
		}
		else
		{
			print_hmonth (&h, opt, month, year);
		}
		month++;
	}

	return 0;
}


/****************************************************
* read and parse config file
****************************************************/
void read_config_file(	FILE *config_file,
						option_list *opt,
						int*	opt_lat,
						int*	opt_lon,
						char*	tz_name_str )
{
	double tz_lat, tz_lon;
	char	*input_string = NULL;
	size_t	input_str_len;	// unnecessary to initialize, per man(3) getline
//	size_t	input_str_len = 200;	// WARNING: if you change this value
									// you will still have to also
									// change a matching value below
									// in the statement that includes:
									// match_count = sscanf(input_string
	char	*input_key;    // unnecessary to initialize, per man(3) sscanf
//	char	*input_key = NULL;
	char	*input_value;  // unnecessary to initialize, per man(3) sscanf
//	char	*input_value = NULL;
	int		menu_item = 0;
	size_t	menu_len = 0;
	int		line_count = 0;
	int		match_count;
	int		end_of_input_file = FALSE;
	int		i;
	const int	num_of_keys = 26;
	const char*	key_list[] = {	"SUNSET_AWARE",		// 0
								"LATITUDE",
								"LONGITUDE",		// 2
								"TIMEZONE",
								"DIASPORA",			// 4
								"FORCE_ISRAEL",
								"PARASHA_NAMES",	// 6
								"SHABBAT_INFO",
								"FORCE_HEBREW",		// 8
								"OUTPUT_BIDI",
								"QUIET_ALERTS",		//10
								"YOM",
								"LESHABBAT",		//12
								"LESEDER",
								"TABULAR",			//14
								"ICAL",
								"SEFIRAT_HAOMER",	//16
								"SHORT_FORMAT",
								"TIMES_OF_DAY",		//18
								"SUN_RISE_SET",
								"ONLY_IF_PARASHA_IS_READ",
								"ONLY_IF_HOLIDAY",
								"JULIAN_DAY",		//22
								"CANDLE_LIGHTING",
								"HAVDALAH",			//24
								"MENU"
								};

//	input_string = malloc(input_str_len+1); unnecessary - done by getline
//	input_key    = malloc(input_str_len+1); unnecessary - done by sscanf
//	input_value  = malloc(input_str_len+1); unnecessary - done by sscanf

	while ( end_of_input_file != TRUE )
	{
		end_of_input_file = getline(&input_string, &input_str_len, config_file);
		if ( end_of_input_file != TRUE )
		{
			errno = 0;
			// BUG - FIXME please
			// The '200' in the next statement is a bug; it is meant to
			// be the value of input_str_len. It would be convenient to
			// use the 'a' conversion specifier here; however, the man
			// pages say that it's a non-standard extension specific to
			// GNU. Let's play with the 'm' conversion specifier which
			// on the one hand, is POSIX and GNU compliant, but on the
			// other hand only for glibc 2.7+ and POSIX.1+
//			match_count = sscanf(input_string,"%[A-Z_]=%200[^\n]",input_key,input_value);
			match_count = sscanf(input_string,"%m[A-Z_]=%m[^\n]",&input_key,&input_value);
			line_count++;
			if (errno != 0) error(0,errno,"scan error at line %d", line_count);
			if (match_count ==2)
			{
				for (i=0; i<num_of_keys; i++)
				{
					if (strcmp(input_key, key_list[i]) == 0)
					{
						switch(i)
						{

///		SUNSET_AWARE
		case  0:if      (strcmp(input_value,"FALSE") == 0) opt->not_sunset_aware = 1;
				else if (strcmp(input_value,"TRUE") == 0) opt->not_sunset_aware = 0;
				break;

///		LATITUDE
		case  1:
				parse_coordinate(1, input_value, &opt->lat, opt_lat);
				break;

///		LONGITUDE
		case  2:
				parse_coordinate(2, input_value, &opt->lon, opt_lon);
				break;

///		TIMEZONE
		case  3:
				if  (!parse_timezone_numeric(input_value, &opt->tz_offset))
				{
					if (parse_timezone_alpha(input_value, tz_name_str, &opt->tz_offset, &tz_lat, &tz_lon))
					{
						// TODO - really, at this point, shouldn't either both be bad or botha be good?
						if (opt->lat  == BAD_COORDINATE) opt->lat = tz_lat;
						if (opt->lon == BAD_COORDINATE) opt->lon = tz_lon;
					}
				}
				break;

///		DIASPORA
		case  4:if      (strcmp(input_value,"FALSE") == 0) opt->diaspora = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->diaspora = 1;
				break;


//		case  5:if      (strcmp(input_value,"FALSE") == 0) opt->force_israel = 0;
//				else if (strcmp(input_value,"TRUE") == 0) opt->force_israel = 1;
				break;

///		PARASHA_NAMES
		case  6:if      (strcmp(input_value,"FALSE") == 0) opt->parasha = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->parasha = 1;
				break;

///		SHABBAT_INFO
 		case  7:if      (strcmp(input_value,"FALSE") == 0)
				{
					// maybe not do anything if set FALSE
					opt->candles = 0;
					opt->havdalah = 0;
					opt->parasha = 0;
				}
				else if (strcmp(input_value,"TRUE") == 0)
				{
					opt->candles = 1;
					opt->havdalah = 1;
					opt->parasha = 1;
				}
				break;

///		FORCE_HEBREW
		case  8:if      (strcmp(input_value,"FALSE") == 0) opt->hebrew = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->hebrew = 1;
				break;

///		OUTPUT_BIDI
		case  9:if      (strcmp(input_value,"FALSE") == 0) opt->bidi = 0;
				else if (strcmp(input_value,"TRUE") == 0)
						{
							opt->bidi = 1;
							opt->hebrew = 1;
						}
				break;

///		QUIET_ALERTS
		case 10:if      (strcmp(input_value,"FALSE") == 0) opt->quiet = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->quiet = 1;
				break;

///		YOM
		case 11:if      (strcmp(input_value,"FALSE") == 0) opt->yom = 0;
				else if (strcmp(input_value,"TRUE") == 0)
						{
							opt->yom = 1;
							opt->hebrew = 1;
						}
				break;

///		LESHABBAT
		case 12:if      (strcmp(input_value,"FALSE") == 0) opt->leShabbat = 0;
				else if (strcmp(input_value,"TRUE") == 0)
						{
							opt->leShabbat = 1;
							opt->yom = 1;
							opt->hebrew = 1;
						}
				break;

///		LESEDER
		case 13:if      (strcmp(input_value,"FALSE") == 0) opt->leSeder = 0;
				else if (strcmp(input_value,"TRUE") == 0)
						{
							opt->leSeder = 1;
							opt->yom = 1;
							opt->hebrew = 1;
						}
				break;

///		TABULAR
		case 14:if      (strcmp(input_value,"FALSE") == 0) opt->tablular_output = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->tablular_output = 1;
				break;

///		ICAL
		case 15:if      (strcmp(input_value,"FALSE") == 0) opt->iCal = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->iCal = 1;
				break;

///		SEFIRAT_HAOMER
		case 16:if      (strcmp(input_value,"FALSE") == 0) opt->omer = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->omer = 1;
				break;

///		SHORT_FORMAT
		case 17:if      (strcmp(input_value,"FALSE") == 0) opt->short_format = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->short_format = 1;
				break;

///		TIMES_OF_DAY
		case 18:if      (strcmp(input_value,"FALSE") == 0) opt->times = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->times = 1;
				break;

///		SUN_RISE_SET
		case 19:if      (strcmp(input_value,"FALSE") == 0)
				{
							opt->sunrise = 0;
							opt->sunset = 0;
				}
				else if (strcmp(input_value,"TRUE") == 0)
				{
							opt->sunrise = 1;
							opt->sunset = 1;
				}
				break;

///		ONLY_IF_PARASHA_IS_READ
		case 20:if      (strcmp(input_value,"FALSE") == 0) opt->only_if_parasha = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->only_if_parasha = 1;
				break;

///		ONLY_IF_HOLIDAY
		case 21:if      (strcmp(input_value,"FALSE") == 0) opt->only_if_holiday = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->only_if_holiday = 1;
				break;

///		JULIAN_DAY
		case 22:if      (strcmp(input_value,"FALSE") == 0) opt->julian = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->julian = 1;
				break;

///		CANDLE_LIGHTING
		case 23:if      (strcmp(input_value,"FALSE") == 0) opt->candles = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->candles = 1;
				else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
				{
					opt->candles = atoi(input_value);
					if (opt->candles < MIN_CANDLES_MINUTES) opt->candles = MIN_CANDLES_MINUTES;
					else if (opt->candles > MAX_CANDLES_MINUTES) opt->candles = MAX_CANDLES_MINUTES;
				}
				break;

///		HAVDALAH
		case 24:if      (strcmp(input_value,"FALSE") == 0) opt->havdalah = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->havdalah = 1;
				else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
				{
					opt->havdalah = atoi(input_value); 
					if (opt->havdalah < MIN_MOTZASH_MINUTES) opt->havdalah = MIN_MOTZASH_MINUTES;
					else if (opt->havdalah > MAX_MOTZASH_MINUTES) opt->havdalah = MAX_MOTZASH_MINUTES;
				}
				break;

///		MENU
		case 25:if (menu_item < MAX_MENU_ITEMS)
				{
					menu_len = strlen(input_value);
					opt->menu_item[menu_item] = malloc(menu_len+1);
					memcpy(opt->menu_item[menu_item], input_value,menu_len);
					menu_item++;
				}
				break;

						}	/// end of switch(i)
					break;	/// if found a match don't continue for loop
					}
				}
			free(input_value);
			}
			if (match_count > 0 ) free(input_key);
		}
	}
	if (input_string != NULL ) free(input_string);
//	free(input_key);
//	free(input_value);
	return;	
}


/****************************************************
* exit elegantly
****************************************************/
void exit_main( option_list *opt, int exit_code)
{
	int i;
	for (i=0; i<MAX_MENU_ITEMS; i++) 
	{
		if (opt->menu_item[i] == NULL) break;
		free(opt->menu_item[i]);
	}
	exit (exit_code);
}



/****************************************************
* parse a command-line or a config-file menu line
*
* It was appropriate to make this a function, outside
* of main, because of its dual use and dual reference
****************************************************/
int parameter_parser( int switch_arg, option_list *opt,
					int *opt_latitude, int *opt_Longitude,
					char* tz_name_str, int long_option_index)
{
	double tz_lat, tz_lon;
	static char *timezone_text  = N_("z (timezone)");

	int error_detected = 0;

	switch (switch_arg)
	{

	case 0: /// long options
		switch (long_option_index)
		/*****************************************************
		* cases of THIS switch should be blank if the final
		* field of structure "option long_options[]", defined
		* in main() is not zero, ie. there is a short option,
		* ie. the option will be processed in the short
		* option switch, below.
		*****************************************************/
		{
/** --version		*/	case 0:	print_version (); exit_main(opt,0); break;
/** --help			*/	case 1:	print_help (); exit_main(opt,0); break;
/** --hebrew			*/	case 2: opt->hebrew = 1; break;
/** --yom			*/	case 3:
								opt->yom = 1;
								opt->hebrew = 1;
								break;
/** --leshabbat		*/	case 4:
								opt->leShabbat = 1;
								opt->yom = 1;
								opt->hebrew = 1;
								break;
/** --leseder		*/	case 5:
								opt->leSeder = 1;
								opt->yom = 1;
								opt->hebrew = 1;
								break;
/** --table			*/	case 6: break;
/** --not-sunset-aware */case 7:	opt->not_sunset_aware = 1; break;
/** --quiet			*/	case 8:	break;
/** --short-format	*/	case 9:	break;
/** --parasha		*/	case 10:break;
/** --holidays		*/	case 11:break;
/** --shabbat-times	*/	case 12:break;
/** --sun			*/	case 13: opt->sunset = 1;
								 opt->sunrise = 1;
								 break;
/** --sunset		*/	case 14: opt->sunset = 1; break;
/** --sunrise		*/	case 15: opt->sunrise = 1; break;
/** --candle-lighting */	case 16:
/** --candles		*/	case 17:
			if (optarg == NULL) opt->candles = 1;
			else
			{
				if (fnmatch( "[[:digit:]]?([[:digit:]])", optarg, FNM_EXTMATCH) == 0)
				{
					opt->candles = atoi(optarg);
					if 	( (opt->candles >= MIN_CANDLES_MINUTES) &&
						(opt->candles <= MAX_CANDLES_MINUTES) ) break;
				}
				print_parm_error("--candles"); // do not gettext!
				error_detected++;
			}
			break;

/** --havdalah		*/	case 18:
			if (optarg == NULL) opt->havdalah = 1;
			else
			{
				if (fnmatch( "[[:digit:]]?([[:digit:]])", optarg, FNM_EXTMATCH) == 0)
				{
					opt->havdalah = atoi(optarg);
					if 	( (opt->havdalah >= MIN_MOTZASH_MINUTES) &&
						(opt->havdalah <= MAX_MOTZASH_MINUTES) ) break;
				}
				print_parm_error("--havdalah"); // do not gettext!
				error_detected++;
			}
			break;

/** --latitude				*/	case 19:break;
/** --longitude				*/	case 20:break;
/** --timezone				*/	case 21:break;
/** --bidi					*/	case 22:
/** --visual				*/	case 23:break;
/** --omer					*/	case 24:break;
/** --ical					*/	case 25:break;
/** --julian				*/	case 26:break;
/** --diaspora				*/	case 27:break;
/** --menu					*/	case 28:break;
/** --alot					*/	case 29:
/** --first-light			*/	case 30: opt->first_light = 1; break;
/** --talit					*/	case 31: opt->talit = 1; break;
/** --netz					*/	case 32: opt->sunrise = 1; break;
/** --shema					*/	case 33: opt->shema = 1; break;
/** --amidah				*/	case 34: opt->amidah = 1; break;
/** --midday				*/	case 35:
/** --noon					*/	case 36:
/** --chatzot				*/	case 37: opt->midday = 1; break;
/** --mincha-gedola			*/	case 38: opt->mincha_gedola = 1; break;
/** --mincha-ketana			*/	case 39: opt->mincha_ketana = 1; break;
/** --plag-hamincha			*/	case 40: opt->plag_hamincha = 1; break;
/** --shekia				*/	case 41: opt->sunset = 1; break;
/** --tzeit					*/	case 42:
/** --first-stars			*/	case 43: opt->first_stars = 1; break;
/** --three-stars			*/	case 44: opt->three_stars = 1; break;
/** --magen-avraham			*/	case 45: opt->magen_avraham = 1; break;
/** --sun-hour				*/	case 46: opt->sun_hour = 1; break;
/** --daf-yomi				*/	case 47: opt->daf_yomi = 1; break;
/** --tabular				*/	case 48: break;
/** --end-eating-chometz-ma	*/	case 49: opt->end_eating_chometz_ma = 1; break;
/** --end-eating-chometz-gra*/	case 50: opt->end_eating_chometz_gra = 1; break;
/** --end-owning-chometz-ma	*/	case 51: opt->end_owning_chometz_ma = 1; break;
/** --end-owning-chometz-gra*/	case 52: opt->end_owning_chometz_gra = 1; break;
/** --times-of-day			*/	case 53: break;
/** --day-times				*/	case 54: break;
/** --israel				*/	case 55: opt->diaspora = 0; break;
/** --la-omer				*/	case 56: opt->la_omer = 1;	/// fall through to option ba-omer
/** --ba-omer				*/	case 57: opt->omer = 3;
/** --quiet-alerts			*/	case 58: if (opt->quiet < QUIET_ALERTS) opt->quiet = QUIET_ALERTS;
/** --quiet-gregorian		*/	case 59: if (opt->quiet < QUIET_GREGORIAN) opt->quiet = QUIET_GREGORIAN;
/** --quiet-descriptions	*/	case 60: if (opt->quiet < QUIET_DESCRIPTIONS) opt->quiet = QUIET_DESCRIPTIONS;
/** --quiet-hebrew			*/	case 61: if (opt->quiet < QUIET_HEBREW) opt->quiet = QUIET_HEBREW;
										 opt->hebrew = 1; break;
/** --data-first			*/	case 62: opt->data_first = TRUE; break;
/** --labels-first			*/	case 63: opt->data_first = FALSE; break;
		} /// end switch for long_options
		break;


	case 'b': opt->bidi = 1; opt->hebrew = 1; break;
	case 'd': opt->diaspora = 1; break;
	case 'H': opt->only_if_holiday = 1; /// There is no break here, because -H implies -h
	case 'h': opt->holidays = 1; break;
	case 'i': opt->iCal = 1; break;
	case 'j': opt->julian = 1; break;
	case 'q': opt->quiet = opt->quiet + 1; break;
	case 'm': opt->menu = 1; break;
	case 'o': opt->omer = opt->omer + 1; break;
	case 'R': opt->only_if_parasha = 1; /// There is no break here, because -R implies -r
	case 'r': opt->parasha = 1; break;
	case 'S': opt->short_format = 1; break;
//	case 's': opt->sunrise = 1; opt->sunset = 1; break;
	case 's': /// candles & havdalah may have custom minhag in minutes
			  if (opt->candles == 0) opt->candles = 1;
			  if (opt->havdalah == 0) opt->havdalah = 1;
			  break;
	case 't': opt->times = opt->times + 1; break;
	case 'T': opt->tablular_output = 1; break;
	case 'l':
		error_detected = error_detected + parse_coordinate(1, optarg, &opt->lat, opt_latitude);
		break;
	case 'L':
		error_detected = error_detected + parse_coordinate(2, optarg, &opt->lon, opt_Longitude);
		break;
	case 'v': opt->afikomen = opt->afikomen + 1; break;
	case 'z':
		if	( (!optarg)
			  || ( (optarg[0] == '-')
		           && (strspn(&optarg[1], "0123456789.:") == 0) ) )
		{
			error_detected = error_detected + 1;
			print_parm_missing_error(timezone_text);
		}
		else if (!parse_timezone_numeric(optarg, &opt->tz_offset))
		{
			if (!parse_timezone_alpha(optarg, tz_name_str, &opt->tz_offset, &tz_lat, &tz_lon))
			{
				error_detected = error_detected + 1;
				print_parm_error(timezone_text);
			}
			else
			{
				if (opt->lat  == BAD_COORDINATE) opt->lat = tz_lat;
				if (opt->lon == BAD_COORDINATE) opt->lon = tz_lon;
			}
		}
		break;
	case '?':
//		if (strchr(short_options,optopt)==NULL)
//			error(0,0,"option %c unknown",optopt);
		print_parm_missing_error((char*) &optopt);
		error_detected = TRUE;
		break;
	default:
		print_usage_hdate();
		print_try_help_hdate();
		exit_main(opt,0);
		break;
	}

	return error_detected;
}


/************************************************************
*************************************************************
*************************************************************
* main function
*************************************************************
*************************************************************
************************************************************/
int main (int argc, char *argv[])
{
	/// defined in local_functions.c
	/// #define HEB_YR_UPPER_BOUND 10999
	/// #define HEB_YR_LOWER_BOUND 3000
	/// #define JUL_DY_LOWER_BOUND = 348021

	/// The Hebrew date structure
	//	TODO - initialize this
	hdate_struct h;
	int c;

	/// The user-input date (Hebrew or gregorian)
	/// defined in local_functions.c
	/// #define BAD_DATE_VALUE -1
	int day   = BAD_DATE_VALUE;
	int month = BAD_DATE_VALUE;
	int year  = BAD_DATE_VALUE;

	// deprecate this next in favor of opt.lat, opt.lon, opt.tz_offset
	// double lat = BAD_COORDINATE;	/// set to this value for error handling
	// double lon = BAD_COORDINATE;	/// set to this value for error handling
	// int tz = BAD_TIMEZONE;			/// -z option Time Zone, default to system local time
	char* tz_name_str = NULL;
	int opt_latitude = 0;			/// -l option latitude
	int opt_Longitude = 0;			/// -L option longitude
	
	int error_detected = FALSE;		/// exit after reporting ALL bad parms


	option_list opt;
	opt.lat = BAD_COORDINATE;
	opt.lon = BAD_COORDINATE;
	opt.tz_offset = BAD_TIMEZONE;
	opt.hebrew = 0;
	opt.bidi = 0;
	opt.yom = 0;
	opt.leShabbat = 0;
	opt.leSeder = 0;
	opt.tablular_output = 0;
	opt.not_sunset_aware = 0;
	opt.quiet = 0;
	opt.print_tomorrow = 0;		/**	This isn't a command line option;
									It's here to restrict sunset_aware
									to single-day outputs */
	opt.first_light = 0;
	opt.talit = 0;
	opt.sunrise = 0;
	opt.magen_avraham = 0;
	opt.shema = 0;
	opt.amidah = 0;
	opt.midday = 0;
	opt.mincha_gedola = 0;
	opt.mincha_ketana = 0;
	opt.plag_hamincha = 0;
	opt.sunset = 0;
	opt.first_stars = 0;
	opt.three_stars = 0;
	opt.sun_hour = 0;
	opt.candles = 0;
	opt.havdalah = 0;
	opt.times = 0;				/// -t option print times of day
	opt.short_format = 0;		/// -S Short format flag
	opt.holidays = 0;			/// -h option holidays
	opt.only_if_holiday = 0;	/// -H option just holidays
	opt.parasha = 0;			/// -r option reading
	opt.only_if_parasha = 0;	/// -R option just reading
	opt.julian = 0;				/// -j option Julian day number
	opt.diaspora = -1;			/// -d option diaspora, if not explicitly set by user or config file
								/// to 0/1 will be set based on timezone/location awareness guess.
	opt.iCal = 0;				/// -i option iCal
	opt.daf_yomi = 0;
	opt.omer = 0;				/// -o option Sfirat Haomer
	opt.la_omer = 0;			/// use lame instead of bet
	opt.menu = 0;				/// -m print menus for user-selection
	opt.afikomen = 0;			/// Hebrew 'easter egg' (lehavdil)
	opt.end_eating_chometz_ma = 0;
	opt.end_eating_chometz_gra = 0;
	opt.end_owning_chometz_ma = 0;
	opt.end_owning_chometz_gra = 0;
	opt.data_first = TRUE;
	opt.custom_days_cnt = 0;
	opt.jdn_list_ptr = NULL;	/// for custom_days
	opt.string_list_ptr= NULL;	/// for custom_days
	// TODO - be sure to free() opt.jdn_list_ptr, opt.string_list_ptr upon exit

	/// for user-defined menus (to be read from config file)
	size_t	menu_len = 0;
	int menu_index;
	char *menuptr, *optptr;
	/// initialize user menu items
	int i; for (i=0; i<MAX_MENU_ITEMS; i++) opt.menu_item[i] = NULL;

	/// support for getopt short options
	const char * short_options = "bcdhHjimoqrRsStTvl:L:z:";

	/// support for getopt long options
	int long_option_index = 0;
	const struct option long_options[] = {
	///       name,  has_arg, flag, val
	/**  0 */{"version", 0, 0, 0},
	/**  1 */{"help", 0, 0, 0},
	/**  2 */{"hebrew", 0, 0, 0},
	/**  3 */{"yom", 0, 0, 0},
	/**  4 */{"leshabbat", 0, 0, 0},
	/**  5 */{"leseder", 0, 0, 0},
	/**  6 */{"table",0,0,'T'},
	/**  7 */{"not-sunset-aware",0,0,0},
	/**  8 */{"quiet",0,0,'q'},
	/**  9 */{"short_format",0,0,'S'},
	/** 10 */{"parasha",0,0,'r'},
	/** 11 */{"holidays",0,0,'h'},
	/** 12 */{"shabbat-times",0,0,'s'},
	/** 13 */{"sun",0,0,0},		
	/** 14 */{"sunset",0,0,0},
	/** 15 */{"sunrise",0,0,0},
	/** 16 */{"candle-lighting",0,0,0},
	/** 17 */{"candles",2,0,0},
	/** 18 */{"havdalah",2,0,0},
	/** 19 */{"latitude", 1, 0, 'l'},
	/** 20 */{"longitude", 1, 0, 'L'},
	/** 21 */{"timezone", 1, 0, 'z'},
	/** 22 */{"bidi", 0, 0, 'b'},
	/** 23 */{"visual", 0, 0, 'b'},
	/** 24 */{"omer", 0, 0, 'o'},
	/** 25 */{"ical", 0, 0, 'i'},
	/** 26 */{"julian", 0, 0, 'j'},
	/** 27 */{"diaspora", 0, 0, 'd'},
	/** 28 */{"menu",0,0,'m'},
	/** 29 */{"alot",0,0,0},
	/** 30 */{"first-light",0,0,0},		
	/** 31 */{"talit",0,0,0},
	/** 32 */{"netz",0,0,0},
	/** 33 */{"shema",0,0,0},
	/** 34 */{"amidah",0,0,0},
	/** 35 */{"midday",0,0,0},
	/** 36 */{"noon",0,0,0},
	/** 37 */{"chatzot",0,0,0},
	/** 38 */{"mincha-gedola",0,0,0},
	/** 39 */{"mincha-ketana",0,0,0},
	/** 40 */{"plag-hamincha",0,0,0},
	/** 41 */{"shekia",0,0,0},
	/** 42 */{"tzeit-hakochavim",0,0,0},
	/** 43 */{"first-stars",0,0,0},
	/** 44 */{"three-stars",0,0,0},
	/** 45 */{"magen-avraham",0,0,0},
	/** 46 */{"sun-hour",0,0,0},
	/** 47 */{"daf-yomi",0,0,0},
	/** 48 */{"tabular",0,0,'T'},
	/** 49 */{"end-eating-chometz-ma",0,0,0},
	/** 50 */{"end-eating-chometz-gra",0,0,0},
	/** 51 */{"end-owning-chometz-ma",0,0,0},
	/** 52 */{"end-owning-chometz-gra",0,0,0},
	/** 53 */{"times-of-day",0,0,'t'},
	/** 54 */{"day-times",0,0,'t'},
	/** 55 */{"israel",0,0,0},
	/** 56 */{"la-omer",0,0,0},
	/** 57 */{"ba-omer",0,0,0},
	/** 58 */{"quiet-alerts",0,0,0},
	/** 59 */{"quiet-gregorian",0,0,0},
	/** 60 */{"quiet-descriptions",0,0,0},
	/** 61 */{"quiet-hebrew",0,0,0},
	/** 62 */{"data-first",0,0,0},
	/** 63 */{"labels-first",0,0,0},
	/** eof*/{0, 0, 0, 0}
		};


	/*************************************************************
	* sub-function process_month
	* - This code was originally a normal part of main(). However,
	*   when I wanted the flexibility of having a single alphabetic
	*   month parameter accepted as the entire month for the
	*   current year, I needed to make this a function and, at least
	*   for now, am keeping it as a sub-function within main()
	**************************************************************/
	// TODO - consider moving this out of main()
	// The comments above seems to be nonsense - this function is only
	// called once, so it can be returned in-line to main
	void process_month()
	{
		/// handle errors
		// Hmm... suspicious. why so little checking here ...
		if (year <= 0) { print_parm_error(year_text); exit_main(&opt,0); }


		/************************************************************
		* process entire Hebrew month
		************************************************************/
		if (year > HEB_YR_LOWER_BOUND)
		{
			/// The parse_date function returns Hebrew month values in
			/// the range 101 - 114
			if (month > 100) month = month - 100;

			/// bounds check for month
			if (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h))
				{ print_parm_error(month_text); exit_main(&opt,0); }


			if (opt.holidays)
			{
				hdate_set_hdate(&h, 1, month, year);
				opt.custom_days_cnt = get_custom_days_list(
											&opt.jdn_list_ptr, &opt.string_list_ptr,
											0, month, year,
											'H', opt.quiet, h, "/hdate", "/custom_days",
											opt.short_format, opt.hebrew);
			}

			if (opt.tablular_output)
			{
				print_tabular_header( &opt );

				/// get date of month start
				hdate_set_hdate (&h, 1, month, year);

				/// if leap year, print both Adar months
				if (h.hd_size_of_year > 365 && month == 6)
				{
					print_tabular_hmonth ( &opt, 13, year);
					print_tabular_hmonth ( &opt, 14, year);
				}
				else
				{
					print_tabular_hmonth ( &opt, month, year);
				}
			}
			else
			{
				if (opt.iCal) print_ical_header ();

				/// get date of month start
				hdate_set_hdate (&h, 1, month, year);

				/// if leap year, print both Adar months
				if (h.hd_size_of_year > 365 && month == 6)
				{
					hdate_set_hdate (&h, 1, 13, year);
					print_hmonth (&h, &opt, 13, year);
					hdate_set_hdate (&h, 1, 14, year);
					print_hmonth (&h, &opt, 14, year);
				}
				else
				{
					print_hmonth (&h, &opt, month, year);
				}
				if (opt.iCal)
					print_ical_footer ();
			}
		}

		/************************************************************
		* process entire Gregorian month
		************************************************************/
		else
		{
			if ((month <= 0) || (month > 12))
				{ print_parm_error(month_text); exit_main(&opt,0); }

			if (opt.holidays)
			{
				hdate_set_gdate(&h, 1, month, year);
				opt.custom_days_cnt = get_custom_days_list(
										&opt.jdn_list_ptr, &opt.string_list_ptr,
										0, month, year,
										'G', opt.quiet, h, "/hdate", "/custom_days",
										opt.short_format, opt.hebrew);
			}

			if (opt.tablular_output)
			{
				print_tabular_header( &opt );
				print_tabular_gmonth( &opt, month, year);
			}

			else
			{
				if (opt.iCal)print_ical_header ();

				print_gmonth (&opt, month, year);

				if (opt.iCal) print_ical_footer ();
			}
		}
	}
	/*************************************************************
	* end sub-function process_month
	*************************************************************/


	/*************************************************************
	* BEGIN - main execution section of main ()
	*************************************************************/

	/// init locale - see man (3) setlocale, and see hcal.c for the full lecture...
	//  TODO - verify that I'm not needlessly doing this again (and again...)
	char* my_locale;
	my_locale = setlocale (LC_ALL, "");


	/// parse config file
	FILE *config_file = get_config_file("/hdate", "/hdaterc", hdate_config_file_text, opt.quiet);
	if (config_file != NULL)
	{
		read_config_file(config_file, &opt, &opt_latitude, &opt_Longitude, tz_name_str);
		fclose(config_file);
	}


	/// parse command line
	opterr = 0; /// we'll do our own error reporting
 	while ((c = getopt_long(argc, argv,
							short_options, long_options,
							&long_option_index)) != -1)
		error_detected = error_detected
						+ parameter_parser(c, &opt,
									&opt_latitude,
									&opt_Longitude,
									tz_name_str, long_option_index);


	/// undocumented feature - afikomen
	if (opt.afikomen)
	{
		if (opt.afikomen > 9) opt.afikomen = 9;
		printf("%s\n", afikomen[opt.afikomen-1]);
		exit(0);
	}


	/**************************************************
	* BEGIN - enable user-defined menu
	*************************************************/
	if (opt.menu)
	{
		i = menu_select( &opt.menu_item[0], MAX_MENU_ITEMS );
		if (i == -1) exit_main(&opt, 0);
		else if ((i < MAX_MENU_ITEMS) && (opt.menu_item[i] != NULL))
		{

			/**************************************************
			* parse user's menu selection
			*************************************************/
			menuptr = opt.menu_item[i];
			menu_len = strlen( menuptr );
			menu_index = 0;
			optptr = NULL;
			optarg = NULL;

			while (( c = menu_item_parse( menuptr, menu_len, &menu_index,
								&optptr, short_options, long_options,
								&long_option_index, &error_detected) ) != -1)
			{
				error_detected = error_detected + 
					parameter_parser(c, &opt, &opt_latitude, &opt_Longitude,
								tz_name_str, long_option_index);
			}
		}
	}
	/// only allow this option on the command line
	opt.afikomen = 0;
	/**************************************************
	* END   - enable user-defined menu
	*************************************************/



	#define PROCESS_NOTHING     0
	#define PROCESS_TODAY       1
	#define PROCESS_HEBREW_DAY  2
	#define PROCESS_GREGOR_DAY  3
	#define PROCESS_JULIAN_DAY  4
	#define PROCESS_MONTH       5
	#define PROCESS_HEBREW_YEAR 6
	#define PROCESS_GREGOR_YEAR 7
	int hdate_action = PROCESS_NOTHING;
	if (argc == optind)
	{
		hdate_set_gdate (&h, 0, 0, 0);
		hdate_action = PROCESS_TODAY;
	}
	else if (argc == (optind + 1))
	{
		// check if numeric first !!
		year = atoi (argv[optind]);
		if (year > JUL_DY_LOWER_BOUND)
		{
			hdate_set_jd (&h, year);
			hdate_action = PROCESS_JULIAN_DAY;
		}
		else
		{
			if (!parse_date( argv[optind], "", "", &year, &month, &day, 1))
				exit_main(&opt,0);
			if (month != BAD_DATE_VALUE) hdate_action = PROCESS_MONTH;
			else if (year > HEB_YR_LOWER_BOUND)
			{
				if (year > HEB_YR_UPPER_BOUND) { print_parm_error(year_text); exit_main(&opt,0); }
				hdate_action = PROCESS_HEBREW_YEAR;
			}
			else hdate_action = PROCESS_GREGOR_YEAR;
		}
	}
	else if (argc == (optind + 2))
	{

		if (!parse_date( argv[optind], argv[optind+1], "", &year, &month, &day, 2))
			exit_main(&opt,0);
		hdate_action = PROCESS_MONTH;
	}
	else if (argc == (optind + 3))
	{
		if (!parse_date( argv[optind], argv[optind+1], argv[optind+2], &year, &month, &day, 3))
			exit_main(&opt,0);
		if (year <= 0) { print_parm_error(year_text); exit_main(&opt,0); }
		if (year > HEB_YR_LOWER_BOUND)
		{
			/// The parse_date function returns Hebrew month values in
			/// the range 101 - 114
			if (month > 100) month = month - 100;

			/// bounds check for month
			if (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h))
				{ print_parm_error(month_text); exit_main(&opt,0); }

			/// bounds check for day
			if (!validate_hdate(CHECK_DAY_PARM, day, month, year, TRUE, &h))
				{ print_parm_error(day_text); exit_main(&opt,0); }

			hdate_set_hdate (&h, day, month, year);
			hdate_action = PROCESS_HEBREW_DAY;
		}
		else
		{
			/// bounds check for month
			if (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h))
				{ print_parm_error(month_text); exit_main(&opt,0); }

			/// bounds check for day
			if (!validate_hdate(CHECK_DAY_PARM, day, month, year, TRUE, &h))
				{ print_parm_error(day_text); exit_main(&opt,0); }

			hdate_set_gdate (&h, day, month, year);
			hdate_action = PROCESS_GREGOR_DAY;
		}

	}
	else
	{
		error(0,0,"%s", N_("too many arguments (expected at most day, month and year after options list)"));
		exit_main(&opt,0);
	}


	/************************************************************
	* functionprocess_location_parms is defined in the include file
	* ./local_functions.c
	* It issues an exit(EXIT_CODE_BAD_PARMS) [==1]
	* if it discovers, um, bad parameters 
	************************************************************/
	// be nice: make sure only one of tz and tz_name_str is set!
	int tz_absolute_val;
	tz_absolute_val = process_location_parms(
						opt_latitude, opt_Longitude,
						&opt.lat, &opt.lon, &opt.tz_offset, tz_name_str, 
						opt.quiet);
	// remember to free() tz_name_str
	// if user input tz as a value, it is absolute (no dst)
	// else try reading tzif /etc/localtime
	/// exit after reporting all bad parameters found */
	if (error_detected)
	{
		print_usage_hdate();
		print_try_help_hdate();
		exit(EXIT_CODE_BAD_PARMS);
	}

						
	/// diaspora-awareness
	//  TODO - this needs to get more sophisticated in step with
	//  increased sophistication in location and timezone awareness.
	if (opt.diaspora == -1)
	{
		if (opt.tz_offset == 2) opt.diaspora = 0;
		else opt.diaspora = 1;
	}
	

	/************************************************************
	* option "t" has three verbosity levels
	************************************************************/
	if (opt.times)
	{
		opt.first_light = 1;
		opt.talit = 1;
		opt.sunrise = 1;
		opt.midday = 1;
		opt.sunset = 1;
		opt.first_stars = 1;
		opt.three_stars = 1;
		opt.sun_hour = 1;
		opt.end_eating_chometz_ma = 1;
		opt.end_eating_chometz_gra = 1;
		opt.end_owning_chometz_ma = 1;
		opt.end_owning_chometz_gra = 1;
		
		if (opt.times > 1)
		{
			opt.shema = 1;
			opt.amidah = 1;
			opt.mincha_gedola = 1;
			opt.mincha_ketana = 1;
			opt.plag_hamincha = 1;
			if (opt.times > 2) opt.magen_avraham = 1;
		}
	}


	time_t t_start, t_end;
	struct tm t;
	t.tm_sec = 0; t.tm_min = 0; t.tm_hour = 0; t.tm_isdst = -1;
	switch (hdate_action)
{
case PROCESS_NOTHING    : break;
case PROCESS_TODAY      :
case PROCESS_HEBREW_DAY :
case PROCESS_GREGOR_DAY :
case PROCESS_JULIAN_DAY :
		t.tm_year = h.gd_year;
		t.tm_mon =  h.gd_mon;
		t.tm_mday = h.gd_day;
		t_start = mktime (&t);
		t.tm_mday += 1;
		t_end = mktime (&t);
		break;
case PROCESS_MONTH      :
		if (month > 100)
		{
			hdate_set_hdate (&h, 1, month-100, year);
			t.tm_year = h.gd_year;
			t.tm_mon =  h.gd_mon;
			t.tm_mday = h.gd_day;
			t_start = mktime (&t);
			hdate_set_jd (&h, h.hd_jd+30);
			t.tm_year = h.gd_year;
			t.tm_mon =  h.gd_mon;
			t.tm_mday = h.gd_day;
			t_end = mktime (&t);
		}
		else
		{
			hdate_set_gdate (&h, 1, month, year);
			t.tm_year = h.gd_year;
			t.tm_mon =  h.gd_mon;
			t.tm_mday = h.gd_day;
			t_start = mktime (&t);
			hdate_set_gdate (&h, 1, (month==12)?1:month+1, (month==12)?year+1:year);
			t.tm_year = h.gd_year;
			t.tm_mon =  h.gd_mon;
			t.tm_mday = h.gd_day;
			t_end = mktime (&t);
		}
		break;
case PROCESS_HEBREW_YEAR:
		hdate_set_hdate (&h, 1, 1, year);
		t.tm_year = h.gd_year;
		t.tm_mon =  h.gd_mon;
		t.tm_mday = h.gd_day;
		t_start = mktime (&t);
		hdate_set_hdate (&h, 1, 1, year+1);
		t.tm_year = h.gd_year;
		t.tm_mon =  h.gd_mon;
		t.tm_mday = h.gd_day;
		t_end = mktime (&t);
		break;
case PROCESS_GREGOR_YEAR:
		t.tm_year = year;
		t.tm_mon =  1;
		t.tm_mday = 1;
		t_start = mktime (&t);
		t.tm_year += 1;
		t_end = mktime (&t);
		break;
}
zdump( tz_name_str, t_start, t_end, &opt.tzif_entries,  &opt.tzif_data );
// opt.tzif_data must be free()d
opt.today_time = t_start - SECONDS_PER_DAY;

	switch (hdate_action)
	{
case PROCESS_TODAY:
		if (opt.holidays)
		{
			opt.custom_days_cnt = get_custom_days_list(
										&opt.jdn_list_ptr, &opt.string_list_ptr,
										h.hd_day, h.hd_mon, h.hd_year,
										'H', opt.quiet, h, "/hdate", "/custom_days",
										opt.short_format, opt.hebrew);
		}
		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_day(&h, &opt);
		}
		else
		{
			if (opt.iCal) print_ical_header ();
			else if (!opt.not_sunset_aware)
				opt.print_tomorrow = check_for_sunset(&h, opt.lat, opt.lon, opt.tz_offset);
			print_day (&h, &opt);
			if (opt.iCal) print_ical_footer ();
		}
		break;
case PROCESS_JULIAN_DAY:
		if (opt.holidays)
		{
			opt.custom_days_cnt = get_custom_days_list(
										&opt.jdn_list_ptr, &opt.string_list_ptr,
										h.hd_day, h.hd_mon, h.hd_year,
										'H', opt.quiet, h, "/hdate", "/custom_days",
										opt.short_format, opt.hebrew);
		}

		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_day(&h, &opt);
		}
		else
		{
case PROCESS_MONTH:
		process_month();
		break;
case PROCESS_HEBREW_YEAR:
		if (opt.holidays)
		{
			hdate_set_hdate(&h, 1, 1, year);
			opt.custom_days_cnt = get_custom_days_list(
										&opt.jdn_list_ptr, &opt.string_list_ptr,
										0, 0, year,
										'H', opt.quiet, h, "/hdate", "/custom_days",
										opt.short_format, opt.hebrew);
		}

		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_hyear( &opt, year);
		}
		else
		{
			if (opt.iCal) print_ical_header ();
			print_hyear ( &opt, year);
			if (opt.iCal) print_ical_footer ();
		}
		break;
case PROCESS_GREGOR_YEAR:
		if (opt.holidays)
		{
			hdate_set_gdate(&h, 1, 1, year);
			opt.custom_days_cnt = get_custom_days_list(
									&opt.jdn_list_ptr, &opt.string_list_ptr,
									0, 0, year,
									'G', opt.quiet, h, "/hdate", "/custom_days",
									opt.short_format, opt.hebrew);
		}

		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_gyear( &opt, year);
		}
		else
		{
			if (opt.iCal) print_ical_header ();
			print_gyear ( &opt, year);
			if (opt.iCal) print_ical_footer ();
		}
		break;
case PROCESS_HEBREW_DAY:
		if (opt.holidays)
		{
			opt.custom_days_cnt = get_custom_days_list(
									&opt.jdn_list_ptr, &opt.string_list_ptr,
									h.hd_day, h.hd_mon, h.hd_year,
									'H', opt.quiet, h, "/hdate", "/custom_days",
									opt.short_format, opt.hebrew);
		}
		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_day(&h, &opt);
		}

		else
		{
			if (opt.iCal) print_ical_header ();
			print_day (&h, &opt);
			if (opt.iCal) print_ical_footer ();
		}
		break;
case PROCESS_GREGOR_DAY:
		if (opt.holidays)
		{
			opt.custom_days_cnt = get_custom_days_list(
									&opt.jdn_list_ptr, &opt.string_list_ptr,
									h.gd_day, h.gd_mon, h.gd_year,
									'G', opt.quiet, h, "/hdate", "/custom_days",
									opt.short_format, opt.hebrew);
		}
		if (opt.tablular_output)
		{
			print_tabular_header( &opt );
			print_tabular_day(&h, &opt);
		}

		else
		{
			if (opt.iCal) print_ical_header ();
			print_day (&h, &opt);
			if (opt.iCal) print_ical_footer ();
		}
		break;
	} /// end of switch (hdate_action)
	}

	return 0;
}
