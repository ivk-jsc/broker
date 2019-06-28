/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SWF_DYNAMIC_H
#define SWF_DYNAMIC_H

#if defined(_WIN32) | defined(WIN32)
#define PLOT_API __cdecl
#else
#define PLOT_API
#endif

using PSTK_HANDLE = void *;
using PCA_HANDLE = void *;

#pragma pack(push, 4)
typedef struct _SWF_UPINFO {
  unsigned char orgname[80];  /* полное имя организации                   */
  unsigned char orgcode[8];   /* код организации                          */
  unsigned char objname[80];  /* полное имя объекта                       */
  unsigned char objcode[2];   /* код объекта                              */
  unsigned char modname[80];  /* полное имя логического модуля            */
  unsigned char modcode[2];   /* код логического модуля                   */
  unsigned char nvu[4];       /* номер вычислительной установки           */
  unsigned char netname[80];  /* полное имя сети                          */
  unsigned char userstat[10]; /* должность пользователя                   */
  unsigned char userfio[18];  /* ФИО пользователя                         */
  unsigned char logadr[4];    /* свой логический адрес                    */
  unsigned char userid[2];    /* идентификатор пользователя               */
  unsigned char userrang[2];  /* ранг пользователя                        */
  unsigned char rezerv[12];   /* резерв                                   */
} SWF_UPINFO;
#pragma pack(pop)

typedef int(PLOT_API *f_regp)(char *, PCA_HANDLE *, PSTK_HANDLE *);
typedef int(PLOT_API *f_unregp)(PCA_HANDLE);
typedef int(PLOT_API *f_wait_event)(PSTK_HANDLE, void *, int, int);
typedef int(PLOT_API *f_post_event)(PSTK_HANDLE, void *, int);
typedef int(PLOT_API *f_GetUPInfo)(PSTK_HANDLE, SWF_UPINFO *);

#endif  // SWF_DYNAMIC_H
