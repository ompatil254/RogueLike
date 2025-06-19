#include <filesystem>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Animation.h"


struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width, height, logW, logH;
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture*> textures;
    SDL_Texture *texIdle;

    SDL_Texture *loadTexture(SDL_Renderer * renderer , const std::string &path) {
        SDL_Texture *tex = IMG_LoadTexture(renderer, path.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    void load(SDLState &state) {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
        texIdle = loadTexture(state.renderer, "data/idle.png");
    }

    void unload() {
        for (SDL_Texture *tex: textures) {
            SDL_DestroyTexture(tex);
        }
    }
};



bool SDLInit(SDLState& state);
void cleanup(SDLState& state);


int main(int argc, char *argv[]) {

    SDLState state;
    state.width = 1600;
    state.height = 900;
    state.logW = 640;
    state.logH = 320;
    if (!SDLInit(state)) {
        return 1;
    }


    // Loading Game Assets
    Resources res;
    res.load(state);


    // setup game data
    const bool *keys = SDL_GetKeyboardState(nullptr);
    float playerX = 150;
    const float floor = state.logH;
    uint64_t prevTime = SDL_GetTicks();
    bool flipHorizontal = false;

    // Game Loop
    bool running = true;
    while (running) {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f;

        SDL_Event event{0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    break;
            }
        }

        // Handling Movement
        float moveAmount = 0, moveValue = 75.0f;
        if (keys[SDL_SCANCODE_A]) {
            moveAmount -= moveValue;
            flipHorizontal = true;
        }
        if (keys[SDL_SCANCODE_D]) {
            moveAmount += moveValue;
            flipHorizontal = false;
        }
        playerX += moveAmount * deltaTime;

        // Drawing Commands
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        constexpr float spriteSize = 32;
        SDL_FRect src{
            .x = 0,
            .y = 0,
            .w = spriteSize,
            .h = spriteSize
        };

        SDL_FRect dst{
            .x = playerX,
            .y = floor - spriteSize,
            .w = spriteSize,
            .h = spriteSize,
        };

        SDL_RenderTextureRotated(state.renderer, res.texIdle, &src, &dst, 0, nullptr,
            (flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE );

        // Swap buffers and present
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }


    res.unload();
    cleanup(state);
    return 0;
}

bool SDLInit(SDLState& state) {
    bool success = true;

    // Initialization
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        success = false;
    }


    // Creating Window
    state.window = SDL_CreateWindow("Roguelike", state.width, state.height, SDL_WINDOW_RESIZABLE);

    if (!state.window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup(state);
        success = false;
    }


    // Creating Renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", nullptr);
        cleanup(state);
        success = false;
    }


    // Configure Presentation
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return success;
}


void cleanup(SDLState& state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}