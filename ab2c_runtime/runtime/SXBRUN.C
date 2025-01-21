/*
	Runtime Rountines For SX-BASIC Compiler

	Programmed By ISHIGAMI tatsuya
				Sep 15th 95

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<jstring.h>
#include	<fctype.h>
#include	<time.h>
#include	<class.h>
#include	<doslib.h>

#include	<memory.h>
#include	<event.h>
#include	<sxgraph.h>
#include	<window.h>
#include	<control.h>
#include	<menu.h>
#include	<text.h>
#include	<task.h>
#include	<dialog.h>
#include	<console.h>

#include	<fml.h>
#include	"d:\engine\engn.h"	/* Please fix on your enviroment */
#include	"sxbrun.h"

register COMVAL	*gp asm("a5");		/* 大域変数モドキへのポインター */

#define	SX_BASIC_SEND	258
#define	FILE_MAX		10	/* 一度に開けるファイルの数 */

#define	PICT	0x50494354
#define	STRN	0x5354524e

static	int	fpfn_table[FILE_MAX];

static	char	*runtimebuff;		/* temporary string buffer */
static	char	*_sxb_str_buff;		/* string buffer */
static	int		_sxb_strS_ptr;		/* pointer for string buffer */

static	void	MemorySizeFix(char*, int);


void
main(void)
{
	COMVAL	cv;
	int	ret;

	gp = &cv;

	/* 変数の初期化 */
	gp->activeWin = NULL;		/* アクティブフラグ初期値設定 */
	gp->winPtr = NULL;			/* メインウィンドウポインタ初期化 */
	gp->activeText = NULL;		/* アクティブテキストなし */

	/* アプリケーションの初期化 */
	ret = InitializeApp();

	if( ret < 0 )
		EndOfProgram( ret );
	_sxb_start();

	/* イベントループ */
	while(TRUE){
	    TSEventAvail(EVENT_MASK, &gp->EventRec );
	    /* 自分をカレントメニューマネージャにする */
	    if(gp->menuMgr) MGSetMgr(gp->menuMgr);
	    /* メニューマネージャにイベントを渡す */
	    ret = MGEvent(&gp->EventRec);
		if(ret)
			DoMenuCommand(ret);
		else switch(gp->EventRec.ts.what){
	    case E_IDLE:
	    	Ev_Idle();
	    	break;
		case E_MSLDOWN:
			Ev_MLDown();
			break;
		case E_MSRDOWN:
			Ev_MRDown();
			break;
		case E_KEYDOWN:
			Ev_KeyDown();
			break;
		case E_UPDATE:
			Ev_Update();
			break;
		case E_ACTIVATE:
			Ev_Activate();
			break;
		case E_SYSTEM1:
		case E_SYSTEM2:
			Ev_System();
			break;
	    }
	}
}


/*
 * アプリケーションの初期化を行う
 */
int
InitializeApp( void )
{
	Task	tBuff;
	int 	ret;
	Rect	rcMainWnd;
	char	drv[2], path[65], node[19], ext[5];

	TSSetAbort(&EndOfProgram, (long) -1); 	/* ｱﾎﾞｰﾄ処理ﾙｰﾁﾝ登録 */

	/* SX-Window system の version を取得する */
	if( (short)SXVer() < 0x300 ){
		TSErrDialogN(1, "ＳＸウィンドウのバージョンが違います。" );
		return(-1);
	}

	TSGetTdb( &tBuff, -1 );
	ret = TSTakeParam((_LASCII )&tBuff.command, &rcMainWnd,NULL,NULL,NULL,NULL);
	if((ret & 1) == 0) {
		rcMainWnd.l.l_t    = TSGetWindowPos();
		rcMainWnd.d.right  = rcMainWnd.d.left + WINH;
		rcMainWnd.d.bottom = rcMainWnd.d.top + WINV;
	}
	RecovSize();
	strsfn(tBuff.name, drv, path, node, ext);
	strmfn(gp->rscfile, drv, path, node, ".lb");

	/* window を作成する */
	gp->winPtr = (Window *)WMOpen( (Window *) NULL,
		&rcMainWnd, (_LASCII )"\020ウィンドデザイナ",
		NULL, WINDEFID, (Window *)(-1), -1, TSGetID());

	if( gp->winPtr == NULL ) return(-1);
	/* サイズボタンをサポートする（表示はしない） */
	gp->winPtr->option = WC_SBOXON;

	/* グラフポートを自分に設定する */
	GMSetGraph (&gp->winPtr->graph);
	GMAPage (7);

	/* バックグランドカラーの設定 */
	GMBackColor (G_LGRAY);

	/* フォアグランドカラーの設定 */
	GMForeColor (G_BLACK);

	/* 次のデータへのハンドルを初期化する */
	gp->data = (comDataHdl )NIL;

	gp->inkey = 0;
	gp->menuMgr = NULL;
	gp->menuBar = NULL;
	gp->rscHdl  = NULL;
	gp->menuHdl = NULL;
	gp->drag = FALSE;
	gp->enableMess = FALSE;
	gp->updateFlag = TRUE;
	gp->isCompiled = FALSE;
	gp->ansHdl = (char **)MMChHdlNew(300);
	if(gp->ansHdl == (char **)NULL) {
		TSErrDialogN(1, "メモリが確保できません");
		return(-1);
	}
	_sxb_str_buff = (char *)MMChPtrNew(1000);
	_sxb_strS_ptr = 0;
	runtimebuff = (char *)MMChPtrNew(1000);
	_sxb_Initialize();
	UpdateMainWindow();	
	return(NULL);
}

int
_sxb_clipboard_PICT_set(int hdl)
{
	void	**cellHdl;
	int		siz, *p;

	siz = MMHdlSizeGet((void **)hdl);
	cellHdl = MMChHdlNew(siz + 4 + 4);
	p = (int *)*cellHdl;
	p[0] = PICT;
	p[1] = siz;
	memcpy(&p[2], *(void **)hdl, siz);
	TSPutScrap(siz + 4 + 4, cellHdl);

	return(TRUE);
}

int
_sxb_clipboard_STRN_set(char *s)
{
	void	**cellHdl;
	int		siz, *p;

	siz = strlen(s);
	cellHdl = MMChHdlNew(siz + 4 + 4);
	p = (int *)*cellHdl;
	p[0] = STRN;
	p[1] = siz;
	memcpy((char *)&p[2], s, siz);
	TSPutScrap(siz + 4 + 4, cellHdl);
	TMFromScrap();

	return(TRUE);
}

void
_sxb_end(void )
{
	MMPtrDispose((Pointer )_sxb_str_buff);
}

void
_sxb_exit(int result)
{
	_sxb_end();
	exit(result);
}

void
_sxb_item_active(char *item, int isarray, int index)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].active", item, index);
	else
		sprintf(buff, "%s.active", item);
	SetProperties(buff);
}

void
_sxb_item4(char *item, int isarray, int index, char *prop, int x1, int y1, int x2, int y2)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%d,%d,%d,%d", item, index, prop, x1,y1,x2,y2);
	else
		sprintf(buff, "%s.%s=%d,%d,%d,%d", item, prop, x1,y1,x2,y2);
	SetProperties(buff);
}

void
_sxb_item3(char *item, int isarray, int index, char *prop, int x, int y, int r)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%d,%d,%d", item, index, prop, x,y,r);
	else
		sprintf(buff, "%s.%s=%d,%d,%d", item, prop, x,y,r);
	SetProperties(buff);
}

void
_sxb_item5(char *item, int isarray, int index, char *prop, int id, int x1, int y1, int x2, int y2)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%d,%d,%d,%d,%d", item, index, prop, id,x1,y1,x2,y2);
	else
		sprintf(buff, "%s.%s=%d,%d,%d,%d,%d", item, prop, id,x1,y1,x2,y2);
	SetProperties(buff);
}

void
_sxb_prop_set(char *item, int isarray, int index, char *prop, int val)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%d", item, index, prop, val);
	else
		sprintf(buff, "%s.%s=%d", item, prop, val);
	SetProperties(buff);
}

void
_sxb_prop_setS(char *item, int isarray, int index, char *prop, char *s)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%s", item, index, prop, s);
	else
		sprintf(buff, "%s.%s=%s", item, prop, s);
	SetProperties(buff);
}

void
_sxb_prop_setF(char *item, int isarray, int index, char *prop, double val)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s=%g", item, index, prop, val);
	else
		sprintf(buff, "%s.%s=%g", item, prop, val);
	SetProperties(buff);
}

int
_sxb_ref_prop(char *item, int isarray, int index, char *prop)
{
	char	buff[100];

	if(isarray)
		sprintf(buff, "%s[%d].%s", item, index, prop);
	else
		sprintf(buff, "%s.%s", item, prop);

	return(RefProperties(buff));
}

char
*_sxb_ref_propS(char *item, int isarray, int index, char *prop)
{
	char	buff[1000];
	char	*p;

	if(isarray)
		sprintf(buff, "%s[%d].%s", item, index, prop);
	else
		sprintf(buff, "%s.%s", item, prop);

	p = (char *)RefProperties(buff);
	strcpy(buff, p);
	return(buff);
}
#if 0
char
*_sxb_add(char *s, ...)
{
	char	*p;
	int	len;
	int	sav_strS_ptr;
	va_list	ap;

	va_start(ap, s);

	sav_strS_ptr = _sxb_strS_ptr;
	len = strlen(s);
	MemorySizeFix(_sxb_str_buff, _sxb_strS_ptr + len + 10);
	strcpy(&_sxb_str_buff[_sxb_strS_ptr], s);
	_sxb_strS_ptr += len;
	while(1) {
		p = va_arg(ap, char *);
		if(p == (char *)-1) break;

		len = strlen(p);
		MemorySizeFix(_sxb_str_buff, _sxb_strS_ptr + len + 10);
		strcat(&_sxb_str_buff[sav_strS_ptr], p);
		_sxb_strS_ptr += len;
	}
	va_end(ap);
	return(&_sxb_str_buff[sav_strS_ptr]);
}
#else

char
*_sxb_add(char *s, ...)
{
	char	*p;
	va_list	ap;
	static	char	buff[1000];

	va_start(ap, s);

	strcpy(buff, s);
	while(1) {
		p = va_arg(ap, char *);
		if(p == (char *)-1) break;

		strcat(buff, p);
	}
	va_end(ap);
	return(buff);
}
#endif

int
_sxb_strcmpGE(char *s ,char *t)
{
	if(strcmp(s, t) == NULL)
		return(TRUE);
	else
		return(FALSE);
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
	if(strcmp(s, t) >= 0)
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
		return(FALSE);
}

int
_sxb_alart(int id, char *s)
{
	int	r;
	r =	DMError(id, s);
	return(r);
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
*_sxb_hex(int i)
{
	static char	buff[40];

	sprintf(buff, "%X", i);
	return(buff);
}

void	_sxb_inkey0(void );
void	_sxb_inkey(void );

/*
	 アイテムリスト
*/
typedef struct dlgItem2 {
	long		dlgIHdl;
	Rect		dlgIBounds;
	unsigned char	dlgIType;
	unsigned char	dlgISize;
	unsigned char	dlgIData[32];
} dlgItem2;
/*
  インプットボックス用のダイアログ
*/
struct {
	short	itemNo;
	dlgItem2 dItem1;
	dlgItem2 dItem2;
	dlgItem2 dItem3;
	dlgItem2 dItem4;
} inputItemList ={
	4-1,
   {
	0,
	{256-8-36,128-8-18,256-8,128-8},
	DT_CHRBTN,
	32,
	"\006 終了 "
   },
   {
	0,
	{4,4,252,16},
	DT_STCTXT+DT_DISABL,
	32,
	"\001\023SX-BASIC INPUT BOX"
   },
   {
	0,
	{4,40,252,52},
	DT_STCTXT+DT_DISABL,
	32,
	"\001\026文字列を入力して下さい"
   },
   {
	0,
	{50,76,200,88},
	DT_EDTTXT+DT_DISABL,
	22,
	"\040"
   }
};

/*
**	Filter Function while Dialog Processing.
*/
void
*MyFilter(Dialog *dlgPtr, Event *ev)
{
	Rect	rc;
	short	type;
	Handle	dItemHdl;

	if( ev->what == E_KEYDOWN ){
		if(ev->whom.key.code==13) {
			DIGet(dlgPtr, 1, &type, &dItemHdl, &rc);
			rc.d.left += 4;
			rc.d.top += 4;
			ev->where.x_y = GMLocalToGlobal(rc.l.l_t);
			ev->what = E_MSLDOWN;
		}
	}
	return 0;
}

/*
**	Input Box を開く
*/
char
*_sxb_inputboxS(char *title)
{
	Rect	dispRect;

	Dialog   *dialogPtr;
	void	**dIHdl;

	short	DI_T;
	Handle	DI_H;
	Rect	DI_R;

	LASCII	strBuf;

	SXGetDispRect(&dispRect);
	GMCenterRect(&dispRect, &dispRect, (256 << 16 | 128), 0);

	dIHdl = (void **)MMChHdlNew(sizeof(inputItemList));
	memcpy(*dIHdl,&inputItemList,sizeof(inputItemList));

	strcpy((char *)&strBuf[1], title);
	strBuf[0] = strlen(title);
	dialogPtr=DMOpen(NULL,&dispRect,(_LASCII )strBuf, TRUE, 38 << 4,
			(Window *)-1,TRUE,TSGetID(),(_Handle )dIHdl);
	if( dialogPtr == NULL ){
		TSErrDialogN(0x101, "Can not Open Dialog Window.");
		_sxb_exit(-1);
	}

	DIGet(dialogPtr, 4, &DI_T, &DI_H, &DI_R);
	DITSet(DI_T, DI_H, (_LASCII )"\0");
	DMDraw(dialogPtr);
	DMControl((void*)MyFilter );
	DITGet(DI_T, DI_H, (_LASCII )&strBuf);

	DMDispose(dialogPtr);
	MMHdlDispose((Handle )dIHdl);

	strBuf[strBuf[0] + 1] = '\0';
	return((char *)&strBuf[1]);
}


static void
MemorySizeFix(char *p ,int size)
{
	int	ret;

	if(MMPtrSizeGet(p) < size + 10) {
		ret = MMPtrSizeSet((Pointer)p, size + 100);
		if(ret) {
			TSErrDialogN(0x101, "Not Enough Memory.");
			_sxb_exit(-1);
		}
	}
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

	MemorySizeFix(runtimebuff, len);
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

/*	MemorySizeFix(runtimebuff, j);	*/
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

	MemorySizeFix(runtimebuff, len);
	while(len-- > 0) {
		*p++ = *s--;
	}
	*p++ = '\0';
	return(runtimebuff);
}

int
_sxb_mousex(void )
{
	return(RefProperties("mousex"));
}

int
_sxb_mousey(void )
{
	return(RefProperties("mousey"));
}

int
_sxb_mousel(void )
{
	return(RefProperties("mousel"));
}

int
_sxb_mouser(void )
{
	return(RefProperties("mouser"));
}

char
*_sxb_octS(int i)
{
	static	char	buff[30];
	sprintf(buff, "%o", i);
	return(buff);
}

int
_sxb_shiftkeybit(void)
{
	return(KBShiftGet(KBCurKbrGet()));
}

char
*_sxb_pathS(void)
{
	char	fWork[200];
	Task	tBuff;
	char	drv[2], path[65], node[19], ext[5];

	TSGetTdb(&tBuff, TSGetID());

	strsfn(tBuff.name, drv, path, node, ext);
	strmfn(fWork, drv, path, "", "");

	return(fWork);
}

char
*_sxb_str(double f)
{
	static	char buff[30];

	sprintf(buff, "%G", f);
	return(buff);
}

int	_sxb_strp(char *);
int	_sxb_strh(char *);


/*
** ファイル番号からファイルポインタを求める
*/
int
fn2fp(int fn)
{
	if(fn < 0 || fn >= FILE_MAX) {
		TSErrDialogN(1, "ファイル番号の値が異常です");
		return(0);
	}
	return(fpfn_table[fn]);
}

int
_sxb_feof(int fn)
{
	long	cur, len;

	fn = fn2fp(fn);
	cur = SEEK(fn, 0, 1);
	len = SEEK(fn, 0, 2);
	SEEK(fn, cur, 0);
	if(cur < 0 || len < 0) {
		TSErrDialogN(1, "ファイル番号の値が異常です");
	}
	if(cur >= len)
		return(-1);

	return(0);
}

int
_sxb_fgetc(int fn)
{
	int	tmp;

	tmp = FGETC(fn2fp(fn));
	return(tmp);
}

int
_sxb_fopen(char *fname, char *mode)
{
	int		tmp, fn;
	
	if(!strcmp(mode, "rw"))		fn = TSOpen(fname, 2);
	else if(!strcmp(mode, "w"))	fn = TSOpen(fname, 1);
	else if(!strcmp(mode, "r"))	fn = TSOpen(fname, 0);
	else if(!strcmp(mode, "c"))	fn = TSCreate(fname, 0);
	else {
		TSErrDialogN(1, "モード指定が異常です");
		return(-1);
	}

	if(fn < 0) {
		TSErrDialogN(1, "ファイルがオープンできません");
		return(-1);
	}
	for(tmp = 0; tmp < FILE_MAX; tmp++) {
		if(fpfn_table[tmp] == NULL) {
			fpfn_table[tmp] = fn;
			return(tmp);
		}
	}
	if(tmp >= FILE_MAX)
		TSErrDialogN(1, "一度にこれ以上のファイルは開けません");
	return(-1);
}

int
_sxb_fputc(int i, int fn)
{
	FPUTC(i, fn2fp(fn));
	return(0);
}

int
_sxb_frename(char *oldname, char *newname)
{
	int	tmp;

	tmp = RENAME((UBYTE *)oldname, (UBYTE *)newname);
	return(tmp);
}

int
_sxb_fseek(int fn, int offset, int md)
{
	int	tmp;

	if(md > 2) {
		TSErrDialogN(1, "パラメータが異常です");
		return(-1);
	}
	tmp = SEEK(fn2fp(fn), offset, md);
	return(tmp);
}

int
_sxb_fread(void *ptr, int size, int len, int fn)
{
	int	tmp;

	fn  = fn2fp(fn);

	if(len > size) {
		TSErrDialogN(1, "要素数が大きすぎます");
		return(-1);
	}

	tmp = READ(fn, (UBYTE *)ptr, len);
	return(tmp);
}

int
_sxb_fwrite(void *p, int size, int len, int fn)
{
	int	tmp;

	fn  = fn2fp(fn);
	if(len > size) {
		TSErrDialogN(1, "要素数が大きすぎます");
		return(-1);
	}
	tmp = WRITE(fn, (UBYTE *)p, len);
	if(tmp < 0) {
		TSErrDialogN(1, "ファイルがアクセスできません");
		return(-1);
	}
	return(0);
}

int
_sxb_freads(char *p, int len, int fn)
{
	struct INPPTR	buff;
	int	ret, tmp = 0;
	int	get = 0;		/* すでに読み込んだ文字数 */

	fn = fn2fp(fn);
	buff.max = 254;

	while(get < len) {
		ret = FGETS(&buff, fn);
		if(ret < 0) {
			TSErrDialogN(1, "ファイルがアクセスできません");
			return(-1);
		}
		for(tmp = 0; tmp < ret; tmp++) {
			if(buff.buffer[tmp] == '\r' || buff.buffer[tmp] == '\n')
				break;
			p[get + tmp] = buff.buffer[tmp];
		}
		if(buff.length != buff.max) break;
		get += buff.length;
	}
	p[get + tmp] = '\0';
	ret = get + tmp;

	return(0);
}

int
_sxb_fwrites(char *s, int fn)
{
	FPUTS((UBYTE *)s, fn2fp(fn));
	return(strlen(s));
}

int
_sxb_fclose(int fn)
{
	int	tmp;

	if(fn < 0 || fn >= FILE_MAX) {
		TSErrDialogN(1, "ファイル番号の値が異常です");
		return(-1);
	}
	tmp = TSClose(fn2fp(fn));
	fpfn_table[fn] = 0;
	if(tmp < 0) {
		TSErrDialogN(1, "ファイルがクローズできません");
		return(-1);
	}
	return(0);
}

void
_sxb_fcloseall(void)
{
	int	i;

	for(i = 0; i < FILE_MAX; i++) {
		if(fpfn_table[i]) {
			TSClose(fpfn_table[i]);
			fpfn_table[i] = 0;
		}
	}
}

void	_sxb_aline_byte(int );
void	_sxb_aline_word(int );
void	_sxb_aline_long(int );
void	_sxb_MMPtrDispose(int );
void	_sxb_MMHdlDispose(int );

int
_sxb_varhdl(void *p, int size)
{
	int		**hdl;

	hdl  = (int **)MMChHdlNew(size);
	if((int )hdl <= 0) {
		TSErrDialogN(0x101, "メモリが不足しました");
		_sxb_exit(-1);
	} else
		memcpy(*hdl, p, size);

	return((int )hdl);
}


char
*_sxb_getenv(char *s, int process)
{
	static	char	envstr[256];
	int		ret;

	ret = GETENV((UBYTE *)s, (UBYTE *)process, (UBYTE *)envstr);
	if(ret < 0) {
		TSErrDialogN(1, "環境変数が読み込めません");
		envstr[0] = '\0';
	}

	return(envstr);
}

int
_sxb_setenv(char *name, char *var, char *p)
{
	int	ret;

	ret = SETENV((UBYTE *)name, (UBYTE *)p, (UBYTE *)var);
	return(ret);
}

int
findtskn(char *s, int id)
{
	int	ret;

	ret =  TSFindTskn(s, id) & 0xffff;
	if(ret > 0xfff0)	ret = -1;
	return(ret);
}

int
fock(char *command)
{
	char	fname[90];
	char	*cptr;
	char	drv[4];
	char	path[66];
	char	node[24];
	char	ext[6];

	int	ret;

	cptr = (char *)jstrchr((unsigned char *)command, ' ');
	if (cptr != 0)
		*cptr = 0;
	strncpy(fname, command, 90);
	strsfn(fname, drv, path ,node, ext);

	if (ext[0] == 0)
		strcat(fname, ".?");
	if (cptr != 0)
		*cptr = strlen(cptr + 1);

	/* コマンドを起動する */;
	ret = TSFockB(0,0,fname,cptr,0,fname);

	if (ret < 0) {
		switch (ret) {
		  case ER_FILENOTFND:
			TSErrDialogN(1,"指定されたファイルが見つかりません");
			break;
		  case ER_SERCHBREAK:
		  	TSErrDialogN(1,"ファイルの検索が中止されました");
			break;
		  default:
			TSErrDialogN(1,"ファイルのオープンに失敗しました");
			break;
		}
	}
	return(ret);
}

/********************************/
/*	文字列を送信する	*/
/********************************/
int
_sxb_sendmes(int id, char *mes)
{
	char	**hdl;
	int	ret;
	int	cnt = 10;	/*メッセージのやりとりを１０回行なって
				   それでもダメならエラーにする		*/
	TsEvent	eventRec;

	hdl = (char **)MMChHdlNew(strlen(mes));
	if(hdl == NULL) {
		TSErrDialogN(0x101,"メモリが確保出来ません");
		return(FALSE);
	}
	strcpy(*hdl, mes);

	eventRec.ts.whom   = (long)hdl;
	eventRec.ts.when   = EMSysTime();
	eventRec.ts.what2  = SX_BASIC_SEND;
	do {
		ret = TSSendMes(id, &eventRec);
		cnt--;
	} while(ret != 0 && ret != 2 && cnt != 0);

	MMHdlDispose((Handle )hdl);
	if(cnt == 0) TSErrDialogN(1, "無効なタスクに通信を行ないました");

	return(TRUE);
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

#if 0
int
_sxb_openres(char *filename)
{
	char	fWork[90];
	char	drv[2], path[65], node[19], ext[5];

	strncpy(fWork, filename, sizeof(fWork));
	/* ファイル名を分解 */
	strsfn(fWork, drv, path, node, ext);

	if(ext[0] == '\0') {		/* 拡張子がない場合 */
		strmfn(fWork, drv, path, node, "lb");
		strcpy(ext, "lb");
	}
	if(drv[0] == '\0' && path[0] == '\0') {		/* ドライブ、パス指定のない場合 */
		char	node2[19], ext2[5];
		Task	tBuff;
	
		TSGetTdb(&tBuff, TSGetID());
		strsfn(tBuff.name, drv, path, node2, ext2);
		strmfn(fWork, drv, path, node, ext);
	}
	ChangeResourceFile(fWork);
	return(0);
}
#else
int
_sxb_openres(char *filename)
{
	char	fWork[90];
	char	drv[3], path[66], node[19], ext[5];

	strncpy(fWork, filename, sizeof(fWork));
	if(strchr(fWork, ':') == NULL && strchr(fWork, '\\') == NULL) {
		/* ドライブ、パス指定のない場合 */
		char	tmp[100];
		Task	tBuff;
	
		TSGetTdb(&tBuff, TSGetID());
		strsfn(tBuff.name, drv, path, node, ext);
		strcpy(tmp, drv);
		strcat(tmp, path);
		strcat(tmp, fWork);
		strcpy(fWork, tmp);
	}
	if(strchr(fWork, '.') == NULL) {		/* 拡張子がない場合 */
		strcat(fWork, ".lb");
	}
	ChangeResourceFile(fWork);
	return(0);
}
#endif

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
