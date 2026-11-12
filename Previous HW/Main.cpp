#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS;
const float ELASTICITY_COEFFICIENT = 1.0f; // 1.0f is a perfect elastic collision, 0.0f is a perfect inelastic collision
const float MAX_STRING_POWER = 300;
const float STOP_ZONE = 5.0f;
const float screenCenterX = WINDOW_WIDTH / 2;
const float screenCenterY = WINDOW_HEIGHT / 2;
const int cellSize = 70;


float GetRandomFloat(float min, float max) {
    float randomVal = min + (max - min) * ((float)GetRandomValue(0, 10000) / 10000.0f);
    return randomVal;
}

Color RandomColor() {
    unsigned char r = GetRandomValue(0, 255);
    unsigned char g = GetRandomValue(0, 255);
    unsigned char b = GetRandomValue(0, 255);
    Color randomColor = {r, g, b, 255};
    return randomColor;
}

struct Ball {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float radius;
    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    bool isBigBall;
    Color color;
    vector<Ball*> collidedBalls;
    bool gridOperated;

    Ball(Vector2 pos, bool big) {
        position = pos;
        radius = big ? 25: GetRandomFloat(5, 10);
        color = RandomColor();
        mass = big ? 10 : 1;
        inverse_mass = 1 / mass;
        acceleration = Vector2Zero();
        velocity = {GetRandomFloat(-200, 200), GetRandomFloat(-200, 200)};
        isBigBall = big;
        gridOperated = false;
    }
};

struct GridCell {
    Vector2 position;
    // Vector 2 for position (1,2) (1,1) or sum shi
    Vector2 size;
    Vector2 round_pos;
    vector<Ball*> ballsInside;

    GridCell(Vector2 pos){
        position = pos;
        size = {(float)cellSize, (float)cellSize };
        round_pos = {pos.x/cellSize, pos.y/cellSize};
    }
};

void CircleToCircleCollision(Ball& ball1, Ball&  ball2){
    Vector2 collision_normal = Vector2Subtract(ball1.position, ball2.position);
    float distance = Vector2Length(collision_normal);
    float radius_sum = ball1.radius + ball2.radius;

    Vector2 relVelA = Vector2Subtract(ball1.velocity, ball2.velocity);
    float velAlongNormal = Vector2DotProduct(relVelA, collision_normal);

    if(distance < radius_sum && velAlongNormal < 0) {
        float impulseNumerator = (1.0f + ELASTICITY_COEFFICIENT) * velAlongNormal;
        float impulseDenominator = Vector2DotProduct(collision_normal, collision_normal) * (ball1.inverse_mass + ball2.inverse_mass);
        float impulse = -(impulseNumerator / impulseDenominator);
        // For Testing
        // SpawnParticles(ball1.position);
        
        ball1.velocity = ball1.velocity + Vector2Scale(collision_normal, impulse * ball1.inverse_mass);
        ball2.velocity = ball2.velocity - Vector2Scale(collision_normal, impulse * ball2.inverse_mass);

        ball1.collidedBalls.push_back(&ball2);
        ball2.collidedBalls.push_back(&ball1);
    }
};

void ballAABB(Ball& ball, int cellsize, vector<GridCell*>& cells) {
    // get ball min max
    Vector2 ball_min = {ball.position.x-ball.radius, ball.position.y-ball.radius};
    Vector2 ball_max = {ball.position.x+ball.radius, ball.position.y+ball.radius};
    // get ball grid coverage
    int ball_min_cell_x = ball_min.x / cellSize;
    int ball_min_cell_y = ball_min.y / cellSize;
    int ball_max_cell_x = ball_max.x / cellSize;
    int ball_max_cell_y = ball_max.y / cellSize;
    // get all the cells that overlaps with ball
    for(int x = ball_min_cell_x; x <= ball_max_cell_x; x++) {
        for(int y = ball_min_cell_y; y <= ball_max_cell_y; y++) {
            for (int i = 0; i < cells.size(); i++) {
                if(cells[i]->position.x == x * cellsize && cells[i]->position.y == y * cellsize) {
                    cells[i]->ballsInside.push_back(&ball);
                }
            }
        }
    }   
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Exercise 5 - Uniform Grid");
    
    int ballCount = 0;

    // Initializing vectors
    vector<Ball> balls;
    vector<GridCell*> cells;
    vector<GridCell*> activeCells;

    for (int y = 0; y < WINDOW_HEIGHT; y += cellSize) {
        for (int x = 0; x < WINDOW_WIDTH; x += cellSize) {
            cells.push_back(new GridCell{{(float)x, (float)y}});
        }
    }

    SetTargetFPS(FPS);

    float accumulator = 0;
    bool paused = false;
    bool gridEnabled = true;
    int clickCount = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        Vector2 forces = Vector2Zero(); // every frame set the forces to a 0 vector

        if (IsKeyPressed(KEY_SPACE)){
            clickCount++;
            if(clickCount < 10){
                ballCount += 25;
                for(int i = 0; i < 25; i++){
                    balls.emplace_back(Vector2{screenCenterX, screenCenterY}, false);
                }
            }else{
                ballCount += 1;
                balls.emplace_back(Vector2{screenCenterX, screenCenterY}, true);
                clickCount = 0;
            }
        }     

        if (IsKeyPressed(KEY_V)){
            gridEnabled = !gridEnabled;
        }

        if (IsKeyPressed(KEY_P)){
            paused = !paused;
            if (paused){
                delta_time = 0;
            }else{
                delta_time = GetFrameTime();
            }
        }

        // remove balls
        activeCells.clear();
        for (GridCell* cell : cells) {
            cell->ballsInside.clear();
        }

        // re-assign grid cells to balls
        for (Ball& ball : balls) {
            ballAABB(ball, cellSize, cells);
            // cout<<"grid assign = POS: "<<ball.position.x<<", "<<ball.position.y<<endl;
            // cout<<"grid assign = VELO: "<<ball.velocity.x<<", "<<ball.velocity.y<<endl;
            ball.collidedBalls.clear();
            ball.gridOperated = false;
        }

        // note which cells have balls
        for (GridCell* cell : cells) {
            if (!cell->ballsInside.empty()) {
                activeCells.push_back(cell);
            }
        }

        // // Does Vector - Scalar multiplication with the sum of all forces and the inverse mass of the ball
        // balls[0].acceleration = Vector2Scale(forces, balls[0].inverse_mass);

        // Physics step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            // ------ SEMI-IMPLICIT EULER INTEGRATION -------
            // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt)
                for (GridCell* cell : activeCells){

                    for (int i = 0; i < cell->ballsInside.size(); i++) {
                        Ball* currentBall = cell->ballsInside[i];
                        // make it so that ball collision is not mistakenly repeated
                        // only check everything after the ball in the list
                        for (int j = i+1; j < cell->ballsInside.size(); j++) {
                            Ball* otherBall = cell->ballsInside[j];

                            // check if other ball has already been collided
                            // auto alreadyCollided = find(currentBall->collidedBalls.begin(), currentBall->collidedBalls.end(), otherBall);
                            // bool alreadyCollided = false;
                            // for (auto ball : currentBall->collidedBalls) {
                            //     if (ball == otherBall) {
                            //         alreadyCollided = true;
                            //         break;
                            //     }
                            // }
                            // check if other ball has already been collided
                            auto alreadyCollided = find(currentBall->collidedBalls.begin(), currentBall->collidedBalls.end(), otherBall);

                            // if not in collidedBalls array, proceed with collision checks
                            // https://www.geeksforgeeks.org/cpp/check-if-vector-contains-given-element-in-cpp/
                            if (alreadyCollided == currentBall->collidedBalls.end()) {
                                // currentBall->collidedBalls.push_back(otherBall);  
                                CircleToCircleCollision(*currentBall, *otherBall);
                            } else {
                                // skip collision check; the other ball has already been collision-checked with
                                continue;
                            }

                            // if not in collidedBalls array, proceed with collision checks
                            // https://www.geeksforgeeks.org/cpp/check-if-vector-contains-given-element-in-cpp/
                            // if (alreadyCollided == false) {
                            //     // currentBall->collidedBalls.push_back(otherBall);  
                            //     CircleToCircleCollision(*currentBall, *otherBall);
                            // } else {
                            //     // skip collision check; the other ball has already been collision-checked with
                            //     continue;
                            
                        }
                        
                        if(!currentBall->gridOperated) {
                            currentBall->velocity = Vector2Add(currentBall->velocity, Vector2Scale(currentBall->acceleration, TIMESTEP));
                            // Computes for change in position using x(t + dt) = x(t) + (v(t + dt) * dt)
                            currentBall->position = Vector2Add(currentBall->position, Vector2Scale(currentBall->velocity, TIMESTEP));

                            // EDGE CHECK
                            // Negates the velocity at x and y if the object hits a wall. (Basic Collision Detection)
                            if(currentBall->position.x + currentBall->radius >= WINDOW_WIDTH || currentBall->position.x - currentBall->radius <= 0) {
                                currentBall->velocity.x *= -1;
                            }
                            if(currentBall->position.y + currentBall->radius >= WINDOW_HEIGHT || currentBall->position.y - currentBall->radius <= 0) {
                                currentBall->velocity.y *= -1;
                            }
                        }

                        currentBall->gridOperated = true;
                        
                    }
                }
            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        if(gridEnabled){
            for (const GridCell* cell : cells) {
                DrawRectangleLines( (int)cell->position.x, (int)cell->position.y, (int)cell->size.x, (int)cell->size.y,RED);
                DrawText(TextFormat("(%d, %d)", (int)cell->round_pos.x, (int)cell->round_pos.y ),  (int)cell->position.x,  (int)cell->position.y, 12, BLACK);
                DrawText(TextFormat(" %d", (int)cell->ballsInside.size()), (int)(cell->position.x + (cellSize/2)-10),  (int)(cell->position.y + (cellSize/2)-10), 20, BLACK);
            }
        }
        
        for (const Ball& ball : balls) {
            DrawCircleV(ball.position, ball.radius, ball.color);
        }
        
        DrawText(TextFormat("Number of Balls: %d", ballCount), 10, 10, 34, BLACK);
        DrawText("Press V to Enable/Disable Grid.", 10, 44, 34, BLACK);
        EndDrawing();
    }

    for (GridCell* cell : cells) {
        delete cell;
    }

    CloseWindow();
    return 0;
}