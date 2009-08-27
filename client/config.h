#ifndef config_header
#define config_header

int config_load();
void config_generate_sig();

char *config_get_sig();
char *config_get_url();
char *config_get_sysname();
char *config_get_version();
char *config_get_winver();
char *config_get_useragent();


#endif
