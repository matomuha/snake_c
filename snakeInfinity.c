#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#define MAX 799

Color colorBackground = {236, 243, 158, 255};
Color colorSnake = {79, 119, 45, 255};
Color colorFood = {19, 42, 19, 255};

int cellSize = 40;
int cellNum = 20;

double LastTimeUpdate = 0;

typedef enum GameScreen{
    START,
    GAME,
    OVER
} GameScreen;

typedef struct Player{
    Vector2 cellsPos[MAX];
    Vector2 direction;
    int length;
    int scores;
} Player;

typedef struct Food{
    Vector2 position;
    int score;
} Food;

typedef struct Button{
    Texture2D buttonTexture;
    Rectangle bounds;
} Button;

typedef struct Game{
    Player player;
    Food food;
} Game;

Vector2 getRandomPosFood(Player *player);
void drawSnake(Player *player);
void drawFood(Food *food);
void drawStartBtn(Button *button);
void drawRestartBtn(Button *button);
void resetSnake(Player *player);
void resetFood(Food *food, Player *player);
bool eventTriggered(double interval);
void moveSnake(Player *player);
void checkCollisionWithFood(Player *player, Food *food);
bool checkCollisionWithSnake(Player *player);
void checkCollisionWithEdge(Player *player);

int main(void){

    SetTargetFPS(60);
    int screenW = cellSize * cellNum;
    int screenH = cellSize * cellNum;

    GameScreen currentScreen = START;

    InitWindow(screenW, screenH, "Snake 🐍");

    Game game = {
        .player = {
            .cellsPos = {
                {10 * cellSize, (10 - 2) * cellSize},
                {10 * cellSize, (10 - 1) * cellSize},
                {10 * cellSize, (10 - 0) * cellSize}
            },

            .direction = {0, cellSize * -1},
            .length = 3,
            .scores = 0
        },

        .food = {
            .position = getRandomPosFood(&game.player), 
            .score = 10
        }
    };

    // Load pngs and resize them
    Button startButton = {
        .buttonTexture = LoadTexture("play.png")};
    startButton.buttonTexture.height /= 2;
    startButton.buttonTexture.width /= 2;
    startButton.bounds = (Rectangle){cellNum / 2 * cellSize - startButton.buttonTexture.width / 2, cellNum / 2 * cellSize - startButton.buttonTexture.height / 2, startButton.buttonTexture.width, startButton.buttonTexture.height};

    Button restartButton = {
        .buttonTexture = LoadTexture("restart.png")};
    restartButton.buttonTexture.height /= 2;
    restartButton.buttonTexture.width /= 2;
    restartButton.bounds = (Rectangle){cellNum / 2 * cellSize - restartButton.buttonTexture.width / 2, cellNum / 2 * cellSize - restartButton.buttonTexture.height / 2, restartButton.buttonTexture.width, restartButton.buttonTexture.height};

    Vector2 mousePoint = {0, 0};

    // Main game loop
    while (!WindowShouldClose()){

        mousePoint = GetMousePosition();

        switch (currentScreen){
        case START:
            if(CheckCollisionPointRec(mousePoint, startButton.bounds)){
                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
                    currentScreen = GAME;
                    LastTimeUpdate = GetTime();
                }
            }
            break;

        case GAME:
            if(eventTriggered(0.2)){
                moveSnake(&game.player);
            }

            checkCollisionWithFood(&game.player, &game.food);

            if(checkCollisionWithSnake(&game.player)){
                currentScreen = OVER;
            }
            checkCollisionWithEdge(&game.player);

            if(IsKeyDown(KEY_LEFT) && game.player.direction.x != 1*cellSize){
                game.player.direction = (Vector2){-1*cellSize, 0};
            }
            if(IsKeyDown(KEY_RIGHT) && game.player.direction.x != -1*cellSize){
                game.player.direction = (Vector2){1*cellSize, 0};
            }
            if(IsKeyDown(KEY_UP) && game.player.direction.y != 1*cellSize){
                game.player.direction = (Vector2){0, -1*cellSize};
            }
            if(IsKeyDown(KEY_DOWN) && game.player.direction.y != -1*cellSize){
                game.player.direction = (Vector2){0, 1*cellSize};
            }
            break;

        case OVER:
            if(CheckCollisionPointRec(mousePoint, restartButton.bounds)){
                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
                    resetSnake(&game.player);
                    resetFood(&game.food, &game.player);
                    currentScreen = GAME;
                    LastTimeUpdate = GetTime();
                }
            }
            break;

        default:
            break;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(colorBackground);

        switch (currentScreen){
            case START:
                DrawText("START GAME", screenW/2 - MeasureText("START GAME", 25)/2, screenH - 300, 25, BLACK);
                drawStartBtn(&startButton);
                break;

            case GAME:
                drawSnake(&game.player);
                drawFood(&game.food);
                break;

            case OVER:
                DrawText(TextFormat("Your score: %d",game.player.scores), screenW/2 - MeasureText("Your score:   ", 25)/2, 250, 25, BLACK);
                DrawText("GAME OVER", screenW/2 - MeasureText("GAME OVER", 25)/2, screenH - 300, 25, BLACK);
                DrawText("Press restart button to play again", screenW/2 - MeasureText("Press restart button to play again", 25)/2, screenH - 260, 25, BLACK);
                drawRestartBtn(&restartButton);
                break;

            default:
                break;
        }

        EndDrawing();
    }
    UnloadTexture(startButton.buttonTexture);
    UnloadTexture(restartButton.buttonTexture);
    CloseWindow();
    return 0;
}

//Get random position of food
Vector2 getRandomPosFood(Player *player){
    Vector2 pos;
    bool positionOccupied;

    do{
        positionOccupied = false;
        pos.x = (GetRandomValue(0, cellNum-1)) * cellSize;
        pos.y = (GetRandomValue(0, cellNum-1)) * cellSize;
        for (int i = 0; i < player->length; i++){
            if (Vector2Equals(pos, player->cellsPos[i])){
                positionOccupied = true;
                break;
            }
        }
    } while (positionOccupied);
    return pos;
}

//Draw snake
void drawSnake(Player *player){
    for (int i = 0; i < player->length; i++){
        Rectangle segment = {player->cellsPos[i].x, player->cellsPos[i].y, cellSize, cellSize};
        DrawRectangleRounded(segment, 0.5, 6, colorSnake);
    }
}

//Draw random food
void drawFood(Food *food){
    Rectangle foodRec = {food->position.x, food->position.y, cellSize, cellSize};
    DrawRectangleRounded(foodRec, 1, 6, colorFood);
}

//Draw start button
void drawStartBtn(Button *button){
    DrawTexture(button->buttonTexture, cellNum / 2 * cellSize - button->buttonTexture.width / 2, cellNum / 2 * cellSize - button->buttonTexture.height / 2, WHITE);
}

//Draw restart button
void drawRestartBtn(Button *button){
    DrawTexture(button->buttonTexture, cellNum / 2 * cellSize - button->buttonTexture.width / 2, cellNum / 2 * cellSize - button->buttonTexture.height / 2, WHITE);
}

//Reset snake for new game
void resetSnake(Player *player){
    for(int i=0; i<MAX;i++){
        player->cellsPos[i] = (Vector2){0, 0};
    }
    player->cellsPos[0] = (Vector2){10 * cellSize, (10 - 2) * cellSize};
    player->cellsPos[1] = (Vector2){10 * cellSize, (10 - 1) * cellSize};
    player->cellsPos[2] = (Vector2){10 * cellSize, (10 - 0) * cellSize};
    player->direction = (Vector2){0, cellSize * -1};
    player->length = 3;
    player->scores = 0;
}

//Reset Food for new game
void resetFood(Food *food, Player *player){
    food->position = getRandomPosFood(player), 
    food->score = 10;
}

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if(currentTime - LastTimeUpdate >= interval){
        LastTimeUpdate = currentTime;
        return true;
    }
    return false;
}

void moveSnake(Player *player){
    for (int i = player->length - 1; i > 0; i--){
        player->cellsPos[i] = player->cellsPos[i - 1];
    }
    player->cellsPos[0] = Vector2Add(player->cellsPos[0], player->direction);
}

void checkCollisionWithFood(Player *player, Food *food){
    if(Vector2Equals(player->cellsPos[0], food->position)){
        player->length++;
        player->cellsPos[player->length -1] = player->cellsPos[player->length -2];
        food->position = getRandomPosFood(player);
        player->scores += 10;
    }
}

bool checkCollisionWithSnake(Player *player){
    for(int i=1; i<player->length; i++){
        if(Vector2Equals(player->cellsPos[0], player->cellsPos[i])){
            return true;
        }
    }
    return false;
}

void checkCollisionWithEdge(Player *player){
    if(player->cellsPos[0].x >= cellSize*cellNum ){
        player->cellsPos[0] = (Vector2){0, player->cellsPos[0].y};
    } else if(player->cellsPos[0].x < 0 ){
        player->cellsPos[0] = (Vector2){799, player->cellsPos[0].y};
    } else if(player->cellsPos[0].y >= cellSize*cellNum ){
        player->cellsPos[0] = (Vector2){player->cellsPos[0].x, 1};
    } else if(player->cellsPos[0].y < 0){
        player->cellsPos[0] = (Vector2){player->cellsPos[0].x, 799};
    }
}
