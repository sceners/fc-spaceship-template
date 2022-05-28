#include "template.h"
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>

#define GAME_STATE_INIT 0
#define GAME_STATE_DEMO_RUN 1
#define GAME_STATE_GAME_RUN 2
#define GAME_STATE_GAME_OVER 3
#define GAME_STATE_NEW_LEVEL 4

#define MAX_LOADSTRING 100
#define LARGEUR_FENETRE 320
#define HAUTEUR_FENETRE 240
#define BITS_PAR_PIXEL 24

//#define FULLSCREEN

#define NB_BOULETS 2
#define NB_E_BOULETS 4

typedef struct s_ennemy{
	int type;
	int x;
	int y;
	int t;
	int mvt;
	int x_togo;
	int y_togo;
	double var1;
} ennemy;

typedef struct s_boulet{
	int actif;
	int x;
	int y;
} boulet;

typedef struct s_ennemy_boulet{
	int actif;
	int x;
	int y;
	double a;
} ennemy_boulet;


void InitStarfield();
void UpdateLevel();
void UpdateStarfield();
void ReinitEnnemies();
void InitBullets();
void LoadCustomFonts();
void LoadSprites();
void DrawBullets();
void CheckCollisions();
void DrawEnnemies();
void DisplayLives();
void DeplacerEnnemis();
void DeplacerBoulets();
void DeplacerAircraft();
void DrawAircraft();
void Shoot();
void EnnemyShoot();
void CheckExplosions();
void DrawSinusScroll();
void DrawScore();

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);
void CALLBACK creationBackBuffer();

#ifdef FULLSCREEN
DEVMODE game_screen;   // global for full screen mode
#endif

typedef HANDLE (*MYPROC)(
  PVOID pbFont,       // font resource
  DWORD cbFont,       // number of bytes in font resource 
  PVOID pdv,          // Reserved. Must be 0.
  DWORD *pcFonts      // number of fonts installed
);

// Global Variables:
HDC hdcFenetre;
HDC hdcBackBuffer;
unsigned char *tableausurfaceBackBuffer;

TCHAR szTitle[MAX_LOADSTRING] = "Galaga";		// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING]= "TITICLASS";
HWND hwndFenetre;
HWND hwndAbout;

HBITMAP hbmpEnnemy1[6];
HBITMAP hbmpEnnemy2[6];
HBITMAP hbmpMaskEnnemy1[6];
HBITMAP hbmpMaskEnnemy2[6];
HBITMAP	hbmpAircraft,hbmpMaskAircraft;
HBITMAP	hbmpBigExplosion;
HBITMAP	hbmpSword, hbmpMaskSword;

typedef struct s_starfield{
	char actif[LARGEUR_FENETRE][HAUTEUR_FENETRE-20];
	char red[LARGEUR_FENETRE][HAUTEUR_FENETRE-20];
	char green[LARGEUR_FENETRE][HAUTEUR_FENETRE-20];
	char blue[LARGEUR_FENETRE][HAUTEUR_FENETRE-20];
} starfield;

char levels[200]={
 4,4,4,4,0,0,4,4,0,0,
 2,0,0,0,0,2,0,0,2,0,
 1,1,1,0,0,1,0,0,0,0,
 1,0,0,0,0,1,0,0,1,0,
 1,0,0,0,0,0,1,1,0,0,

 0,0,0,4,4,4,4,0,0,0,
 0,2,2,2,2,2,2,2,2,0,
 0,2,2,2,2,2,2,2,2,0,
 1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,

 0,0,4,4,4,4,4,4,0,0,
 0,2,2,2,2,2,2,2,2,0,
 1,1,1,1,1,1,1,1,1,1,				 
 1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,1,1,

 0,0,4,4,4,4,4,4,0,0,
 0,2,2,2,2,2,2,2,2,0,
 1,1,1,1,1,1,1,1,1,1,				 
 1,1,1,1,0,0,1,1,1,1,
 0,1,1,0,0,0,0,1,1,0
};

ennemy_boulet eb[NB_E_BOULETS];
boulet b[NB_BOULETS];
ennemy e[50];
starfield s;

DWORD new_timer=0;
DWORD timer=0;
int score=0;
int hiscore=0;
int nblives=2;
int nbswords=2;
int level=-1;
int x=LARGEUR_FENETRE/2-8,y=HAUTEUR_FENETRE-41,x_togo=LARGEUR_FENETRE/2,depl=1;
int actif=1;
int sc_t=0;
int game = GAME_STATE_INIT;
int previous_game = GAME_STATE_DEMO_RUN;

HANDLE hFont,hFontScore;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//HINSTANCE hInst;

BOOL CALLBACK DlgProc2( HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
   switch (uMsg)
   {
	   case WM_LBUTTONDBLCLK:
	   case WM_RBUTTONDOWN:
	   case WM_CLOSE:
			KillTimer(hwnd,IDT_TIMER);
   			EndDialog(hwnd, 0); 
   			break;

	   case WM_NCLBUTTONDOWN:
		   PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
		   return TRUE;
		   break;

		case WM_INITDIALOG:
			game = GAME_STATE_INIT;
			hwndAbout=hwnd;
			SetTimer(hwnd, IDT_TIMER, 40, (TIMERPROC)creationBackBuffer);
			break;
 		default:
   			return FALSE;
   }
   
   return TRUE;
}

void CALLBACK creationBackBuffer()
{
	//necessaire a la creation de la surface du backbuffer

	HBITMAP hbmsurfaceBackBuffer;
	BITMAPINFO bmi;

//	int i;
	int j=0;

	hdcFenetre	  = GetDC(GetDlgItem(hwndAbout,IDC_GAME));
	hdcBackBuffer = CreateCompatibleDC(hdcFenetre);		

	timer++;

	// on vide la structure par precaution
	ZeroMemory(&bmi,sizeof(BITMAPINFO));

	// on la renseigne
	bmi.bmiHeader.biSize			=   sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			=	LARGEUR_FENETRE ;
	bmi.bmiHeader.biHeight			=	-HAUTEUR_FENETRE ;
	bmi.bmiHeader.biPlanes			=	1 ;
	bmi.bmiHeader.biBitCount		=	BITS_PAR_PIXEL ;  //16 = 16 bits  24 = 24 bits et 32 = 32 bits
	bmi.bmiHeader.biCompression		=	BI_RGB ;
	bmi.bmiHeader.biSizeImage		=	0	;
	bmi.bmiHeader.biXPelsPerMeter	=	0 ;
	bmi.bmiHeader.biYPelsPerMeter	=	0 ;
	bmi.bmiHeader.biClrUsed			=	0 ;
	bmi.bmiHeader.biClrImportant	=	0 ;

	hbmsurfaceBackBuffer = CreateDIBSection(hdcFenetre,&bmi,DIB_RGB_COLORS,(void **)&tableausurfaceBackBuffer,NULL,0);
	if (!hbmsurfaceBackBuffer)	
		MessageBox(NULL,"Je n'ai pu creer la surface du backbuffer","Erreur",MB_ICONERROR);

	else
	{
		// Séquenceur

		SelectObject(hdcBackBuffer,hbmsurfaceBackBuffer);

		if((GetAsyncKeyState(VK_ESCAPE)&0xC000)!=0){
				PostQuitMessage(0);
		}

		switch(game){
		case GAME_STATE_INIT:
			srand(GetTickCount());
			InitStarfield();
			InitBullets();
			LoadCustomFonts();
			LoadSprites();
			ReinitEnnemies();
			previous_game=GAME_STATE_GAME_RUN;
			game=GAME_STATE_NEW_LEVEL;
			UpdateLevel();
			break;

		case GAME_STATE_DEMO_RUN:
			if((GetAsyncKeyState(VK_TAB)&0xC000)!=0){	// Switche le mode du jeu
				ReinitEnnemies();
				nblives=2;
				nbswords=2;
				previous_game=GAME_STATE_GAME_RUN;
				game=GAME_STATE_GAME_RUN;
				actif=1;
				score=0;
				level = -1;
			}

			if(depl==0)depl=1;
			if(x>=x_togo && depl>=0){ x_togo=rand()%x; depl*=-1;}		// Déplacement auto du vaisseau
			else if(x<=x_togo && depl<=0) {x_togo=x+rand()%(LARGEUR_FENETRE-x-25); depl*=-1;}

			if(rand()%20==0 && actif==1){							// Tirs automatiques
				Shoot();
			}
			if(rand()%20==0){
				EnnemyShoot();
			}			
			UpdateLevel();
			UpdateStarfield();

			DeplacerAircraft();
			DeplacerBoulets();								
			DeplacerEnnemis();
			CheckCollisions();
			CheckExplosions();

			// Texte
			DrawSinusScroll();
			DrawBullets();
			DrawAircraft();
			DrawEnnemies();
			DrawScore();
			DisplayLives();

			if(timer%20<10){						// INSERT COIN clignotant
				SetTextColor(hdcBackBuffer,RGB(255,0,0));
				TextOut(hdcBackBuffer,2,0,"INSERT COIN",11);
			}
			break;

		case GAME_STATE_NEW_LEVEL:
			if(new_timer==0){
				new_timer=timer;
			}
			if(timer-new_timer>100){
				new_timer=0;
				game=previous_game;
			}

			//UpdateLevel();
			UpdateStarfield();

			x=LARGEUR_FENETRE/2-8;
			DeplacerBoulets();								
			DeplacerEnnemis();
			CheckExplosions();

			// Texte
			DrawSinusScroll();
			DrawBullets();
			DrawAircraft();
			DrawEnnemies();
			DrawScore();
			DisplayLives();

			char level_txt[15];
			sprintf(level_txt,"STAGE %u",level+1);
			SetTextColor(hdcBackBuffer,RGB(0,0xFA,0x9A));
			if(timer-new_timer<34)
				TextOut(hdcBackBuffer,130,HAUTEUR_FENETRE/2-6,"PLAYER 1",8);
			if((timer-new_timer>34) && (timer-new_timer<67))
				TextOut(hdcBackBuffer,133,HAUTEUR_FENETRE/2-6,level_txt,7);
			if(timer-new_timer>67){
				TextOut(hdcBackBuffer,130,HAUTEUR_FENETRE/2-6-12,"PLAYER 1",8);
				TextOut(hdcBackBuffer,133,HAUTEUR_FENETRE/2-6,level_txt,7);
			}
			break;

		case GAME_STATE_GAME_RUN:
			previous_game=GAME_STATE_GAME_RUN;
			if((GetAsyncKeyState(VK_TAB)&0xC000)!=0){	// Switche le mode du jeu
				ReinitEnnemies();
				if(score>hiscore)
					hiscore=score;
				score=0;
				depl=1;
				game=GAME_STATE_DEMO_RUN;
				previous_game=GAME_STATE_DEMO_RUN;
				actif=1;
			}

			depl=0;									// Déplacement au clavier
			if(GetAsyncKeyState(VK_LEFT))
				if(x>2)
					depl=-1;
			if(GetAsyncKeyState(VK_RIGHT))
				if(x<LARGEUR_FENETRE-18)
					depl=1;

			if((GetAsyncKeyState(VK_CONTROL)&0xC000)!=0 && actif==1){
				Shoot();
			}
			if(rand()%10==0){
				EnnemyShoot();
			}

			UpdateLevel();

			UpdateStarfield();

			DeplacerAircraft();
			DeplacerBoulets();								
			DeplacerEnnemis();
			CheckCollisions();
			CheckExplosions();

			// Texte
			DrawSinusScroll();
			DrawBullets();
			DrawAircraft();
			DrawEnnemies();
			DrawScore();
			DisplayLives();

			TextOut(hdcBackBuffer,32,0,"1UP",3);
			break;

		case GAME_STATE_GAME_OVER:

			if((GetAsyncKeyState(VK_TAB)&0xC000)!=0){	// Switche le mode du jeu
				ReinitEnnemies();
				nblives=2;
				nbswords=2;					
				game=GAME_STATE_GAME_RUN;
				actif=1;
				score=0;
			}
			x=LARGEUR_FENETRE/2-8;
			if(rand()%20==0){
				EnnemyShoot();
			}
			UpdateStarfield();
			DeplacerBoulets();								
			DeplacerEnnemis();

			// Texte
			DrawSinusScroll();
			DrawBullets();
			DrawEnnemies();
			DrawScore();

			TextOut(hdcBackBuffer,50,HAUTEUR_FENETRE-15,"GAME OVER",9);
			break;
		}
		
		// Affichage final
		BitBlt(hdcFenetre,0,0,LARGEUR_FENETRE,HAUTEUR_FENETRE,hdcBackBuffer,0,0,SRCCOPY);

	}
	DeleteObject(hbmsurfaceBackBuffer);

	DeleteDC(hdcBackBuffer);
	ReleaseDC(hwndFenetre,hdcFenetre);
}


HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask;
    BITMAP bm;

    // Create monochrome (1 bit) mask bitmap.  

    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    // Get some HDCs that are compatible with the display driver

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

    SelectObject(hdcMem, hbmColour);
    SelectObject(hdcMem2, hbmMask);

    // Set the background colour of the colour image to the colour
    // you want to be transparent.
    SetBkColor(hdcMem, crTransparent);

    // Copy the bits from the colour image to the B+W mask... everything
    // with the background colour ends up white while everythig else ends up
    // black...Just what we wanted.

    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    // Take our new mask and use it to turn the transparent colour in our
    // original colour image to black so the transparency effect will
    // work right.
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

    // Clean up.

    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);

    return hbmMask;
}


void LoadCustomFonts(){
	HRSRC hRes;
	HRSRC hResLoad;
	char *lpResLock;
	HMODULE hinstLib;
	MYPROC ProcAdd;
	unsigned long nbfonts=0;
	
	
	hinstLib=LoadLibrary("gdi32.dll");
	ProcAdd=(MYPROC)GetProcAddress(hinstLib,"AddFontMemResourceEx");
	FreeLibrary(hinstLib);

	hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_FONT1), RT_FONT);
	if (hRes == NULL)
		MessageBox(NULL,"Font not loaded","Font not loaded",MB_OK);

	hResLoad = (HRSRC)LoadResource(hInst,hRes);
	lpResLock = (char *)LockResource(hResLoad);
	if(lpResLock==0)
		MessageBox(NULL,"Font not loaded","loaded",MB_OK);

			
	(ProcAdd)(lpResLock,SizeofResource(NULL,hRes),0,&nbfonts);
	__asm{sub esp,4*4}

	hFont=CreateFont(20, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "04b30");
	if(nbfonts==0)
		MessageBox(NULL,"crap","crap",MB_OK);
	
	//hFont=CreateFont(40, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "system");
	hFontScore=CreateFont(12, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "fixedsys");
}

void CheckExplosions(){		// Explosion: décrémentation du compteur de durée
	int i;
	for(i=0;i<50;i++){
		if(e[i].type==5){
			if(e[i].t!=0) e[i].t--;
			else e[i].type=0;
		}
	}
}

void LoadSprites(){
	int i;
	hbmpSword=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SWORD));
	hbmpMaskSword = CreateBitmapMask(hbmpSword, RGB(0,0,0));

	hbmpAircraft=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_AIRCRAFT));
	hbmpMaskAircraft = CreateBitmapMask(hbmpAircraft, RGB(0,0,0));

	hbmpEnnemy1[0]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY));
	hbmpEnnemy1[1]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY2));
	hbmpEnnemy1[2]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY3));
	hbmpEnnemy1[3]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY4));
	hbmpEnnemy1[4]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_EXPLOSION));

	hbmpEnnemy2[0]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY_));
	hbmpEnnemy2[1]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY2_));
	hbmpEnnemy2[2]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY3_));
	hbmpEnnemy2[3]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ENNEMY4_));
	hbmpEnnemy2[4]=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_EXPLOSION));

	for(i=0;i<5;i++){
		hbmpMaskEnnemy1[i]=CreateBitmapMask(hbmpEnnemy1[i], RGB(0,0,0));
		hbmpMaskEnnemy2[i]=CreateBitmapMask(hbmpEnnemy2[i], RGB(0,0,0));
	}

	hbmpBigExplosion=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BIG_EXPLOSION));
}

void InitStarfield(){
	// Fills starfield with random values
	int i,j;
	for(i=0;i<LARGEUR_FENETRE;i++){
		for(j=0;j<HAUTEUR_FENETRE-20;j++){
			if(rand()%500==0){
				s.actif[i][j]=1;
				s.red[i][j]=rand()%255;
				s.green[i][j]=rand()%255;
				s.blue[i][j]=rand()%255;
			}
			else{
				s.actif[i][j]=0;
				s.red[i][j]=0;
				s.green[i][j]=0;
				s.blue[i][j]=0;
			}

		}
	}
}

void UpdateStarfield()
{
	int i,j;
	// Shift the starfield
	for(i=0; i<LARGEUR_FENETRE; i++){
		for(j = HAUTEUR_FENETRE - 20 - 1; j >= 0; j--)
		{
			s.actif[i][j+1] = s.actif[i][j];
			s.red[i][j+1] = s.red[i][j];
			s.green[i][j+1] = s.green[i][j];
			s.blue[i][j+1] = s.blue[i][j];
		}
	}

	// Fill the last line with random values
	for(i=0; i < LARGEUR_FENETRE; i++)
	{
		if(rand() % 500 == 0)
		{
			s.actif[i][0] = 1;
			s.red[i][0] = rand();
			s.green[i][0] = rand();
			s.blue[i][0] = rand();
		}
		else
		{
			s.actif[i][0] = 0;
			s.red[i][0] = 0;
			s.green[i][0] = 0;
			s.blue[i][0] = 0;
		}
	}

	for(i=0;i<LARGEUR_FENETRE;i++)
	{
		for(j=0;j<HAUTEUR_FENETRE-20;j++)
		{
			tableausurfaceBackBuffer[3 * LARGEUR_FENETRE * j + 3 * i] = s.red[i][j];
			tableausurfaceBackBuffer[3 * LARGEUR_FENETRE * j + 3 * i + 1] = s.green[i][j];
			tableausurfaceBackBuffer[3 * LARGEUR_FENETRE * j + 3 * i + 2] = s.blue[i][j];
		}
	}
}

void ReinitEnnemies()
{
	int i;
	for(i = 0; i < 50; i++)
		e[i].type=0;
}

void DrawEnnemies()
{
	// Dessine les ennemis selon leur type
	int i,j;
	HDC hdcEnnemy = CreateCompatibleDC(hdcFenetre);
	HBITMAP hbm;
	HBITMAP hbmMask;

	for(j=0;j<10;j++)
	{
		for(i=0;i<5;i++)
		{
			if(e[10*i+j].type!=0)
			{
				int type=e[10*i+j].type;

				if(timer % 20 < 10)
				{
					hbm=hbmpEnnemy1[type - 1];
					hbmMask=hbmpMaskEnnemy1[type - 1];
				}
				else
				{
					hbm=hbmpEnnemy2[type - 1];
					hbmMask=hbmpMaskEnnemy2[type - 1];
				}
				SelectObject(hdcEnnemy, hbmMask);
				BitBlt(hdcBackBuffer, e[10 * i + j].x, e[10* i + j].y, 24, 18, hdcEnnemy, 0, 0, SRCAND);
				SelectObject(hdcEnnemy, hbm);
				BitBlt(hdcBackBuffer, e[10 * i + j].x,e[10 * i + j].y, 24, 18, hdcEnnemy, 0, 0, SRCPAINT);
			}
		}
	}
	DeleteDC(hdcEnnemy);
}


void InitBullets()
{
	
	int i;
	for(i = 0;i < NB_BOULETS; i++)
		b[i].actif=0;
	for(i = 0;i < NB_E_BOULETS; i++)
		b[i].actif=0;
}

void DeplacerBoulets(){
	int i;

	// On rend dispo le boulet s'il est arrivé en haut de l'écran
	for(i=0; i < NB_BOULETS; i++)
	{
		if(b[i].y <= 4)
			b[i].actif=0;
	}

	for(i=0; i < NB_E_BOULETS; i++)
	{
		if(eb[i].y >= HAUTEUR_FENETRE - 20)
		{
			eb[i].actif = 0;
		}
	}

	// Si le boulet est encore sur l'écran on le fait bouger
	for(i=0;i<NB_BOULETS;i++)
		if(b[i].actif)
			b[i].y-=8;
	for(i=0;i<NB_E_BOULETS;i++){
		if(eb[i].actif){						
			eb[i].y+=4;
			if(eb[i].a!=0)
				eb[i].x+=(int)(4/eb[i].a);
		}
	}
}

void DrawBullets(){
	int i;
	
	for(i=0;i<NB_BOULETS;i++){
		if(b[i].actif==1){
			SetPixel(hdcBackBuffer,b[i].x,b[i].y+3,RGB(255,0,0));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y+2,RGB(255,0,0));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y+1,RGB(255,0,0));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y,RGB(255,0,0));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y,RGB(180,180,180));
			SetPixel(hdcBackBuffer,b[i].x-1,b[i].y,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x+1,b[i].y,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x-1,b[i].y-1,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y-1,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x+1,b[i].y-1,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y-2,RGB(0,0,255));
			SetPixel(hdcBackBuffer,b[i].x,b[i].y-3,RGB(0,0,255));
		}	
	}

	for(i=0;i<NB_E_BOULETS;i++){
		if(eb[i].actif==1){
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y-3,RGB(255,255,255));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y-2,RGB(255,255,255));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y-1,RGB(255,255,255));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y,RGB(255,255,255));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y,RGB(180,180,180));
			SetPixel(hdcBackBuffer,eb[i].x-1,eb[i].y,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x+1,eb[i].y,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x-1,eb[i].y+1,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y+1,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x+1,eb[i].y+1,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y+2,RGB(255,0,0));
			SetPixel(hdcBackBuffer,eb[i].x,eb[i].y+3,RGB(255,0,0));
		}	
	}
}

void CheckCollisions(){
	int i,j,l;

	// Collision boulet du vaisseau / ennemi
	if(actif==1){
		for(l=0;l<NB_BOULETS;l++){
			if(b[l].actif==1){
				for(j=0;j<10;j++){
					for(i=0;i<5;i++){
						if(e[10*i+j].type!=0){
							if(b[l].x>e[10*i+j].x && b[l].x<e[10*i+j].x+16)
								if(b[l].y>e[10*i+j].y && b[l].y<e[10*i+j].y+16){
									switch(e[10*i+j].type){
									case 1:
										score+=100;
									case 2:
										score+=100;
									case 3:
										score+=100;
										e[10*i+j].type=5;
										b[l].actif=0;
										e[10*i+j].t=3;
										break;
									case 4:
										score+=300;
										b[l].actif=0;
										e[10*i+j].type=3;												
										break;
									}
								}
						}
					}
				}
			}
		}
	}

	// Collision boulet ennemi / vaisseau
	for(l=0;l<NB_E_BOULETS;l++){
		if(eb[l].actif==1 && actif==1){
			if(eb[l].x>x && eb[l].x<x+16)
				if(eb[l].y>y && eb[l].y>y-16){
					if(nbswords==0){
						if(nblives==0){
							game=GAME_STATE_GAME_OVER;
						}
						else{
							nblives--;
							actif=0;
							sc_t=30;
							nbswords=2;
						}
					}
					else nbswords--;
					eb[l].actif=0;
				}
		}
	}

	// Collision vaisseau / vaisseau ennemi
	for(i=0;i<50;i++){
		if(e[i].type!=0 && e[i].type!=5 && actif==1){
			if(e[i].x>x && e[i].x<x+16)
				if(e[i].y>y && e[i].y>y-16){
					switch(e[i].type){
					case 1:
						score+=100;
					case 2:
						score+=100;
					case 3:
						score+=100;
						e[i].type=5;
						e[i].t=3;
						break;
					case 4:
						score+=300;
						e[i].type=3;												
						break;
					}							
					if(nbswords==0){
						if(nblives==0){
							actif=2;
						}
						else{
							nblives--;
							actif=0;
							sc_t=30;
							nbswords=2;
						}
					}
					else nbswords--;
				}
		}
	}
}

void DisplayLives(){			// Affichage du nombre de vies
	HDC hdc;
	int i;	

	hdc=CreateCompatibleDC(hdcFenetre);
	for(i=0;i<nbswords;i++){
		SelectObject(hdc, hbmpMaskSword);
		BitBlt(hdcBackBuffer, LARGEUR_FENETRE-4-8*(i+1),HAUTEUR_FENETRE-18,8,16,hdc, 0, 0, SRCAND);
		SelectObject(hdc,hbmpSword);
		BitBlt(hdcBackBuffer, LARGEUR_FENETRE-4-8*(i+1),HAUTEUR_FENETRE-18,8,16,hdc, 0, 0, SRCPAINT);
	}

	for(i=0;i<nblives;i++){
		SelectObject(hdc, hbmpMaskAircraft);
		BitBlt(hdcBackBuffer, 18*i+2,HAUTEUR_FENETRE-18, 25,25, hdc, 0, 0, SRCAND);
		SelectObject(hdc,hbmpAircraft);
		BitBlt(hdcBackBuffer, 18*i+2,HAUTEUR_FENETRE-18, 25,25, hdc, 0, 0, SRCPAINT);
	}
	DeleteDC(hdc);
}

void DrawAircraft(){
	HDC hdcAircraft;

	if(actif==1){
		hdcAircraft=CreateCompatibleDC(hdcFenetre);
		SelectObject(hdcAircraft, hbmpMaskAircraft);
		BitBlt(hdcBackBuffer, x,y, 25,25, hdcAircraft, 0, 0, SRCAND);
		SelectObject(hdcAircraft, hbmpAircraft);
		BitBlt(hdcBackBuffer, x,y, 25,25, hdcAircraft, 0, 0, SRCPAINT);
		DeleteDC(hdcAircraft);
	}
	else{
		if(actif==0){
			hdcAircraft=CreateCompatibleDC(hdcFenetre);
			SelectObject(hdcAircraft, hbmpBigExplosion);
			BitBlt(hdcBackBuffer, x,y, 32,32, hdcAircraft, 0, 0, SRCPAINT);
			DeleteDC(hdcAircraft);
		}
	}
}

void DeplacerEnnemis(){
	int i,j;
	static double ennemy_step=0;
	static int ennemy_dir=1;

	#define PI 3.14159265
	#define NB_MOUVEMENTS 3
		
	if(rand()%100==0){					// Affecte un mouvement spécial à un ennemi
		int mouv=rand()%NB_MOUVEMENTS;
		int col=rand()%10;
		j=4;
		while(j>=0 && e[10*j+col].mvt!=0)
			j--;
		if(j!=-1){							
			e[10*j+col].mvt=mouv;
			if(mouv==1)
				e[10*j+col].var1=30*3*PI/2;
			if(mouv==2)
				e[10*j+col].var1=0;
			e[10*j+col].y_togo=e[10*j+col].y;
		}
	}
	for(i=0;i<5;i++){
		for(j=0;j<10;j++){
			int left=(int)((LARGEUR_FENETRE-(10*16+9*ennemy_step))/2);
			int top=(int)((140-(5*16+4*ennemy_step))/2);

			switch(e[10*i+j].mvt){
			case 0:										// Mouvement normal
				e[10*i+j].x=(int)(left+j*(16+ennemy_step));
				e[10*i+j].y=(int)(top+i*(16+ennemy_step));
				break;
			case 1:										// Mouvement 2
				if(e[10*i+j].y<e[10*i+j].y_togo){
					e[10*i+j].mvt=0;
					e[10*i+j].x=(int)(left+j*(16+ennemy_step));
					e[10*i+j].y=(int)(top+i*(16+ennemy_step));
				}
				else{
					double param=e[10*i+j].var1;
					double x_t=sin(param/30)*cos(3*param/30);
					double y_t=cos(param/30);

					e[10*i+j].x_togo=(int)(left+j*(16+ennemy_step));
					e[10*i+j].y_togo=(int)(top+i*(16+ennemy_step));

					e[10*i+j].x=(int)(120*x_t+e[10*i+j].x_togo);
					e[10*i+j].y=(int)(120*y_t+e[10*i+j].y_togo);
					e[10*i+j].var1++;
				}
				break;
			case 2:
				if(e[10*i+j].var1>30*(2*PI)){
					e[10*i+j].mvt=0;
					e[10*i+j].x=(int)(left+j*(16+ennemy_step));
					e[10*i+j].y=(int)(top+i*(16+ennemy_step));
					e[10*i+j].var1=0;
				}
				else{
					double param=e[10*i+j].var1;
					double x_t=sin(3*param/30);
					double y_t=sin(param/30)*cos(param/30);
					e[10*i+j].x_togo=(int)(left+j*(16+ennemy_step));
					e[10*i+j].y_togo=(int)(top+i*(16+ennemy_step));
					e[10*i+j].x=(int)(120*x_t+e[10*i+j].x_togo);
					e[10*i+j].y=(int)(120*y_t+e[10*i+j].y_togo);
					e[10*i+j].var1++;
				}
				break;
			}
		}
	}
	
	ennemy_step+=0.3*ennemy_dir;
	if(ennemy_step>=10){
		ennemy_dir=-1;
	}
	if(ennemy_step<0){
		ennemy_step=0;
		ennemy_dir=1;
	}
}

void Shoot(){
	int i=0;
	while(i<=NB_BOULETS && b[i].actif==1)
		i++;
		if(i!=NB_BOULETS){
			if(b[i].actif==0){
				b[i].actif=1;
				b[i].x=x+8;
				b[i].y=HAUTEUR_FENETRE-41;
		}
	}
}

void EnnemyShoot(){
	int i=0,j;
	while(i<=NB_E_BOULETS && eb[i].actif==1)
		i++;
	if(i!=NB_E_BOULETS){
		if(eb[i].actif==0){
			// On cherche qui va lancer le boulet
			int col=rand()%10;
			j=4;
			while(j>=0 && (e[10*j+col].type==0 || e[10*j+col].mvt!=0))
				j--;
			if(j!=-1){								
				eb[i].actif=1;
				eb[i].x=e[10*j+col].x+8;
				eb[i].y=e[10*j+col].y;
				eb[i].a=(y+8-(eb[i].y));
				eb[i].a/=(x+8-(eb[i].x));
			}
		}
	}

}


void DrawSinusScroll(){
	unsigned char *arTexteBuffer;
	unsigned char *arMapTexteBuffer;

	HBITMAP hbmTexte,hbmMapTexte;
	HDC hdcTexte, hdcMapTexte;
	static BITMAPINFO bmi;	
	int i,j;
	static unsigned int scroll_y=0;
	static unsigned int nbphr=0;

	#define NB_PHRASES 5

	char fff[NB_PHRASES][150]={
		PROGNAME " - Keygen by jB",
		"Greets fly out to:",
		"all FFF members, Forumcrack members, Bigbang, eedy31, Neitsa, snatch, phoenix, WiteG, chendler, Amenesia, Beggins, mac, pDriLl, Brainrain"
	};

	bmi.bmiHeader.biSize			=   sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			=	LARGEUR_FENETRE ;
	bmi.bmiHeader.biHeight			=	-HAUTEUR_FENETRE ;
	bmi.bmiHeader.biPlanes			=	1 ;
	bmi.bmiHeader.biBitCount		=	BITS_PAR_PIXEL ;  //16 = 16 bits  24 = 24 bits et 32 = 32 bits
	bmi.bmiHeader.biCompression		=	BI_RGB ;
	bmi.bmiHeader.biSizeImage		=	0	;
	bmi.bmiHeader.biXPelsPerMeter	=	0 ;
	bmi.bmiHeader.biYPelsPerMeter	=	0 ;
	bmi.bmiHeader.biClrUsed			=	0 ;
	bmi.bmiHeader.biClrImportant	=	0 ;

	hdcTexte = CreateCompatibleDC(hdcFenetre);
	hdcMapTexte = CreateCompatibleDC(hdcFenetre);

	hbmTexte = CreateDIBSection(hdcFenetre,&bmi,DIB_RGB_COLORS,(void **)&arTexteBuffer,NULL,0);
	hbmMapTexte = CreateDIBSection(hdcFenetre,&bmi,DIB_RGB_COLORS,(void **)&arMapTexteBuffer,NULL,0);

	for(i=0;i<HAUTEUR_FENETRE;i++){
		for(j=0;j<LARGEUR_FENETRE*3;j+=3){			
		arMapTexteBuffer[3*i*LARGEUR_FENETRE+j] = j*8%255;
		arMapTexteBuffer[3*i*LARGEUR_FENETRE+j+1] = j*5%255;
		arMapTexteBuffer[3*i*LARGEUR_FENETRE+j+2] = j*2%255;
		}
	}

	SelectObject(hdcTexte,hbmTexte);
	SelectObject(hdcMapTexte,hbmMapTexte);
	SetBkColor(hdcTexte,TRANSPARENT);

	SetTextColor(hdcTexte,RGB(255,255,255));

	if(hFont==NULL)
		MessageBox(NULL,"Error","Font can't be loaded",MB_OK);
	SelectObject(hdcTexte,hFont);
	
	for(i=0; i<(int)(strlen(fff[nbphr])); i++){
		double g=scroll_y+20*i;
		g/=15;
		double f=30*sin(g/3);
		TextOut(hdcTexte,LARGEUR_FENETRE-scroll_y+20*i,(int)f+HAUTEUR_FENETRE/2-20,fff[nbphr]+i,1);
	}
	
	scroll_y=(scroll_y+3);

	if(scroll_y>(LARGEUR_FENETRE+20*strlen(fff[nbphr]))){		// Changement de texte (en boucle)
		scroll_y=0;
		nbphr=(nbphr+1)%NB_PHRASES;
	}

	BitBlt(hdcTexte, 0,0, LARGEUR_FENETRE,HAUTEUR_FENETRE, hdcMapTexte, 0, 0, SRCAND);	
	BitBlt(hdcBackBuffer, 0,0, LARGEUR_FENETRE,HAUTEUR_FENETRE, hdcTexte, 0, 0, SRCPAINT);

	DeleteObject(hbmTexte);
	DeleteObject(hbmMapTexte);
	DeleteDC(hdcTexte);
	DeleteDC(hdcMapTexte);
}

void DrawScore(){
	char txt_score[15];
	char txt_hiscore[7];

	SetBkMode(hdcBackBuffer,TRANSPARENT);
	sprintf(txt_score,"% 6lu",score);
	sprintf(txt_hiscore,"% 6lu",hiscore);
	SetTextColor(hdcBackBuffer,RGB(255,255,255));
	SelectObject(hdcBackBuffer,hFontScore);
	TextOut(hdcBackBuffer,0,14,txt_score,6);

	TextOut(hdcBackBuffer,LARGEUR_FENETRE/2-40,12,txt_hiscore,6);
	
	SetTextColor(hdcBackBuffer,RGB(255,0,0));
	TextOut(hdcBackBuffer,LARGEUR_FENETRE/2-50,0,"HIGH SCORE",10);
}

void UpdateLevel(){
	int i=0,j;
	while(e[i].type==0 && i<50)
		i++;
	if(i==50){
		level=(level+1)%4;
		for(j=0;j<50;j++)
			e[j].type=levels[50*level+j];
		game=GAME_STATE_NEW_LEVEL;
		
	}
}

void DeplacerAircraft(){
	// Ne bouge que s'il n'est pas touché
	if(actif==1){
		if(depl!=0)
			x+=3*depl;
	}
	else{
		if(actif==0){
			sc_t--;
			if(sc_t==0) actif=1;
		}
	}
}