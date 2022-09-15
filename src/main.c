#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <dirent.h>

// Max number of records in data structure
#define NUMBER_OF_IS 40
#define MAX_IS_NAME_SIZE 256

#define	BUTTON_A	SDLK_SPACE
#define	BUTTON_B	SDLK_LCTRL
#define	BUTTON_X	SDLK_LSHIFT
#define	BUTTON_Y	SDLK_LALT
#define	BUTTON_START	SDLK_RETURN
#define	BUTTON_SELECT	SDLK_RCTRL
#define	BUTTON_MENU	SDLK_ESCAPE
#define	BUTTON_L2	SDLK_TAB
#define	BUTTON_R2	SDLK_BACKSPACE
#define BUTTON_RIGHT SDLK_RIGHT
#define BUTTON_LEFT SDLK_LEFT

#define ICONS_PATH "icon_packs"
#ifdef PLATFORM_PC
#define COPY_PATH "SDCARD"
#else
#define COPY_PATH "/mnt/SDCARD"
#endif


// Global vars
char is[NUMBER_OF_IS][MAX_IS_NAME_SIZE];
SDL_Surface* video;
SDL_Surface* screen;
TTF_Font* font40;
SDL_Surface* surfaceName;
SDL_Surface* surfaceIMG;
SDL_Surface* surfacePages;
SDL_Surface* surfaceArrowLeft;
SDL_Surface* surfaceArrowRight;
SDL_Rect rectArrowLeft = {22, 217, 32, 36};
SDL_Rect rectArrowRight = {586, 217, 32, 36};
SDL_Rect rectName;
SDL_Rect rectPages = {515, 425, 85, 54};
SDL_Rect *rectIcons;
SDL_Rect rectIcon1 = {27, 85, 120, 130};
SDL_Rect rectIcon2 = {182, 85, 120, 130};
SDL_Rect rectIcon3 = {337, 85, 120, 130};
SDL_Rect rectIcon4 = {492, 85, 120, 130};
SDL_Rect rectIcon5 = {27, 255, 120, 130};
SDL_Rect rectIcon6 = {182, 255, 120, 130};
SDL_Rect rectIcon7 = {337, 255, 120, 130};
SDL_Rect rectIcon8 = {492, 255, 120, 130};
SDL_Color color_white = {255, 255, 255, 0};
int nCurrentPage = 0;
int levelPage = 0;
int isCount = 0;


void logMessage(char* Message) {
    FILE *file = fopen("log_IS.txt", "a");

    char valLog[200];
    sprintf(valLog, "%s%s", Message, "\n");
    fputs(valLog, file);
    fclose(file);
}

bool file_exists(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int pngFilter(const struct dirent *entry) {
   return (1 == endsWith(entry->d_name, ".png"));
}

int alphasort_no_case(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

// Draw the icon set #nis
void showIS(int nis) {
    char cISPath[250];
    char cPages[10];
    char icons[8][MAX_IS_NAME_SIZE];
    int iconsCount = 0;

    struct dirent **files;
    sprintf(cISPath, ICONS_PATH"/%s/Icons", is[nis]);
    int n = scandir(cISPath, &files, pngFilter, alphasort_no_case);
    if (n < 0) {
        perror("Couldn't open the directory");
        exit(EXIT_FAILURE);
    } else if (n > 8) {
        n = 8;
    }
    for (int i = 0; i < n; i++) {
        struct dirent *ent = files[i];
        if (ent->d_type == DT_REG)  {
            sprintf(cISPath, ICONS_PATH"/%s/Icons/%s", is[nis], ent->d_name);
            if (file_exists(cISPath) == 1) {
                strcpy(icons[iconsCount], ent->d_name);
                iconsCount++;
            }
        }
        free(files[i]);
    }
    free(files);

    sprintf(cISPath, "ressources/background.png");
    surfaceIMG = IMG_Load(cISPath);
    SDL_BlitSurface(surfaceIMG, NULL, screen, NULL);
    SDL_FreeSurface(surfaceIMG);
    for (int i = 0; i < iconsCount; i++) {
        sprintf(cISPath, "icon_packs/%s/Icons/%s", is[nis], icons[i]);
        surfaceIMG = IMG_Load(cISPath);
        SDL_BlitSurface(surfaceIMG, NULL, screen, &rectIcons[i]);
        SDL_FreeSurface(surfaceIMG);
    }

    if (nis != 0) {
        SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
    }
    if (nis != (isCount-1)) {
        SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
    }
    surfaceName = TTF_RenderUTF8_Blended(font40, is[nis], color_white);
    rectName = {320 - surfaceName->w/2, 5, surfaceName->w, surfaceName->h};
    SDL_BlitSurface(surfaceName, NULL, screen, &rectName);
    SDL_FreeSurface(surfaceName);
    sprintf(cPages, "%d/%d", (nis+1), isCount);
    surfacePages = TTF_RenderUTF8_Blended(font40, cPages, color_white);
    SDL_BlitSurface(surfacePages, NULL, screen, &rectPages);
    SDL_FreeSurface(surfacePages);
}

int main(void) {
    int running = 1;
    char cISPath[250];
    char cCommand[250];

    rectIcons = (SDL_Rect *) malloc (8 * sizeof(SDL_Rect));
    rectIcons[0] = rectIcon1;
    rectIcons[1] = rectIcon2;
    rectIcons[2] = rectIcon3;
    rectIcons[3] = rectIcon4;
    rectIcons[4] = rectIcon5;
    rectIcons[5] = rectIcon6;
    rectIcons[6] = rectIcon7;
    rectIcons[7] = rectIcon8;

    struct dirent **files;
    int n = scandir(ICONS_PATH, &files, NULL, alphasort_no_case);
    if (n < 0) {
        perror("Couldn't open the directory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        struct dirent *ent = files[i];
        if (ent->d_type == DT_DIR)  {
            sprintf(cISPath, ICONS_PATH"/%s/Icons", ent->d_name);
            if (file_exists(cISPath) == 1) {
                strcpy(is[isCount], ent->d_name);
                isCount++;
            }
        }
        free(files[i]);
    }
    free(files);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();
    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    surfaceArrowLeft = IMG_Load("ressources/arrowLeft.png");
    surfaceArrowRight = IMG_Load("ressources/arrowRight.png");
    font40 = TTF_OpenFont("ressources/Exo-2-Bold-Italic.ttf", 40);

    showIS(nCurrentPage);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (((int)event.key.keysym.sym) == BUTTON_B) {
                    if (levelPage==0) {
                        //exit program
                        running = 0;
                    } else {
                        levelPage = 0;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_A) {
                    if (levelPage == 1) {
                        // Install IS
                        sprintf(cCommand, "cp -r %s/%s/* %s; sync", ICONS_PATH, is[nCurrentPage], COPY_PATH);
                        system(cCommand);
                        running = 0;
                    } else {
                        levelPage = 1;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_RIGHT) {
                    if (nCurrentPage < (isCount-1)){
                        nCurrentPage ++;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_LEFT) {
                    if (nCurrentPage > 0){
                        nCurrentPage --;
                    }
                }
            } else if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Show IS #nCurrentPage
            showIS(nCurrentPage);
            if (levelPage == 1) {
                // Show confirmation alert
                surfaceIMG = IMG_Load("ressources/confirm.png");
                SDL_BlitSurface(surfaceIMG, NULL, screen, NULL);
                SDL_FreeSurface(surfaceIMG);
            }

        }
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }

    SDL_FreeSurface(surfaceArrowLeft);
    SDL_FreeSurface(surfaceArrowRight);

    return EXIT_SUCCESS;
}
