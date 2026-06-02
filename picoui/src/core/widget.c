#include "internal.h"
#include "picoui/widget.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ldBase_t ldBase_t;
typedef struct ldSwitch_t ldSwitch_t;
typedef struct arm_2d_control_node_t arm_2d_control_node_t;
typedef struct {
    int16_t iX;
    int16_t iY;
} picoui_ld_location_t;
typedef struct {
    int16_t iWidth;
    int16_t iHeight;
} picoui_ld_size_t;
typedef struct {
    picoui_ld_location_t tLocation;
    picoui_ld_size_t tSize;
} picoui_ld_region_t;
typedef enum {
    picoui_ld_align_left = 1 << 0,
    picoui_ld_align_right = 1 << 1,
    picoui_ld_align_top = 1 << 2,
    picoui_ld_align_bottom = 1 << 3,
    picoui_ld_align_center = 0,
} picoui_ld_align_t;

int16_t ldBaseGetX(ldBase_t *ptWidget);
int16_t ldBaseGetY(ldBase_t *ptWidget);
int16_t ldBaseGetWidth(ldBase_t *ptWidget);
int16_t ldBaseGetHeight(ldBase_t *ptWidget);
uint16_t ldBaseGetOpacity(ldBase_t *ptWidget);
bool ldBaseIsHidden(ldBase_t *ptWidget);
bool ldBaseIsSelectable(ldBase_t *ptWidget);
bool ldBaseIsSelected(ldBase_t *ptWidget);
bool ldBaseIsCorner(ldBase_t *ptWidget);
int ldBaseGetWidgetType(ldBase_t *ptWidget);
uint16_t ldBaseGetNameId(ldBase_t *ptWidget);
ldBase_t *ldBaseGetParent(ldBase_t *ptWidget);
ldBase_t *ldBaseGetChildList(ldBase_t *ptWidget);
ldBase_t *ldBaseGetNextSibling(ldBase_t *ptWidget);
uint16_t ldBaseGetChildCount(ldBase_t *ptWidget);
picoui_ld_location_t ldBaseGetAbsoluteLocation(ldBase_t *ptWidget, picoui_ld_location_t tLocation);
picoui_ld_location_t ldBaseGetRelativeLocation(ldBase_t *ptWidget, picoui_ld_location_t tLocation);
picoui_ld_region_t ldBaseGetAlignRegion(picoui_ld_region_t parentRegion,
                                         picoui_ld_region_t childRegion,
                                         picoui_ld_align_t tAlign);
arm_2d_control_node_t *ldBaseGetRootNode(arm_2d_control_node_t *ptNode);
int16_t ldBaseAutoVerticalGridAlign(picoui_ld_region_t widgetRegion,
                                    int16_t currentOffset,
                                    uint8_t itemCount,
                                    uint8_t itemHeight,
                                    uint8_t space);
void ldBaseFocusNavigateInit(void);
void ldBaseSetX(ldBase_t *ptWidget, int16_t x);
void ldBaseSetY(ldBase_t *ptWidget, int16_t y);
void ldBaseSetWidth(ldBase_t *ptWidget, int16_t width);
void ldBaseSetHeight(ldBase_t *ptWidget, int16_t height);
void ldBaseSetCenter(ldBase_t *ptWidget);
void ldBaseSetHidden(ldBase_t *ptWidget, bool isHidden);
void ldBaseSetOpacity(ldBase_t *ptWidget, uint8_t opacity);
void ldBaseSetSelectable(ldBase_t *ptWidget, bool isSelectable);
void ldBaseSetSelect(ldBase_t *ptWidget, bool isSelect);
void ldBaseSetCorner(ldBase_t *ptWidget, bool isCorner);
void ldBaseSetFlexMinWidth(ldBase_t *ptWidget, int16_t minWidth);
void ldBaseSetFlexMinHeight(ldBase_t *ptWidget, int16_t minHeight);
void ldBaseSetFlexMaxWidth(ldBase_t *ptWidget, int16_t maxWidth);
void ldBaseSetFlexMaxHeight(ldBase_t *ptWidget, int16_t maxHeight);
void ldSwitchSetDisabled(ldSwitch_t *ptWidget, bool isDisabled);

static int picoui_widget_is_valid(struct picoui_widget *widget)
{
    return widget != 0;
}

static ldBase_t *picoui_widget_get_ld_base(struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (!picoui_widget_is_valid(widget) || widget->backend_widget == 0) {
        return 0;
    }

    backend_widget = (struct picoui_backend_widget *)widget->backend_widget;
    if (backend_widget->ld_widget == 0) {
        return 0;
    }

    return (ldBase_t *)backend_widget->ld_widget;
}

static struct picoui_backend_widget *picoui_widget_get_backend(const struct picoui_widget *widget)
{
    if (widget == 0 || widget->backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)widget->backend_widget;
}

static struct picoui_widget *picoui_backend_widget_get_host(struct picoui_backend_widget *backend_widget)
{
    if (backend_widget == 0) {
        return 0;
    }

    return backend_widget->host_widget;
}

static int picoui_align_to_ld_horizontal(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return picoui_ld_align_left;
    case PICOUI_ALIGN_END:
        return picoui_ld_align_right;
    case PICOUI_ALIGN_CENTER:
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_SPACE_EVENLY:
    case PICOUI_ALIGN_SPACE_AROUND:
    case PICOUI_ALIGN_SPACE_BETWEEN:
    default:
        return picoui_ld_align_center;
    }
}

static int picoui_align_to_ld_vertical(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return picoui_ld_align_top;
    case PICOUI_ALIGN_END:
        return picoui_ld_align_bottom;
    case PICOUI_ALIGN_CENTER:
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_SPACE_EVENLY:
    case PICOUI_ALIGN_SPACE_AROUND:
    case PICOUI_ALIGN_SPACE_BETWEEN:
    default:
        return picoui_ld_align_center;
    }
}

static picoui_ld_region_t picoui_rect_to_ld_region(struct picoui_rect rect)
{
    picoui_ld_region_t region;

    region.tLocation.iX = (int16_t)rect.x;
    region.tLocation.iY = (int16_t)rect.y;
    region.tSize.iWidth = (int16_t)rect.width;
    region.tSize.iHeight = (int16_t)rect.height;
    return region;
}

static struct picoui_rect picoui_rect_from_ld_region(picoui_ld_region_t region)
{
    struct picoui_rect rect;

    rect.x = region.tLocation.iX;
    rect.y = region.tLocation.iY;
    rect.width = region.tSize.iWidth;
    rect.height = region.tSize.iHeight;
    return rect;
}

static enum picoui_widget_type picoui_widget_type_from_backend_kind(enum picoui_backend_widget_kind kind)
{
    switch (kind) {
    case PICOUI_BACKEND_WIDGET_BACKGROUND:
        return PICOUI_WIDGET_TYPE_BACKGROUND;
    case PICOUI_BACKEND_WIDGET_WINDOW:
        return PICOUI_WIDGET_TYPE_WINDOW;
    case PICOUI_BACKEND_WIDGET_LABEL:
        return PICOUI_WIDGET_TYPE_LABEL;
    case PICOUI_BACKEND_WIDGET_BUTTON:
        return PICOUI_WIDGET_TYPE_BUTTON;
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
        return PICOUI_WIDGET_TYPE_CHECKBOX;
    case PICOUI_BACKEND_WIDGET_SWITCH:
        return PICOUI_WIDGET_TYPE_SWITCH;
    case PICOUI_BACKEND_WIDGET_SLIDER:
        return PICOUI_WIDGET_TYPE_SLIDER;
    case PICOUI_BACKEND_WIDGET_ARC:
        return PICOUI_WIDGET_TYPE_ARC;
    case PICOUI_BACKEND_WIDGET_GAUGE:
        return PICOUI_WIDGET_TYPE_GAUGE;
    case PICOUI_BACKEND_WIDGET_ICON_SLIDER:
        return PICOUI_WIDGET_TYPE_ICON_SLIDER;
    case PICOUI_BACKEND_WIDGET_RADIAL_MENU:
        return PICOUI_WIDGET_TYPE_RADIAL_MENU;
    case PICOUI_BACKEND_WIDGET_PROGRESS_BAR:
        return PICOUI_WIDGET_TYPE_PROGRESS_BAR;
    case PICOUI_BACKEND_WIDGET_QRCODE:
        return PICOUI_WIDGET_TYPE_QRCODE;
    case PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL:
        return PICOUI_WIDGET_TYPE_PROGRESS_WHEEL;
    case PICOUI_BACKEND_WIDGET_ANIMATION:
        return PICOUI_WIDGET_TYPE_ANIMATION;
    case PICOUI_BACKEND_WIDGET_LIST:
        return PICOUI_WIDGET_TYPE_LIST;
    case PICOUI_BACKEND_WIDGET_MESSAGE_BOX:
        return PICOUI_WIDGET_TYPE_MESSAGE_BOX;
    case PICOUI_BACKEND_WIDGET_DATE_TIME:
        return PICOUI_WIDGET_TYPE_DATE_TIME;
    case PICOUI_BACKEND_WIDGET_CLOCK:
        return PICOUI_WIDGET_TYPE_CLOCK;
    case PICOUI_BACKEND_WIDGET_TEXT:
        return PICOUI_WIDGET_TYPE_TEXT;
    case PICOUI_BACKEND_WIDGET_KEYBOARD:
        return PICOUI_WIDGET_TYPE_KEYBOARD;
    case PICOUI_BACKEND_WIDGET_COMBO_BOX:
        return PICOUI_WIDGET_TYPE_COMBO_BOX;
    case PICOUI_BACKEND_WIDGET_SCROLL_SELECTER:
        return PICOUI_WIDGET_TYPE_SCROLL_SELECTER;
    case PICOUI_BACKEND_WIDGET_TABLE:
        return PICOUI_WIDGET_TYPE_TABLE;
    case PICOUI_BACKEND_WIDGET_GRAPH:
        return PICOUI_WIDGET_TYPE_GRAPH;
    case PICOUI_BACKEND_WIDGET_IMAGE:
        return PICOUI_WIDGET_TYPE_IMAGE;
    case PICOUI_BACKEND_WIDGET_CALENDAR:
        return PICOUI_WIDGET_TYPE_CALENDAR;
    case PICOUI_BACKEND_WIDGET_CANVAS:
        return PICOUI_WIDGET_TYPE_CANVAS;
    default:
        return PICOUI_WIDGET_TYPE_UNKNOWN;
    }
}

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->x = x;
    widget->y = y;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetX(ld_base, (int16_t)x);
        ldBaseSetY(ld_base, (int16_t)y);
    }
    return 0;
}

int picoui_widget_set_size(struct picoui_widget *widget, int width, int height)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || width < 0 || height < 0) {
        return -1;
    }

    widget->width = width;
    widget->height = height;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetWidth(ld_base, (int16_t)width);
        ldBaseSetHeight(ld_base, (int16_t)height);
    }
    return 0;
}

int picoui_widget_set_text(struct picoui_widget *widget, const char *text)
{
    if (!picoui_widget_is_valid(widget) || text == 0) {
        return -1;
    }

    widget->text = text;
    return 0;
}

int picoui_widget_set_style_class(struct picoui_widget *widget, const char *style_class)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->style_class = style_class;
    if (widget->backend_widget != 0) {
        return picoui_backend_widget_set_style_class(widget->backend_widget, style_class);
    }
    return 0;
}

int picoui_widget_set_user_data(struct picoui_widget *widget, void *user_data)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->user_data = user_data;
    if (widget->backend_widget != 0) {
        return picoui_backend_widget_set_user_data(widget->backend_widget, user_data);
    }
    return 0;
}

int picoui_widget_set_bg_color(struct picoui_widget *widget, unsigned int rgb)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->bg_color = rgb;
    return 0;
}

int picoui_widget_set_text_color(struct picoui_widget *widget, unsigned int rgb)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->text_color = rgb;
    return 0;
}

int picoui_widget_set_border_color(struct picoui_widget *widget, unsigned int rgb)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->border_color = rgb;
    return 0;
}

int picoui_widget_set_radius(struct picoui_widget *widget, int radius)
{
    if (!picoui_widget_is_valid(widget) || radius < 0) {
        return -1;
    }

    widget->radius = radius;
    return 0;
}

int picoui_widget_set_padding(struct picoui_widget *widget, int padding)
{
    if (!picoui_widget_is_valid(widget) || padding < 0) {
        return -1;
    }

    widget->padding = padding;
    if (widget->backend_widget != 0) {
        return picoui_backend_widget_set_padding(widget->backend_widget, padding);
    }
    return 0;
}

int picoui_widget_set_center(struct picoui_widget *widget)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetCenter(ld_base);
    }
    return 0;
}

int picoui_widget_set_visible(struct picoui_widget *widget, int visible)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->visible = visible != 0;
    if (widget->visible == 0) {
        (void)picoui_widget_release_focus(widget);
    }
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetHidden(ld_base, widget->visible == 0);
    }
    return 0;
}

int picoui_widget_set_opacity(struct picoui_widget *widget, int opacity)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || opacity < 0 || opacity > 255) {
        return -1;
    }

    widget->opacity = opacity;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetOpacity(ld_base, (uint8_t)opacity);
    }
    return 0;
}

int picoui_widget_set_selectable(struct picoui_widget *widget, int selectable)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->selectable = selectable != 0;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetSelectable(ld_base, widget->selectable != 0);
    }
    return 0;
}

int picoui_widget_set_selected(struct picoui_widget *widget, int selected)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->selected = selected != 0;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetSelect(ld_base, widget->selected != 0);
    }
    return 0;
}

int picoui_widget_set_corner(struct picoui_widget *widget, int corner)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->corner = corner != 0;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetCorner(ld_base, widget->corner != 0);
    }
    return 0;
}

int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled)
{
    struct picoui_backend_widget *backend_widget;
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->enabled = enabled != 0;
    if (widget->enabled == 0) {
        (void)picoui_widget_release_focus(widget);
    }
    if (widget->backend_widget != 0) {
        backend_widget = (struct picoui_backend_widget *)widget->backend_widget;
        ld_base = picoui_widget_get_ld_base(widget);
        if (ld_base != 0) {
            ldBaseSetSelectable(ld_base, widget->enabled != 0);
        }
        if (backend_widget->kind == PICOUI_BACKEND_WIDGET_LIST && backend_widget->ld_widget != 0) {
            ldBaseSetSelectable((ldBase_t *)backend_widget->ld_widget, widget->enabled != 0);
        }
        if (backend_widget->kind == PICOUI_BACKEND_WIDGET_SWITCH && backend_widget->ld_widget != 0) {
            ldSwitchSetDisabled((ldSwitch_t *)backend_widget->ld_widget, widget->enabled == 0);
        }
    }
    return 0;
}

int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    if (!picoui_widget_is_valid(widget) || grow < 0) {
        return -1;
    }

    widget->flex_grow = grow;
    return picoui_backend_widget_set_flex_grow(widget, grow);
}

int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->flex_new_track = new_track != 0;
    return picoui_backend_widget_set_flex_new_track(widget, widget->flex_new_track);
}

int picoui_widget_set_flex_min_width(struct picoui_widget *widget, int min_width)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || min_width < 0) {
        return -1;
    }

    widget->flex_min_width = min_width;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMinWidth(ld_base, (int16_t)min_width);
    }
    return 0;
}

int picoui_widget_set_flex_min_height(struct picoui_widget *widget, int min_height)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || min_height < 0) {
        return -1;
    }

    widget->flex_min_height = min_height;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMinHeight(ld_base, (int16_t)min_height);
    }
    return 0;
}

int picoui_widget_set_flex_max_width(struct picoui_widget *widget, int max_width)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || max_width < 0) {
        return -1;
    }

    widget->flex_max_width = max_width;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMaxWidth(ld_base, (int16_t)max_width);
    }
    return 0;
}

int picoui_widget_set_flex_max_height(struct picoui_widget *widget, int max_height)
{
    ldBase_t *ld_base;

    if (!picoui_widget_is_valid(widget) || max_height < 0) {
        return -1;
    }

    widget->flex_max_height = max_height;
    ld_base = picoui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMaxHeight(ld_base, (int16_t)max_height);
    }
    return 0;
}

int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->ignore_layout = ignore_layout != 0;
    return picoui_backend_widget_set_ignore_layout(widget, widget->ignore_layout);
}

int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align)
{
    if (!picoui_widget_is_valid(widget) || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    widget->grid_col = col;
    widget->grid_row = row;
    widget->grid_col_span = col_span;
    widget->grid_row_span = row_span;
    widget->grid_x_align = x_align;
    widget->grid_y_align = y_align;
    return picoui_backend_widget_set_grid_cell(widget, col, row, col_span, row_span, x_align, y_align);
}

int picoui_widget_remove_from_parent(struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    backend_widget = picoui_widget_get_backend(widget);
    if (backend_widget == 0 ||
        backend_widget->kind == PICOUI_BACKEND_WIDGET_WINDOW ||
        backend_widget->parent == 0) {
        return -1;
    }

    return picoui_backend_widget_detach_from_parent(backend_widget);
}

int picoui_widget_destroy(struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;
    struct picoui_app *owner;

    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    backend_widget = picoui_widget_get_backend(widget);
    if (backend_widget == 0 ||
        backend_widget->kind == PICOUI_BACKEND_WIDGET_WINDOW ||
        backend_widget->parent == 0) {
        return -1;
    }

    owner = backend_widget->owner;
    if (owner != 0) {
        if (owner->focus_owner == widget) {
            (void)picoui_widget_release_focus(widget);
        }
        if (owner->editing_owner == widget) {
            (void)picoui_widget_release_editing(widget);
        }
    }

    if (picoui_backend_widget_detach_from_parent(backend_widget) != 0) {
        return -1;
    }
    if (picoui_backend_widget_unbind_host(backend_widget) != 0) {
        return -1;
    }
    widget->backend_widget = 0;
    return 0;
}

int picoui_widget_get_x(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetX(ld_base) : widget->x;
}

int picoui_widget_get_y(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetY(ld_base) : widget->y;
}

int picoui_widget_get_width(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetWidth(ld_base) : widget->width;
}

int picoui_widget_get_height(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetHeight(ld_base) : widget->height;
}

int picoui_widget_get_visible(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsHidden(ld_base) ? 0 : 1) : widget->visible;
}

int picoui_widget_get_opacity(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (int)ldBaseGetOpacity(ld_base) : widget->opacity;
}

int picoui_widget_get_selectable(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsSelectable(ld_base) ? 1 : 0) : widget->selectable;
}

int picoui_widget_get_selected(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsSelected(ld_base) ? 1 : 0) : widget->selected;
}

int picoui_widget_get_corner(const struct picoui_widget *widget)
{
    ldBase_t *ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);

    if (!picoui_widget_is_valid((struct picoui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsCorner(ld_base) ? 1 : 0) : widget->corner;
}

struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return 0;
    }

    backend_widget = picoui_backend_widget_get_parent(picoui_widget_get_backend(widget));
    return picoui_backend_widget_get_host(backend_widget);
}

struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return 0;
    }

    backend_widget = picoui_backend_widget_get_first_child(picoui_widget_get_backend(widget));
    return picoui_backend_widget_get_host(backend_widget);
}

struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return 0;
    }

    backend_widget = picoui_backend_widget_get_next_sibling(picoui_widget_get_backend(widget));
    return picoui_backend_widget_get_host(backend_widget);
}

struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return 0;
    }

    backend_widget = picoui_backend_widget_get_root(picoui_widget_get_backend(widget));
    return picoui_backend_widget_get_host(backend_widget);
}

int picoui_widget_get_child_count(const struct picoui_widget *widget)
{
    ldBase_t *ld_base;

    if (widget == 0) {
        return -1;
    }

    ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);
    if (ld_base != 0) {
        return (int)ldBaseGetChildCount(ld_base);
    }

    {
        int count = 0;
        struct picoui_backend_widget *child =
            picoui_backend_widget_get_first_child(picoui_widget_get_backend(widget));
        while (child != 0) {
            count++;
            child = picoui_backend_widget_get_next_sibling(child);
        }
        return count;
    }
}

int picoui_widget_get_name_id(const struct picoui_widget *widget)
{
    ldBase_t *ld_base;
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return -1;
    }

    ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);
    if (ld_base != 0) {
        return (int)ldBaseGetNameId(ld_base);
    }

    backend_widget = picoui_widget_get_backend(widget);
    return backend_widget != 0 ? backend_widget->ld_name_id : -1;
}

struct picoui_widget *picoui_widget_find_by_name_id(const struct picoui_widget *root, int name_id)
{
    struct picoui_backend_widget *backend_widget;

    if (root == 0 || name_id < 0 || name_id > 65535) {
        return 0;
    }

    backend_widget = picoui_backend_widget_find_by_name_id(picoui_widget_get_backend(root),
                                                           (uint16_t)name_id);
    return picoui_backend_widget_get_host(backend_widget);
}

enum picoui_widget_type picoui_widget_get_type(const struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend_widget;

    if (widget == 0) {
        return PICOUI_WIDGET_TYPE_UNKNOWN;
    }

    backend_widget = picoui_widget_get_backend(widget);
    if (backend_widget != 0) {
        return picoui_widget_type_from_backend_kind(backend_widget->kind);
    }

    return PICOUI_WIDGET_TYPE_UNKNOWN;
}

struct picoui_point picoui_widget_get_absolute_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point)
{
    ldBase_t *ld_base;
    picoui_ld_location_t location;
    struct picoui_point result = {-1, -1};

    if (widget == 0) {
        return result;
    }

    ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);
    if (ld_base == 0) {
        result.x = point.x + widget->x;
        result.y = point.y + widget->y;
        return result;
    }

    location.iX = (int16_t)point.x;
    location.iY = (int16_t)point.y;
    location = ldBaseGetAbsoluteLocation(ld_base, location);
    result.x = location.iX;
    result.y = location.iY;
    return result;
}

struct picoui_point picoui_widget_get_relative_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point)
{
    ldBase_t *ld_base;
    picoui_ld_location_t location;
    struct picoui_point result = {-1, -1};

    if (widget == 0) {
        return result;
    }

    ld_base = picoui_widget_get_ld_base((struct picoui_widget *)widget);
    if (ld_base == 0) {
        result.x = point.x - widget->x;
        result.y = point.y - widget->y;
        return result;
    }

    location.iX = (int16_t)point.x;
    location.iY = (int16_t)point.y;
    location = ldBaseGetRelativeLocation(ld_base, location);
    result.x = location.iX;
    result.y = location.iY;
    return result;
}

struct picoui_rect picoui_rect_align(struct picoui_rect parent,
                                     struct picoui_rect child,
                                     enum picoui_align x_align,
                                     enum picoui_align y_align)
{
    picoui_ld_region_t aligned;
    int ld_align = picoui_align_to_ld_horizontal(x_align) | picoui_align_to_ld_vertical(y_align);

    if (child.width < 0 || child.height < 0 || parent.width < 0 || parent.height < 0) {
        struct picoui_rect invalid = {0, 0, -1, -1};
        return invalid;
    }

    aligned = ldBaseGetAlignRegion(picoui_rect_to_ld_region(parent),
                                   picoui_rect_to_ld_region(child),
                                   (picoui_ld_align_t)ld_align);
    return picoui_rect_from_ld_region(aligned);
}

struct picoui_rect picoui_rect_center(struct picoui_rect parent,
                                      struct picoui_rect child)
{
    return picoui_rect_align(parent, child, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
}

int picoui_vertical_grid_align_offset(struct picoui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space)
{
    if (widget.width < 0 || widget.height < 0 || item_count < 0 || item_height < 0 || space < 0) {
        return -1;
    }

    return (int)ldBaseAutoVerticalGridAlign(picoui_rect_to_ld_region(widget),
                                            (int16_t)current_offset,
                                            (uint8_t)item_count,
                                            (uint8_t)item_height,
                                            (uint8_t)space);
}

int picoui_focus_reset(struct picoui_app *app)
{
    if (app == 0) {
        return -1;
    }

    if (app->focus_owner != 0) {
        (void)picoui_widget_release_focus(app->focus_owner);
    }
    ldBaseFocusNavigateInit();
    return 0;
}

int picoui_focus_navigate(struct picoui_app *app, enum picoui_native_nav_dir dir)
{
    struct picoui_widget *current;
    struct picoui_widget *candidate;
    struct picoui_widget *best = 0;
    struct picoui_point current_pos;
    struct picoui_point candidate_pos;
    int best_distance = 0x7fffffff;

    if (app == 0 ||
        (dir != PICOUI_NATIVE_NAV_LEFT &&
         dir != PICOUI_NATIVE_NAV_RIGHT &&
         dir != PICOUI_NATIVE_NAV_UP &&
         dir != PICOUI_NATIVE_NAV_DOWN &&
         dir != PICOUI_NATIVE_NAV_ENTER &&
         dir != PICOUI_NATIVE_NAV_BACK)) {
        return -1;
    }

    current = app->focus_owner;
    if (current == 0) {
        current = app->root_window != 0 ? &app->root_window->widget : 0;
        if (current == 0) {
            return -1;
        }
        candidate = picoui_widget_get_first_child(current);
        while (candidate != 0) {
            if (candidate->visible != 0 && candidate->enabled != 0 && candidate->selectable != 0) {
                return picoui_widget_claim_focus(candidate);
            }
            candidate = picoui_widget_get_next_sibling(candidate);
        }
        return -1;
    }

    if (dir == PICOUI_NATIVE_NAV_ENTER) {
        candidate = picoui_widget_get_first_child(current);
        if (candidate != 0 &&
            candidate->visible != 0 &&
            candidate->enabled != 0 &&
            candidate->selectable != 0) {
            return picoui_widget_claim_focus(candidate);
        }
        return 0;
    }

    if (dir == PICOUI_NATIVE_NAV_BACK) {
        candidate = picoui_widget_get_parent(current);
        if (candidate != 0 &&
            candidate->visible != 0 &&
            candidate->enabled != 0) {
            return picoui_widget_claim_focus(candidate);
        }
        return 0;
    }

    current_pos = picoui_widget_get_absolute_pos(current, (struct picoui_point){0, 0});
    candidate = picoui_widget_get_first_child(picoui_widget_get_parent(current));
    while (candidate != 0) {
        int dx;
        int dy;
        int distance;

        if (candidate == current ||
            candidate->visible == 0 ||
            candidate->enabled == 0 ||
            candidate->selectable == 0) {
            candidate = picoui_widget_get_next_sibling(candidate);
            continue;
        }

        candidate_pos = picoui_widget_get_absolute_pos(candidate, (struct picoui_point){0, 0});
        dx = candidate_pos.x - current_pos.x;
        dy = candidate_pos.y - current_pos.y;

        if ((dir == PICOUI_NATIVE_NAV_LEFT && dx >= 0) ||
            (dir == PICOUI_NATIVE_NAV_RIGHT && dx <= 0) ||
            (dir == PICOUI_NATIVE_NAV_UP && dy >= 0) ||
            (dir == PICOUI_NATIVE_NAV_DOWN && dy <= 0)) {
            candidate = picoui_widget_get_next_sibling(candidate);
            continue;
        }

        distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (distance < best_distance) {
            best_distance = distance;
            best = candidate;
        }
        candidate = picoui_widget_get_next_sibling(candidate);
    }

    return best != 0 ? picoui_widget_claim_focus(best) : 0;
}
