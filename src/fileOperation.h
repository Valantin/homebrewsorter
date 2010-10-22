#define MAX_HB 500
#define MAX_CAT 500

struct homebrew{
    char name[262];
    char path[262];
    char category[262];
    struct ScePspDateTime dateModify;
    char dateForSort[21];
    short int type;
};

struct categories{
    char name[262];
    char path[262];
    struct ScePspDateTime dateModify;
    char dateForSort[21];
};

int getHBList(struct homebrew *HBlist);
int moveHBup(int index, struct homebrew *HBlist);
int moveHBdown(int index, struct homebrew *HBlist);
int saveHBlist(struct homebrew *HBlist, int HBcount);
