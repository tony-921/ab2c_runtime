/*
	ウィンド・エンジン Ver 0.5

	Programmed By Ishigami TATSUYA

	註）このプログラムは、シャープ製「ＳＸ開発キット」の「簡易ドロー」
	をもとに、改造を加え作成したものです。

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<fctype.h>
#include	<class.h>

#include	<memory.h>
#include	<event.h>
#include	<sxgraph.h>
#include	<window.h>
#include	<control.h>
#include	<menu.h>
#include	<text.h>
#include	<task.h>

#include	<fml.h>

#include	"ENGN.H"	/* このアプリケーション固有のヘッダファイル */


/* カーネル起動コマンド */
char  _sxkernelcomm[] = "sxkernel.x -R7 -L0 -K -D";

register COMVAL	*gp asm("a5");		/* 大域変数モドキへのポインター */

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

	if( ret < 0 ){
		/* 初期化に失敗したので終了する */
		EndOfProgram( ret );
	}

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

	TSSetAbort(&EndOfProgram, (long) -1); 	/* ｱﾎﾞｰﾄ処理ﾙｰﾁﾝ登録 */

	/* SX-Window system の version を取得する */
	if( (short)SXVer() < 0x300 ){
		TSErrDialogN(1, "ＳＸウィンドウのバージョンが違います。" );
		return(-1);
	}

	TSGetTdb( &tBuff, -1 );
	ret = TSTakeParam((_LASCII )&tBuff.command, &rcMainWnd,NULL,NULL,NULL,NULL);
	if(ret & 1) {
		/* SX-Windowの再起動時に起動しないように */
		/* （起動はすべてSX-BASICから行なう） */
		if(tBuff.parentID == 0)	exit(0);
	}
	else {
		rcMainWnd.l.l_t    = TSGetWindowPos();
		rcMainWnd.d.right  = rcMainWnd.d.left + WINH;
		rcMainWnd.d.bottom = rcMainWnd.d.top + WINV;
	}

	RecovSize();

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
	return(NULL);
}

