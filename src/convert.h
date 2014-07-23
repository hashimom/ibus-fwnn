/* vim:set et sts=4: */
#ifndef __CONVERT_H__
#define __CONVERT_H__

#include <ibus.h>

int conv_run_romajiconv(GString *gStr, gssize curpos);
int conv_get_bytesize_laststr(GString *gStr, gssize curpos);
int conv_get_bytesize_nextstr(GString *gStr, gssize curpos);

#endif
