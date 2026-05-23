#include "Virtual_TFT_Port.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "arm_2d.h"
#include <time.h>
#include "arm_2d_disp_adapters.h"
#include "ldConfig.h"
#include "__arm_2d_impl.h"

#undef main



//#define monochrome_2_RGB888(color)                (color < 128 ? 0x00000000 : __RGB32(0xFF, 0xA5, 0x00))             /* nixie tube */
//#define monochrome_2_RGB888(color)                (color < 128 ? 0x00000000 : __RGB32(0x00, 200, 0x00))             /* green screen inverse */
#define monochrome_2_RGB888(color)                (color < 128 ? 0x76837a : 0x1e1a17)             /* gray screen */
//#   define monochrome_2_RGB888(color)                (color < 128 ? 0x7bd01b : 0x003700)             /* green screen 2 */
//#   define monochrome_2_RGB888(color)                (color < 128 ? 0xb6c7e7 : 0x2043a4)             /* blue screen */

#define GRAY8_2_RGB888(color)                     (((color&0xFF)<<16)+((color&0xFF)<<8)+((color&0xFF)))
#define RGB565_2_RGB888(color)                    (((color&0xF800)<<8)+((color&0x7E0)<<5)+((color&0x1F)<<3))

#define RGB888_2_GRAY8(color)                     (((((color&0xff0000)>>16)) + (((color&0xff00)>>8)) + (((color&0xff)))) / 3)
#define RGB888_2_RGB565(color)                    ((((color&0xff0000)>>19) << 11) + (((color&0xff00)>>10) << 5) + (((color&0xff)>>3)))
#define RGB888_2_monochrome(color)                ((RGB888_2_GRAY8(color) < 128 )?0:1)

// 1 8(233) 16(565) 24(888) 32(8888)
#if VT_COLOR_DEPTH == 1
#define DEV_2_VT_RGB(color)                  monochrome_2_RGB888(color)
#define VT_RGB_2_DEV(color)                  RGB888_2_monochrome(color)

static uint8_t s_tFramebuffer[3][VT_WIDTH * VT_HEIGHT];

#elif VT_COLOR_DEPTH == 8
#define DEV_2_VT_RGB(color)                  GRAY8_2_RGB888(color)
#define VT_RGB_2_DEV(color)                  RGB888_2_GRAY8(color)

static uint8_t s_tFramebuffer[3][VT_WIDTH * VT_HEIGHT];

#elif VT_COLOR_DEPTH == 16
#define DEV_2_VT_RGB(color)                  RGB565_2_RGB888(color)
#define VT_RGB_2_DEV(color)                  RGB888_2_RGB565(color)

static uint16_t s_tFramebuffer[3][VT_WIDTH * VT_HEIGHT];

#elif VT_COLOR_DEPTH == 24 || VT_COLOR_DEPTH == 32
#define DEV_2_VT_RGB(color)                 (color)
#define VT_RGB_2_DEV(color)                 (color)
static uint32_t s_tFramebuffer[3][VT_WIDTH * VT_HEIGHT];

#endif

#ifndef VT_SCALING_RATIO
#   define VT_SCALING_RATIO     1
#endif

#if VT_WIDTH >= 1024 || VT_HEIGHT >= 1024
#   define VT_WINDOW_WIDTH     (VT_WIDTH / 4)
#   define VT_WINDOW_HEIGHT    (VT_HEIGHT / 4)
#elif VT_WIDTH >= 240 || VT_HEIGHT >= 240
#   define VT_WINDOW_WIDTH     (VT_WIDTH * VT_SCALING_RATIO)
#   define VT_WINDOW_HEIGHT    (VT_HEIGHT * VT_SCALING_RATIO)
#else
#   define VT_WINDOW_WIDTH      (VT_WIDTH * 2)
#   define VT_WINDOW_HEIGHT     (VT_HEIGHT * 2)
#endif

static SDL_Window * window;
static SDL_Renderer * renderer;
static SDL_Texture * texture;
static uint32_t tft_fb[VT_WIDTH * VT_HEIGHT];
static uint32_t tft_fb2[VT_WIDTH * VT_HEIGHT];

#define VT_REFRESH_WAIT_MS         (1000 / 60)
#define VT_IDLE_EVENT_WAIT_MS      200U
#define VT_GUI_IDLE_WAIT_MS        10U
#define VT_POINTER_DRAG_COMMIT_MIN_MS   8U
#define VT_POINTER_DRAG_COMMIT_MIN_DELTA 2

uintptr_t __DISP_ADAPTER0_3FB_FB0_ADDRESS__;
uintptr_t __DISP_ADAPTER0_3FB_FB1_ADDRESS__;
uintptr_t __DISP_ADAPTER0_3FB_FB2_ADDRESS__;

static volatile bool sdl_inited = false;
static volatile bool sdl_refr_cpl = false;
static volatile bool sdl_quit_qry = false;
static volatile bool sdl_joined = false;
static volatile bool sdl_refresh_pending = false;
static volatile bool sdl_redraw_pending = false;
static SDL_mutex *s_ptRefreshMutex = NULL;
static SDL_cond *s_ptRefreshCond = NULL;
static Uint32 s_nRefreshEvent = SDL_USEREVENT;
static bool s_bDirtyRegionValid = false;
static arm_2d_region_t s_tDirtyRegion = {0};
static bool s_bLastPresentRegionValid = false;
static arm_2d_region_t s_tLastPresentRegion = {0};
static bool s_bDirtyTraceEnabled = false;

static bool left_button_is_down = false;
static int16_t last_x = 0;
static int16_t last_y = 0;
static bool s_bPointerPending = false;
static Uint32 s_nLastPointerCommitTick = 0;

typedef struct VT_sdl_pointer_state_t {
    int16_t iX;
    int16_t iY;
    bool bButtonDown;
    bool bButtonChanged;
} VT_sdl_pointer_state_t;

static VT_sdl_pointer_state_t s_tPendingPointer = {0};

bool keyUp = false;
bool keyDown = false;
bool keyLeft = false;
bool keyRight = false;
bool keyEnter = false;
bool keyEsc = false;

extern void VT_Init(void);
extern void VT_Fill_Single_Color(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_typedef color);
extern void VT_Fill_Multiple_Colors(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_typedef * color_p);
extern void VT_Set_Point(int32_t x, int32_t y, color_typedef color);
extern color_typedef VT_Get_Point(int32_t x, int32_t y);
extern void VT_Clear(color_typedef color);
extern bool VT_Mouse_Get_Point(int16_t *x,int16_t *y);


bool VT_mouse_get_location(arm_2d_location_t *ptLocation)
{
    assert(NULL != ptLocation);
    if (NULL != s_ptRefreshMutex) {
        SDL_LockMutex(s_ptRefreshMutex);
    }
    ptLocation->iX = last_x;
    ptLocation->iY = last_y;
    bool bPressed = left_button_is_down;
    if (NULL != s_ptRefreshMutex) {
        SDL_UnlockMutex(s_ptRefreshMutex);
    }
    return bPressed;
}

#if 0
int quit_filter(void * userdata, SDL_Event * event)
{
    (void)userdata;

    if(event->type == SDL_QUIT) {
        sdl_quit_qry = true;
    }

    return 1;
}
#endif

typedef struct RecursiveMutex {
    SDL_mutex *mutex;
    SDL_threadID owner;
    int lock_count;
} RecursiveMutex;

RecursiveMutex* RecursiveMutex_Create(void) 
{
    RecursiveMutex* rmutex = (RecursiveMutex*)malloc(sizeof(RecursiveMutex));
    rmutex->mutex = SDL_CreateMutex();
    rmutex->owner = 0;
    rmutex->lock_count = 0;
    return rmutex;
}

void RecursiveMutex_Lock(RecursiveMutex* rmutex) {
    assert(NULL != rmutex);

    SDL_threadID tid = SDL_ThreadID();
    if (rmutex->owner == tid) {
        // 如果是同一线程，增加锁计数
        rmutex->lock_count++;
    } else {
        // 不同线程则锁定
        SDL_LockMutex(rmutex->mutex);
        rmutex->owner = tid;
        rmutex->lock_count = 1;
    }
}

void RecursiveMutex_Unlock(RecursiveMutex* rmutex) {
    assert(NULL != rmutex);
    if (rmutex->owner == SDL_ThreadID()) {
        rmutex->lock_count--;
        if (rmutex->lock_count == 0) {
            rmutex->owner = 0;
            SDL_UnlockMutex(rmutex->mutex);
        }
    }
}

void RecursiveMutex_Destroy(RecursiveMutex* rmutex) {
    assert(NULL != rmutex);
    SDL_DestroyMutex(rmutex->mutex);
    free(rmutex);
}

static 
RecursiveMutex *s_ptGlobalMutex = NULL;

static void VT_sdl_broadcast_state(void)
{
    if ((NULL != s_ptRefreshMutex) && (NULL != s_ptRefreshCond)) {
        SDL_LockMutex(s_ptRefreshMutex);
        SDL_CondBroadcast(s_ptRefreshCond);
        SDL_UnlockMutex(s_ptRefreshMutex);
    }
}

static void VT_sdl_push_refresh_event(void)
{
    SDL_Event tEvent = {0};
    tEvent.type = s_nRefreshEvent;
    SDL_PushEvent(&tEvent);
}

static int32_t VT_sdl_abs_i32(int32_t nValue)
{
    return (nValue < 0) ? -nValue : nValue;
}

static bool VT_sdl_resolve_motion_button_locked(const SDL_MouseMotionEvent *ptMotion)
{
    bool bButtonDown = ((ptMotion->state & SDL_BUTTON_LMASK) != 0U);

    if (s_bPointerPending) {
        return s_tPendingPointer.bButtonDown;
    }

    if (!bButtonDown) {
        bButtonDown = left_button_is_down;
    }

    return bButtonDown;
}

static void VT_sdl_stage_pointer_locked(int16_t iX,
                                        int16_t iY,
                                        bool bButtonDown,
                                        bool bButtonChanged)
{
    s_tPendingPointer.iX = iX;
    s_tPendingPointer.iY = iY;
    s_tPendingPointer.bButtonDown = bButtonDown;
    s_tPendingPointer.bButtonChanged = s_tPendingPointer.bButtonChanged || bButtonChanged;
    s_bPointerPending = true;
}

static bool VT_sdl_should_commit_pending_pointer_locked(Uint32 nNow, bool bForce)
{
    int32_t nDeltaX;
    int32_t nDeltaY;
    bool bDragging;

    if (!s_bPointerPending) {
        return false;
    }

    if (bForce || s_tPendingPointer.bButtonChanged) {
        return true;
    }

    bDragging = s_tPendingPointer.bButtonDown || left_button_is_down;
    if (!bDragging) {
        return true;
    }

    nDeltaX = VT_sdl_abs_i32((int32_t)s_tPendingPointer.iX - last_x);
    nDeltaY = VT_sdl_abs_i32((int32_t)s_tPendingPointer.iY - last_y);
    if ((nDeltaX >= VT_POINTER_DRAG_COMMIT_MIN_DELTA) ||
        (nDeltaY >= VT_POINTER_DRAG_COMMIT_MIN_DELTA)) {
        return true;
    }

    return SDL_TICKS_PASSED(nNow, s_nLastPointerCommitTick + VT_POINTER_DRAG_COMMIT_MIN_MS);
}

static bool VT_sdl_commit_pending_pointer(bool bForce)
{
    bool bCommitted = false;
    Uint32 nNow = SDL_GetTicks();

    if (NULL == s_ptRefreshMutex) {
        return false;
    }

    SDL_LockMutex(s_ptRefreshMutex);
    if (VT_sdl_should_commit_pending_pointer_locked(nNow, bForce)) {
        last_x = s_tPendingPointer.iX;
        last_y = s_tPendingPointer.iY;
        left_button_is_down = s_tPendingPointer.bButtonDown;
        s_nLastPointerCommitTick = nNow;
        s_tPendingPointer.bButtonChanged = false;
        s_bPointerPending = false;
        bCommitted = true;
    }
    SDL_UnlockMutex(s_ptRefreshMutex);

    if (bCommitted) {
        VT_sdl_broadcast_state();
    }

    return bCommitted;
}

static int VT_sdl_get_event_wait_timeout(void)
{
    bool bNeedFastWakeup = false;

    if (NULL == s_ptRefreshMutex) {
        return VT_REFRESH_WAIT_MS;
    }

    SDL_LockMutex(s_ptRefreshMutex);
    bNeedFastWakeup = sdl_refresh_pending || sdl_redraw_pending || s_bPointerPending;
    SDL_UnlockMutex(s_ptRefreshMutex);

    return bNeedFastWakeup ? VT_REFRESH_WAIT_MS : (int)VT_IDLE_EVENT_WAIT_MS;
}

static bool VT_sdl_normalize_region(const arm_2d_region_t *ptRegion,
                                    arm_2d_region_t *ptClippedRegion)
{
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;

    assert(NULL != ptRegion);
    assert(NULL != ptClippedRegion);

    if ((ptRegion->tSize.iWidth <= 0) || (ptRegion->tSize.iHeight <= 0)) {
        return false;
    }

    x1 = ptRegion->tLocation.iX;
    y1 = ptRegion->tLocation.iY;
    x2 = x1 + ptRegion->tSize.iWidth - 1;
    y2 = y1 + ptRegion->tSize.iHeight - 1;

    if ((x2 < 0) || (y2 < 0) || (x1 >= VT_WIDTH) || (y1 >= VT_HEIGHT)) {
        return false;
    }

    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= VT_WIDTH) {
        x2 = VT_WIDTH - 1;
    }
    if (y2 >= VT_HEIGHT) {
        y2 = VT_HEIGHT - 1;
    }

    ptClippedRegion->tLocation.iX = (int16_t)x1;
    ptClippedRegion->tLocation.iY = (int16_t)y1;
    ptClippedRegion->tSize.iWidth = (int16_t)(x2 - x1 + 1);
    ptClippedRegion->tSize.iHeight = (int16_t)(y2 - y1 + 1);
    return true;
}

static void VT_sdl_union_dirty_region_locked(const arm_2d_region_t *ptRegion)
{
    arm_2d_region_t tClippedRegion;

    if (!VT_sdl_normalize_region(ptRegion, &tClippedRegion)) {
        return;
    }

    if (!s_bDirtyRegionValid) {
        s_tDirtyRegion = tClippedRegion;
        s_bDirtyRegionValid = true;
        return;
    }

    int32_t x1 = s_tDirtyRegion.tLocation.iX;
    int32_t y1 = s_tDirtyRegion.tLocation.iY;
    int32_t x2 = x1 + s_tDirtyRegion.tSize.iWidth - 1;
    int32_t y2 = y1 + s_tDirtyRegion.tSize.iHeight - 1;
    int32_t dirtyX1 = tClippedRegion.tLocation.iX;
    int32_t dirtyY1 = tClippedRegion.tLocation.iY;
    int32_t dirtyX2 = dirtyX1 + tClippedRegion.tSize.iWidth - 1;
    int32_t dirtyY2 = dirtyY1 + tClippedRegion.tSize.iHeight - 1;

    if (dirtyX1 < x1) {
        x1 = dirtyX1;
    }
    if (dirtyY1 < y1) {
        y1 = dirtyY1;
    }
    if (dirtyX2 > x2) {
        x2 = dirtyX2;
    }
    if (dirtyY2 > y2) {
        y2 = dirtyY2;
    }

    s_tDirtyRegion.tLocation.iX = (int16_t)x1;
    s_tDirtyRegion.tLocation.iY = (int16_t)y1;
    s_tDirtyRegion.tSize.iWidth = (int16_t)(x2 - x1 + 1);
    s_tDirtyRegion.tSize.iHeight = (int16_t)(y2 - y1 + 1);
}

void VT_sdl_mark_dirty_region(const arm_2d_region_t *ptRegion)
{
    assert(NULL != ptRegion);

    if (NULL == s_ptRefreshMutex) {
        return;
    }

    SDL_LockMutex(s_ptRefreshMutex);
    VT_sdl_union_dirty_region_locked(ptRegion);
    SDL_UnlockMutex(s_ptRefreshMutex);
}

static arm_2d_region_t VT_sdl_full_screen_region(void)
{
    arm_2d_region_t tRegion = {
        .tLocation = {
            .iX = 0,
            .iY = 0,
        },
        .tSize = {
            .iWidth = VT_WIDTH,
            .iHeight = VT_HEIGHT,
        },
    };

    return tRegion;
}

static arm_2d_region_t VT_sdl_resolve_redraw_region(bool bHasDirtyRegion,
                                                    const arm_2d_region_t *ptDirtyRegion)
{
    arm_2d_region_t tRegion = VT_sdl_full_screen_region();

    if (bHasDirtyRegion) {
        assert(NULL != ptDirtyRegion);
        return *ptDirtyRegion;
    }

    SDL_LockMutex(s_ptRefreshMutex);
    if (s_bLastPresentRegionValid) {
        tRegion = s_tLastPresentRegion;
    }
    SDL_UnlockMutex(s_ptRefreshMutex);

    return tRegion;
}

static bool VT_sdl_present_texture(const arm_2d_region_t *ptDirtyRegion)
{
    SDL_Rect tRect;
    const void *pTextureBase = NULL;

    assert(NULL != ptDirtyRegion);

    tRect.x = ptDirtyRegion->tLocation.iX;
    tRect.y = ptDirtyRegion->tLocation.iY;
    tRect.w = ptDirtyRegion->tSize.iWidth;
    tRect.h = ptDirtyRegion->tSize.iHeight;

    if (s_bDirtyTraceEnabled) {
        printf("[SDL][DIRTY] x=%d y=%d w=%d h=%d\n",
               tRect.x,
               tRect.y,
               tRect.w,
               tRect.h);
        fflush(stdout);
    }

#if VT_COLOR_DEPTH == 1
    arm_2d_region_t tValidRegionOnVirtualScree = {
        .tSize = {
            .iWidth = VT_WIDTH,
            .iHeight = VT_HEIGHT,
        },
    };

    arm_2d_filter_iir_blur_descriptor_t tBlurOP = {0};
    arm_2dp_filter_iir_blur_mode_set(&tBlurOP, ARM_IIR_BLUR_MODE_FORWARD);

    memcpy(tft_fb2, tft_fb, sizeof(tft_fb2));

    __arm_2d_impl_cccn888_filter_iir_blur(
        tft_fb2,
        VT_WIDTH,
        &tValidRegionOnVirtualScree,
        &tValidRegionOnVirtualScree,
        64,
        &tBlurOP
    );

    pTextureBase = &tft_fb2[tRect.y * VT_WIDTH + tRect.x];
#else
    pTextureBase = &tft_fb[tRect.y * VT_WIDTH + tRect.x];
#endif
    if (0 != SDL_UpdateTexture(texture, &tRect, pTextureBase, VT_WIDTH * sizeof(uint32_t))) {
        return false;
    }
    if (0 != SDL_RenderCopy(renderer, texture, &tRect, &tRect)) {
        return false;
    }
    SDL_RenderPresent(renderer);

    SDL_LockMutex(s_ptRefreshMutex);
    s_tLastPresentRegion = *ptDirtyRegion;
    s_bLastPresentRegionValid = true;
    SDL_UnlockMutex(s_ptRefreshMutex);
    return true;
}

static void VT_sdl_handle_event(const SDL_Event *ptEvent)
{
    switch(ptEvent->type) {
        case SDL_QUIT:
            SDL_LockMutex(s_ptRefreshMutex);
            sdl_quit_qry = true;
            sdl_joined = false;
            SDL_CondBroadcast(s_ptRefreshCond);
            SDL_UnlockMutex(s_ptRefreshMutex);
            break;

        case SDL_MOUSEBUTTONUP:
            if (ptEvent->button.button == SDL_BUTTON_LEFT) {
                SDL_LockMutex(s_ptRefreshMutex);
                VT_sdl_stage_pointer_locked((int16_t)ptEvent->button.x,
                                            (int16_t)ptEvent->button.y,
                                            false,
                                            true);
                SDL_UnlockMutex(s_ptRefreshMutex);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (ptEvent->button.button == SDL_BUTTON_LEFT) {
                SDL_LockMutex(s_ptRefreshMutex);
                VT_sdl_stage_pointer_locked((int16_t)ptEvent->button.x,
                                            (int16_t)ptEvent->button.y,
                                            true,
                                            true);
                SDL_UnlockMutex(s_ptRefreshMutex);
            }
            break;

        case SDL_MOUSEMOTION:
            SDL_LockMutex(s_ptRefreshMutex);
            VT_sdl_stage_pointer_locked((int16_t)ptEvent->motion.x,
                                        (int16_t)ptEvent->motion.y,
                                        VT_sdl_resolve_motion_button_locked(&ptEvent->motion),
                                        false);
            SDL_UnlockMutex(s_ptRefreshMutex);
            break;

        case SDL_WINDOWEVENT:
            switch(ptEvent->window.event) {
#if SDL_VERSION_ATLEAST(2, 0, 5)
                case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_LockMutex(s_ptRefreshMutex);
                    sdl_redraw_pending = true;
                    SDL_CondBroadcast(s_ptRefreshCond);
                    SDL_UnlockMutex(s_ptRefreshMutex);
                    break;

                default:
                    break;
            }
            break;

        case SDL_KEYDOWN:
            switch (ptEvent->key.keysym.sym)
            {
            case SDLK_UP:
                keyUp = true;
                break;
            case SDLK_DOWN:
                keyDown = true;
                break;
            case SDLK_LEFT:
                keyLeft = true;
                break;
            case SDLK_RIGHT:
                keyRight = true;
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                keyEnter = true;
                break;
            case SDLK_ESCAPE:
                keyEsc = true;
                break;
            default:
                break;
            }
            VT_sdl_broadcast_state();
            break;

        case SDL_KEYUP:
            switch (ptEvent->key.keysym.sym)
            {
            case SDLK_UP:
                keyUp = false;
                break;
            case SDLK_DOWN:
                keyDown = false;
                break;
            case SDLK_LEFT:
                keyLeft = false;
                break;
            case SDLK_RIGHT:
                keyRight = false;
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                keyEnter = false;
                break;
            case SDLK_ESCAPE:
                keyEsc = false;
                break;
            default:
                break;
            }
            VT_sdl_broadcast_state();
            break;

        default:
            break;
    }
}

void VT_enter_global_mutex(void)
{
    RecursiveMutex_Lock(s_ptGlobalMutex);
}

void VT_leave_global_mutex(void)
{
    RecursiveMutex_Unlock(s_ptGlobalMutex);
}


static void monitor_sdl_clean_up(void)
{
    RecursiveMutex_Destroy(s_ptGlobalMutex);
    SDL_DestroyCond(s_ptRefreshCond);
    SDL_DestroyMutex(s_ptRefreshMutex);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void monitor_sdl_init(void)
{
#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
    __DISP_ADAPTER0_3FB_FB0_ADDRESS__ = (uintptr_t)s_tFramebuffer[0];
    __DISP_ADAPTER0_3FB_FB1_ADDRESS__ = (uintptr_t)s_tFramebuffer[1];
    __DISP_ADAPTER0_3FB_FB2_ADDRESS__ = (uintptr_t)s_tFramebuffer[2];
#endif

    /*Initialize the SDL*/
    SDL_Init(SDL_INIT_VIDEO);

    s_ptGlobalMutex = RecursiveMutex_Create();
    s_ptRefreshMutex = SDL_CreateMutex();
    s_ptRefreshCond = SDL_CreateCond();
    s_nRefreshEvent = SDL_RegisterEvents(1);
    if ((Uint32)-1 == s_nRefreshEvent) {
        s_nRefreshEvent = SDL_USEREVENT;
    }
    s_bDirtyTraceEnabled = (NULL != getenv("SDL_DIRTY_TRACE"));

    //SDL_SetEventFilter(quit_filter, NULL);

    window = SDL_CreateWindow("ldgui Simulator",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                VT_WIDTH, VT_HEIGHT,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                                //| SDL_WINDOW_BORDERLESS                         /*last param. SDL_WINDOW_BORDERLESS to hide borders*/
                                );       
#if VT_VIRTUAL_MACHINE
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
#else
    renderer = SDL_CreateRenderer(window, -1, 0);
#endif
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, VT_WIDTH, VT_HEIGHT);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_RenderSetLogicalSize(renderer, VT_WIDTH, VT_HEIGHT);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    /*Initialize the frame buffer to gray (77 is an empirical value) */
    memset(tft_fb, 77, VT_WIDTH * VT_HEIGHT * sizeof(uint32_t));
    SDL_UpdateTexture(texture, NULL, tft_fb, VT_WIDTH * sizeof(uint32_t));
    sdl_inited = true;
}

extern
void __arm_2d_impl_cccn888_filter_iir_blur(
    uint32_t *__RESTRICT pwTarget,
    int16_t iTargetStride,
    arm_2d_region_t *__RESTRICT ptValidRegionOnVirtualScreen,
    arm_2d_region_t *ptTargetRegionOnVirtualScreen,
    uint8_t chBlurDegree,
    arm_2d_filter_iir_blur_descriptor_t *ptThis);

bool VT_sdl_refresh_task(void)
{
    SDL_Event event;
    int nEventWaitTimeout;
    bool bNeedPresent = false;
    bool bNeedRedraw = false;
    bool bNeedExposeRedraw = false;
    bool bHasDirtyRegion = false;
    arm_2d_region_t tDirtyRegion = VT_sdl_full_screen_region();

    nEventWaitTimeout = VT_sdl_get_event_wait_timeout();
    if (SDL_WaitEventTimeout(&event, nEventWaitTimeout)) {
        VT_sdl_handle_event(&event);
        while(SDL_PollEvent(&event)) {
            VT_sdl_handle_event(&event);
        }
    }

    SDL_LockMutex(s_ptRefreshMutex);
    bNeedPresent = sdl_refresh_pending;
    bNeedRedraw = sdl_redraw_pending;
    bHasDirtyRegion = s_bDirtyRegionValid;
    if (bHasDirtyRegion) {
        tDirtyRegion = s_tDirtyRegion;
    }
    sdl_refresh_pending = false;
    sdl_redraw_pending = false;
    s_bDirtyRegionValid = false;
    SDL_UnlockMutex(s_ptRefreshMutex);

    bNeedExposeRedraw = bNeedRedraw;
    if (!bNeedPresent) {
        bNeedRedraw = false;
    }
    VT_sdl_commit_pending_pointer(bNeedRedraw);

#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
    if (bNeedPresent) {
        void *pFrameBuffer = disp_adapter0_3fb_get_flush_pointer();
        color_typedef *pColorBuffer = (color_typedef *)pFrameBuffer;
        if (!bHasDirtyRegion) {
            VT_Fill_Multiple_Colors(0, 0, VT_WIDTH - 1, VT_HEIGHT - 1, pColorBuffer);
            tDirtyRegion = VT_sdl_full_screen_region();
        } else {
            pColorBuffer += tDirtyRegion.tLocation.iY * VT_WIDTH + tDirtyRegion.tLocation.iX;
            VT_Fill_Multiple_Colors(tDirtyRegion.tLocation.iX,
                                    tDirtyRegion.tLocation.iY,
                                    tDirtyRegion.tLocation.iX + tDirtyRegion.tSize.iWidth - 1,
                                    tDirtyRegion.tLocation.iY + tDirtyRegion.tSize.iHeight - 1,
                                    pColorBuffer);
        }
    }
#endif

    if (bNeedPresent || bNeedExposeRedraw) {
        arm_2d_region_t tPresentRegion = bNeedPresent
                                       ? tDirtyRegion
                                       : VT_sdl_resolve_redraw_region(bHasDirtyRegion, &tDirtyRegion);
        (void)VT_sdl_present_texture(&tPresentRegion);
    }

    if (bNeedPresent) {
        SDL_LockMutex(s_ptRefreshMutex);
        sdl_refr_cpl = true;
        SDL_CondBroadcast(s_ptRefreshCond);
        SDL_UnlockMutex(s_ptRefreshMutex);
    }

	return !sdl_joined;
}

bool VT_is_request_quit(void)
{
    if (sdl_quit_qry) {
        sdl_quit_qry = false;
        sdl_joined = true;
        return true;
    }
    return false;
}

void VT_deinit(void)
{
    monitor_sdl_clean_up();
    exit(0);
}


bool VT_sdl_vsync(void)
{
    bool bResult = true;

    SDL_LockMutex(s_ptRefreshMutex);
    sdl_refresh_pending = true;
    sdl_refr_cpl = false;
    VT_sdl_push_refresh_event();
    while (!sdl_refr_cpl && !sdl_quit_qry) {
        SDL_CondWait(s_ptRefreshCond, s_ptRefreshMutex);
    }
    if (sdl_quit_qry) {
        bResult = false;
    }
    SDL_UnlockMutex(s_ptRefreshMutex);

    return bResult;
}

void VT_sdl_signal_refresh(void)
{
    SDL_LockMutex(s_ptRefreshMutex);
    sdl_refresh_pending = true;
    SDL_UnlockMutex(s_ptRefreshMutex);
    VT_sdl_push_refresh_event();
}

void VT_sdl_wait(uint32_t timeout_ms)
{
    SDL_LockMutex(s_ptRefreshMutex);
    if (!sdl_quit_qry) {
        SDL_CondWaitTimeout(s_ptRefreshCond, s_ptRefreshMutex, timeout_ms);
    }
    SDL_UnlockMutex(s_ptRefreshMutex);
}

void VT_init(void)
{
    monitor_sdl_init();

    while(sdl_inited == false);
}

void VT_Fill_Single_Color(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_typedef color)
{
    /*Return if the area is out the screen*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > VT_WIDTH - 1) return;
    if(y1 > VT_HEIGHT - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > VT_WIDTH - 1 ? VT_WIDTH - 1 : x2;
    int32_t act_y2 = y2 > VT_HEIGHT - 1 ? VT_HEIGHT - 1 : y2;

    int32_t x;
    int32_t y;

    for(x = act_x1; x <= act_x2; x++) {
        for(y = act_y1; y <= act_y2; y++) {
            tft_fb[y * VT_WIDTH + x] = 0xff000000|DEV_2_VT_RGB(color);
        }
    }
}

void VT_Fill_Multiple_Colors(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_typedef * color_p)
{
    /*Return if the area is out the screen*/
    if(x2 < 0) return;
    if(y2 < 0) return;
    if(x1 > VT_WIDTH - 1) return;
    if(y1 > VT_HEIGHT - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > VT_WIDTH - 1 ? VT_WIDTH - 1 : x2;
    int32_t act_y2 = y2 > VT_HEIGHT - 1 ? VT_HEIGHT - 1 : y2;

    int32_t x;
    int32_t y;

    for(y = act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
            tft_fb[y * VT_WIDTH + x] = 0xff000000|DEV_2_VT_RGB(*color_p);
            color_p++;
        }

        color_p += x2 - act_x2;
    }
}

void VT_Set_Point(int32_t x, int32_t y, color_typedef color)
{
    /*Return if the area is out the screen*/
    if(x < 0) return;
    if(y < 0) return;
    if(x > VT_WIDTH - 1) return;
    if(y > VT_HEIGHT - 1) return;

    tft_fb[y * VT_WIDTH + x] = 0xff000000|DEV_2_VT_RGB(color);
}

color_typedef VT_Get_Point(int32_t x, int32_t y)
{
    uint32_t color=0;
    /*Return if the area is out the screen*/
    if(x < 0) return 0;
    if(y < 0) return 0;
    if(x > VT_WIDTH - 1) return 0;
    if(y > VT_HEIGHT - 1) return 0;

    color=tft_fb[y * VT_WIDTH + x] ;

    return VT_RGB_2_DEV(color);
}

bool vtIsKeyPress(uint16_t value,void* pUser)
{
    (void)(pUser);

    switch (value)
    {
    case KEY_NUM_UP:
    {
        return keyUp;
    }
    case KEY_NUM_DOWN:
    {
        return keyDown;
    }
    case KEY_NUM_LEFT:
    {
        return keyLeft;
    }
    case KEY_NUM_RIGHT:
    {
        return keyRight;
    }
    case KEY_NUM_ENTER:
    {
        return keyEnter;
    }
    case KEY_NUM_ESC:
    {
        return keyEsc;
    }
    default:
        break;
    }
    return 0;
}
