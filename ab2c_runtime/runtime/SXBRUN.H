void	_sxb_start(void);
void	_sxb_Initialize(void );
int		_sxb_sendmes(int, char *);
void	_sxb_SetFunc(int, void (*)());
int		DMBeep(int );
int		_sxb_clipboard_PICT_set(int );
int		_sxb_clipboard_STRN_set(char *);
int		EMDeCross(void );
int		EMEnCross(void);
void	_sxb_end(void );
void	_sxb_exit(int );
void	_sxb_ltnl(void );
void	_sxb_print(char *);
void	_sxb_item_active(char *, int, int);
void	_sxb_item3(char *, int, int, char *, int ,int, int);
void	_sxb_item4(char *, int, int, char *, int, int, int, int);
void	_sxb_item5(char *, int, int, char *, int ,int, int, int, int);

void	_sxb_prop_set(char *,int, int, char *, int);
void	_sxb_prop_setS(char *,int, int, char *, char *);
void	_sxb_prop_setF(char *,int, int, char *, double);
int		_sxb_ref_prop(char *, int, int, char *);
char	*_sxb_ref_propS(char *, int, int, char *);

char	*_sxb_add(char *, ...);
int		_sxb_strcmpEQ(char *,char *);
int		_sxb_strcmpNE(char *,char *);
int		_sxb_strcmpGE(char *,char *);
int		_sxb_strcmpGT(char *,char *);
int		_sxb_strcmpLE(char *,char *);
int		_sxb_strcmpLT(char *,char *);

int		_sxb_alart(int, char *);
char	*_sxb_bin(int );
char	*_sxb_chrS(int );
char	*_sxb_hex(int );
void	_sxb_inkey0(void );
void	_sxb_inkey(void );
char	*_sxb_inputboxS(char *);
char	*_sxb_rightS(char *, int);
char	*_sxb_leftS(char *, int);
char	*_sxb_midS(char *, int, int);
char	*_sxb_mirrorS(char *);

int	_sxb_mousex(void );
int	_sxb_mousey(void );
int	_sxb_mousel(void );
int	_sxb_mouser(void );
char	*_sxb_octS(int );
char	*_sxb_pathS(void);
char	*_sxb_str(double );
int	_sxb_shiftkeybit(void);
int	_sxb_strp(char *);
int	_sxb_strh(char *);

int	_sxb_feof(int);
int	_sxb_fgetc(int );
int	_sxb_fopen(char *, char *);
int	_sxb_fputc(int, int);
int	_sxb_frename(char *, char *);
int	_sxb_fseek(int, int, int);
int	_sxb_fread(void *, int, int, int);
int	_sxb_fwrite(void *, int, int, int);
int	_sxb_freads(char *, int, int);
int	_sxb_fwrites(char *, int);
int	_sxb_fclose(int );

void	_sxb_aline_byte(int );
void	_sxb_aline_word(int );
void	_sxb_aline_long(int );
void	_sxb_MMPtrDispose(int );
void	_sxb_MMHdlDispose(int );
int		_sxb_varhdl(void *, int );


char	*_sxb_getenv(char *, int);
int		_sxb_setenv(char *, char *, char *);
int		findtskn(char *, int);
int		fock(char *);

char	*timeS(void);
char	*dateS(void);
char	*dayS(void);
void	srand(unsigned);
int		rand(void);
double	val(char *);
int		_sxb_openres(char *);
