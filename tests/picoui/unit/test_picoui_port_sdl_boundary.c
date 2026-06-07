#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *fp = fopen("/Users/cys/embedded/LingDongGUI/picoui/src/backend/ldgui/backend_app.c", "rb");
    char buffer[4096];
    size_t n;

    assert(fp != NULL);
    n = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);
    buffer[n] = '\0';

    assert(strstr(buffer, "<SDL.h>") == NULL);
    assert(strstr(buffer, "SDL_Init(") == NULL);
    assert(strstr(buffer, "SDL_CreateWindow(") == NULL);
    assert(strstr(buffer, "SDL_PollEvent(") == NULL);
    assert(strstr(buffer, "SDL_RenderPresent(") == NULL);
    return 0;
}
