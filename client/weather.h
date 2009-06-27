#ifndef weather_header
#define weather_header

#include <wchar.h>

typedef struct _weather_t{
	int condition;
	wchar_t temp[6];
	wchar_t *description;
}weather_t;

typedef struct _weather_fc_t{
	int condition;
	wchar_t temp_hi[6];
	wchar_t temp_lo[6];
	wchar_t *description;
}weather_fc_t;

void weather_init();
void weather_exit();
int weather_update();
weather_t *weather_getcurrent();
weather_fc_t **weather_getforecast();

#endif

