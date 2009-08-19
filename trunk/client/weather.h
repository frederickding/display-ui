#ifndef weather_header
#define weather_header

#include <wchar.h>

typedef struct _weather_t{
	int condition;
	char temp[6];
	char *description;
}weather_t;

typedef struct _weather_fc_t{
	int condition;
	char temp_hi[6];
	char temp_lo[6];
	char *description;
}weather_fc_t;

void weather_init();
void weather_exit();
unsigned long weather_update(void *p);
weather_t *weather_getcurrent();
weather_fc_t **weather_getforecast();

FIBITMAP *weather_getimage_current();
FIBITMAP *weather_getimage_fc(int num);

#endif






















