#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "fileOperation.h"

/* Get extension of a file:*/

static short int CATfound=0;
static struct categories CATlist[MAX_CAT];

/** Controlla che ci sia Un eboot.pbp
    \param *path
        Indirizzo nel quale controllare la presenza dell'eboot.pbp
*/
int isHomeBrew(char *path) {
    char file[262];
    if (path[strlen(path)-1] != '/')
        sprintf(file, "%s/%s", path, "eboot.pbp");
    else
        sprintf(file, "%s%s", path, "eboot.pbp");
    SceIoStat stat;
    memset(&stat, 0, sizeof(stat));
    if (sceIoGetstat(file, &stat) < 0) return 0;
    else return 1;
}

void getExtension(char *fileName, char *extension, int extMaxLength){
    int i = 0, j = 0, count = 0;
    for (i = strlen(fileName) - 1; i >= 0; i--){
        if (fileName[i] == '.'){
            if (i == strlen(fileName) - 1)
                return;
            for (j = i+1; j < strlen(fileName); j++){
                extension[count++] = toupper(fileName[j]);
                if (count > extMaxLength)
                    return;
            }
            extension[count] = '\0';
            return;
        }
    }
}

int check(struct homebrew *HBlist, char* dir,int HBfound){
    static SceIoDirent oneDir;
    char fullName[262];
    SceIoStat stats;
    int oDir = sceIoDopen(dir);
    if (oDir < 0){
        //dirScanned++;
        return HBfound;
    }
    while (1){
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if (sceIoDread(oDir, &oneDir) <= 0)
            break;
        if (!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, ".."))
            continue;
        if (dir[strlen(dir)-1] != '/')
            sprintf(fullName, "%s/%s", dir, oneDir.d_name);
        else
            sprintf(fullName, "%s%s", dir, oneDir.d_name);

            //Directory (HB):
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            //Check for 1.50 hb:
			//__SCE__ - %__SCE__ style
            int j;
            char check150[8] = "";
            strncpy(check150, oneDir.d_name, 8);
            if (!stricmp(check150, "__SCE__"))
                continue;
			//folder - folder% style
            int uDir;
            char old_format_style[262];
            strcpy(old_format_style, fullName);
            strcat(old_format_style, "%");
            uDir = sceIoDopen(old_format_style);
            if (!( uDir < 0)){//if there's a folder% for current folder, skip this one as it holds data.psp named eboot.pbp and doesn't need sorting.
                sceIoDclose(uDir);
                old_format_style[0]='\0';
                continue;
            }
            old_format_style[0]='\0';
            //check CAT_ :
            char checkCAT_[5] = "";
            strncpy(checkCAT_, oneDir.d_name, 5);
            if (!stricmp(checkCAT_, "CAT_")) {
                sceIoGetstat(fullName, &stats);
                strcpy(CATlist[CATfound].name, oneDir.d_name);
                strcpy(CATlist[CATfound].path, fullName);
                CATlist[CATfound].dateModify = stats.st_mtime;
                sprintf(CATlist[CATfound].dateForSort, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i%6.6i", stats.st_mtime.year, stats.st_mtime.month, stats.st_mtime.day, stats.st_mtime.hour, stats.st_mtime.minute, stats.st_mtime.second, stats.st_mtime.microsecond);
                CATfound++;
                HBfound=check(HBlist,fullName,HBfound);
                continue;
            }
            if(isHomeBrew(fullName)){
                sceIoGetstat(fullName, &stats);
                strcpy(HBlist[HBfound].name, oneDir.d_name);
                strcpy(HBlist[HBfound].path, fullName);
                HBlist[HBfound].dateModify = stats.st_mtime;
                sprintf(HBlist[HBfound].dateForSort, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i%6.6i", stats.st_mtime.year, stats.st_mtime.month, stats.st_mtime.day, stats.st_mtime.hour, stats.st_mtime.minute, stats.st_mtime.second, stats.st_mtime.microsecond);
                HBlist[HBfound].type=0;
                HBfound++;
            }
        } else if (FIO_S_ISREG(oneDir.d_stat.st_mode)){
            char ext[4];
            getExtension(fullName, ext, 3);
            if (!strncmp(dir, "ms0:/ISO",8) && (!stricmp(ext, "CSO") || !stricmp(ext, "ISO"))){
                strcpy(HBlist[HBfound].name, oneDir.d_name);
                sceIoGetstat(fullName, &stats);
                strcpy(HBlist[HBfound].path, fullName);
                HBlist[HBfound].dateModify = stats.st_mtime;
                sprintf(HBlist[HBfound].dateForSort, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i%6.6i", stats.st_mtime.year, stats.st_mtime.month, stats.st_mtime.day, stats.st_mtime.hour, stats.st_mtime.minute, stats.st_mtime.second, stats.st_mtime.microsecond);
                HBlist[HBfound].type=1;
                HBfound++;
            }
        }
    }
    sceIoDclose(oDir);
    return HBfound;
}

int getCATList(struct categories *CAT){
    int i = 0;
    while (i < CATfound){
        if (i == 0 || stricmp(CATlist[i-1].dateForSort, CATlist[i].dateForSort) >= 0)
            i++;
        else{
            struct categories tmp = CATlist[i];
            CATlist[i] = CATlist[i-1];
            CATlist[--i] = tmp;
        }
    }
    for(i = 0; i<CATfound; i++){
        CAT[i]=CATlist[i];
    }
    return CATfound;
}

/* Get homebrew list: */
int getHBList(struct homebrew *HBlist){
    int dirScanned = 0,
    dirToScanNumber = 0,
    HBfound = 0;
    char dirToScan[10][262];

    strcpy(dirToScan[0], "ms0:/PSP/GAME");
    strcpy(dirToScan[1], "ms0:/PSP/GAME340");
    strcpy(dirToScan[2], "ms0:/PSP/GAME352");
    strcpy(dirToScan[3], "ms0:/PSP/GAME371");
    strcpy(dirToScan[4], "ms0:/PSP/GAME380");
    strcpy(dirToScan[5], "ms0:/PSP/GAME4XX");
    strcpy(dirToScan[6], "ms0:/PSP/GAME5XX");
    strcpy(dirToScan[7], "ms0:/PSP/GAME150");
    strcpy(dirToScan[8], "ms0:/ISO");
    dirToScanNumber = 9;
    
    while (dirScanned < dirToScanNumber){
        HBfound=check(HBlist,dirToScan[dirScanned],HBfound);
        dirScanned++;
    }

    //Sort list:
    int i = 0;
    while (i < HBfound){
        if (i == 0 || stricmp(HBlist[i-1].dateForSort, HBlist[i].dateForSort) >= 0)
            i++;
        else {
            struct homebrew tmp = HBlist[i];
            HBlist[i] = HBlist[i-1];
            HBlist[--i] = tmp;
        }
    }
    return HBfound;
}

/* Move HB up: */

int moveHBup(int index, struct homebrew *HBlist){
    if (index > 0){
        struct homebrew tmp = HBlist[index];
        HBlist[index] = HBlist[index - 1];
        HBlist[index - 1] = tmp;
    }
    return 0;
}

/* Move HB down: */
int moveHBdown(int index, struct homebrew *HBlist){
    struct homebrew tmp = HBlist[index];
    HBlist[index] = HBlist[index + 1];
    HBlist[index + 1] = tmp;
    return 0;
}

/* Save HB list: */

int saveHBlist(struct homebrew *HBlist, int HBcount){
    int i = 0;
	
    struct tm * ptm;
    time_t mytime;
    time(&mytime);
    ptm = localtime(&mytime);

    SceIoStat stat;
    ScePspDateTime start;

    start.year = ptm->tm_year + 1900;
    start.month = ptm->tm_mon + 1;
    start.day = ptm->tm_mday;
    start.hour = ptm->tm_hour;
    start.minute = ptm->tm_min;
    start.second = ptm->tm_sec;
    start.microsecond = 00;

    memset(&stat, 0, sizeof(SceIoStat));
    stat.st_mode = 0777;

    for (i=HBcount - 1; i>=0; i--){
        stat.st_mtime = start;
        stat.st_ctime = start;
        sceIoChstat(HBlist[i].path, &stat, 0x1);
        sceIoChstat(HBlist[i].path, &stat, 0x20);
        sceIoChstat(HBlist[i].path, &stat, 0x8);
        if (start.second < 50){
            start.second += 10;
        }else{
            if (start.minute < 59)
                start.minute++;
            else{
                start.hour++;
                start.minute = 00;
            }
            start.second = 00;
        }
    }
    return 0;
}

int moveCATup(int index, struct categories *CATlist){
    if (index > 0){
        struct categories tmp = CATlist[index];
        CATlist[index] = CATlist[index - 1];
        CATlist[index - 1] = tmp;
    }
    return 0;
}

/* Move HB down: */
int moveCATdown(int index, struct categories *CATlist){
    struct categories tmp = CATlist[index];
    CATlist[index] = CATlist[index + 1];
    CATlist[index + 1] = tmp;
    return 0;
}

int saveCATlist(struct categories *CATlist, int CATcount){
    int i = 0;

    struct tm * ptm;
    time_t mytime;
    time(&mytime);
    ptm = localtime(&mytime);

    SceIoStat stat;
    ScePspDateTime start;

    start.year = ptm->tm_year + 1900;
    start.month = ptm->tm_mon + 1;
    start.day = ptm->tm_mday;
    start.hour = ptm->tm_hour;
    start.minute = ptm->tm_min;
    start.second = ptm->tm_sec;
    start.microsecond = 00;

    memset(&stat, 0, sizeof(SceIoStat));
    stat.st_mode = 0777;

    for (i=CATcount - 1; i>=0; i--){
        stat.st_mtime = start;
        stat.st_ctime = start;
        sceIoChstat(CATlist[i].path, &stat, 0x1);
        sceIoChstat(CATlist[i].path, &stat, 0x20);
        sceIoChstat(CATlist[i].path, &stat, 0x8);
        if (start.second < 50){
            start.second += 10;
        }else{
            if (start.minute < 59)
                start.minute++;
            else{
                start.hour++;
                start.minute = 00;
            }
            start.second = 00;
        }
    }
    return 0;
}
