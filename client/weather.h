#ifndef weather_header
#define weather_header

#include <wchar.h>

typedef struct _weather_t{
	int condition;
	wchar_t temp[6];
	wchar_t *description;
}weather_t;

int weather_update();
weather_t *weather_getcurrent();

#endif

