/* vim:set et sts=4: */
#ifndef __FWNN_H__
#define __FWNN_H


int fwnnserver_open();
int fwnnserver_close();
int fwnnserver_adddic(char *dicfilename);
unsigned char *fwnnserver_kanren(unsigned char *yomi);

#endif
