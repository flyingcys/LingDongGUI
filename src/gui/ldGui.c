/*
 * Copyright (c) 2023-2025 Ou Jianbo (59935554@qq.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define __ARM_2D_IMPL__
#define __ARM_2D_INHERIT__
#include "arm_2d.h"
#include "ldGui.h"
#include <stdio.h>
#include <stdlib.h>

void ldGuiDisposeNodeTree(ld_scene_t *ptScene, ldBase_t *ptWidget)
{
    ldBase_t *ptChild;

    if (NULL == ptWidget) {
        return;
    }

    while ((ptChild = ldBaseGetChildList(ptWidget)) != NULL) {
        ldGuiDisposeNodeTree(ptScene, ptChild);
    }

    ptWidget->ptGuiFunc->depose(ptScene, ptWidget);
}
#include "ldScene0.h"
#include "ldScene1.h"

static volatile arm_2d_location_t pressLocation;
static volatile int16_t deltaMoveTime;
static volatile arm_2d_location_t prevLocation;
static void *prevWidget;
static uint8_t pageNumNow=0;
uint8_t cursorBlinkCount=0;
bool cursorBlinkFlag=false;

#if USE_SCENE_SWITCHING == 2
bool isGuiSwthcnScene=false;
static uint8_t sysSceneNum=0;
#endif
static ldPageFuncGroup_t *ptSysGuiFuncGroup[2]={0};

#ifndef LD_EMIT_SIZE
#define LD_EMIT_SIZE                    8
#endif

bool isFullWidgetUpdate=false;

ldTimer_t sysTimer10ms=0;

bool ldKeyboardHitTest(ld_scene_t *ptScene, ldKeyboard_t *ptWidget, arm_2d_location_t clickPoint);

static bool ldGuiTouchLogEnabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("TINYUI_TOUCH_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled != 0;
}

bool __arm_2d_helper_control_user_whether_ignore_node(arm_2d_control_node_t *ptRoot,
                                                       arm_2d_control_node_t *ptNode,
                                                       arm_2d_location_t tLocation)
{
    ldBase_t *widget = (ldBase_t *)ptNode;

    ARM_2D_UNUSED(ptRoot);

    if (widget == NULL || widget->widgetType != widgetTypeKeyboard)
    {
        return false;
    }

    return !ldKeyboardHitTest(NULL, (ldKeyboard_t *)widget, tLocation);
}

static const char *ldGuiWidgetTypeName(ldWidgetType_t type)
{
    switch(type)
    {
    case widgetTypeBackground: return "background";
    case widgetTypeWindow: return "window";
    case widgetTypeButton: return "button";
    case widgetTypeImage: return "image";
    case widgetTypeText: return "text";
    case widgetTypeLineEdit: return "line_edit";
    case widgetTypeGraph: return "graph";
    case widgetTypeCheckBox: return "checkbox";
    case widgetTypeSlider: return "slider";
    case widgetTypeSwitch: return "switch";
    case widgetTypeProgressBar: return "progress_bar";
    case widgetTypeGauge: return "gauge";
    case widgetTypeQRCode: return "qrcode";
    case widgetTypeDateTime: return "date_time";
    case widgetTypeIconSlider: return "icon_slider";
    case widgetTypeComboBox: return "combo_box";
    case widgetTypeArc: return "arc";
    case widgetTypeRadialMenu: return "radial_menu";
    case widgetTypeScrollSelecter: return "scroll_selecter";
    case widgetTypeLabel: return "label";
    case widgetTypeTable: return "table";
    case widgetTypeKeyboard: return "keyboard";
    case widgetTypeAnimation: return "animation";
    case widgetTypeList: return "list";
    case widgetTypeMessageBox: return "message_box";
    case widgetTypeCalendar: return "calendar";
    case widgetTypeProgressWheel: return "progress_wheel";
    case widgetTypeClock: return "clock";
    case widgetTypeCanvas: return "canvas";
    default: return "unknown";
    }
}

static const char *ldGuiSignalName(uint8_t signal)
{
    switch(signal)
    {
    case SIGNAL_NO_OPERATION: return "none";
    case SIGNAL_PRESS: return "press";
    case SIGNAL_HOLD_DOWN: return "hold";
    case SIGNAL_RELEASE: return "release";
    case SIGNAL_CLICKED_ITEM: return "clicked_item";
    case SIGNAL_FINISHED: return "finished";
    case SIGNAL_VALUE_CHANGED: return "value_changed";
    default: return "unknown";
    }
}

static void ldGuiLogHit(uint8_t touchSignal,
                        const arm_2d_location_t *point,
                        const ldBase_t *widget)
{
    arm_2d_region_t absolute_region = {0};

    if (!ldGuiTouchLogEnabled() || point == NULL) {
        return;
    }

    if (widget == NULL) {
        printf("[TINYUI_TOUCH][HIT] signal=%s point=(%d,%d) widget=null\n",
               ldGuiSignalName(touchSignal),
               point->iX,
               point->iY);
        fflush(stdout);
        return;
    }

    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t *)widget,
                                              &absolute_region,
                                              true);
    printf("[TINYUI_TOUCH][HIT] signal=%s point=(%d,%d) ld_id=%u ld_type=%s region=(%d,%d,%d,%d)\n",
           ldGuiSignalName(touchSignal),
           point->iX,
           point->iY,
           (unsigned int)widget->nameId,
           ldGuiWidgetTypeName(widget->widgetType),
           absolute_region.tLocation.iX,
           absolute_region.tLocation.iY,
           absolute_region.tSize.iWidth,
           absolute_region.tSize.iHeight);
    fflush(stdout);
}

void ldGuiClickedAction(ld_scene_t *ptScene,uint8_t touchSignal,arm_2d_location_t tLocation)
{
    ldBase_t *ptWidget=NULL;
    uint64_t u64Temp=0;

    if(ptScene==NULL)
    {
        return;
    }

    switch(touchSignal)
    {
    case SIGNAL_NO_OPERATION:
    {
        break;
    }
    case SIGNAL_PRESS:
    {
        if(ptScene->ptNodeRoot!=NULL)
        {
            ptWidget=(ldBase_t*)arm_2d_helper_control_find_node_with_location(ptScene->ptNodeRoot,tLocation);
        }
        ldGuiLogHit(touchSignal,&tLocation,ptWidget);

#if (USE_LOG_LEVEL>=LOG_LEVEL_DEBUG)
        if(ptWidget!=NULL)
        {
            LOG_DEBUG("click widget id:%d",ptWidget->nameId);
        }
#endif

        prevLocation=tLocation;
        prevWidget=ptWidget;//准备数据,释放时候使用
        pressLocation=tLocation;
        deltaMoveTime=arm_2d_helper_convert_ticks_to_ms(arm_2d_helper_get_system_timestamp());

        if(ptWidget!=NULL)
        {
            u64Temp=tLocation.iX;
            u64Temp<<=16;
            u64Temp+=tLocation.iY;
            emit(ptWidget->nameId,touchSignal,u64Temp);
        }
        break;
    }
    case SIGNAL_HOLD_DOWN:
    {
        if((prevLocation.iX!=tLocation.iX)||(prevLocation.iY!=tLocation.iY))
        {
            ptWidget=prevWidget;//不可以把static变量作为函数变量调用
            ldGuiLogHit(touchSignal,&tLocation,ptWidget);
            if(ptWidget!=NULL)
            {
                if (ldGuiTouchLogEnabled()) {
                    printf("[TINYUI_TOUCH][MOVE] press=(%d,%d) current=(%d,%d)\n",
                           pressLocation.iX,
                           pressLocation.iY,
                           tLocation.iX,
                           tLocation.iY);
                    fflush(stdout);
                }
                u64Temp=tLocation.iX-pressLocation.iX;
                u64Temp<<=16;
                u64Temp+=tLocation.iY-pressLocation.iY;
                u64Temp<<=16;
                u64Temp+=tLocation.iX;
                u64Temp<<=16;
                u64Temp+=tLocation.iY;

                emit(ptWidget->nameId,touchSignal,u64Temp);
            }
            prevLocation=tLocation;
        }
        break;
    }
    case SIGNAL_RELEASE:
    {
        ptWidget=prevWidget;
        ldGuiLogHit(touchSignal,&prevLocation,ptWidget);
        if(ptWidget!=NULL)
        {
            //cal speed
            deltaMoveTime=arm_2d_helper_convert_ticks_to_ms(arm_2d_helper_get_system_timestamp())-deltaMoveTime;
            if(deltaMoveTime<=0)
            {
                deltaMoveTime=1;
            }
            pressLocation.iX=(prevLocation.iX-pressLocation.iX);
            pressLocation.iY=(prevLocation.iY-pressLocation.iY);
            pressLocation.iX=(pressLocation.iX*100)/deltaMoveTime;
            pressLocation.iY=(pressLocation.iY*100)/deltaMoveTime;

            // x speed,y speed,x,y
            u64Temp=pressLocation.iX;
            u64Temp<<=16;
            u64Temp+=pressLocation.iY;
            u64Temp<<=16;
            u64Temp+=prevLocation.iX;
            u64Temp<<=16;
            u64Temp+=prevLocation.iY;

            emit(ptWidget->nameId,touchSignal,u64Temp);
        }
        break;
    }
    default:
        break;
    }
}

#define TOUCH_NO_CLICK           0
#define TOUCH_CLICK              1

void ldGuiTouchProcess(ld_scene_t *ptScene)
{
    arm_2d_location_t clickLocation;
    bool nowState;
    static bool prevState=TOUCH_NO_CLICK;
    uint8_t touchSignal=SIGNAL_NO_OPERATION;
    extern bool ldCfgTouchGetPoint(int16_t *x,int16_t *y);

    nowState = ldCfgTouchGetPoint(&clickLocation.iX,&clickLocation.iY);

    if(nowState==TOUCH_CLICK)
    {
        if(prevState==TOUCH_NO_CLICK)//按下,                下降沿触发
        {
            touchSignal=SIGNAL_PRESS;
        }
        else// prevState==TOUCH_CLICK 按住                低电平
        {
            touchSignal=SIGNAL_HOLD_DOWN;
        }
    }
    else// nowState==TOUCH_NO_CLICK 无按下
    {
        if(prevState==TOUCH_NO_CLICK)//无按下                高电平
        {
            touchSignal=SIGNAL_NO_OPERATION;
        }
        else// prevState==TOUCH_CLICK 按下,上升沿触发       上降沿触发
        {
            touchSignal=SIGNAL_RELEASE;
        }
    }
    prevState=nowState;

    ldGuiClickedAction(ptScene,touchSignal,clickLocation);
}

void ldGuiSceneInit(ld_scene_t *ptScene)
{
    ldMsgInit(&ptScene->ptMsgQueue,LD_EMIT_SIZE);
    LOG_DEBUG("[sys] free memory:%zu",ldGetFreeMemory());

    if(ptScene->ldGuiFuncGroup!=NULL)
    {
        ptScene->ldGuiFuncGroup->init(ptScene);
    }
#if USE_SCENE_SWITCHING == 2
    isGuiSwthcnScene=false;
#endif
    ldBaseFocusNavigateInit();
    LOG_INFO("[sys] page %s init",ptScene->ldGuiFuncGroup->pageName);
    LOG_DEBUG("[sys] after init free memory:%zu",ldGetFreeMemory());
}

void ldGuiUpdateScene(void)
{
    isFullWidgetUpdate=true;
}

void ldGuiLoad(ld_scene_t *ptScene)
{
    if(ptScene->ptNodeRoot!=NULL)
    {
        arm_ctrl_enum(ptScene->ptNodeRoot, ptItem, PREORDER_TRAVERSAL)
        {
            if(((ldBase_t*)ptItem)->ptGuiFunc->load!=NULL)
            {
                ((ldBase_t*)ptItem)->ptGuiFunc->load(ptScene,ptItem);
            }
        }
    }
}

void ldGuiFrameStart(ld_scene_t *ptScene)
{
    if(isFullWidgetUpdate==true)
    {
        if(ptScene->use_as__arm_2d_scene_t.ptPlayer!=NULL)
        {
            arm_2d_scene_player_update_scene_background(ptScene->use_as__arm_2d_scene_t.ptPlayer);
        }
        isFullWidgetUpdate=false;
    }

    if(ptScene->ldGuiFuncGroup!=NULL)
    {
        if(ptScene->ldGuiFuncGroup->loop)
        {
            ptScene->ldGuiFuncGroup->loop(ptScene);
        }
    }

    if(ptScene->ptNodeRoot!=NULL)
    {
        arm_ctrl_enum(ptScene->ptNodeRoot, ptItem, PREORDER_TRAVERSAL)
        {
            if(((ldBase_t*)ptItem)->ptGuiFunc->frameStart!=NULL)
            {
                ((ldBase_t*)ptItem)->ptGuiFunc->frameStart(ptScene,ptItem);
            }
        }
    }

    // draw arm 2d code
    if(ptScene->ldGuiFuncGroup!=NULL && ptScene->ldGuiFuncGroup->frameStart!=NULL)
    {
        ptScene->ldGuiFuncGroup->frameStart(ptScene);
    }

    if(ldTimeOut(SYS_TICK_CYCLE_MS,true,&sysTimer10ms))
    {
        xBtnTick(SYS_TICK_CYCLE_MS,ptScene);
        cursorBlinkCount++;
    }
}

void ldGuiDraw(ld_scene_t *ptScene,arm_2d_tile_t *ptTile,bool bIsNewFrame)
{
    // draw ldgui background
    if(ptScene->ptNodeRoot!=NULL)
    {
        ((ldBase_t*)ptScene->ptNodeRoot)->ptGuiFunc->show(ptScene,ptScene->ptNodeRoot,ptTile,bIsNewFrame);
    }

    // draw arm 2d code
    if(ptScene->ldGuiFuncGroup!=NULL && ptScene->ldGuiFuncGroup->draw!=NULL)
    {
        ptScene->ldGuiFuncGroup->draw(ptScene,ptTile,bIsNewFrame);
    }

    // draw ldgui code
    if(ptScene->ptNodeRoot!=NULL)
    {
        ldBase_t *child=ldBaseGetChildList((ldBase_t*)ptScene->ptNodeRoot);
        if(child!=NULL)
        {
            arm_ctrl_enum(child, ptItem, PREORDER_TRAVERSAL) {
                ((ldBase_t*)ptItem)->ptGuiFunc->show(ptScene,ptItem,ptTile,bIsNewFrame);
            }
        }
    }
}

void ldGuiFrameComplete(ld_scene_t *ptScene)
{
    if(ptScene->ptNodeRoot!=NULL)
    {
        arm_ctrl_enum(ptScene->ptNodeRoot, ptItem, PREORDER_TRAVERSAL)
        {
            if(((ldBase_t*)ptItem)->ptGuiFunc->frameComplete!=NULL)
            {
                ((ldBase_t*)ptItem)->ptGuiFunc->frameComplete(ptScene,ptItem);
            }
        }
    }

    // draw arm 2d code
    if(ptScene->ldGuiFuncGroup!=NULL && ptScene->ldGuiFuncGroup->frameComplete!=NULL)
    {
        ptScene->ldGuiFuncGroup->frameComplete(ptScene);
    }

#if USE_SCENE_SWITCHING == 2
    if(isGuiSwthcnScene)
    {
        isGuiSwthcnScene=false;
        arm_2d_scene_player_switch_to_next_scene(ptScene->use_as__arm_2d_scene_t.ptPlayer);
        prevWidget=NULL;
    }
#elif USE_SCENE_SWITCHING == 1
    if(ptSysGuiFuncGroup[0]!=ptSysGuiFuncGroup[1])
    {
        ldGuiDespose(ptScene);
        ptSysGuiFuncGroup[0]=ptSysGuiFuncGroup[1];
        ptScene->ldGuiFuncGroup=ptSysGuiFuncGroup[0];
        arm_2d_scene_player_update_scene_background(ptScene->use_as__arm_2d_scene_t.ptPlayer);
        ldGuiSceneInit(ptScene);
        prevWidget=NULL;
    }
#else
    if(ptSysGuiFuncGroup[0]!=ptSysGuiFuncGroup[1])
    {
        ldGuiDespose(ptScene);
        ptSysGuiFuncGroup[1]=ptSysGuiFuncGroup[0];
        arm_2d_scene_player_update_scene_background(ptScene->use_as__arm_2d_scene_t.ptPlayer);
        ldGuiSceneInit(ptScene);
        prevWidget=NULL;
    }
#endif
}

void ldGuiDespose(ld_scene_t *ptScene)
{
    ldBase_t *ptRoot;

    if(ptScene->ldGuiFuncGroup!=NULL)
    {
        if(ptScene->ldGuiFuncGroup->quit)
        {
            ptScene->ldGuiFuncGroup->quit(ptScene);
        }
    }

    ptRoot = (ldBase_t *)ptScene->ptNodeRoot;
    if(ptRoot != NULL)
    {
        ldGuiDisposeNodeTree(ptScene, ptRoot);
        ptScene->ptNodeRoot = NULL;
    }

    ldMsgDeinit(&ptScene->ptMsgQueue);

    LOG_INFO("[sys] page %s quit",ptScene->ldGuiFuncGroup->pageName);
}

void __ldGuiJumpPage(ldPageFuncGroup_t *ptFuncGroup,arm_2d_scene_switch_mode_t *ptMode,uint16_t switchTimeMs)
{
#if USE_SCENE_SWITCHING == 2
    ldGuiUpdateScene();

    if (sysSceneNum ==0)
    {
        sysSceneNum = 1;
    }
    else
    {
        sysSceneNum = 0;
    }

    ptSysGuiFuncGroup[sysSceneNum]=ptFuncGroup;

    __arm_2d_scene_player_set_switching_mode(&DISP0_ADAPTER,ptMode,0);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, switchTimeMs);
    isGuiSwthcnScene=true;
#elif USE_SCENE_SWITCHING == 1 || USE_SCENE_SWITCHING == 0
    ptSysGuiFuncGroup[1]=ptFuncGroup;
#endif
}

#if USE_SCENE_SWITCHING == 2
void scene0_loader(void)
{
    __arm_2d_scene0_init(&DISP0_ADAPTER,NULL,ptSysGuiFuncGroup[0]);
}

void scene1_loader(void)
{
    __arm_2d_scene1_init(&DISP0_ADAPTER,NULL,ptSysGuiFuncGroup[1]);
}

typedef void scene_loader_t(void);

static scene_loader_t * const c_SceneLoaders[] = {
    scene0_loader,
    scene1_loader,
};

void before_scene_switching_handler(void *pTarget,arm_2d_scene_player_t *ptPlayer,arm_2d_scene_t *ptScene)
{
    ARM_2D_UNUSED(pTarget);
    ARM_2D_UNUSED(ptPlayer);
    ARM_2D_UNUSED(ptScene);

    ldGuiUpdateScene();
    c_SceneLoaders[sysSceneNum]();
}
#endif

#if USE_LCD_TEST == 1
static void _ldGuiFillRect(uint16_t xs,uint16_t ys,uint16_t w, uint16_t h,ldColor color)
{
    extern void Disp0_DrawBitmap (uint32_t x,uint32_t y,uint32_t width,uint32_t height,const uint8_t *bitmap);
    for(uint16_t y=0;y<h;y++)
    {
        for(uint16_t x=0;x<w;x++)
        {
            Disp0_DrawBitmap(xs+x, ys+y, 1,1, (uint8_t*)&color);
        }
    }
}

void ldGuiLcdTest(void)
{
#define BLOCK_COLUMNS                   3
#define BLOCK_ROWS                      3

    const uint16_t bw = LD_CFG_SCREEN_WIDTH  / BLOCK_COLUMNS;
    const uint16_t bh = LD_CFG_SCREEN_HEIGHT / BLOCK_ROWS;
    const uint16_t blockWidthLast = LD_CFG_SCREEN_WIDTH-bw*BLOCK_COLUMNS;
    const uint16_t blockHeightLast = LD_CFG_SCREEN_HEIGHT-bh*BLOCK_ROWS;
    
    static const uint8_t rgb[36] = {
        0xFF,0xFF,0xFF,    0xFF,0xFF,0x00,    0xFF,0x00,0xFF,
        0x00,0xFF,0xFF,    0x00,0xFF,0x00,    0xFF,0x00,0x00,
        0x00,0x00,0xFF,    0x00,0x00,0x00,    0x80,0x80,0x80,
    };

    for(uint16_t r=0;r<BLOCK_ROWS;r++)
    {
        for(uint16_t c=0;c<BLOCK_COLUMNS;c++)
        {
            uint8_t idx = (r*BLOCK_COLUMNS + c) * 3;
            ldColor color = __RGB(rgb[idx],rgb[idx+1],rgb[idx+2]);
            uint16_t w=bw,h=bh;
            if(c==(BLOCK_COLUMNS-1))
            {
                w+=blockWidthLast;
            }
            if(r==(BLOCK_ROWS-1))
            {
                h+=blockHeightLast;
            }
            _ldGuiFillRect(c*bw, r*bh, w, h, color);
        }
    }
}
#endif

void ldGuiInit(ldPageFuncGroup_t *ptFuncGroup)
{
    arm_irq_safe
    {
        arm_2d_init();
    }

    disp_adapter0_init();
#if __DISP0_CFG_DISABLE_DEFAULT_SCENE__

    if(ptFuncGroup==NULL)
    {
        return;
    }
    ptSysGuiFuncGroup[0]=ptFuncGroup;
    ptSysGuiFuncGroup[1]=ptFuncGroup;
#if USE_SCENE_SWITCHING == 2
    arm_2d_scene_player_register_before_switching_event_handler(&DISP0_ADAPTER,before_scene_switching_handler);
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,ARM_2D_SCENE_SWITCH_MODE_NONE);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 0);
    arm_2d_scene_player_switch_to_next_scene(&DISP0_ADAPTER);
#elif USE_SCENE_SWITCHING == 1 || USE_SCENE_SWITCHING == 0
    __arm_2d_scene0_init(&DISP0_ADAPTER,NULL,ptSysGuiFuncGroup[0]);
#endif
#endif
}

void ldGuiLoop(void)
{
    disp_adapter0_task();
}
