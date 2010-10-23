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
#ifndef __DIROPERATION_H__
    #define __DIROPERATION_H__
    #ifdef __cplusplus
        extern "C" {
    #endif

            #include <stdio.h>
            #include <pspiofilemgr.h>
            #include <string.h>
            #include <malloc.h>

            void sceIoMove(char *oldPath, char *newPath);
            void sceIoMvdir(char *oldPath, char *newPath);

            void sceIoCopy(char *oldPath, char *newPath);
            void sceIoCpdir(char *oldPath, char *newPath);

    #ifdef __cplusplus
        }
    #endif
#endif