#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "display_ui.h"
#include "sha1.h"
#include "debug.h"

char g_api_key[65];
char g_sys_name[65];
char g_sig[41];
char g_version[24];
char g_winver[12];
char g_useragent[128];

// length of server url cannot exceed 255 characters!
char g_server_url[256];

// Load data from .ini file.
int config_load(){
	char path[MAX_PATH + 16];
	
	GetCurrentDirectory(MAX_PATH, path);
	strcat_s(path, MAX_PATH + 16, "\\display_ui.ini");
	debug_print("[config_load] ini path: %s\n", path);
	
	GetPrivateProfileString("system", "api_key", "undefined", g_api_key, 65, path);
	debug_print("[config_load] api_key = %s\n", g_api_key);
	
	GetPrivateProfileString("system", "sys_name", "undefined", g_sys_name, 65, path);
	debug_print("[config_load] sys_name = %s\n", g_sys_name);
	
	GetPrivateProfileString("system", "server_location", "undefined", g_server_url, 256, path);
	debug_print("[config_load] server_url = %s\n", g_server_url);
	
	DWORD version_info_size = GetFileVersionInfoSize("display_ui.exe", NULL);
	char *version_info = (char *)malloc(version_info_size);
	GetFileVersionInfo("display_ui.exe", NULL, version_info_size, version_info);
	
	VS_FIXEDFILEINFO *vs_ffi;// = (VS_FIXEDFILEINFO *) malloc(sizeof(VS_FIXEDFILEINFO));

	unsigned int vs_ffi_size;
	VerQueryValue(version_info, "\\", (void **)&vs_ffi, &vs_ffi_size);
	debug_print("[config_load] vs_ffi: given %d, req'd %d\n", sizeof(VS_FIXEDFILEINFO), vs_ffi_size);

	sprintf_s(g_version, "%d.%d.%d.%d", HIWORD(vs_ffi->dwFileVersionMS), LOWORD(vs_ffi->dwFileVersionMS), 
		HIWORD(vs_ffi->dwFileVersionLS), LOWORD(vs_ffi->dwFileVersionLS));
	
	debug_print("[config_load] file version: %s\n", g_version);
	
	//free(vs_ffi);
	free(version_info);
	
	
	// see link for how to read windows versions:
	// http://msdn.microsoft.com/en-us/library/ms724834%28VS.85%29.aspx

	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osver);

	sprintf_s(g_winver, "%d.%d", osver.dwMajorVersion, osver.dwMinorVersion);
	debug_print("[config_load] windows version: %s\n", g_winver);
	
	sprintf_s(g_useragent, "Mozilla/5.0 (compatible; Windows NT %s) DisplayUIClient/%s", g_winver, g_version);
	
	debug_print("[config_load] user agent: %s\n", g_useragent);
	
	return 0;
}

// Signature is a SHA1 hash of the date in Y-m-d format appended to the api key
void config_generate_sig() {
	char date[11];
	time_t rawtime;
	tm newtime;


	debug_print("[config_generate_sig] api key: %s\n", g_api_key);
	debug_print("[config_generate_sig] sys_name: %s\n", g_sys_name);
	debug_print("[config_generate_sig] g_server_url: %s\n", g_server_url);

	time(&rawtime);
	gmtime_s(&newtime, &rawtime);
	
	strftime(date, 11, "%Y-%m-%d", &newtime);
	
	debug_print("[config_generate_sig] date: %s\n", date);

	memset(g_sig, 0, 41);
	
	// SHA1 hash the produced string
	SHA1Context sha;
	uint8_t Message_Digest[20];
	SHA1Reset(&sha);
	SHA1Input(&sha, (const unsigned char *) g_api_key, strlen(g_api_key));
	SHA1Input(&sha, (const unsigned char *) date, strlen(date));
	if(!SHA1Result(&sha, Message_Digest)) {
		for(int i = 0; i < 20; i++) {
			char temp[3];
			memset(temp, 0, 3);
			sprintf_s(temp, 3, "%02x", Message_Digest[i]);
			strcat_s(g_sig, 41, temp);
		}
	} else {
		// TODO: handle errors
	}
	debug_print("[config_generate_sig] sig: %s\n", g_sig);
}

char *config_get_sig(){
	return g_sig;
}

char *config_get_url(){
	return g_server_url;
}

char *config_get_sysname(){
	return g_sys_name;
}

char *config_get_version(){
	return g_version;
}

char *config_get_winver(){
	return g_winver;
}

char *config_get_useragent(){
	return g_useragent;
}