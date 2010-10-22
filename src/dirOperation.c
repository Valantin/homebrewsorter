void sceIoMvFile(char *path1,char *path2) { 
    SceUID file1; 
    SceUID file2;    
    int readSize; 
    char filebuf[0x8000]; 
    file1 = sceIoOpen(path1, PSP_O_RDONLY, 0777); 
    file2 = sceIoOpen(path2, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777); 
    while ((readSize = sceIoRead(file1, filebuf, 0x08000)) > 0) { 
        sceIoWrite(file2, filebuf, readSize);
    }
    sceIoClose(file2);
    sceIoClose(file1);
    sceIoRemove(file1);	
}

sceIoMvDir(char *path, char *destpath){
    sceIoMkdir(destpath, 0777);
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
        if (FIO_S_ISDIR(oneDir.d_stat.st_mode)){
            /*sceIoMkdir(destpath, 0777);
			sceIoMvDir(fullpath, fulldest);*/
        } else if(FIO_S_ISREG(oneDir.d_stat.st_mode)){
            /*sceIoMvFile(fullpath,fulldest);*/
        }
    }
	sceIoDclose(oDir);
	sceIoRmdir(path);
}