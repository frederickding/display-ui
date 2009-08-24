#ifndef config_header
#define config_header

int config_load();
void config_generate_sig();

char *config_get_sig();
char *config_get_url();
char *config_get_sysname();

#endif