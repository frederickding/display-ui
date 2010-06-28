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
//#define CURL_STATICLIB

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <time.h>

#include <curl\curl.h>

#include "display_ui.h"
#include "freeimage.h"
#include "weather.h"
#include "debug.h"


FIBITMAP *fbmp_weather_current = NULL;
FIBITMAP *fbmp_weather_fc[2] = {NULL, NULL};

weather_t current;
weather_fc_t **forecast = (weather_fc_t **) 0;
bool initialized = false;

// Initialize weather
void weather_init() {
	forecast = (weather_fc_t **) malloc(sizeof(weather_fc_t *) * 2);
	forecast[0] = (weather_fc_t *) malloc(sizeof(weather_fc_t));
	forecast[1] = (weather_fc_t *) malloc(sizeof(weather_fc_t));
	memset(forecast[0], 0, sizeof(weather_fc_t));
	memset(forecast[1], 0, sizeof(weather_fc_t));

	initialized = true;
	
	// weather_update(NULL);
}

// Unitialize weather
// TODO: free weather icons
void weather_exit() {
	if(forecast[0]) free(forecast[0]);
	if(forecast[1]) free(forecast[1]);
	if(forecast) free(forecast);
}

// Update weather info.
// params:
//		p:		 	  HWND of the window in which weather is being drawn.
//		download_new: If false, loads existing weather info from file.
unsigned long weather_update(void *p, bool download_new){
	
	int result = -1;

	if(download_new){
		result = dui_download_retry("/api/weather/current/?location=CAXX0401", "data\\weather\\weather_c.dat");
	}

	// Read from the file if download was OK, or if redownload wasn't requested
	if(result == CURLE_OK || !download_new){
		FILE *fp;

		if(!fopen_s(&fp, "data\\weather\\weather_c.dat", "r")){
			int temp = 0;
			fscanf_s(fp, "%d", &current.condition);
			fscanf_s(fp, "%d", &temp);
			
			// YESSSS no more widechar crap. GDI owns.
			sprintf_s(current.temp, 6, "%d°C", temp);

			int length;
			fscanf_s(fp, "%d ", &length);
			if(current.description) free(current.description);
			current.description = (char *) malloc(length + 2);

			memset(current.description, 0, length + 2);
			fgets(current.description, length + 1, fp);

			debug_print("[weather_update] current description at %08lX (%d)\n", current.description, length);
			
			fclose(fp);

			char imgurl[48];
			time_t rawtime;
			tm newtime;
			time (&rawtime);
			localtime_s(&newtime, &rawtime);

			// Create URL based on condition and time of day
			sprintf_s(imgurl, 48, "http://l.yimg.com/a/i/us/nws/weather/gr/%d%c.png", current.condition, (newtime.tm_hour < 6 || newtime.tm_hour > 19) ? 'n' : 'd');
			
			// Download a weather icon
			download_curl(imgurl, "img\\current.png");
			
			// If an image is already loaded, unload that one.
			if(fbmp_weather_current){
				FreeImage_Unload(fbmp_weather_current);
				fbmp_weather_current = NULL;
			}	
			fbmp_weather_current = FreeImage_Load(FIF_PNG, "img\\current.png", NULL);
			
			// Resize it to the appropriate size. This is done here to avoid lag during redrawing. FreeImage's
			// rescaling is way better than StretchDIBits, anyways.
			FIBITMAP *temp_img = fbmp_weather_current;
			fbmp_weather_current = FreeImage_Rescale(fbmp_weather_current, 250, 180, FILTER_BICUBIC);
			FreeImage_Unload(temp_img);
			FreeImage_PreMultiplyWithAlpha(fbmp_weather_current);
		

		} else {
			return 1;
		}
	}

	if(download_new){
		result = dui_download_retry("/api/weather/forecast/?location=CAXX0401&day=2", "data\\weather\\weather_fc.dat");
	}
	
	// If forecast info was downloaded correctly, or if no download was requested, read from the data file.
	if(result == CURLE_OK || !download_new) {
		FILE *fp;
		
		if(!fopen_s(&fp, "data\\weather\\weather_fc.dat", "r")) {
			for(int i = 0; i < 2; i++){
				int temp_hi = 0;
				int temp_lo = 0;
				fscanf_s(fp, "\n%d", &forecast[i]->condition);
				fscanf_s(fp, "%d", &temp_hi);
				fscanf_s(fp, "%d", &temp_lo);

				sprintf_s(forecast[i]->temp_hi, 6, "%d°", temp_hi);
				sprintf_s(forecast[i]->temp_lo, 6, "%d", temp_lo);

				int length;
				fscanf_s(fp, "%d ", &length);
				if(forecast[i]->description) free(forecast[i]->description);
				forecast[i]->description = (char *) malloc(length + 1);

				memset(forecast[i]->description, 0, length + 1);
				fgets(forecast[i]->description, length + 1, fp);

				debug_print("[weather_update -- forecast] Day %d: %s/%s [%s]\n", i, forecast[i]->temp_hi, forecast[i]->temp_lo, forecast[i]->description);

				char imgurl[48];
				char filename[19];

				// Create URL based on condition and filename based on FC day number
				sprintf_s(imgurl, 48, "http://l.yimg.com/a/i/us/nws/weather/gr/%dd.png", forecast[i]->condition);
				sprintf_s(filename, 19, "img\\forecast_%d.png", i);
				
				download_curl(imgurl, filename);
				
				// If the image already exists, unload it
				if(fbmp_weather_fc[i]){
					FreeImage_Unload(fbmp_weather_fc[i]);
					fbmp_weather_fc[i] = NULL;
				}	
				fbmp_weather_fc[i] = FreeImage_Load(FIF_PNG, filename, NULL);
				
				// Resize here to avoid lag later.
				FIBITMAP *temp_img = fbmp_weather_fc[i];
				fbmp_weather_fc[i] = FreeImage_Rescale(fbmp_weather_fc[i], 188, 135, FILTER_BICUBIC);
				FreeImage_Unload(temp_img);

				FreeImage_PreMultiplyWithAlpha(fbmp_weather_fc[i]);
			}
			fclose(fp);

		} else {
			return 1;
		}
	}

	// Tell Windows to send a WM_PAINT message for the weather panel rectangle, as it needs to be redrawn.
	RECT rt;
	rt.top = CONTENT_TOP; rt.left = CONTENT_WIDTH; rt.right = SCREEN_WIDTH; rt.bottom = CONTENT_BOTTOM;
	InvalidateRect((HWND) p, &rt, false);

	return 0;
}

weather_t *weather_getcurrent() {
	if(initialized)
		return &current;
	return NULL;
}

weather_fc_t **weather_getforecast() {
	if(initialized)
		return forecast;
	return NULL;
}


FIBITMAP *weather_getimage_current(){
	return fbmp_weather_current;
}

FIBITMAP *weather_getimage_fc(int num){
	if(num > 1) return NULL;

	return fbmp_weather_fc[num];
}
