#include <raylib.h>
#include <raymath.h>
#include <fstream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float FRICTION = 0.5;
const float ELASTICITY_COEFFICIENT = 1.0f; // 1.0f is a perfect elastic collision, 0.0f is a perfect inelastic collision

struct Ball {
    Vector2 position;
    float radius;
    Color color;

    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    Vector2 acceleration;
    Vector2 velocity;

    Ball(Vector2 pos, float r, Color c, float m) {
        position = pos;
        radius = r;
        color = c;
        mass = m;
        inverse_mass = 1 / mass;
        acceleration = Vector2Zero();
        velocity = Vector2Zero();
    }
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

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Pool");
    float screenCenterX = WINDOW_WIDTH / 2;
    float screenCenterY = WINDOW_HEIGHT / 2;
    Texture texture = LoadTexture("five-pointed-star.png");
    //Initializing Balls
    float ballRadius = 25.0f;
    float ballMass = 1.0f;
    Ball cueBall = Ball({screenCenterX - 200, screenCenterY}, ballRadius, WHITE, ballMass);
    Ball ballOne = Ball({screenCenterX + 100, screenCenterY}, ballRadius, BLUE, ballMass);
    Ball ballTwo = Ball({screenCenterX + 140, screenCenterY + 40}, ballRadius, BLUE, ballMass);
    Ball ballThree = Ball({screenCenterX + 180, screenCenterY}, ballRadius, BLUE, ballMass);
    Ball ballFour = Ball({screenCenterX + 140, screenCenterY - 40}, ballRadius, BLUE, ballMass);
    //Initializing Holes
    float pocketRadius = 40.0f;
    float pocketDiameter = pocketRadius*2;
    Ball uLPocket = Ball({pocketRadius, pocketRadius}, pocketRadius, BLACK, 0.0f);
    Ball uRPocket = Ball({WINDOW_WIDTH - pocketRadius, pocketRadius}, pocketRadius, BLACK, 0.0f);
    Ball dLPocket = Ball({pocketRadius, WINDOW_HEIGHT - pocketRadius}, pocketRadius, BLACK, 0.0f);
    Ball dRPocket = Ball({WINDOW_WIDTH - pocketRadius, WINDOW_HEIGHT - pocketRadius}, pocketRadius, BLACK, 0.0f);
    //Intialize Walls
    float wallHeight = 40.0f;
    Wall topWall = Wall({pocketDiameter, 0}, WINDOW_WIDTH - (pocketDiameter*2), wallHeight, 0);
    Wall botWall = Wall({pocketDiameter, WINDOW_HEIGHT - wallHeight}, WINDOW_WIDTH - (pocketDiameter*2), wallHeight, 0);
    Wall rightWall = Wall({WINDOW_WIDTH - wallHeight, pocketDiameter}, wallHeight, WINDOW_HEIGHT - (pocketDiameter*2), 0);
    Wall leftWall = Wall({0, pocketDiameter}, wallHeight, WINDOW_HEIGHT - (pocketDiameter*2), 0);
    //Initializing Arrays
    int ballCount = 5;
    Ball balls[5] = {cueBall, ballOne, ballTwo, ballThree, ballFour};
    Ball pockets[4] = {uLPocket, uRPocket, dRPocket, dLPocket};
    Wall walls[4] = {topWall, botWall, leftWall, rightWall};

    SetTargetFPS(FPS);

    float accumulator = 0;
    float force = 500.0f;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        Vector2 forces = Vector2Zero(); // every frame set the forces to a 0 vector

        // Adds forces with the magnitude of 100 in the direction given by WASD inputs
        if(IsKeyDown(KEY_W)) {
            forces = Vector2Add(forces, {0, -force});
        }
        if(IsKeyDown(KEY_A)) {
            forces = Vector2Add(forces, {-force, 0});
        }
        if(IsKeyDown(KEY_S)) {
            forces = Vector2Add(forces, {0, force});
        }
        if(IsKeyDown(KEY_D)) {
            forces = Vector2Add(forces, {force, 0});
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
                
                // Negates the velocity at x and y if the object hits a wall. (Basic Collision Detection)
                if(balls[i].position.x + balls[i].radius >= WINDOW_WIDTH || balls[i].position.x - balls[i].radius <= 0) {
                    balls[i].velocity.x *= -1;
                }
                if(balls[i].position.y + balls[i].radius >= WINDOW_HEIGHT || balls[i].position.y - balls[i].radius <= 0) {
                    balls[i].velocity.y *= -1;
                }
            }
            accumulator -= TIMESTEP;
            
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
                // DrawCircle(particles[i].position.x, particles[i].position.y, 5, particles[i].color);
                DrawTextureEx(texture, {particles[i].position.x, particles[i].position.y}, particles[i].rotation , 0.25, particles[i].color);
            }
        }
        // DrawCircleV(ball.position, ball.radius, ball.color);
        EndDrawing();
    }


    UnloadTexture(texture);
    delete[] particles;
    CloseWindow();
    return 0;
}