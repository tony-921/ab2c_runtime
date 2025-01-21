/*
	Definition of the Global Variables
		shared with WINDOW ENGINE
		For SX-BASIC Compiler

*/

typedef	char	bool;

typedef	struct	rect {
	short left;
	short top;
	short right;
	short bottom;
} Rect;

typedef	struct tsevent {
	short	what;
	long	whom;
	long	when;
	long	whom2;
	short	what2;
	short	tskid;
} TsEvent;
typedef	unsigned int	ULONG;
typedef	void	Control;
typedef	void	Menu;
typedef	void	**DMenuHandle;
typedef	void	*FWindowPtr;
typedef	void	Window;
typedef	void	TEdit;
typedef	void	GScript;
typedef	void	**Handle;
typedef	void	**DMenuMgrHandle;
typedef	void	**DMenuBarHandle;

typedef struct comdata {
	int 	   id;			/* 図形の種類		*/
	struct comdata	 **next;		/* 次のデータへのハンドル	*/
	char	   name[19];	/* コントロールの名前		*/
	bool	   isarray;		/* 配列型か？		*/
	Rect	   bounds;		/* データを囲む枠	*/
	int 	   Fcolor;		/* 描画色			*/
	int 	   Bcolor;		/* 背景色			*/
	int		   fontSize;	/* フォントサイズ	*/
	int		   editable;	/* 編集可能か		*/
	int		   arrange;		/* 行揃え			*/
	int		   frame;		/* 枠				*/
	int		   visible;		/* 可視状態か		*/
	int		   index;		/* 配列型の場合		*/
	ULONG	   settime;		/* セットした時間	*/
	ULONG	   lasttime;	/* 前回の時間		*/
	int		   rscId;		/* リソースＩＤ		*/
	unsigned short	 **bitmap;	/* ビットマップデータのハンドル	*/
	char	   caption[90];		/* 文字列、ファイル名など	*/
	Control	 **cntHdl;		/* コントロールへのハンドル	*/
	Menu	 **menuHdl;		/* メニューのハンドル	*/
	DMenuHandle	dmenuHdl;	/* デラックスメニューへのハンドル */
	TEdit	 **teHdl;		/* テキストへのハンドル		*/
	int		   lineHeight;	/* テキストの改行幅 */
	bool	   enable;		/* イネーブル状態か */
	bool	   isGeneral;	/* メインページに載せるか？ */
	FWindowPtr winPtr;		/* ウィンドウポインタ(ウィンドウアイテム） */
	Window	  *where;		/* 現在、どのウィンドウ上にあるか？ */
	Window	  *domes;		/* 本来、どのウィンドウ上にあるか？ */
	GScript	 **gscHdl;		/* スクリプト */
	bool	   updateFlag;	/* アップデート許可フラグ（０：禁止）	*/
	short	   eventmask;	/* アイテム毎のイベントマスク */
	void	(*click)();			/* _Click, _Close, _Timer */
	void	(*change)();		/* _Change, _Select, _Menuselect	*/
	int		(*addmenuselect)();	/* _Addmenuselect	*/
	void	(*keyin)();			/* _KeyIn			*/
	int		(*rev1)();			/* Reserved */
	int		(*rev2)();			/* Reserved */
} COMDATA, **comDataHdl;


/* プログラム固有の情報 */
typedef struct comval{
	TsEvent    EventRec;		/* イベントレコード		 */
	Window	  *activeWin;		/* アクティブウィンドウへのポインタ	 */
	Window	  *winPtr;	 		/* <window> へのポインタ	 */
	Window	  *iconPtr;
	int		   inkey;			/* 文字入力 */
	comDataHdl data;			/* データ格納域へのハンドル	 */
	comDataHdl activeText;		/* アクティブなテキスト（編集可）*/
	char	 **ansHdl;			/* 返答用のハンドル		 */
	Menu	 **menuHdl;			/* メニュー用のハンドル  */
	int		   drag;			/* ドラッグ受入可能か */
	Handle	   rscHdl;			/* リソースファイルのID */
	char	   rscfile[90];		/* リソースファイル */
	DMenuMgrHandle	menuMgr;	/* メニューマネージャ */
	DMenuBarHandle	menuBar;	/* メニューバー */
	bool	   iconFlag;		/* アイコンか？ */
	bool	   enableMess;		/* SX-BASICがメッセージ受信状態にあるか？ */
	bool	   updateFlag;		/* アップデート許可フラグ（０：禁止）	*/
	bool	   isCompiled;		/* 親タスクはコンパイルされているか */
	comDataHdl lastItem;		/* The Item MainProgram accessed last */
} COMVAL;

void		CreateItems(char *);
register COMVAL	*gp asm("a5");		/* 大域変数モドキへのポインター */

