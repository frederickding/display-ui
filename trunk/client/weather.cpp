
#include <urlmon.h>
#include <stdio.h>
#include "rhhstv.h"
#include "weather.h"
#include "debug.h"



weather_t current;

int weather_update(){
	if(download("http://du.geekie.org/server/api/weather/current/?sys_name=1&sig=%s&location=CAXX0401", "data\\weather\\weather_c.dat") == S_OK){
		
		FILE *fp = fopen("data\\weather\\weather_c.dat", "r");
		
		if(fp){
			int temp = 0;
			fscanf(fp, "%d", &current.condition);
			fscanf(fp, "%d", &temp);
			
			// stupid gdi+ needs widechars.
			swprintf(current.temp, L"%d°C", temp);
			
			int length;
			fscanf(fp, "%d ", &length);
			if(current.description) free(current.description);
			current.description = (wchar_t *) malloc((length + 1) * 2);
			
			char *tmp = (char *) malloc(length + 1);
			memset(tmp, 0, length + 1);
			memset(current.description, 0, (length + 1) * 2);
			fgets(tmp, length + 1, fp);
			
			debug_print("desc: %s\n", tmp);
			
			mbstowcs(current.description, (const char *) tmp, strlen(tmp));
			
			fclose(fp);
			
			char imgurl[48];
			sprintf(imgurl, "http://l.yimg.com/a/i/us/nws/weather/gr/%dd.png", current.condition);
			URLDownloadToFile(NULL, imgurl, "img\\current.png", 0, NULL);
			
			return 0;
		}else{
			return 1;
		}
	}
}

weather_t *weather_getcurrent(){
	return &current;
}
