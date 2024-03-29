/*
    PSP HomeBrewSorter GUI
        Copyright (C) valantin <valantin89@gmail.com>

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "fileOperation.h"
#include "dirOperation.h"

/* Get extension of a file:*/

static short int CATfound=0;
static short int HBCATfound=0;
static struct categories CATlist[MAX_CAT], CAT_nodup[MAX_CAT];

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
/*
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
}*/

int getExtension(char *fileName, char *extension, int extMaxLength) {
    char *extpos;
    extpos = strrchr(fileName, '.');
    extpos++;
    if(strlen(extpos) > extMaxLength){ free(extpos); return -1; }
    int i = 0;
    while(i < strlen(extpos)){
        extension[i] = toupper(extpos[i]);
        ++i;
    }
    free(extpos);
    return 0;
}

/*int getExtension(char *fileName, char *extension, int extMaxLength){
    if(strncmp(fileName+(strlen(fileName)-extMaxLength-1),".",1)) return -1;
    strcpy(extension,fileName+(strlen(fileName)-extMaxLength));
    int i = 0;
    while(i <extMaxLength){
        extension[i] = toupper(extension[i]);
        ++i;
    }
    return 0;
}*/

static char cur_cat[262] = "Uncategorized";
int check(struct homebrew *HBlist, char* dir, int HBfound){
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
            char check150[8] = "";
            strncpy(check150, oneDir.d_name, 7);
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
            strncpy(checkCAT_, oneDir.d_name, 4);
            if (!stricmp(checkCAT_, "CAT_")) {
                sceIoGetstat(fullName, &stats);
                strcpy(CATlist[CATfound].name, oneDir.d_name+4);
                printf("%s\n",CATlist[CATfound].name);
                strcpy(cur_cat, CATlist[CATfound].name);
                strcpy(CATlist[CATfound].path, fullName);
                CATlist[CATfound].dateModify = stats.st_mtime;
                sprintf(CATlist[CATfound].dateForSort, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i%6.6i", stats.st_mtime.year, stats.st_mtime.month, stats.st_mtime.day, stats.st_mtime.hour, stats.st_mtime.minute, stats.st_mtime.second, stats.st_mtime.microsecond);
                CATfound++;
                HBfound=check(HBlist,fullName,HBfound);
                strcpy(cur_cat, "Uncategorized");
                continue;
            }
            if(isHomeBrew(fullName)){
                sceIoGetstat(fullName, &stats);
                strcpy(HBlist[HBfound].name, oneDir.d_name);
                strcpy(HBlist[HBfound].path, fullName);
                HBlist[HBfound].dateModify = stats.st_mtime;
                sprintf(HBlist[HBfound].dateForSort, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i%6.6i", stats.st_mtime.year, stats.st_mtime.month, stats.st_mtime.day, stats.st_mtime.hour, stats.st_mtime.minute, stats.st_mtime.second, stats.st_mtime.microsecond);
                HBlist[HBfound].type=0;
                strcpy(HBlist[HBfound].category, cur_cat);
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
                strcpy(HBlist[HBfound].category, cur_cat);
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

void noDupCATList(){
    HBCATfound = CATfound;
    int i = 0, j = 1, k = 1;
    while(i < CATfound){
        strcpy(CAT_nodup[i].name,CATlist[i].name);
        strcpy(CAT_nodup[i].path,CATlist[i].path);
        strcpy(CAT_nodup[i].dateForSort,CATlist[i].dateForSort);
        CAT_nodup[i].dateModify = CATlist[i].dateModify;
        ++i;
    }
    i = 0;
    while(i < HBCATfound){
        printf("%d\n",HBCATfound);
        while(j < HBCATfound){
                printf("%s %s\n",CAT_nodup[i].name,CAT_nodup[j].name);
            if(!strcmp(CAT_nodup[i].name,CAT_nodup[j].name)){
                k = j;
                while( k < HBCATfound-1 ){
                    strcpy(CAT_nodup[k].name,CAT_nodup[k+1].name);
                    strcpy(CAT_nodup[k].path,CAT_nodup[k+1].path);
                    strcpy(CAT_nodup[k].dateForSort,CAT_nodup[k+1].dateForSort);
                    CAT_nodup[k].dateModify = CAT_nodup[k].dateModify;
                    ++k;
                }
                --HBCATfound;
            }
            ++j;
        }
        ++i;
    }
    strcpy(CAT_nodup[i].name,"Uncategorized");
    HBCATfound++;
}

int getHBCATList(struct homebrew HBlist[], struct hbcategories *HBCATlist, int HBfound){
    noDupCATList();
    /*int h = 0;
    while( h < HBCATfound){
        printf("%s\n",CAT_nodup[h].name);
        h++;
    }
    printf("%d\n",HBCATfound);*/
    int k = 0;
    int j = 1;
    int i = 0;
    strcpy(HBCATlist[0].name, CAT_nodup[0].name);
    HBCATlist[0].type = 2;
    while(k < HBCATfound){
        while(i < HBfound){
            printf("k %d - j %d - i %d\n",k, j, i);
            printf("- %s - %s -\n",HBlist[i].category,CAT_nodup[k].name);
            if(!strcmp(HBlist[i].category,CAT_nodup[k].name)){
                printf("same cat\n");
                strcpy(HBCATlist[j].name,HBlist[i].name);
                strcpy(HBCATlist[j].path,HBlist[i].path);
                strcpy(HBCATlist[j].category,HBlist[i].category);
                strcpy(HBCATlist[j].oldcategory,HBlist[i].category);
                strcpy(HBCATlist[j].dateForSort,HBlist[i].dateForSort);
                HBCATlist[j].type = HBlist[i].type;
                HBCATlist[j].dateModify = HBlist[i].dateModify;
                ++j;
            }
            ++i;
        }
        ++k;
        strcpy(HBCATlist[j].name, CAT_nodup[k].name);
        HBCATlist[j].type = 2;
        ++j;
        i = 0;
    }
    return j-1;
}

/* Get homebrew list: */
int getHBList(struct homebrew *HBlist){
    memset(CATlist, 0, sizeof(CATlist));
    memset(CAT_nodup, 0, sizeof(CAT_nodup));
    CATfound = 0;
    HBCATfound=0;
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

int moveHBCATup(int index, struct hbcategories *HBCATlist){
    if (index > 0){
        if(HBCATlist[index].type != 2){
            if(HBCATlist[index - 1].type == 2) {
                strcpy(HBCATlist[index].category,HBCATlist[index - 2].category); //prendo la categoria superiore altrimenti resta la stessa.
            }
            printf("%s\n",HBCATlist[index].category);
            struct hbcategories tmp = HBCATlist[index - 1];
            HBCATlist[index - 1] = HBCATlist[index];
            HBCATlist[index] = tmp;
        } else return -1;
    }
    return 0;
}

/* Move HB down: */
int moveHBCATdown(int index, struct hbcategories *HBCATlist){
    if(HBCATlist[index].type != 2){
        if(HBCATlist[index + 1].type == 2) {
            strcpy(HBCATlist[index].category,HBCATlist[index + 1].name);
        }
        printf("%s\n",HBCATlist[index].category);
        struct hbcategories tmp = HBCATlist[index];
        HBCATlist[index] = HBCATlist[index + 1];
        HBCATlist[index + 1] = tmp;
    } else return -1;
    return 0;
}

/* Save HB list: */

int saveHBCATlist(struct hbcategories *HBCATlist, int HBCATcount){
    int i = 0;
    char *newpath;
    char oldpath[262];
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

    for (i=HBCATcount - 1; i>=0; i--){
        stat.st_mtime = start;
        stat.st_ctime = start;
        if(strcmp(HBCATlist[i].category, HBCATlist[i].oldcategory) != 0) {
            strcpy(oldpath,HBCATlist[i].path);
            newpath = strrchr(HBCATlist[i].path, '/');
            int ln = strlen(HBCATlist[i].path) - strlen(newpath);
            char *path = calloc(ln, sizeof(char));
            strncpy(path,HBCATlist[i].path, ln);
            //free(newpath);
            if(strcmp(HBCATlist[i].oldcategory,"Uncategorized") != 0) {
                newpath = strrchr(path, '/');
                ln = strlen(path) - strlen(newpath);
                //free(newpath);
                //free(path);
                path = calloc(ln, sizeof(char));
                strncpy(path,HBCATlist[i].path, ln);
            }
            if(strcmp(HBCATlist[i].category,"Uncategorized") != 0)
                 sprintf(HBCATlist[i].path,"%s/CAT_%s/%s",path,HBCATlist[i].category,HBCATlist[i].name);
            else
                 sprintf(HBCATlist[i].path,"%s/%s",path,HBCATlist[i].name);
            sceIoMvdir(oldpath,HBCATlist[i].path);
            strcpy(HBCATlist[i].oldcategory,HBCATlist[i].category);
            //free(path);
            printf("from %s to %s\n",oldpath,HBCATlist[i].path);
        }
        sceIoChstat(HBCATlist[i].path, &stat, 0x1);
        sceIoChstat(HBCATlist[i].path, &stat, 0x20);
        sceIoChstat(HBCATlist[i].path, &stat, 0x8);
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
        if(i > 0 && strcmp(HBCATlist[i].category, HBCATlist[i - 1].category) != 0) {
            start.year = ptm->tm_year + 1900;
            start.month = ptm->tm_mon + 1;
            start.day = ptm->tm_mday;
            start.hour = ptm->tm_hour;
            start.minute = ptm->tm_min;
            start.second = ptm->tm_sec;
            start.microsecond = 00;

            memset(&stat, 0, sizeof(SceIoStat));
            stat.st_mode = 0777;
		}
    }
    return 0;
}

/*int saveHBCATlist(struct hbcategories *HBCATlist, int HBCATcount){
    //char oldPath[262], newPath[262];
    //strcpy(oldPath,HBCATlist[4].path);
    //strcpy(newPath,);
    sceIoMvdir("ms0:/PSP/GAME/CAT_Emulatori/pspTwitter", "ms0:/PSP/GAME/CAT_TEST/pspTwitter_moved");
    return 0;
}*/
