/*
	Runtime Rountines For SX-BASIC Compiler

	Programmed By ISHIGAMI tatsuya
    9/25/1995 : Originally create for SX-Basic
	1/30/2025 : Updated for Reiwa Oh!X for X-Basic

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<stdarg.h>


#include	"ab2c_run.h"


#define	FILE_MAX		10	/* 一度に開けるファイルの数 */

static	FILE	*_fpfn_table[FILE_MAX];

static	char	*runtimebuff;		/* temporary string buffer */
static	char	*_sxb_str_buff;		/* string buffer */
static	int		_sxb_strS_ptr;		/* pointer for string buffer */

void
main(void)
{
	int	ret;

	/* アプリケーションの初期化 */
	ret = _sxb_Initialize();

	if( ret < 0 )
		exit ( ret );

	_sxb_start();
}


/*
 * Initialize runtime routine
 */
int
_sxb_Initialize( void )
{
	int i;

	for (i = 0; i < FILE_MAX; i++) {
		_fpfn_table[i] = NULL;
	}
	return(NULL);
}


char
*_sxb_add(char *s, ...)
{
	char	*p;
	va_list	ap;
	static	char	buff[1000];

	va_start(ap, s);

	strcpy(buff, s);
	while(1) {
		p = va_arg(ap, char*);
		if(p == (char *)-1) break;

		strcat(buff, p);
	}
	va_end(ap);
	return(buff);
}

int
_sxb_strcmpGE(char *s ,char *t)
{
	while (*s) {
		if (*t == '\0') {
			return TRUE;
		}
		if (*s > *t) {
			return TRUE;
		}
		if (*s < *t) {
			return FALSE;
		}
		s++;
		t++;
	}
	return(TRUE);
}

int
_sxb_strcmpNE(char *s, char *t)
{
	if(strcmp(s, t) != NULL)
		return(TRUE);
	else
		return(FALSE);
}

int
_sxb_strcmpLE(char *s, char *t)
{
	if(strcmp(s, t) <= 0)
		return(TRUE);
	else
		return(FALSE);
}

int
_sxb_strcmpLT(char *s, char *t)
{
	if(strcmp(s, t) < 0)
		return(TRUE);
	else
		return(FALSE);
}

int
_sxb_strcmpGT(char *s, char *t)
{
	if(strcmp(s, t) > 0)
		return(TRUE);
	else
		return(FALSE);
}

int
_sxb_strcmpEQ(char *s ,char *t)
{
	if(strcmp(s, t) == 0)
		return(TRUE);
	else
		return(0);
}


int
_sxb_ascS(char* s)
{
	if (s == NULL)
		return 0;
	return (int) *s;
}

char
*_sxb_bin(int i)
{
	static char	buff[40];

	sprintf(buff, "%40b", i);
	return(buff);
}

char
*_sxb_chrS(int i)
{
	static	char	buff[2];

	buff[0] = i;
	buff[1] = '\0';
	return(buff);
}

char
*_sxb_hexS(int i)
{
	static char	buff[40];

	sprintf(buff, "%X", i);
	return(buff);
}

char
*_sxb_rightS(char *s, int i)
{
	static	char buff[1000];
	int		len;
	char	*p = s;

	i--;
	len = strlen(s);
	if(len > i)
		p = &s[len - i];
	strncpy(buff, p, sizeof(buff));
	return(buff);
}

char
*_sxb_leftS(char *s, int i)
{
	int		len;

	len = strlen(s);
	if(len < i) i = len;

	strncpy(runtimebuff, s, len);
	return(runtimebuff);
}

char
*_sxb_midS(char *s, int i, int j)
{
	int		len;

	/* count starts from 1 */
	if(i > 0) i--;

	if(i >= strlen(s))	return("");

	s = &s[i];
	len = strlen(s);
	j = min(j, len);

	strncpy(runtimebuff, s, j);
	runtimebuff[j] = '\0';
	return(runtimebuff);
}

char
*_sxb_mirrorS(char *s)
{
	char	*p = runtimebuff;
	int		len;

	len = strlen(s);
	s = &s[len];

	while(len-- > 0) {
		*p++ = *s--;
	}
	*p++ = '\0';
	return(runtimebuff);
}


char
*_sxb_octS(int i)
{
	static	char	buff[30];
	sprintf(buff, "%o", i);
	return(buff);
}


char
*_sxb_str(double f)
{
	static	char buff[30];

	sprintf(buff, "%G", f);
	return(buff);
}


/*
** ファイル番号からファイルポインタを求める
*/
FILE * fn2fp(int fn)
{
	if(fn < 0 || fn >= FILE_MAX) {
		_sxb_error("Invalid File number");
		return(0);
	}
	return _fpfn_table[fn];
}

int
_sxb_feof(int fn)
{
	long	cur, len;

	FILE *fp = fn2fp(fn);
	return feof(fp);
}

int
_sxb_fgetc(int fn)
{
	int	tmp;

	tmp = fgetc(fn2fp(fn));
	return(tmp);
}

int
_sxb_fopen(char *fname, char *b_mode)
{
	int		i;
	FILE* fp;
	char	c_mode = "r";

	if (!strcmp(b_mode, "c")) {
		remove(fname);
		fp = fopen(fname, "w");
	} else if (!strcmp(b_mode, "rw")) {
		fp = fopen(fname, "rw");
	} else if (!strcmp(b_mode, "r")) {
		fp = fopen(fname, "r");
	} else if (!strcmp(b_mode, "w")) {
		fp = fopen(fname, "w");
	} else {
		_sxb_error("Invalid fopen mode");
		return -1;
	}

	if(fp == NULL) {
		_sxb_error("Unable to open the file");
		return(-1);
	}
	for(i = 0; i < FILE_MAX; i++) {
		if(_fpfn_table[i] == NULL) {
			_fpfn_table[i] = fp;
			return(i);
		}
	}
	if (i >= FILE_MAX) {
		_sxb_error("Tried to open too many files");
	}
	return -1;
}

int
_sxb_fputc(int i, int fn)
{
	fputc(i, fn2fp(fn));
	return(0);
}

int
_sxb_frename(char *oldname, char *newname)
{
	int	tmp;

	tmp = rename(oldname, newname);
	return(tmp);
}

int
_sxb_fseek(int fn, int offset, int md)
{
	int	tmp;

	if(md > 2) {
		_sxb_error("Invalid parameter for fseek");
		return(-1);
	}
	tmp = fseek(fn2fp(fn), offset, md);
	return(tmp);
}

int
_sxb_fread(void *ptr, int size, int len, int fn)
{
	int	tmp;
	FILE* fp;

	fp  = fn2fp(fn);

	if(len > size) {
		_sxb_error("String too long");
		return(-1);
	}

	tmp = fread(ptr, len, 1, fp);
	return(tmp);
}

int
_sxb_fwrite(void *p, int size, int len, int fn)
{
	int	tmp;
	FILE* fp;

	fp  = fn2fp(fn);
	if(len > size) {
		_sxb_error("String too long");
		return(-1);
	}
	tmp = fwrite(p, size, len, fp);
	if(tmp < 0) {
		_sxb_error("Access failure.");
		return(-1);
	}
	return(0);
}

int
_sxb_freads(char *p, int len, int fn)
{
	int		ret;
	FILE* fp;

	fp = fn2fp(fn);

	ret = fgets(p, len, fp);
	if(ret == NULL) {
		_sxb_error("Access failure.");
		return(-1);
	}
	return(0);
}

int
_sxb_fwrites(char *s, int fn)
{
	return fputs(s, fn2fp(fn));
}

int
_sxb_fclose(int fn)
{
	FILE* fp;

	fp = fn2fp(fn);
	if (fp == NULL) {
		return 0;
	} else {
		return fclose(fp);
	}
	return 0;
}

void
_sxb_fcloseall(void)
{
	int	i;
	FILE* fp;

	for(i = 0; i < FILE_MAX; i++) {
		fp = _fpfn_table[i];
		if(fp != NULL) {
			fclose(_fpfn_table[i]);
			_fpfn_table[i] = NULL;
		}
	}
}



char
*timeS(void)
{
	struct	tm *tp;
	time_t	time1;
	static char	buf[20];

	time(&time1);
	tp = localtime(&time1);
	strftime(buf, sizeof(buf), "%H:%M:%S", tp);
	return(buf);
}

char
*dateS(void)
{
	struct	tm *tp;
	time_t	time1;
	static char	buf[20];

	time(&time1);
	tp = localtime(&time1);
	strftime(buf, sizeof(buf), "%y/%m/%d", tp);
	return(buf);
}		

char
*dayS(void)
{
	struct	tm *tp;
	time_t	time1;

	static	char	*buf[] = {"日","月","火","水","木","金","土"};

	time(&time1);
	tp = localtime(&time1);
	return(buf[tp->tm_wday & 7]);
}


/*
 dの後に'\0'を入れるバージョン
 本当は良くない。
*/
char
*strncpy(char *d, const char *s, size_t len)
{
	while(*s && len-- >= 0) {
		*d++ = *s++;
	}
	*d = '\0';
	return(d);
}

void
_sxb_error(char* s) {
	puts(s);
	exit(-1);
}