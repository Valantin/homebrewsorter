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
#ifndef __FILEOPERATION_H__
    #define __FILEOPERATION_H__
    #ifdef __cplusplus
        extern "C" {
    #endif

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

            struct hbcategories{
                char name[262];
                char path[262];
                char category[262];
                char oldcategory[262];
                struct ScePspDateTime dateModify;
                char dateForSort[21];
                short int type;
            };

            int getHBCATList(struct homebrew HBlist[], struct hbcategories *HBCATlist, int HBfound);
            int getCATList(struct categories *CAT);
            int getHBList(struct homebrew *HBlist);

            int moveHBup(int index, struct homebrew *HBlist);
            int moveHBdown(int index, struct homebrew *HBlist);
            int saveHBlist(struct homebrew *HBlist, int HBcount);

            int moveCATup(int index, struct categories *CATlist);
            int moveCATdown(int index, struct categories *CATlist);
            int saveCATlist(struct categories *CATlist, int HBcount);

            int moveHBCATup(int index, struct hbcategories *HBCATlist);
            int moveHBCATdown(int index, struct hbcategories *HBCATlist);
            int saveHBCATlist(struct hbcategories *HBCATlist, int HBCATcount);

    #ifdef __cplusplus
        }
    #endif
#endif
