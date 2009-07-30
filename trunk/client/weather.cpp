/**
 * weather.cpp, weather API functionality
 *
 * Copyright 2009 Kirill Peretoltchine
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 *      http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Id$
 */

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <time.h>

#include "display_ui.h"
#include "weather.h"
#include "debug.h"



weather_t current;
weather_fc_t **forecast = (weather_fc_t **) 0;

void weather_init() {
	forecast = (weather_fc_t **) malloc(sizeof(weather_fc_t *) * 2);
	forecast[0] = (weather_fc_t *) malloc(sizeof(weather_fc_t));
	forecast[1] = (weather_fc_t *) malloc(sizeof(weather_fc_t));
	memset(forecast[0], 0, sizeof(weather_fc_t));
	memset(forecast[1], 0, sizeof(weather_fc_t));

	// weather_update(NULL);
}

void weather_exit() {
	if(forecast[0]) free(forecast[0]);
	if(forecast[1]) free(forecast[1]);
	if(forecast) free(forecast);
}

unsigned long weather_update(void *p){
	if(download("http://du.geekie.org/server/api/weather/current/?sys_name=1&sig=%s&location=CAXX0401", "data\\weather\\weather_c.dat") == S_OK) {

		FILE *fp = fopen("data\\weather\\weather_c.dat", "r");

		if(fp) {
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
			time_t rawtime;
			tm *ptm;
			time (&rawtime);
			ptm = localtime(&rawtime);
			sprintf(imgurl, "http://l.yimg.com/a/i/us/nws/weather/gr/%d%c.png", current.condition, (ptm->tm_hour < 6 || ptm->tm_hour > 19) ? 'n' : 'd');
			URLDownloadToFile(NULL, imgurl, "img\\current.png", 0, NULL);

		} else {
			return 1;
		}
	}

	if(download("http://du.geekie.org/server/api/weather/forecast/?sys_name=1&sig=%s&location=CAXX0401&day=2", "data\\weather\\weather_fc.dat") == S_OK) {
		FILE *fp = fopen("data\\weather\\weather_fc.dat", "r");

		if(fp) {
			for(int i = 0; i < 2; i++){
				int temp_hi = 0;
				int temp_lo = 0;
				fscanf(fp, "\n%d", &forecast[i]->condition);
				fscanf(fp, "%d", &temp_hi);
				fscanf(fp, "%d", &temp_lo);

				// stupid gdi+ needs widechars.
				swprintf(forecast[i]->temp_hi, L"%d°", temp_hi);
				swprintf(forecast[i]->temp_lo, L"%d", temp_lo);

				int length;
				fscanf(fp, "%d ", &length);
				if(forecast[i]->description) free(forecast[i]->description);
				forecast[i]->description = (wchar_t *) malloc((length + 1) * 2);

				// MessageBox(NULL, NULL, NULL, NULL);
				char *tmp = (char *) malloc(length + 1);
				memset(tmp, 0, length + 1);
				memset(forecast[i]->description, 0, (length + 1) * 2);
				fgets(tmp, length + 1, fp);

				debug_print("desc: %s\n", tmp);

				mbstowcs(forecast[i]->description, (const char *) tmp, strlen(tmp));

				char imgurl[48];
				char filename[19];
				sprintf(imgurl, "http://l.yimg.com/a/i/us/nws/weather/gr/%dd.png", forecast[i]->condition);
				sprintf(filename, "img\\forecast_%d.png", i);
				debug_print("%s -> %s\n", imgurl, filename);
				URLDownloadToFile(NULL, imgurl, filename, 0, NULL);
			}
			fclose(fp);

		} else {
			return 1;
		}
	}
	RECT rt;
	rt.top = 112; rt.left = 980; rt.right = 1280; rt.bottom = 664;
	InvalidateRect((HWND) p, &rt, false);

	return 0;
}

weather_t *weather_getcurrent() {
	return &current;
}

weather_fc_t **weather_getforecast() {
	return forecast;
}
