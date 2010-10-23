#include <pspiofilemgr.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>

void sceIoMove(char *oldPath,char *newPath) { 
    SceUID oldFile; 
    SceUID newFile;    
    int readSize; 
    char filebuf[0x8000]; 
    oldFile = sceIoOpen(oldPath, PSP_O_RDONLY, 0777); 
    newFile = sceIoOpen(newPath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777); 
    while ((readSize = sceIoRead(oldFile, filebuf, 0x08000)) > 0) { 
        sceIoWrite(newFile, filebuf, readSize);
    }
    sceIoClose(newFile);
    sceIoClose(oldFile);
    sceIoRemove(oldPath);	
}

sceIoMvdir(char *oldPath, char *newPath){
    sceIoMkdir(destpath, 0777);
    char *fullOldPath;
    char *fullNewPath;
    SceIoDirent oneDir;
    int oDir = sceIoDopen(path);
    if (oDir < 0){
        return;
    }
	while (1){
        memset(&oneDir, 0, sizeof(SceIoDirent));
        if (sceIoDread(oDir, &oneDir) <= 0)
            break;
        if (!strcmp(oneDir.d_name, ".") || !strcmp(oneDir.d_name, ".."))
            continue;
        if (path[strlen(oldPath)-1] != '/'){
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+2,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+2,sizeof(char));
            sprintf(fullOldPath,"%/%",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%/%",newPath,oneDir.d_name);
        } else {
            fullOldPath = (char *)calloc(strlen(oldPath)+strlen(oneDir.d_name)+1,sizeof(char));
            fullNewPath = (char *)calloc(strlen(newPath)+strlen(oneDir.d_name)+1,sizeof(char));
            sprintf(fullOldPath,"%%",oldPath,oneDir.d_name);
            sprintf(fullNewPath,"%%",newPath,oneDir.d_name);
        }
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            sceIoMvdir(fullOldPath,fullNewPath);
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)){
            sceIoMove(fullOldPath,fullNewPath);
        }
        free(fullOldPath);
        free(fullNewPath);
    }
	sceIoDclose(oDir);
	sceIoRmdir(path);
}
