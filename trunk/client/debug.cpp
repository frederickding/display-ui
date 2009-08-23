#include <stdio.h>
#include <windows.h>
#include "debug.h"

FILE *log;

void debug_init(){
	log = fopen("log.txt", "w");
}

int debug_print(char *format, ...){
	char buf[512];
	memset(buf, 0, 512);
	va_list args; va_start(args, format);	
	int i = vsprintf(buf, format, args);
	fprintf(log, buf);	
	fflush(log);
	va_end(args);
	return i;
}

void debug_close(){
	
	fclose(log);
}
