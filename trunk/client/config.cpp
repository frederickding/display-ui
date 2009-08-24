#include <windows.h>
#include <stdio.h>


#include "display_ui.h"
#include "debug.h"

char g_api_key[64];

int config_load(){
	GetProfileString("config", "api_key", "undefined", g_api_key, 64);
	debug_print("[config_load] api_key = %s\n", g_api_key);


	return 0;
}