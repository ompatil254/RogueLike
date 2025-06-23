#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

#include "GameObject.h"


//*************** STRUCTS *********************

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width, height, logW, logH;
    const bool *keys;

    SDLState(): keys(SDL_GetKeyboardState(nullptr)) {
    }

};



struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture*> textures;
    SDL_Texture *texIdle{}, *texRun{}, *texBrick{}, *texGrass{}, *texGround{}, *texPanel{};

    SDL_Texture *loadTexture(SDL_Renderer * renderer , const std::string &path) {
        SDL_Texture *tex = IMG_LoadTexture(renderer, path.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    void load(const SDLState &state) {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
        playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
        texIdle = loadTexture(state.renderer, "data/idle.png");
        texRun = loadTexture(state.renderer, "data/run.png");
        texBrick = loadTexture(state.renderer, "data/tiles/brick.png");
        texGrass = loadTexture(state.renderer, "data/tiles/grass.png");
        texGround = loadTexture(state.renderer, "data/tiles/ground.png");
        texPanel = loadTexture(state.renderer, "data/tiles/panel.png");
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
constexpr int MAP_ROWS = 5;
constexpr int MAP_COLS = 50;
constexpr int TILE_SIZE = 32;

struct GameState {
    std::array<std::vector<GameObject>, 2> layers;
    int playerIndex;

    GameState() {
        playerIndex = 0;
    }
};


//---------------------- STRUCT ENDS ----------------------




//******************* FUNCTION DEFINITIONS **********************

bool SDLInit(SDLState& state);
void cleanup(const SDLState& state);
void drawObject(const SDLState& state, GameState& gs, const GameObject& obj, float deltaTime);
void update(const SDLState& state, GameState& gs, GameObject& obj, Resources& res, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void collisionResponse(const SDLState &state, GameState &gs, Resources &res, const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC, GameObject &objA, GameObject &objB, float deltaTime);

//------------------- FUNC DEF ENDS -------------------------



//*********************** MAIN *******************************


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
    createTiles(state, gs, res);

    uint64_t prevTime = SDL_GetTicks();



    //******************* GAME LOOP **********************

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
                default:
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
                update(state, gs, obj, res, deltaTime);
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

    //---------------------- GAME LOOP ENDS ---------------------





    res.unload();
    cleanup(state);
    return 0;
}



//------------------------ MAIN ENDS -----------------------------




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
    float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame()*spriteSize : 0;
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




void update(const SDLState& state, GameState& gs, GameObject& obj, Resources& res, float deltaTime) {
    if (obj.dynamic) {
        obj.velocity += glm::vec2(0, 300) * deltaTime;
    }
    int currentDirection = 0;
    if (obj.type == ObjectType::player) {
        if (state.keys[SDL_SCANCODE_A]) {
            currentDirection -= 1;
        }
        if (state.keys[SDL_SCANCODE_D]) {
            currentDirection += 1;
        }
        if (currentDirection) {
            obj.direction = currentDirection;
        }

        switch (obj.data.player.state) {
            case PlayerState::idle: {
                if (currentDirection) {
                    obj.data.player.state = PlayerState::running;
                    obj.texture = res.texRun;
                    obj.currentAnimation = res.ANIM_PLAYER_RUN;
                }
                else {
                    if (obj.velocity.x) {}
                    const float factor = obj.velocity.x > 0 ? -1.2 : 1.2;
                    float amount = factor * obj.acceleration.x * deltaTime;
                    if (std::abs(obj.velocity.x) < std::abs(amount)) {
                        obj.velocity.x = 0;
                    }
                    else {
                        obj.velocity.x += amount;
                    }
                }
                break;
            }
            case PlayerState::running: {
                if (!currentDirection) {
                    obj.data.player.state = PlayerState::idle;
                    obj.texture = res.texIdle;
                    obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                }
                break;
            }

            default: {
                break;
            }
        }

        obj.velocity += obj.acceleration * (deltaTime * currentDirection);
        // obj.velocity.x += currentDirection * obj.acceleration.x * deltaTime;
        // obj.velocity.y += currentDirection * obj.acceleration.y * deltaTime;
        if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
            obj.velocity.x = obj.maxSpeedX * currentDirection;
        }
    }
    obj.position += obj.velocity * deltaTime;

    for (auto &layer : gs.layers) {
        for (auto &objB : layer) {
            if (&objB != &obj) {
                checkCollision(state, gs, res, obj, objB, deltaTime);
            }
        }
    }
}


void collisionResponse(const SDLState &state, GameState &gs, Resources &res, const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC, GameObject &objA, GameObject &objB, float deltaTime) {
    if (objA.type == ObjectType::player) {
        switch (objB.type) {
            case ObjectType::enemy: {
                break;
            }
            case ObjectType::level: {
                if (rectC.w < rectC.h) { // Horizontal Collision
                    if (objA.velocity.x > 0) { // Right side
                        objA.position.x -= rectC.w;
                    }
                    else if (objA.velocity.x < 0) {
                        objA.position.x += rectC.w;
                    }
                    objA.velocity.x = 0;
                }
                else {
                    if (objA.velocity.y > 0) {
                        objA.position.y -= rectC.h;
                    }
                    else if (objA.velocity.y < 0) {
                        objA.position.y += rectC.h;
                    }
                    objA.velocity.y = 0;
                }
                break;
            }
            default: {
                break;
            }
        }

    }
}


void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime) {
    SDL_FRect rectA{
        .x = a.position.x + a.collider.x,
        .y = a.position.y + a.collider.y,
        .w = a.collider.w,
        .h = a.collider.h
    };

    SDL_FRect rectB{
        .x = b.position.x + b.collider.x,
        .y = b.position.y + b.collider.y,
        .w = b.collider.w,
        .h = b.collider.h
    };

    SDL_FRect rectC{0};

    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC)) {
        collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime);
    }

}


void createTiles(const SDLState& state, GameState& gs, const Resources& res) {
    /*
        1 - Ground
        2 - Panel
        3 - Enemy
        4 - Player
        5 - Grass
        6 - Brick
    */

    short map[MAP_ROWS][MAP_COLS] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 0, 0, 2, 2, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 0, 2, 2, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

    const auto createObject = [&state](int r, int c, SDL_Texture *tex, ObjectType type) {
        GameObject obj;
        obj.type = type;
        obj.texture = tex;
        obj.position = glm::vec2(
            c * TILE_SIZE,
            state.logH - (MAP_ROWS - r) * TILE_SIZE
        );
        obj.collider = {
            .x = 0, .y = 0,
            .w = TILE_SIZE, .h = TILE_SIZE
        };
        return obj;
    };


    for (int i = 0; i < MAP_ROWS; i++) {
        for (int j = 0; j < MAP_COLS; j++) {
            switch (map[i][j]) {
                case 1: {
                    GameObject obj = createObject(i, j, res.texGround, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                    break;
                }

                case 2: {
                    GameObject obj = createObject(i, j, res.texPanel, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                    break;
                }

                case 3: {

                    break;
                }

                case 4: {
                    GameObject player = createObject(i, j, res.texIdle, ObjectType::player);
                    player.data.player = PlayerData();
                    player.animations = res.playerAnims;
                    player.currentAnimation = res.ANIM_PLAYER_IDLE;
                    player.acceleration = glm::vec2(300, 0);
                    player.maxSpeedX = 100;
                    player.dynamic = true;
                    player.collider = {
                        .x = 11, .y = 6,
                        .w = 10, .h = 26
                    };
                    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
                    break;
                }

                case 5: {
                    GameObject obj = createObject(i, j, res.texGrass, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                    break;
                }

                case 6: {
                    GameObject obj = createObject(i, j, res.texBrick, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(obj);
                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

}
