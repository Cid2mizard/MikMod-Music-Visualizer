#include <switch.h>

#include <sys/dir.h> //DIR

#include <SDL.h>
#include <SDL_ttf.h>

#include <mikmod.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define POSBAS 550

#define FONT_SIZE 29
#define TILE_SIZE 32

typedef struct{
	u8 ancien;
	u8 nouveau;
} channeltype;
channeltype vumetreCol[32];

bool explorer = true;
bool visualizer = false;

BOOL canal [32] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

SDL_Window * 	gwindow;
SDL_Renderer * 	grenderer;
SDL_Surface *	gsurface;

TTF_Font* ttfont;

u32 kDown;

typedef struct 
{
	SDL_Texture * texture;
	SDL_Rect SrcR;
	SDL_Rect DestR;
} 
images;
images tiles, sprite;

SDL_Color WHITE = {255, 255, 255, 0};
SDL_Color BLUE = {45, 163, 201, 0};

BOOL Muted = false;
BOOL Normal = true;
BOOL Repeat = false;
BOOL Random = false;

MODULE * module = NULL;
u8 chargement;
u8 LectureEnCours = 0;
u8 Lecture = 0;
u8 nbrcanaux = 32;

u8 navigateur = 0;
u16 page_fin = 0;
s16 page_courante = 0;
s16 page_courante_sel = 0;
s16 ligne_courante_sel = 0;
u8 position_choix = 0;
u16 fichier_choix = 0;
u16 fichier_debut = 0;
u16 fichier_fin = 0;
u8 macolonne = 0;
u32 FileNumber = 0;
u8 volume = 128;
u8 zz = 0;
u8 zzz = 0;

u32 MIN = 11000;
u32 MAX = 99000;

typedef struct{
	char filename[100];
	char ext[10];
}	filetype;

filetype files[1782], mikmod[19];

PadState pad;

int compterFichier(DIR* dir)
{
	strcpy(mikmod[0].ext, "669");
	strcpy(mikmod[1].ext, "AMF");
	strcpy(mikmod[2].ext, "APUN");
	strcpy(mikmod[3].ext, "DSM");
	strcpy(mikmod[4].ext, "FAR");
	strcpy(mikmod[5].ext, "GDM");
	strcpy(mikmod[6].ext, "IMF");
	strcpy(mikmod[7].ext, "IT");
	strcpy(mikmod[8].ext, "MED");
	strcpy(mikmod[9].ext, "MOD");
	strcpy(mikmod[10].ext, "MPTM");
	strcpy(mikmod[11].ext, "MTM");
	strcpy(mikmod[12].ext, "OKT");
	strcpy(mikmod[13].ext, "S3M");
	strcpy(mikmod[14].ext, "STM");
	strcpy(mikmod[15].ext, "STX");
	strcpy(mikmod[16].ext, "ULT");
	strcpy(mikmod[17].ext, "UNI");
	strcpy(mikmod[18].ext, "XM");

	int nbr = 0;
	struct dirent* ent = NULL;

	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
		{
			memset(files[nbr].filename, '\0', sizeof(files[nbr].filename));
			strcpy(files[nbr].filename, ent->d_name);

			char * pch;
			pch = strrchr(files[nbr].filename,'.') + 1;

			bool compatible = false;
			int j;
			for (j = 0; j < 19; j++)
			{
				if (strcmp(mikmod[j].ext, strupr(pch)) == 0)
				{
					compatible = true;
				}
			}

			if (compatible == true)
				nbr++;
		}
	}

	return nbr;
}

void manipulation_fichier(u32 fichier_choix)
{
	//traitement pour lire le fichier
	Player_Stop();
	LectureEnCours = 0;
	Lecture = 1;

	//Manipulation fichier
	char debut[80]="//Musics//";
	char *fin;
	strcpy(debut, "//Musics//");
	fin = files[fichier_choix].filename;
	strcat(debut, fin);

	Player_Free(module);
	chargement = 1;
	module = Player_Load(debut,64,0);

	nbrcanaux = module->numchn;

	u8 c;
	for (c = 0; c <  nbrcanaux; c++)
	{
		canal [c] = 1;
	}

	Player_Start(module);
	Player_SetVolume(volume);

	chargement = 0;
	LectureEnCours = 1;
}

void SDL_DrawText(SDL_Renderer *renderer, TTF_Font *font, int x, int y, SDL_Color colour, const char *text)
{
	SDL_Surface *surface = TTF_RenderText_Solid(font, text, colour);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	SDL_Rect position;
	position.x = x;
	position.y = y;

	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_RenderCopy(renderer, texture, NULL, &position);
	SDL_DestroyTexture(texture);
}

void SDL_DrawTextf(SDL_Renderer *renderer, TTF_Font *font, int x, int y, SDL_Color colour, const char* text, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 256, text, args);
	SDL_DrawText(renderer, font, x, y, colour, buffer);
	va_end(args);
}

void affichage_liste_fichier()
{
	fichier_choix = (page_courante * 18) + position_choix;
	fichier_debut = page_courante * 18;
	fichier_fin = (page_courante + 1) * 18;

	if (fichier_fin > FileNumber)
	{
		fichier_fin = FileNumber;
	}
}

void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, int Srcx, int Srcy, int Destx, int Desty, int w, int h)
{
	SDL_Rect srce;
	srce.x = Srcx;
	srce.y = Srcy;
	srce.w = w;
	srce.h = h;

	SDL_Rect dest;
	dest.x = Destx;
	dest.y = Desty;
	dest.w = w;
	dest.h = h;

	SDL_RenderCopy(renderer, texture, &srce, &dest);
}

int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
		offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {

		status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
									x + offsety, y + offsetx);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
									x + offsetx, y + offsety);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
									x + offsetx, y - offsety);
		status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
									x + offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

void aff_explorer()
{
	SDL_DrawTextf(grenderer, ttfont, 50, 40, WHITE, "MikMod Music Visualizer v1.1");
	SDL_DrawTextf(grenderer, ttfont, 1020, 40, WHITE, "MikMod v%d.%d.%d",LIBMIKMOD_VERSION_MAJOR,LIBMIKMOD_VERSION_MINOR,LIBMIKMOD_REVISION);

	int f;

	for (f = fichier_debut; f < fichier_fin; f++)
	{
		if ((f- page_courante*18) == position_choix)
			SDL_DrawTextf(grenderer, ttfont, 60, 100 + (f- page_courante*18)*FONT_SIZE, WHITE, "%s", strupr(files[f].filename));
		else
			SDL_DrawTextf(grenderer, ttfont, 60, 100 + (f- page_courante*18)*FONT_SIZE, BLUE, "%s", strupr(files[f].filename));

		macolonne++;
	}

	//ME
	SDL_DrawTextf(grenderer, ttfont, 990, 650, WHITE, "Cid2mizard (2022)");

	if (navigateur == 1)
	{
		//afficher nombre de page
		u16 np;
		for (np = 0; np < page_fin + 1; np++)
		{
			SDL_RenderDrawCircle(grenderer, SCREEN_WIDTH/2 - (page_fin + 1)*12/2 + np*12, 635, 5);

			if (np == page_courante)
				SDL_RenderFillCircle(grenderer, SCREEN_WIDTH/2 - (page_fin + 1)*12/2 + np*12, 635, 3);
		}
	}
}

void aff_vumetre(u8 colonnes, u8 canal)
{
	u32 valeur;

	if (Voice_Stopped(colonnes) == 0)
	{
		valeur = (Voice_GetFrequency(colonnes)*canal);
	}
	else
	{
		valeur = 0;
	}

	u32 TRANCHE = (MAX-MIN);
	u8 i = 0;

	//echelle actuelle (MIN à MAX)
	if (valeur < MIN) {valeur = MIN;}
	else if (valeur > MAX) {valeur = MAX;}

	//commencer à zéro la tranche
	valeur -= MIN;

	//changement d'échelle (0 a 48) pour la gestion en TILES
	u8 new_valeur = ceil (  (valeur*48)  / TRANCHE);
	//u8 new_valeur = colonnes;

	//notre vumétre en 12 morceaux
	u8 MesTiles[12] = {4,4,4,4,4,4,4,4,4,4,4,4};

	u8 Morceaux_plein = new_valeur / 4;
	u8 Morceaux_reste = new_valeur - (Morceaux_plein * 4);

	for (i = 1; i <= (12 - Morceaux_plein); i++)
	{
	//effacer ceux qui ne sont pas plein en partant du haut
		MesTiles[12-i] = 0;
	}

	if (Morceaux_reste != 0)
	{
		//y'en a un qui n'est pas vide et qui n'est pas plein (ni 0, ni 4)
		MesTiles[Morceaux_plein] = Morceaux_reste;
	}

	//le sprite
	vumetreCol[colonnes].nouveau = new_valeur;

	if (vumetreCol[colonnes].ancien <= vumetreCol[colonnes].nouveau)
	{
		vumetreCol[colonnes].ancien = vumetreCol[colonnes].nouveau;
	}
	else
	{
		vumetreCol[colonnes].ancien--;
	}

	SDL_Rect rect;
	rect.x = (SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2) - 6;
	rect.y = 159;
	rect.w = nbrcanaux*TILE_SIZE + 7;
	rect.h = 395;

	SDL_SetRenderDrawColor(grenderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(grenderer, &rect);

	//afficher
	//vert foncé - 3 de haut
	renderTexture(grenderer, tiles.texture, 0, MesTiles[0]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 0, MesTiles[1]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 1*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 0, MesTiles[2]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 2*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	//vert  clair - 2 de haut
	renderTexture(grenderer, tiles.texture, 32, MesTiles[3]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 3*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 32, MesTiles[4]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 4*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	//jaune - 1 de haut
	renderTexture(grenderer, tiles.texture, 64, MesTiles[5]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 5*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	//orange pale - 2 de haut
	renderTexture(grenderer, tiles.texture, 96, MesTiles[6]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 6*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 96, MesTiles[7]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 7*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	//orange foncé - 2 de haut
	renderTexture(grenderer, tiles.texture, 128, MesTiles[8]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 8*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 128, MesTiles[9]*TILE_SIZE, SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 9*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	//rouge - 2 de haut
	renderTexture(grenderer, tiles.texture, 160, MesTiles[10]*TILE_SIZE,SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 10*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	renderTexture(grenderer, tiles.texture, 160, MesTiles[11]*TILE_SIZE,SCREEN_WIDTH/2 - nbrcanaux*TILE_SIZE/2 + colonnes*TILE_SIZE, POSBAS - TILE_SIZE - 11*TILE_SIZE, TILE_SIZE, TILE_SIZE);

	//Le Sprite
	renderTexture(grenderer, sprite.texture, 0, 0, ((SCREEN_WIDTH/2) - (nbrcanaux*TILE_SIZE)/2) + colonnes*TILE_SIZE, POSBAS - (vumetreCol[colonnes].ancien * 2 * 4) - 2, 27, 2);

	if ( LectureEnCours != 0 )
	{
		//rien ???
	}
	else
	{
		Player_Stop();
	}
}

void manageInput()
{
	padUpdate(&pad);
	kDown = padGetButtonsDown(&pad);

	if (visualizer == true)
	{
		//On change l'echelle du vusualiseur
		if (kDown & HidNpadButton_R) MAX += 11000;
		else if (kDown & HidNpadButton_L) MAX -= 11000;
		if ( MAX <= 22000 ) MAX = 22000;
		else if ( MAX >= 99000 ) MAX = 99000;
	}

	if (explorer == true)
	{
		if (kDown & HidNpadButton_Up && position_choix > 0)
		{
			position_choix--;
			affichage_liste_fichier();
		}
		else if (kDown & HidNpadButton_Down && position_choix < 17 && fichier_choix < FileNumber-1)
		{
			position_choix++;
			affichage_liste_fichier();
		}

		//PAGE
		if (kDown & HidNpadButton_Left && navigateur == 1)
		{
			if (page_courante > 0)
			{
				page_courante--;
				position_choix = 0;
				affichage_liste_fichier();
			}
		}

		if (kDown & HidNpadButton_Right && navigateur == 1)
		{
			if (page_courante < page_fin )
			{
				page_courante++;
				position_choix = 0;
				affichage_liste_fichier();
			}
		}
	}

	//La Lecture avec le bouton A
	if (kDown & HidNpadButton_A)
	{
		fichier_choix = (page_courante * 18) + position_choix;

		if (fichier_choix < fichier_fin)
		{
			//info pour le changement de page et retrouver la bonne sélection
			page_courante_sel = page_courante;
			ligne_courante_sel = position_choix;
			explorer = false;

			manipulation_fichier(fichier_choix);
			visualizer = true;
		}
	}
	//Stop la lecture avec le bouton B
	if (kDown & HidNpadButton_B)
	{
		LectureEnCours = 0;
		Player_Stop();

		u8 v = 0;
		for (v = 0; v < 32; v++)
			vumetreCol[v].ancien = 0;

		explorer = true;
		visualizer = false;
	}

	//La Pause avec le bouton X
	if (kDown & HidNpadButton_X)
	{
		Player_TogglePause();
	}
}

int main(int argc, char **argv)
{
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	//PadState pad;
	padInitializeDefault(&pad);

	hidInitializeTouchScreen();

	// Initialize
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	romfsInit();

	// Create an SDL window & renderer
	gwindow = SDL_CreateWindow("MikMod Music Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	grenderer = SDL_CreateRenderer(gwindow, -1, SDL_RENDERER_SOFTWARE);

	//TILES
	SDL_Surface *surface_TILES = SDL_LoadBMP("romfs:/resources/TILES.bmp");
	tiles.texture = SDL_CreateTextureFromSurface(grenderer, surface_TILES);
	SDL_FreeSurface(surface_TILES);

	//SPRITE
	SDL_Surface *surface_SPRITE = SDL_LoadBMP("romfs:/resources/SPRITE.bmp");
	sprite.texture = SDL_CreateTextureFromSurface(grenderer, surface_SPRITE);
	SDL_FreeSurface(surface_SPRITE);

	// Create font:
	ttfont = TTF_OpenFont("romfs:/resources/Rounded_Elegance.ttf", FONT_SIZE);

	/* register all the drivers */
	MikMod_RegisterDriver(&drv_switch);
	MikMod_RegisterAllLoaders();
	MikMod_Init("");

	DIR * rep;
	rep = opendir ("/Musics/");
	FileNumber = compterFichier(rep);

	//18 fichiers par page, maximum 99 pages (donc 99 * 18 = 1782 fichiers de possibles)
	page_courante = 0;
	affichage_liste_fichier();

	//navigateur bas, si > 18 fichiers ?
	//créer le module de navigation ?
	navigateur = 0;

	if (FileNumber > 18)
	{
		//on a plus d'une page
		navigateur = 1;
	}

	if (navigateur == 1)
	{
		page_fin = FileNumber/18;
	}

	Player_SetVolume(0);
	zz = 0;

	// Game loop:
	while (appletMainLoop())
	{
		manageInput();

		//Le Fond coloré
		SDL_SetRenderDrawColor(grenderer, 45, 45, 45, 255);

		//CLEAR
		SDL_RenderClear(grenderer);

		//Les Lignes grise
		SDL_SetRenderDrawColor(grenderer, 187, 187, 187, 255);
		SDL_RenderDrawLine(grenderer, 50, 98, 1230, 98);
		SDL_RenderDrawLine(grenderer, 50, 622, 1230, 622);

		if (explorer == true)
		{
			aff_explorer();
		}

		if (visualizer == true)
		{
			SDL_DrawTextf(grenderer, ttfont, 50, 40, WHITE, "Visualizer Mode");
			SDL_DrawTextf(grenderer, ttfont, 1020, 40, WHITE, "TIME : %d:%02d:%02d",module->sngtime/60000,module->sngtime/1000%60,module->sngtime/10%100);

			if ( !Player_Active())
			{
				LectureEnCours = 0;

				u8 e;
				for (e = 0; e < 32; e++)
				{
					canal [e] = 0;
				}
			}

			for (zzz = 0; zzz < nbrcanaux; zzz++)
			{
				if (vumetreCol[zzz].ancien <= vumetreCol[zzz].nouveau)
				{
					//que dalle
				}
				else
				{
					vumetreCol[zzz].ancien--;
				}

				aff_vumetre(zzz, canal [zzz]); 
			}

			if (!Player_Paused() && Player_Active())
				SDL_DrawTextf(grenderer, ttfont, 50, 650, WHITE, "Playing > %s ", files[fichier_choix].filename);
			else if (Player_Paused() && Player_Active())
				SDL_DrawTextf(grenderer, ttfont, 50, 650, WHITE, "Paused > %s ", files[fichier_choix].filename);
			else
				SDL_DrawTextf(grenderer, ttfont, 50, 650, WHITE, "Stopped > %s ", files[fichier_choix].filename);
		}

		//REFRESH
		SDL_RenderPresent(grenderer);

		MikMod_Update();

		if (kDown & HidNpadButton_Plus)
			break;
	}

	SDL_DestroyTexture(tiles.texture);
	SDL_DestroyTexture(sprite.texture);

	SDL_DestroyRenderer(grenderer);
	SDL_DestroyWindow(gwindow);

	TTF_Quit();
	MikMod_Exit();
	SDL_Quit();
	romfsExit();

	return EXIT_SUCCESS;
}
