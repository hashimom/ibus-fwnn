/* vim:set et sts=4: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <wnn/jslib.h>

#include "fwnnserver.h"

#define FZK_FILE	"pubdic/full.fzk"
#define KIHON_DIC	"pubdic/kihon.dic"
#define SETTO_DIC	"pubdic/setsuji.dic"
#define CHIMEI_DIC	"pubdic/chimei.dic"
#define JINMEI_DIC	"pubdic/jinmei.dic"
#define KOYUU_DIC	"pubdic/koyuu.dic"
#define BIO_DIC		"pubdic/bio.dic"
#define PC_DIC		"pubdic/computer.dic"
#define SYMBOL_DIC	"pubdic/symbol.dic"

#define MAX_CONV_STRLEN	(1024)
#define CONV_UTF82EUC	(0)
#define CONV_EUC2UTF8	(1)

static WNN_JSERVER_ID *jserver;
static WNN_ENV *wnnenv;
static struct wnn_ret_buf wnnbuf= {0, NULL};

/* もちろん仮。malloc して返すように変更する予定 */
static unsigned char wnn_yomi_buf[MAX_CONV_STRLEN];
static unsigned char wnn_out_kanstr[MAX_CONV_STRLEN];


static void set_wnn_env_pram()
{
	struct wnn_param pa;
	pa.n= 2;		/* n_bun */
	pa.nsho = 10;	/* nshobun */
	pa.p1 = 2;	/* hindoval */
	pa.p2 = 40;	/* lenval */
	pa.p3 = 0;	/* jirival */
	pa.p4 = 100;	/* flagval */
	pa.p5 = 5;	/* jishoval */
	pa.p6 = 1;	/* sbn_val */
	pa.p7 = 15;	/* dbn_len_val */
	pa.p8 = -20;	/* sbn_cnt_val */
	pa.p9 = 0;	/* kan_len_val */

	js_param_set(wnnenv, &pa);
}

static void exec_iconv(char *instr, char *outstr, int ch)
{
	iconv_t cd;
	size_t src_len = strlen(instr);
	size_t dest_len = MAX_CONV_STRLEN - 1;
	
	if (ch == CONV_UTF82EUC)
		cd = iconv_open("EUC-JP", "UTF-8");
	else
		cd = iconv_open("UTF-8", "EUC-JP");
	
	memset(outstr, '\0', MAX_CONV_STRLEN);
	
	iconv(cd, &instr, &src_len, &outstr, &dest_len);
	iconv_close(cd);
}

static void strtows(w_char *u, unsigned char *e)
{
	int x;
	for(;*e;){
		x= *e++;
		if(x & 0x80)
			x = (x << 8)  | *e++;
		*u++= x;
	}
	*u=0;
}

static int putws(unsigned short *s, unsigned char *outstr)
{
	int ret = 0;
	
	while(*s) {
		outstr[ret] = *s >> 8;
		outstr[ret + 1] =  *s;
		ret = ret + 2;
		s++;
	}
	return ret;
}

static void output_js2char(struct wnn_dai_bunsetsu *dlist, int cnt)
{
	int i, tmpbuf_len;
	struct wnn_sho_bunsetsu  *sbn;
	unsigned char kanstr_tmp[MAX_CONV_STRLEN];
	unsigned char kanstr_utf8[MAX_CONV_STRLEN];

    for ( ; cnt > 0; dlist++, cnt --) {
		sbn = dlist->sbn;
		for (i = dlist->sbncnt; i > 0; i--) {
			/* 単文節ごとに解析 */
			memset(kanstr_tmp, '\0', MAX_CONV_STRLEN);
			memset(kanstr_utf8, '\0', MAX_CONV_STRLEN);
			tmpbuf_len = putws(sbn->kanji, kanstr_tmp); 
			tmpbuf_len = putws(sbn->fuzoku, kanstr_tmp + tmpbuf_len);
			
			/* 文字コード変換して格納 */
			exec_iconv(kanstr_tmp, kanstr_utf8, CONV_EUC2UTF8);
			strncat(wnn_out_kanstr, kanstr_utf8, strlen(kanstr_utf8));
			sbn++;
		}
    }
}

int fwnnserver_open()
{
	int ret = -1;
	int fzkfile;

	jserver = js_open(NULL, 0);
	if (jserver != NULL) {
		wnnenv = js_connect(jserver, "kana");
		ret = js_isconnect(wnnenv);
		if (ret == 0) {
			wnnbuf.buf = (char *)malloc((unsigned)(wnnbuf.size = 0));
			set_wnn_env_pram();
			
			fzkfile = js_file_read(wnnenv, FZK_FILE);
			js_fuzokugo_set(wnnenv, fzkfile);
			
			fwnnserver_adddic(KIHON_DIC);
			fwnnserver_adddic(SETTO_DIC);
			fwnnserver_adddic(CHIMEI_DIC);
			fwnnserver_adddic(JINMEI_DIC);
			fwnnserver_adddic(KOYUU_DIC);
			fwnnserver_adddic(BIO_DIC);
			fwnnserver_adddic(PC_DIC);
			fwnnserver_adddic(SYMBOL_DIC);
		}
	}
	
	return ret;
}

int fwnnserver_close()
{
	int ret = -1;
	
	if (js_disconnect(wnnenv) == 0) {
		ret = js_close(jserver);
		if (wnnbuf.buf)
			free(wnnbuf.buf);
	}
	return ret;
}

int fwnnserver_adddic(char *dicfilename)
{
	int dicno = -1;
	
	dicno = js_file_read(wnnenv, dicfilename);
	if (dicno != -1) {
		js_dic_add(wnnenv, dicno, -1,
				WNN_DIC_ADD_NOR,1,WNN_DIC_RDONLY, 
				WNN_DIC_RDONLY, NULL, NULL);
	}
	
	return dicno;
}

unsigned char *fwnnserver_kanren(unsigned char *yomi)
{
	// とりあえずサイズ決め打ち
	unsigned char yomi_euc[MAX_CONV_STRLEN];
	w_char upstrings[MAX_CONV_STRLEN];
	int count = 0;
	int i=0, yomilen = strlen(yomi);

	// 未確定文字ポインタ（暫定実装で削除予定）
	for (i = 0; i < yomilen;) {
		if (yomi[i] < 0x7F)
			break;
		i += 3;
	}
	
	memset(yomi_euc, 0, MAX_CONV_STRLEN);
	memset(upstrings, 0, MAX_CONV_STRLEN);
	memset(wnn_out_kanstr, 0, MAX_CONV_STRLEN);
	
	exec_iconv(yomi, yomi_euc, CONV_UTF82EUC);
	strtows(upstrings, yomi_euc);
	
	count = js_kanren(wnnenv, upstrings, WNN_ALL_HINSI, NULL,
			WNN_VECT_KANREN, WNN_VECT_NO, WNN_VECT_BUNSETSU,&wnnbuf);
	output_js2char((struct wnn_dai_bunsetsu *)wnnbuf.buf, count);

	if (i < yomilen)
		strcat(wnn_out_kanstr, yomi+i);

	return wnn_out_kanstr;
}


