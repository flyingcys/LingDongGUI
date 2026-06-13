/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
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

/*
 * TinyUI demo dispatcher — mirrors lvgl/demos/lv_demos.c.
 *
 * A name→function table maps demo name strings to their entry points.
 * tinyui_demos_create(argv+1, argc-1) picks one by name and calls it.
 *
 *   ./tinyui_demo arc_basic
 *   ./tinyui_demo hello_world
 */

#include "tinyui_demos.h"
#include <string.h>
#include <stdio.h>

typedef void (*demo_method_cb)(void);

typedef struct {
    const char     *name;
    demo_method_cb  entry_cb;
} demo_entry_info_t;

static const demo_entry_info_t demos_entry_info[] = {
    { "animation_basic",       tinyui_demo_animation_basic       },
    { "arc_basic",             tinyui_demo_arc_basic             },
    { "basic_widgets",         tinyui_demo_basic_widgets         },
    { "calendar_basic",        tinyui_demo_calendar_basic        },
    { "clock_basic",           tinyui_demo_clock_basic           },
    { "combo_box_basic",       tinyui_demo_combo_box_basic       },
    { "date_time_basic",       tinyui_demo_date_time_basic       },
    { "gauge_basic",           tinyui_demo_gauge_basic           },
    { "graph_basic",           tinyui_demo_graph_basic           },
    { "grid_parity",           tinyui_demo_grid_parity           },
    { "hello_world",           tinyui_demo_hello_world           },
    { "icon_slider_basic",     tinyui_demo_icon_slider_basic     },
    { "keyboard_basic",        tinyui_demo_keyboard_basic        },
    { "layout_flex",           tinyui_demo_layout_flex           },
    { "layout_grid",           tinyui_demo_layout_grid           },
    { "layout_parity",         tinyui_demo_layout_parity         },
    { "legacy_widget_parity",  tinyui_demo_legacy_widget_parity  },
    { "line_edit_basic",       tinyui_demo_line_edit_basic       },
    { "list_basic",            tinyui_demo_list_basic            },
    { "message_box_basic",     tinyui_demo_message_box_basic     },
    { "progress_bar_basic",    tinyui_demo_progress_bar_basic    },
    { "progress_wheel_basic",  tinyui_demo_progress_wheel_basic  },
    { "qrcode_basic",          tinyui_demo_qrcode_basic          },
    { "radial_menu_basic",     tinyui_demo_radial_menu_basic     },
    { "scroll_selecter_basic", tinyui_demo_scroll_selecter_basic },
    { "settings_panel",        tinyui_demo_settings_panel        },
    { "table_basic",           tinyui_demo_table_basic           },
    { "theme_showcase",        tinyui_demo_theme_showcase        },
    { "", NULL }  /* sentinel */
};

#define TINYUI_DEMOS_COUNT \
    ((int)(sizeof(demos_entry_info) / sizeof(demo_entry_info_t)) - 1)

bool tinyui_demos_create(char *info[], int size)
{
    const int count = TINYUI_DEMOS_COUNT;

    if (count <= 0) {
        printf("tinyui_demos: no demos available\n");
        return false;
    }

    const demo_entry_info_t *entry = NULL;

    if (size <= 0) {
        /* default: first demo */
        entry = &demos_entry_info[0];
    } else if (info) {
        const char *name = info[0];
        for (int i = 0; i < count; i++) {
            if (strcmp(name, demos_entry_info[i].name) == 0) {
                entry = &demos_entry_info[i];
                break;
            }
        }
    }

    if (entry == NULL) {
        printf("tinyui_demos: demo '%s' not found\n",
               (size > 0 && info) ? info[0] : "");
        return false;
    }

    if (entry->entry_cb) {
        entry->entry_cb();
        return true;
    }

    return false;
}

void tinyui_demos_show_help(void)
{
    int i;
    const int count = TINYUI_DEMOS_COUNT;

    if (count == 0) {
        printf("tinyui_demos: no demo available!\n");
        return;
    }

    printf("\nUsage: tinyui_demo <demo_name>\n\ndemo list:\n");
    for (i = 0; i < count; i++) {
        printf("    %s\n", demos_entry_info[i].name);
    }
}
