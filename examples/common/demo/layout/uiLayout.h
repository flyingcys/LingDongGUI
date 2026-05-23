#ifndef __UI_LAYOUT_H__
#define __UI_LAYOUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldBase.h"

extern const ldPageFuncGroup_t uiLayoutFunc;
extern const ldPageFuncGroup_t uiGridFunc;

void uiLayoutInit(ld_scene_t *ptScene);
void uiLayoutLoop(ld_scene_t *ptScene);
void uiLayoutQuit(ld_scene_t *ptScene);

void uiGridInit(ld_scene_t *ptScene);
void uiGridLoop(ld_scene_t *ptScene);
void uiGridQuit(ld_scene_t *ptScene);

#ifdef __cplusplus
}
#endif

#endif
