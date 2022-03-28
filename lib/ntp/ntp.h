
#include <time.h>                   // time() ctime()

const float GMT = 0;
const float UTC = 0;
const float ECT = 1.00;
const float EET = 2.00;
const float ART = 2.00;
const float EAT = 3.00;
const float MET = 3.30;
const float NET = 4.00;
const float PLT = 5.00;
const float IST = 5.30;
const float BST = 6.00;
const float VST = 7.00;
const float CTT = 8.00;
const float JST = 9.00;
const float ACT = 9.30;
const float AET = 10.00;
const float SST = 11.00;
const float NST = 12.00;
const float MIT = -11.00;
const float HST = -10.00;
const float AST = -9.00;
const float PST = -8.00;
const float PNT = -7.00;
const float MST = -7.00;
const float CST = -6.00;
const float EST = -5.00;
const float IET = -5.00;
const float PRT = -4.00;
const float CNT = -3.30;
const float AGT = -3.00;
const float BET = -3.00;
const float CAT = -1.00;

const int EPOCH_1_1_2019 = 1546300800; //1546300800 =  01/01/2019 @ 12:00am (UTC)
const char *DAYS_OF_WEEK[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *DAYS_OF_WEEK_3[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};


void setuptime() ;
void looptime () ;
extern tm tm;   


String localTime();
