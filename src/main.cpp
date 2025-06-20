#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

#include "GameObject.h"

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width, height, logW, logH;
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture*> textures;
    SDL_Texture *texIdle{};

    SDL_Texture *loadTexture(SDL_Renderer * renderer , const std::string &path) {
        SDL_Texture *tex = IMG_LoadTexture(renderer, path.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    void load(const SDLState &state) {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
        texIdle = loadTexture(state.renderer, "data/idle.png");
    }

    void unload() const {
        for (SDL_Texture *tex: textures) {
            SDL_DestroyTexture(tex);
        }
    }
};

// Storing the game objects such as enemies, bullets, etc
constexpr size_t LAYER_IDX_LEVEL = 0;
constexpr size_t LAYER_IDX_CHARACTERS = 1;
struct GameState {
    std::array<std::vector<GameObject>, 2> layers;
    int playerIndex;

    GameState() {
        playerIndex = 0;
    }
};



bool SDLInit(SDLState& state);
void cleanup(const SDLState& state);
void drawObject(const SDLState& state, GameState& gs, const GameObject& obj, float deltaTime);

int main(int argc, char *argv[]) {

    SDLState state{};
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
    GameState gs;
    GameObject player;
    player.type = ObjectType::player;
    player.texture = res.texIdle;
    player.animations = res.playerAnims;
    player.currentAnimation = res.ANIM_PLAYER_IDLE;
    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

    const bool *keys = SDL_GetKeyboardState(nullptr);
    uint64_t prevTime = SDL_GetTicks();
    // Game Loop
    bool running = true;
    while (running) {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f; // NOLINT(*-narrowing-conversions)

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

        // ***************INEFFICIENT******************

        // Handling Movement
        // float moveAmount = 0, moveValue = 75.0f;
        // if (keys[SDL_SCANCODE_A]) {
        //     moveAmount -= moveValue;
        //     flipHorizontal = true;
        // }
        // if (keys[SDL_SCANCODE_D]) {
        //     moveAmount += moveValue;
        //     flipHorizontal = false;
        // }
        // playerX += moveAmount * deltaTime;

        //*********************************************

        //update all objects
        for (auto& layer: gs.layers) {
            for (auto& obj: layer) {
                if (obj.currentAnimation != -1) {
                    obj.animations[obj.currentAnimation].step(deltaTime);
                }
            }
        }

        // Drawing Commands
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        //draw all objects
        for (auto& layer: gs.layers) {
            for (auto& obj: layer) {
                drawObject(state, gs, obj, deltaTime);
            }
        }


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


void cleanup(const SDLState& state) {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void drawObject(const SDLState& state, GameState& gs, const GameObject& obj, float deltaTime) {
    constexpr float spriteSize = 32;
    float srcX = obj.animations[obj.currentAnimation].currentFrame()*spriteSize;
    SDL_FRect src{
        .x = srcX,
        .y = 0,
        .w = spriteSize,
        .h = spriteSize
    };

    SDL_FRect dst{
        .x = obj.position.x,
        .y = obj.position.y,
        .w = spriteSize,
        .h = spriteSize,
    };
    SDL_FlipMode flipMode = (obj.direction == -1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
}