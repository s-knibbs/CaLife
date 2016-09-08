#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __EMSCRIPTEN__
#include <SDL/SDL.h>
#include <emscripten.h>
#define NDEBUG
#else
#include <SDL2/SDL.h>
#endif
#include <assert.h>

#define DEFAULT_FILL 50
#define AMASK 0xFF000000

#define SWAP_PTRS(a, b) \
    void * tmp = a; \
    a = b; \
    b = tmp

#define USAGE "Usage: %s [-s SURVIVES][-b BORN][-f FILL_PERCENT][-w WIDTH][-H HEIGHT]\n"
#define ARG_HELP "\n" \
    "  -s  Survive rules, default: '23'\n" \
    "  -b  Born rules, default: '3'\n" \
    "  -f  Initial fill percentage, default: 50\n" \
    "  -w  Output width, default: 800\n" \
    "  -H  Output height, default: 600\n" \
    "\n  Other options:\n" \
    "  -a  Alive colour in rgb hex, default: ff1a1a\n" \
    "  -d  Dying colour in rgb hex, default: a4e443\n" \
    "  -h  Print this help message and exit\n"

static unsigned char fill_percent = DEFAULT_FILL;
static uint32_t alive = 0xFFFF1A1A;
static uint32_t dying = 0xFFA4E443;

// Rules look up table in born, survives order.
static unsigned char rules[2][9] = {{0, 0, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 1, 1, 0, 0, 0, 0, 0}};

static int16_t plane_width;
static int16_t plane_height;
static uint8_t * plane;
static uint8_t * next_plane;

// SDL declarations
static SDL_Window * window;
static SDL_Renderer * renderer;
static SDL_Surface * screen;
static SDL_Texture * screen_tx;

static int mod(int x, int n)
{
    return ((x % n) + n) % n;
}

static void parseCounts(char* cell_counts, unsigned char rules_arr[])
{
    int idx = 0;
    char c = cell_counts[idx];

    memset(rules_arr, 0, sizeof(rules[0]));
    while (isdigit(c))
    {
        int digit = c - 48;
        if (digit >= 0 && digit < 9)
            rules_arr[digit] = 1;
        c = cell_counts[++idx];
    }
}

static int cellularAutomata(uint8_t * in, uint8_t * out, int16_t width, int16_t height)
{
    int16_t x, y;
    char xn, yn;
    uint32_t * pixels = (uint32_t *) screen->pixels;
    int count = 0;  // Count of pixels drawn.

    assert(pixels != NULL);
    assert(in != NULL);
    assert(out != NULL);

    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            int neighbours = 0;
            int idx = 0;
            uint8_t current;
            uint8_t next;
            for (xn = -1; xn <= 1; xn++)
            {
                for (yn = -1; yn <= 1; yn++)
                {
                    if (xn == 0 && yn == 0) continue;
                    // Neighbourhood index, wrap to the opposite side at edges.
                    idx = mod((x + xn), width) + width * mod((y + yn), height);
                    neighbours += in[idx];
                }
            }
            idx = x + width * y;
            current = in[idx];
            next = rules[current][neighbours];
            // Keep count of the number of transitions
            if (current != next) count++;

            out[idx] = next;
            if (next)
            {
                pixels[idx] = alive;
            }
            else if (current != next)
            {
                pixels[idx] = dying;
            }
            else
            {
                // Shift out alpha bits to fade out dying cells
                pixels[idx] = ((pixels[idx] & AMASK) << 1) | (~AMASK & pixels[idx]);
            }
        }
    }

    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    return count;
}

void initPlane(uint8_t * plane, int16_t width, int16_t height)
{
    unsigned int i;
    memset(plane, 0, (size_t)(width * height));

    for (i = 0; i < width * height; i++)
    {
        if (rand() % 101 <= fill_percent)
        {
            plane[i] = 1;
        }
    }
}

static void shutdown()
{
    printf("Shutdown\n");
    free(plane);
    free(next_plane);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(screen_tx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static inline void oneIter()
{
    int count = 0;
    count = cellularAutomata(plane, next_plane, plane_width, plane_height);
    SDL_UpdateTexture(screen_tx, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screen_tx, NULL, NULL);
    SDL_RenderPresent(renderer);
    SWAP_PTRS(plane, next_plane);
    if (count == 0)
    {
        printf("Resetting\n");
        initPlane(plane, plane_width, plane_height);
    }
}

static void renderLoop(int16_t screen_width, int16_t screen_height)
{
    int quit = 0;

    plane_width = screen_width / 2;
    plane_height = screen_height / 2;
    plane = (uint8_t *) malloc((size_t)(plane_width * plane_height));
    next_plane = (uint8_t *) malloc((size_t)(plane_width * plane_height));
    memset(next_plane, 0, (size_t)(plane_width * plane_height));
    
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("ca", 0, 0, screen_width, screen_height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
#ifdef __EMSCRIPTEN__
    // Prevent capturing keyboard events from the whole screen.
#ifdef SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#else
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
    SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
    SDL_EventState(SDL_KEYUP, SDL_DISABLE);
#endif
#endif

    screen = SDL_CreateRGBSurface(
        0, (int)plane_width, (int)plane_height, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
    );

    screen_tx = SDL_CreateTextureFromSurface(renderer, screen);

    initPlane(plane, plane_width, plane_height);
    printf("Initialised\n");

    SDL_Event event;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(oneIter, 0, 1);
#else

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
        }
        oneIter();
    }

#endif

    shutdown();
}

static void setColour(const char * html_hex, uint32_t * argb_colour)
{
    uint32_t rgb_colour = (uint32_t)strtoul(html_hex, NULL, 16);
    *argb_colour = AMASK | rgb_colour;
}

// Emscripten methods
#ifdef __EMSCRIPTEN__
void exitLoop()
{
    emscripten_cancel_main_loop();
    shutdown();
}

void setAliveColour(const char * html_hex)
{
    setColour(html_hex, &alive);
}

void setDyingColour(const char * html_hex)
{
    setColour(html_hex, &dying);
}
#endif

int main(int argc, char** argv)
{
    int16_t screen_width = 800;
    int16_t screen_height = 600;
    int opt;

    while ((opt = getopt(argc, argv, "s:b:f:w:hH:a:d:")) != -1)
    {
        switch (opt)
        {
            case 's':
                parseCounts(optarg, rules[1]);
                break;
            case 'b':
                parseCounts(optarg, rules[0]);
                break;
            case 'f':
                fill_percent = (uint8_t)strtol(optarg, NULL, 10);
                if (fill_percent > 100)
                {
                    fill_percent = DEFAULT_FILL;
                }
                break;
            case 'w':
                screen_width = (int16_t)strtol(optarg, NULL, 10);
                break;
            case 'h':
                printf(USAGE, argv[0]);
                printf(ARG_HELP);
                exit(0);
                break;
            case 'a':
                setColour(optarg, &alive);
                break;
            case 'd':
                setColour(optarg, &dying);
                break;
            case 'H':
                screen_height = (int16_t)strtol(optarg, NULL, 10);
                break;
            default: /* '?' */
                fprintf(stderr, USAGE, argv[0]);
                exit(1);
        }
    }

    // Allow main to be called again with different args in emscripten
    optind = 0;

    renderLoop(screen_width, screen_height);
    return 0;
}
