#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <iostream>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float FRICTION = 0.5;
const float ELASTICITY_COEFFICIENT = 1.0f; // 1.0f is a perfect elastic collision, 0.0f is a perfect inelastic collision
const Vector2 GRAVITY = {0, 1000};
const float MAX_STRING_POWER = 300;
const float STOP_ZONE = 5.0f;
const float screenCenterX = WINDOW_WIDTH / 2;
const float screenCenterY = WINDOW_HEIGHT / 2;

struct Ball {
    Vector2 position;
    float radius;
    Color color;

    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    Vector2 acceleration;
    Vector2 velocity;

    bool isCueBall;

    Ball(Vector2 pos, float r, Color c, float m, bool cue) {
        position = pos;
        radius = r;
        color = c;
        mass = m;
        inverse_mass = 1 / mass;
        acceleration = Vector2Zero();
        velocity = Vector2Zero();
        isCueBall = cue;
    }
};

struct Spring {
    Vector2 spring_start;
    Vector2 spring_end;
    float rest_length;
    float b;
    float k;
};

struct Wall {
    Vector2 position;
    Color color;
    float width;
    float height;

    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    Vector2 acceleration;
    Vector2 velocity;

    Wall(Vector2 pos, float w, float h, float m){
        position = pos;
        width = w;
        height = h;
        mass = m;
        inverse_mass = 1/mass;
        acceleration = Vector2Zero();
        velocity = Vector2Zero();
    }
};


struct Particle {
    bool isActive; //is active or nah
    Vector2 position; //particle position
    Vector2 direction; // where the particle is going
    float speed; //how fast is moving pixel per second
    float rotation;
    float lifetime; //how long particle is active in seconds
    float maxLifetime; //how long particle is active in seconds
    Color color; // particle color

    void initialize(Vector2 pos, Vector2 dir, float spd, float life, Color col, float rot) {
        isActive = true;
        position = pos;
        direction = dir;
        speed = spd;
        lifetime = life;
        maxLifetime = life;
        color = col;
        rotation = rot;
    }
};
int particleCount = 100;
Particle* particles = new Particle[particleCount];

void DrawBoard(Wall (&wallArray)[4], Ball (&holeArray)[4]){
    for(int i = 0; i < 4; i++){
        DrawRectangle(wallArray[i].position.x, wallArray[i].position.y, wallArray[i].width, wallArray[i].height, RED);
    };
    for(int i = 0; i < 4; i++){
        DrawCircleV(holeArray[i].position, holeArray[i].radius, holeArray[i].color);
    };
}

float GetRandomFloat(float min, float max) {
    float randomVal = min + (max - min) * ((float)GetRandomValue(0, 10000) / 10000.0f);
    return randomVal;
}


bool AllBallsStopped(Ball (&ballArray)[5], int count){
    for(int i = 0; i < count; i++){
        if(Vector2Length(ballArray[i].velocity) > STOP_ZONE){
            return false;
        }
    }
    return true;
}

// This is for spawning multiple at a time since the collision impulse will only happen once
// This can be put when the pocket and ball collission is implemented 
void SpawnParticles(Vector2 spawnPosition){
    for (int j = 0; j < particleCount; j++) {
        if (!particles[j].isActive) {
            particles[j].initialize(spawnPosition, {GetRandomFloat(-1,1), -3.f}, GetRandomValue(25, 50), 1.0f, YELLOW, GetRandomFloat(1, 90) );
            particles[j+1].initialize(spawnPosition, {GetRandomFloat(-1,1), -3.f},  GetRandomValue(25, 50), 1.0f, YELLOW, GetRandomFloat(1, 90) );
            particles[j+2].initialize(spawnPosition, {GetRandomFloat(-1,1), -3.f},  GetRandomValue(25, 50), 1.0f, YELLOW, GetRandomFloat(1, 90) );
            particles[j+3].initialize(spawnPosition, {GetRandomFloat(-1,1), -3.f},  GetRandomValue(25, 50), 1.0f, YELLOW, GetRandomFloat(1, 90) );
            particles[j+4].initialize(spawnPosition, {GetRandomFloat(-1,1), -3.f},  GetRandomValue(25, 50), 1.0f, YELLOW, GetRandomFloat(1, 90) );
            // std::cout << particleBirthed << std::endl;
            break;
        }
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
    }
};

void CircleToAABBCollision(Ball& ball, Wall& wall) {
    Vector2 min = wall.position; 
    Vector2 max = {wall.position.x+wall.width, wall.position.y+wall.height}; 
    Vector2 closest_point = Vector2Clamp(ball.position, min, max);  
    Vector2 collision_normal = Vector2Subtract(ball.position, closest_point);
    float distance = Vector2Length(collision_normal);
    
    if(distance < ball.radius) {
        // do collision response
        Vector2 relVelA = Vector2Subtract(ball.velocity, wall.velocity);
        float velAlongNormal = Vector2DotProduct(relVelA, collision_normal);
        float impulseNumerator = (1.0f + ELASTICITY_COEFFICIENT) * velAlongNormal;
        float impulseDenominator = Vector2DotProduct(collision_normal, collision_normal) * (ball.inverse_mass + wall.inverse_mass);
        float impulse = -(impulseNumerator / impulseDenominator);
        ball.velocity = ball.velocity + Vector2Scale(collision_normal, impulse * ball.inverse_mass);

    }
};

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Pool");
    Texture texture = LoadTexture("five-pointed-star.png");
    //Initializing Balls
    float ballRadius = 25.0f;
    float ballMass = 1.0f;
    Ball cueBall = Ball({screenCenterX - 200, screenCenterY}, ballRadius, WHITE, ballMass, true);
    Ball ballOne = Ball({screenCenterX + 100, screenCenterY}, ballRadius, BLUE, ballMass, false);
    Ball ballTwo = Ball({screenCenterX + 140, screenCenterY + 40}, ballRadius, BLUE, ballMass, false);
    Ball ballThree = Ball({screenCenterX + 180, screenCenterY}, ballRadius, BLUE, ballMass, false);
    Ball ballFour = Ball({screenCenterX + 140, screenCenterY - 40}, ballRadius, BLUE, ballMass, false);
    //Initializing Holes
    float pocketRadius = 40.0f;
    float pocketDiameter = pocketRadius*2;
    Ball uLPocket = Ball({pocketRadius, pocketRadius}, pocketRadius, BLACK, 0.0f, false);
    Ball uRPocket = Ball({WINDOW_WIDTH - pocketRadius, pocketRadius}, pocketRadius, BLACK, 0.0f, false);
    Ball dLPocket = Ball({pocketRadius, WINDOW_HEIGHT - pocketRadius}, pocketRadius, BLACK, 0.0f, false);
    Ball dRPocket = Ball({WINDOW_WIDTH - pocketRadius, WINDOW_HEIGHT - pocketRadius}, pocketRadius, BLACK, 0.0f, false);
    //Intialize Walls
    float wallHeight = 40.0f;
    float wallMass = 100000.0f;
    Wall topWall = Wall({pocketDiameter, 0}, WINDOW_WIDTH - (pocketDiameter*2), wallHeight, wallMass);
    Wall botWall = Wall({pocketDiameter, WINDOW_HEIGHT - wallHeight}, WINDOW_WIDTH - (pocketDiameter*2), wallHeight, wallMass);
    Wall rightWall = Wall({WINDOW_WIDTH - wallHeight, pocketDiameter}, wallHeight, WINDOW_HEIGHT - (pocketDiameter*2), wallMass);
    Wall leftWall = Wall({0, pocketDiameter}, wallHeight, WINDOW_HEIGHT - (pocketDiameter*2), wallMass);
    //Initializing Arrays
    int ballCount = 5;
    Ball balls[5] = {cueBall, ballOne, ballTwo, ballThree, ballFour};
    Ball pockets[4] = {uLPocket, uRPocket, dRPocket, dLPocket};
    Wall walls[4] = {topWall, botWall, leftWall, rightWall};
    // Initializing Spring
    Spring spring;
    spring.spring_start = balls[0].position;
    spring.spring_end = balls[0].position;
    // spring.rest_length = Vector2Distance(spring.spring_start, Vector2Scale(spring.spring_end, 1.1));
    spring.rest_length = 0.0f;
    spring.b = 1.0f;
    spring.k = 100.0f;

    SetTargetFPS(FPS);

    float accumulator = 0;
    bool stretched = false;
    bool clicked = false;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        // reset key - R
        if (IsKeyPressed(KEY_R)) {
            cout << "RESET\n";
            ballCount = 5; // because this count changes when a ball falls into a pocket
            balls[0] = cueBall;
            balls[1] = ballOne;
            balls[2] = ballTwo;
            balls[3] = ballThree;
            balls[4] = ballFour;
        }

        Vector2 forces = Vector2Zero(); // every frame set the forces to a 0 vector
        bool ballsStopped = AllBallsStopped(balls, ballCount);
        bool hoveringOnBall = CheckCollisionPointCircle(GetMousePosition(), balls[0].position, balls[0].radius) && balls[0].isCueBall;
        // Do spring physics
        Vector2 spring_force;

        if((hoveringOnBall && ballsStopped) || clicked) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // forces = Vector2Zero();
                clicked = true;
            }
        }else{
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }


        

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && clicked && ballsStopped) {
            spring.spring_end = GetMousePosition();
            stretched = true;
        }

        spring.spring_start = balls[0].position;
        Vector2 D = Vector2Subtract(spring.spring_end, spring.spring_start);
        float D_length = Vector2Length(D);
        if (D_length > MAX_STRING_POWER) {
                D_length = MAX_STRING_POWER;
        }
        // cout << spring.spring_end.x << ", " << spring.spring_end.y << endl;
        
        Vector2 D_norm = Vector2Normalize(D);
        // cout << D_length << endl;
 
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && clicked && stretched && ballsStopped) { 
            // spring_force = Vector2Scale(D_norm, -spring.k * (D_length - spring.rest_length));
            // spring_force = Vector2Subtract(spring_force, Vector2Scale(balls[0].velocity, spring.b)); // damper
            // forces = Vector2Add(forces, spring_force);
            float shot_power = 3.0f;
            Vector2 impulse = Vector2Scale(D_norm,-shot_power*D_length);
            balls[0].velocity = Vector2Add(balls[0].velocity, Vector2Scale(impulse, balls[0].inverse_mass));

            stretched = false;
            clicked = false;
        }

        // Does Vector - Scalar multiplication with the sum of all forces and the inverse mass of the ball
        balls[0].acceleration = Vector2Scale(forces, balls[0].inverse_mass);

        // Physics step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            
            // ------ SEMI-IMPLICIT EULER INTEGRATION -------
            // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt)
            for(int i = 0; i < ballCount; i++) {
                int currentBall = i;
                
                // check collision ball to ball
                for(int j = 0; j < ballCount; j++){
                    if(j == currentBall){
                        continue;
                    }else{
                        CircleToCircleCollision(balls[i], balls[j]);
                    }
                }

                balls[i].velocity = Vector2Add(balls[i].velocity, Vector2Scale(balls[i].acceleration, TIMESTEP));
                balls[i].velocity = Vector2Subtract(balls[i].velocity, Vector2Scale(balls[i].velocity, FRICTION * balls[i].inverse_mass * TIMESTEP));
                // Computes for change in position using x(t + dt) = x(t) + (v(t + dt) * dt)
                balls[i].position = Vector2Add(balls[i].position, Vector2Scale(balls[i].velocity, TIMESTEP));
                
                // rectangle wall collision
                for(int w = 0; w < 4; w++) {
                    // check collosion with walls
                    CircleToAABBCollision(balls[i], walls[w]); 
                }

                //Stopping the Balls
                for (int i = 0; i < ballCount; i++){
                    if (Vector2Length(balls[i].velocity) < STOP_ZONE){
                        balls[i].velocity = Vector2Zero();
                    }
                }

            }

            accumulator -= TIMESTEP;
            
        }

        
        // killing balls process
        for (int i = 0; i < ballCount; i++) {
            for (int j = 0; j < 4; j ++) {
                bool score = CheckCollisionCircles(balls[i].position, balls[i].radius, pockets[j].position, pockets[j].radius-10.0f);
                if (score) {
                    if(i == 0){
                        balls[i].velocity = Vector2Zero();
                        balls[i].position = {screenCenterX - 200, screenCenterY};
                        break;
                    }
                    SpawnParticles(pockets[j].position);
                    ballCount = ballCount - 1;
                    for (int k = i; k < ballCount; k++)
                        balls[k] = balls[k + 1];
                    break;
                }
            }
        }
            
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].isActive) {
                particles[i].position.x += particles[i].direction.x * particles[i].speed * delta_time;
                particles[i].position.y += particles[i].direction.y * particles[i].speed * delta_time;
                particles[i].lifetime -= delta_time;

                float lifeRatio = particles[i].lifetime / particles[i].maxLifetime;
                particles[i].color.a = (lifeRatio * 255); // Fade
                
                //Clamp so it wouldn't go below 0
                if (particles[i].color.a < 0) { 
                    particles[i].color.a = 0;
                }

                if (particles[i].lifetime <= 0) {
                    particles[i].isActive = false;
                }
            }
        }


        BeginDrawing();
        ClearBackground(GREEN);
        DrawBoard(walls, pockets);
        for(int i = 0; i < ballCount; i++) {
            DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
        }
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].isActive) {
                DrawTextureEx(texture, {particles[i].position.x, particles[i].position.y}, particles[i].rotation , 0.25, particles[i].color);
            }
        }
        // DrawCircleV(ball.position, ball.radius, ball.color);
        // Draw the stick
        // Vector2 stick = Vector2Subtract(spring.spring_end, spring.spring_start);
        // Vector2 stick_clamped = Vector2ClampValue(stick, 0, MAX_STRING_POWER);
        if (clicked && stretched && ballsStopped) {
            Vector2 stick = Vector2Subtract(spring.spring_end, spring.spring_start);
            Vector2 stick_clamp = Vector2ClampValue(stick, 0, MAX_STRING_POWER);
            Vector2 stick_end = Vector2Add(spring.spring_start, stick_clamp);

            DrawLineEx(spring.spring_start, stick_end, 5.0f, YELLOW);
        }
        EndDrawing();
    }


    UnloadTexture(texture);
    delete[] particles;
    CloseWindow();
    return 0;
}