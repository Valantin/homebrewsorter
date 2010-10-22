#include <pspkernel.h>
//#include <pspctrl.h>
//#include <pspdebug.h>
#include <pspsdk.h>
#include <time.h>
#include <oslib/oslib.h>
#include "fileOperation.h"
#include "media.h"
PSP_MODULE_INFO("homebrewSorter", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(12*1024);

#define ANALOG_SENS 80
#define VERSION "1.2.1"

/* Globals: */
int runningFlag = 1;
struct homebrew HBlist[MAX_HB];
int HBcount = 0;
struct categories CATlist[MAX_CAT];
int CATcount = 0;
OSL_IMAGE *bkg,*startb,*cross,*circle,*folder,*iso,*icon0,*R,*L;
OSL_FONT *pgfFont;

/* Save list order: */
/*int saveList(){
	//non stampa XD
    oslStartDrawing();
    oslDrawImageXY(bkg, 0, 0);
    oslDrawFillRect(240, 150, 360, 250, RGB(0, 104, 139));
    oslDrawString(260,160,"Saving list order...");
	if(mode == 0)
		saveHBlist(HBlist, HBcount);
	else if(mode == 1)
		saveCATlist(CATlist, CATcount);
    oslEndDrawing();
    oslEndFrame();
    return 0;
}*/

/* Draw toolbars: */
char t[100];
char hbfound[100];
void drawToolbars(){
    oslDrawFillRect(0,0,480,15,RGBA(0,0,0,170));
    oslDrawString(5,0,"HomeBrew Sorter by Sakya");
    sprintf(hbfound,"HomeBrews found: %i", HBcount);
    oslDrawString(195,0,hbfound);
    //Current time:
    struct tm * ptm;
    time_t mytime;
    time(&mytime);
    ptm = localtime(&mytime);
    sprintf(t,"%2.2d/%2.2d/%4.4d %2.2d:%2.2d",ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900, ptm->tm_hour,ptm->tm_min);
    oslDrawString(360,0,t);
}
void getIcon0(char* filename){
    //unsigned char _header[40];
    int icon0Offset, icon1Offset;
    char file[256];
    sprintf(file,"%s/eboot.pbp",filename);
    SceUID fd = sceIoOpen(file, 0x0001/*O_RDONLY*/, 0777);
	if(fd < 0){
		icon0 = oslLoadImageFilePNG("ram:/Media/icon0.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
		return;
	}
    //sceIoRead(fd, _header, 40);
    //printf("letto header\n");
    sceIoLseek(fd, 12, SEEK_SET);
    sceIoRead(fd, &icon0Offset, 4);
    //sceIoLseek(fd, 23, SEEK_SET);
    sceIoRead(fd, &icon1Offset, 4);
    int icon0_size = icon1Offset - icon0Offset;
    sceIoLseek(fd, icon0Offset, SEEK_SET);
    unsigned char icon[icon0_size];
    if(icon0_size){
        sceIoRead(fd, icon, icon0_size);
        oslSetTempFileData(icon, icon0_size, &VF_MEMORY);
        icon0 = oslLoadImageFilePNG(oslGetTempFileName(), OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
    }else{
        icon0 = oslLoadImageFilePNG("ram:/Media/icon0.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
    }
    sceIoClose(fd);
}

/*  Main menu: */
int mainMenu(){
	int mode = 0;//0 for hb, 1 for CAT
    int skip = 0;
    int start = 27;
    int first = 0, catFirst =0, hbFirst =0;
    int total = HBcount;
    int visible = 13;
    int selected = 0,
		catSelected=0,
		hbSelected=0,
		oldSelected=-1;
    int i = 0;
    int flag=0;
    int enable = 1;
    while (!osl_quit){
        if(!skip){
            oslStartDrawing();
            oslDrawImageXY(bkg, 0, 0);
            drawToolbars();
            oslDrawFillRect(5,22,285,248,RGBA(0,0,0,170));
            oslDrawFillRect(290,22,475,113,RGBA(0,0,0,170));
			oslDrawImageXY(cross,302,35);
			oslDrawString(335,35,"Select/Release");
			oslDrawImageXY(circle,302,60);
			if(mode){
				oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,100), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
				oslDrawString(335,60,"Hide/Show icon0");
                oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			} else oslDrawString(335,60,"Hide/Show icon0");
			oslDrawImageXY(startb,295,84);
			oslDrawString(335,84,"Save List");
			oslDrawFillRect(0,254,480,272,RGBA(0,0,0,170));
            //oslDrawFillRect(290,128,475,248,RGBA(0,0,0,170));//icon
			oslDrawFillRect(290,118,475,163,RGBA(0,0,0,170));
			oslDrawImageXY(L,295,122);
			oslDrawString(355, 122, "Change view");
			if(mode == 0){
				oslDrawString(355, 135, "HomeBrew");
			} else if (mode ==1){
				oslDrawString(355, 135, "Categories");
			}
			oslDrawImageXY(R,452,122);
            //Draw menu:
            for (i=first; i<=first+visible; i++){
                if (i == selected){
                    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(20,20,20,255), RGBA(255,255,255,200), INTRAFONT_ALIGN_LEFT);
                    oslSetFont(pgfFont);
                    if(enable && !HBlist[i].type && !mode){
                        if(oldSelected != selected){
                            if(icon0!=NULL){
                                oslDeleteImage(icon0);
                            }
                            oldSelected = selected;
                        getIcon0(HBlist[i].path);
                        }
                        if(icon0!=NULL)
                            //oslDrawImageXY(icon0, 315,150);
							oslDrawImageXY(icon0, 312,168);
                    }
                }else{
                    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
                    oslSetFont(pgfFont);
                }
                if(i<total){
					if(mode == 0){//HB
						if(HBlist[i].type == 0)
							oslDrawImageXY(folder,12,start +(i - first)*oslGetImageHeight(folder));
						else 
							oslDrawImageXY(iso,12,start +(i - first)*oslGetImageHeight(folder));
						oslDrawString(15+oslGetImageWidth(folder),start +(i - first)*oslGetImageHeight(folder), HBlist[i].name);
					} else if(mode == 1){//CAT
						oslDrawImageXY(folder,12,start +(i - first)*oslGetImageHeight(folder));
						oslDrawString(15+oslGetImageWidth(folder),start +(i - first)*oslGetImageHeight(folder), CATlist[i].name);
					}
                }
            }
            oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
            oslSetFont(pgfFont);
			oslDrawString(30,256,"GUI & compatibility with game categories by Valantin");

            oslEndDrawing();
        }
        oslEndFrame();
        skip = oslSyncFrame();

        oslReadKeys();
        if (osl_keys->pressed.down) {
			if(flag==0){
					if (selected < total - 1){
						if (++selected > first + visible)
							first++;
					}
			} else {
				if(mode ==0){
					if (selected < HBcount - 1){
						moveHBdown(selected, HBlist);
						if (++selected > first + visible)
							first++;
					}
				} else if(mode ==1){
					if (selected < CATcount - 1){
						moveCATdown(selected, CATlist);
						if (++selected > first + visible)
							first++;
					}
				}
			}
        } else if (osl_keys->pressed.up){
			if(flag==0){
				if (selected > 0){
					if (--selected < first)
						first--;
				}
			} else {
				if (mode ==0){
					if (selected > 0){
						moveHBup(selected, HBlist);
						if (--selected < first)
                    	    first--;
					}
				} else if (mode ==1){
					if (selected > 0){
						moveCATup(selected, CATlist);
						if (--selected < first)
                    	    first--;
					}
				}
			}
        } else if (osl_keys->released.cross){
			//if(flag==1) flag=0; else flag=1;
			flag ^= 1;
        } else if (osl_keys->released.circle){
			//if(enable) enable =0; else enable =1;
			if(mode == 0)
				enable ^= 1;
        } else if (osl_keys->released.L || osl_keys->released.R){
            //mode ^=1;
			if(mode == 0){
				mode=1;
				hbSelected = selected;
				selected = catSelected;
				total = CATcount;
				hbFirst = first;
				first = catFirst;
			} else if(mode == 1){
				mode=0;
				catSelected = selected;
				selected = hbSelected;
				total = HBcount;
				catFirst = first;
				first = hbFirst;
			}
        } else if (osl_keys->released.start){
            //saveList();
			if(mode == 0)
				saveHBlist(HBlist, HBcount);
			else if(mode == 1)
				saveCATlist(CATlist, CATcount);
        }
    }
    return 0;
}

const OSL_VIRTUALFILENAME __image_ram_files[] = {
	{"ram:/Media/bkg.png", (void*)bkg_png, sizeof(bkg_png), &VF_MEMORY},
	{"ram:/Media/start.png", (void*)start_png, sizeof(start_png), &VF_MEMORY},
	{"ram:/Media/cross.png", (void*)cross_png, sizeof(cross_png), &VF_MEMORY},
	{"ram:/Media/circle.png", (void*)circle_png, sizeof(circle_png), &VF_MEMORY},
	{"ram:/Media/folder.png", (void*)folder_png, sizeof(folder_png), &VF_MEMORY},
	{"ram:/Media/iso.png", (void*)iso_png, sizeof(iso_png), &VF_MEMORY},
	{"ram:/Media/icon0.png", (void*)icon0_png, sizeof(icon0_png), &VF_MEMORY},
	{"ram:/Media/R.png", (void*)R_png, sizeof(R_png), &VF_MEMORY},
	{"ram:/Media/L.png", (void*)L_png, sizeof(L_png), &VF_MEMORY}
};

int initOSLib(){
    oslInit(0);
    oslInitGfx(OSL_PF_8888, 1);
    oslInitAudio();
    oslSetQuitOnLoadFailure(1);
	oslAddVirtualFileList((OSL_VIRTUALFILENAME*)__image_ram_files, oslNumberof(__image_ram_files));
	bkg = oslLoadImageFilePNG("ram:/Media/bkg.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	startb = oslLoadImageFilePNG("ram:/Media/start.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	cross = oslLoadImageFilePNG("ram:/Media/cross.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	circle = oslLoadImageFilePNG("ram:/Media/circle.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	folder = oslLoadImageFilePNG("ram:/Media/folder.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	iso = oslLoadImageFilePNG("ram:/Media/iso.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	R = oslLoadImageFilePNG("ram:/Media/R.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	L = oslLoadImageFilePNG("ram:/Media/L.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
    oslSetKeyAutorepeatInit(40);
    oslSetKeyAutorepeatInterval(10);
    oslIntraFontInit(INTRAFONT_CACHE_MED);
	pgfFont = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
    oslSetFont(pgfFont);
	oslSetKeyAnalogToDPad(ANALOG_SENS);
    return 0;
}
/* Main: */

int main(){

    initOSLib();
    tzset();
    HBcount = getHBList(HBlist);
    CATcount = getCATList(CATlist);
	printf("ci arrivo?\n");
    //while(!osl_quit)
		mainMenu();
    oslEndGfx();
	oslQuit();
    return 0;

}
