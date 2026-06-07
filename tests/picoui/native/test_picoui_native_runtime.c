#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#ifndef PICOUI_BASIC_WIDGETS_DEMO_PATH
#define PICOUI_BASIC_WIDGETS_DEMO_PATH ""
#endif

int picoui_native_background_get_rendered_size(const struct picoui_background *background,
                                               int *width,
                                               int *height);
int picoui_native_text_get_rendered_text(const struct picoui_text *text, const char **value);

static int ppm_read_header(FILE *capture, int *width, int *height)
{
    char magic[3] = {0};

    if (capture == 0 || width == 0 || height == 0) {
        return -1;
    }
    if (fscanf(capture, "%2s", magic) != 1 || strcmp(magic, "P6") != 0) {
        return -1;
    }
    if (fscanf(capture, "%d %d", width, height) != 2) {
        return -1;
    }
    if (fscanf(capture, "%*d") != 0) {
        return -1;
    }
    if (fgetc(capture) == EOF) {
        return -1;
    }
    return 0;
}

static int color_is_blue_track(unsigned char red, unsigned char green, unsigned char blue)
{
    int dr = (int)red - 33;
    int dg = (int)green - 150;
    int db = (int)blue - 243;

    if (dr < 0) {
        dr = -dr;
    }
    if (dg < 0) {
        dg = -dg;
    }
    if (db < 0) {
        db = -db;
    }
    return dr <= 12 && dg <= 12 && db <= 12;
}

static int color_is_background(unsigned char red, unsigned char green, unsigned char blue)
{
    int dr = (int)red - 0xF6;
    int dg = (int)green - 0xF8;
    int db = (int)blue - 0xFA;

    if (dr < 0) {
        dr = -dr;
    }
    if (dg < 0) {
        dg = -dg;
    }
    if (db < 0) {
        db = -db;
    }
    return dr <= 8 && dg <= 8 && db <= 8;
}

static int color_is_blueish(unsigned char red, unsigned char green, unsigned char blue)
{
    return blue > green && blue > red;
}

static int color_is_greenish(unsigned char red, unsigned char green, unsigned char blue)
{
    return green > red && green > blue;
}

static void assert_capture_size(const char *path, int expected_width, int expected_height)
{
    FILE *capture;
    int width = 0;
    int height = 0;

    capture = fopen(path, "rb");
    assert(capture != 0);
    assert(ppm_read_header(capture, &width, &height) == 0);
    fclose(capture);
    assert(width == expected_width);
    assert(height == expected_height);
}

static unsigned char *read_capture_pixels(const char *path, int *width, int *height)
{
    FILE *capture;
    unsigned char *pixels;
    size_t pixel_count;

    capture = fopen(path, "rb");
    assert(capture != 0);
    assert(ppm_read_header(capture, width, height) == 0);
    assert(*width > 0);
    assert(*height > 0);

    pixel_count = (size_t)(*width) * (size_t)(*height) * 3U;
    pixels = (unsigned char *)malloc(pixel_count);
    assert(pixels != 0);
    assert(fread(pixels, 1, pixel_count, capture) == pixel_count);
    fclose(capture);
    return pixels;
}

static void assert_capture_has_visible_content_and_blue_track(const char *path)
{
    int width = 0;
    int height = 0;
    unsigned char *pixels;
    size_t pixel_count;
    size_t i;
    unsigned char bg_red;
    unsigned char bg_green;
    unsigned char bg_blue;
    int non_background_pixels = 0;
    int blue_track_pixels = 0;

    pixels = read_capture_pixels(path, &width, &height);
    pixel_count = (size_t)width * (size_t)height * 3U;

    bg_red = pixels[pixel_count - 3U];
    bg_green = pixels[pixel_count - 2U];
    bg_blue = pixels[pixel_count - 1U];

    for (i = 0; i + 2U < pixel_count; i += 3U) {
        unsigned char red = pixels[i];
        unsigned char green = pixels[i + 1U];
        unsigned char blue = pixels[i + 2U];

        if (red != bg_red || green != bg_green || blue != bg_blue) {
            non_background_pixels++;
        }
        if (color_is_blue_track(red, green, blue)) {
            blue_track_pixels++;
        }
    }

    free(pixels);
    assert(non_background_pixels > 128);
    assert(blue_track_pixels > 8);
}

static unsigned char *read_capture_pixels_and_background(const char *path,
                                                         int *width,
                                                         int *height,
                                                         unsigned char *bg_red,
                                                         unsigned char *bg_green,
                                                         unsigned char *bg_blue)
{
    unsigned char *pixels = read_capture_pixels(path, width, height);
    size_t pixel_count = (size_t)(*width) * (size_t)(*height) * 3U;

    *bg_red = pixels[pixel_count - 3U];
    *bg_green = pixels[pixel_count - 2U];
    *bg_blue = pixels[pixel_count - 1U];
    return pixels;
}

static void assert_capture_has_basic_widget_bands(const char *path)
{
    int width = 0;
    int height = 0;
    unsigned char *pixels;
    int y;
    int run_count = 0;
    int in_run = 0;

    pixels = read_capture_pixels(path, &width, &height);

    for (y = 0; y < height; ++y) {
        int x;
        int active = 0;

        for (x = 0; x < width; ++x) {
            size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3U;
            if (!color_is_background(pixels[index], pixels[index + 1U], pixels[index + 2U])) {
                active++;
            }
        }
        if (active >= 8) {
            if (!in_run) {
                run_count++;
                in_run = 1;
            }
        } else {
            in_run = 0;
        }
    }

    free(pixels);
    assert(run_count >= 5);
}

static void assert_capture_has_expected_switch_bbox(const char *path)
{
    int width = 0;
    int height = 0;
    unsigned char *pixels;
    int min_x = -1;
    int min_y = -1;
    int max_x = -1;
    int max_y = -1;
    int x;
    int y;
    size_t left_mid;
    size_t right_mid;
    size_t top_left;
    size_t top_right;
    size_t bottom_left;
    size_t bottom_right;

    pixels = read_capture_pixels(path, &width, &height);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3U;

            if (!color_is_blue_track(pixels[index], pixels[index + 1U], pixels[index + 2U])) {
                continue;
            }
            if (min_x < 0 || x < min_x) {
                min_x = x;
            }
            if (max_x < 0 || x > max_x) {
                max_x = x;
            }
            if (min_y < 0 || y < min_y) {
                min_y = y;
            }
            if (max_y < 0 || y > max_y) {
                max_y = y;
            }
        }
    }

    assert(min_x == 16);
    assert(min_y == 24);
    assert(max_x == 63);
    assert(max_y == 47);

    top_left = ((size_t)24 * (size_t)width + 16U) * 3U;
    top_right = ((size_t)24 * (size_t)width + 63U) * 3U;
    bottom_left = ((size_t)47 * (size_t)width + 16U) * 3U;
    bottom_right = ((size_t)47 * (size_t)width + 63U) * 3U;
    left_mid = ((size_t)35 * (size_t)width + 16U) * 3U;
    right_mid = ((size_t)35 * (size_t)width + 63U) * 3U;

    assert(!color_is_blue_track(pixels[top_left], pixels[top_left + 1U], pixels[top_left + 2U]));
    assert(!color_is_blue_track(pixels[top_right], pixels[top_right + 1U], pixels[top_right + 2U]));
    assert(!color_is_blue_track(pixels[bottom_left], pixels[bottom_left + 1U], pixels[bottom_left + 2U]));
    assert(!color_is_blue_track(pixels[bottom_right], pixels[bottom_right + 1U], pixels[bottom_right + 2U]));
    assert(color_is_blue_track(pixels[left_mid], pixels[left_mid + 1U], pixels[left_mid + 2U]));
    assert(color_is_blue_track(pixels[right_mid], pixels[right_mid + 1U], pixels[right_mid + 2U]));
    free(pixels);
}

static int collect_non_background_colors(const unsigned char *pixels,
                                         int width,
                                         unsigned char bg_red,
                                         unsigned char bg_green,
                                         unsigned char bg_blue,
                                         int x0,
                                         int y0,
                                         int x1,
                                         int y1,
                                         int *has_blueish,
                                         int *has_greenish,
                                         int *color_count)
{
    int y;
    int x;
    unsigned char colors[32][3];
    int seen = 0;

    *has_blueish = 0;
    *has_greenish = 0;
    *color_count = 0;

    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3U;
            unsigned char red = pixels[index];
            unsigned char green = pixels[index + 1U];
            unsigned char blue = pixels[index + 2U];
            int i;
            int duplicate = 0;

            if (red == bg_red && green == bg_green && blue == bg_blue) {
                continue;
            }
            if (color_is_blueish(red, green, blue)) {
                *has_blueish = 1;
            }
            if (color_is_greenish(red, green, blue)) {
                *has_greenish = 1;
            }

            for (i = 0; i < seen; ++i) {
                if (colors[i][0] == red && colors[i][1] == green && colors[i][2] == blue) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate && seen < (int)(sizeof(colors) / sizeof(colors[0]))) {
                colors[seen][0] = red;
                colors[seen][1] = green;
                colors[seen][2] = blue;
                seen++;
            }
        }
    }

    *color_count = seen;
    return seen > 0;
}

struct capture_band {
    int start_y;
    int end_y;
};

struct capture_band_signature {
    int non_background_pixels;
    int blueish_pixels;
    int greenish_pixels;
    int height;
};

static int collect_content_bands(const unsigned char *pixels,
                                 int width,
                                 int height,
                                 unsigned char bg_red,
                                 unsigned char bg_green,
                                 unsigned char bg_blue,
                                 struct capture_band *bands,
                                 int max_bands)
{
    int y;
    int in_run = 0;
    int band_count = 0;
    int run_start = 0;

    for (y = 0; y < height; ++y) {
        int x;
        int active = 0;

        for (x = 0; x < width; ++x) {
            size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3U;

            if (pixels[index] != bg_red || pixels[index + 1U] != bg_green || pixels[index + 2U] != bg_blue) {
                active++;
            }
        }

        if (active >= 8) {
            if (!in_run) {
                run_start = y;
                in_run = 1;
            }
            continue;
        }

        if (in_run) {
            if (y - run_start >= 10 && band_count < max_bands) {
                bands[band_count].start_y = run_start;
                bands[band_count].end_y = y - 1;
                band_count++;
            }
            in_run = 0;
        }
    }

    if (in_run && height - run_start >= 10 && band_count < max_bands) {
        bands[band_count].start_y = run_start;
        bands[band_count].end_y = height - 1;
        band_count++;
    }

    return band_count;
}

static struct capture_band_signature capture_band_signature_for(const unsigned char *pixels,
                                                               int width,
                                                               unsigned char bg_red,
                                                               unsigned char bg_green,
                                                               unsigned char bg_blue,
                                                               struct capture_band band)
{
    struct capture_band_signature signature = {0};
    int y;
    int x;

    signature.height = band.end_y - band.start_y + 1;
    for (y = band.start_y; y <= band.end_y; ++y) {
        for (x = 0; x < width; ++x) {
            size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3U;
            unsigned char red = pixels[index];
            unsigned char green = pixels[index + 1U];
            unsigned char blue = pixels[index + 2U];

            if (red == bg_red && green == bg_green && blue == bg_blue) {
                continue;
            }
            signature.non_background_pixels++;
            if (color_is_blueish(red, green, blue)) {
                signature.blueish_pixels++;
            }
            if (color_is_greenish(red, green, blue)) {
                signature.greenish_pixels++;
            }
        }
    }

    return signature;
}

static void assert_capture_has_text_image_separation(const char *path)
{
    int width = 0;
    int height = 0;
    unsigned char bg_red = 0;
    unsigned char bg_green = 0;
    unsigned char bg_blue = 0;
    unsigned char *pixels;
    struct capture_band bands[16];
    struct capture_band_signature signatures[16];
    int band_count;
    int i;
    int switch_index = -1;
    int checkbox_index = -1;
    int slider_index = -1;
    int remaining[16];
    int remaining_count = 0;
    int image_index = -1;
    int text_index = -1;

    pixels = read_capture_pixels_and_background(path,
                                                &width,
                                                &height,
                                                &bg_red,
                                                &bg_green,
                                                &bg_blue);
    band_count = collect_content_bands(pixels,
                                       width,
                                       height,
                                       bg_red,
                                       bg_green,
                                       bg_blue,
                                       bands,
                                       (int)(sizeof(bands) / sizeof(bands[0])));
    assert(band_count >= 5);
    for (i = 0; i < band_count; ++i) {
        signatures[i] = capture_band_signature_for(pixels,
                                                   width,
                                                   bg_red,
                                                   bg_green,
                                                   bg_blue,
                                                   bands[i]);
    }

    for (i = 0; i < band_count; ++i) {
        if (switch_index < 0 || signatures[i].greenish_pixels > signatures[switch_index].greenish_pixels) {
            switch_index = i;
        }
    }
    assert(switch_index >= 0);

    for (i = 0; i < band_count; ++i) {
        if (i == switch_index) {
            continue;
        }
        if (signatures[i].height > 18 || signatures[i].blueish_pixels <= 0) {
            continue;
        }
        if (checkbox_index < 0 ||
            signatures[i].non_background_pixels < signatures[checkbox_index].non_background_pixels) {
            checkbox_index = i;
        }
    }
    assert(checkbox_index >= 0);

    for (i = 0; i < band_count; ++i) {
        if (i == switch_index || i == checkbox_index) {
            continue;
        }
        if (signatures[i].blueish_pixels <= 0 || signatures[i].greenish_pixels <= 0) {
            continue;
        }
        if (slider_index < 0 ||
            signatures[i].non_background_pixels > signatures[slider_index].non_background_pixels) {
            slider_index = i;
        }
    }
    assert(slider_index >= 0);

    for (i = 0; i < band_count; ++i) {
        if (i == switch_index || i == checkbox_index || i == slider_index) {
            continue;
        }
        remaining[remaining_count++] = i;
    }
    assert(remaining_count >= 2);

    for (i = 0; i < remaining_count; ++i) {
        int idx = remaining[i];

        if (image_index < 0 ||
            signatures[idx].blueish_pixels > signatures[image_index].blueish_pixels ||
            (signatures[idx].blueish_pixels == signatures[image_index].blueish_pixels &&
             signatures[idx].non_background_pixels > signatures[image_index].non_background_pixels)) {
            image_index = idx;
        }
    }
    assert(image_index >= 0);

    for (i = 0; i < remaining_count; ++i) {
        int idx = remaining[i];

        if (idx == image_index) {
            continue;
        }
        if (text_index < 0 || bands[idx].start_y < bands[text_index].start_y) {
            text_index = idx;
        }
    }
    assert(text_index >= 0);
    assert(signatures[text_index].non_background_pixels > 0);
    assert(signatures[image_index].non_background_pixels > 0);
    assert(signatures[text_index].non_background_pixels != signatures[image_index].non_background_pixels ||
           signatures[text_index].blueish_pixels != signatures[image_index].blueish_pixels ||
           signatures[text_index].greenish_pixels != signatures[image_index].greenish_pixels ||
           signatures[text_index].height != signatures[image_index].height);
    free(pixels);
}

static int count_substring(const char *haystack, const char *needle)
{
    int count = 0;
    size_t needle_len;
    const char *cursor;

    if (haystack == 0 || needle == 0 || needle[0] == '\0') {
        return 0;
    }

    needle_len = strlen(needle);
    cursor = haystack;
    while ((cursor = strstr(cursor, needle)) != 0) {
        count++;
        cursor += needle_len;
    }

    return count;
}

static void assert_stdout_has_single_capture_marker_group(const char *path)
{
    FILE *capture;
    char buffer[2048];
    size_t read_size;

    capture = fopen(path, "rb");
    assert(capture != 0);
    read_size = fread(buffer, 1, sizeof(buffer) - 1U, capture);
    fclose(capture);
    buffer[read_size] = '\0';

    assert(count_substring(buffer, "PICOUI_RUNTIME_READY\n") >= 1);
    assert(count_substring(buffer, "PICOUI_SMOKE_LAYOUT_USED=0\n") == 1);
    assert(count_substring(buffer, "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n") == 1);
    assert(count_substring(buffer, "PICOUI_BACKEND_REAL_WIDGET_IDS=wifi,agree,volume,submit,title,logo\n") == 1);
    assert(count_substring(buffer, "PICOUI_BACKEND_IMAGE_SOURCE=logo:img=null,mask=null\n") == 1);
    assert(count_substring(buffer, "PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK\n") == 0);
    assert(count_substring(buffer, "PICOUI_BACKEND_FALLBACK_WIDGET_IDS=") == 0);
}

static void run_basic_widgets_demo_and_assert_capture(void)
{
    pid_t pid;
    int status = 0;
    char capture_path[128];
    char stdout_path[128];
    int stdout_fd;

    assert(PICOUI_BASIC_WIDGETS_DEMO_PATH[0] != '\0');
    snprintf(capture_path, sizeof(capture_path), "/tmp/picoui-native-runtime-capture-%ld.ppm", (long)getpid());
    snprintf(stdout_path, sizeof(stdout_path), "/tmp/picoui-native-runtime-stdout-%ld.log", (long)getpid());
    unlink(capture_path);
    unlink(stdout_path);

    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        stdout_fd = creat(stdout_path, 0600);
        if (stdout_fd < 0) {
            _exit(127);
        }
        if (dup2(stdout_fd, STDOUT_FILENO) < 0 || dup2(stdout_fd, STDERR_FILENO) < 0) {
            close(stdout_fd);
            _exit(127);
        }
        close(stdout_fd);
        if (setenv("SDL_VIDEODRIVER", "dummy", 1) != 0
            || setenv("PICOUI_CAPTURE_FILE", capture_path, 1) != 0
            || setenv("PICOUI_DEMO_AUTO_QUIT_MS", "1200", 1) != 0) {
            _exit(127);
        }
        execl(PICOUI_BASIC_WIDGETS_DEMO_PATH, PICOUI_BASIC_WIDGETS_DEMO_PATH, (char *)0);
        _exit(127);
    }

    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);
    assert_capture_size(capture_path, 320, 480);
    assert_capture_has_visible_content_and_blue_track(capture_path);
    assert_capture_has_expected_switch_bbox(capture_path);
    assert_capture_has_basic_widget_bands(capture_path);
    assert_capture_has_text_image_separation(capture_path);
    assert_stdout_has_single_capture_marker_group(stdout_path);
    assert(unlink(capture_path) == 0);
    assert(unlink(stdout_path) == 0);
}

static void test_runtime_init_deinit_is_idempotent(void)
{
    assert(setenv("SDL_VIDEODRIVER", "dummy", 1) == 0);
    unsetenv("PICOUI_DEMO_AUTO_QUIT_MS");
    assert(picoui_init() == 0);
    assert(picoui_init() == 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    assert(picoui_init() == 0);
    picoui_deinit();
    picoui_deinit();
}

static void test_runtime_auto_quit_uses_wall_clock_time(void)
{
    int rc;

    assert(setenv("SDL_VIDEODRIVER", "dummy", 1) == 0);
    assert(setenv("PICOUI_DEMO_AUTO_QUIT_MS", "50", 1) == 0);
    assert(picoui_init() == 0);
    rc = picoui_timer_handler();
    assert(rc == 0);
    usleep(100 * 1000);
    rc = picoui_timer_handler();
    assert(rc == 1);
    picoui_deinit();
    unsetenv("PICOUI_DEMO_AUTO_QUIT_MS");
}

static void test_runtime_screen_load_renders_screen_root_window(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_text *text;
    const char *rendered_text = 0;
    int rc;

    assert(setenv("SDL_VIDEODRIVER", "dummy", 1) == 0);
    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "runtime_root");
    assert(window != 0);
    text = picoui_text_create(window, "title");
    assert(text != 0);
    assert(picoui_text_set_text(text, "runtime root load") == 0);
    assert(picoui_screen_get_root_window(screen) == window);

    assert(picoui_screen_load(screen) == 0);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "runtime root load") == 0);

    picoui_deinit();
}

static void test_runtime_screen_load_writes_visible_capture_file(void)
{
    run_basic_widgets_demo_and_assert_capture();
}

int main(void)
{
    test_runtime_init_deinit_is_idempotent();
    test_runtime_auto_quit_uses_wall_clock_time();
    test_runtime_screen_load_renders_screen_root_window();
    test_runtime_screen_load_writes_visible_capture_file();
    return 0;
}
