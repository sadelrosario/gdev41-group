#include <raylib.h>
#include <raymath.h>

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

    // void Ball(vector2 pos, float r, Color c, float m) {
    //     position = pos;
    //     radius = r;
    //     color = c;
    //     mass = m;
    //     inverse_mass = 1 / mass;
    //     acceleration = Vector2Zero();
    //     velocity = Vector2Zero();
    // }
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

        ball1.velocity = ball1.velocity + Vector2Scale(collision_normal, impulse * ball1.inverse_mass);
        ball2.velocity = ball2.velocity - Vector2Scale(collision_normal, impulse * ball2.inverse_mass);
    }
}

int main() {
    Ball ball;
    ball.position = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    ball.radius = 50.0f;
    ball.color = RED;
    ball.mass = 1.0f;
    ball.inverse_mass = 1 / ball.mass;
    ball.acceleration = Vector2Zero();
    ball.velocity = Vector2Zero();

    Ball redBall;
    redBall.position = {WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2 + 100};
    redBall.radius = 50.0f;
    redBall.color = BLUE;
    redBall.mass = 1.0f;
    redBall.inverse_mass = 1 / redBall.mass;
    redBall.acceleration = Vector2Zero();
    redBall.velocity = Vector2Zero();

    Ball greenBall;
    greenBall.position = {WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2 - 100};
    greenBall.radius = 50.0f;
    greenBall.color = GREEN;
    greenBall.mass = 1.0f;
    greenBall.inverse_mass = 1 / greenBall.mass;
    greenBall.acceleration = Vector2Zero();
    greenBall.velocity = Vector2Zero();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Demo");
    
    Ball balls[3] = {ball, redBall, greenBall};

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
            for(int i = 0; i < 3; i++) {
                int nextBall = (i + 1) % 3;
                if (nextBall != i) {
                    CircleToCircleCollision(balls[i], balls[nextBall]);
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

        BeginDrawing();
        ClearBackground(WHITE);
        for(int i = 0; i < 3; i++) {
            DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
        }
        // DrawCircleV(ball.position, ball.radius, ball.color);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}