#include <raylib.h>
#include <raymath.h>


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any ver small value.
const Vector2 GRAVITY = {0, 1000};

struct Ball {
    Vector2 position;
    float radius;
    Color color;

    float mass;
    float inverse_mass; 
    Vector2 acceleration;
    Vector2 velocity;
};

struct Spring {
    Vector2 spring_start;
    Vector2 spring_end;
    float rest_length;
    float b;
    float k;
};

int main() {
    Ball ball;
    ball.position = {WINDOW_WIDTH / 2, 200};
    ball.radius = 50;
    ball.color = RED;
    ball.mass = 1.0f;
    ball.inverse_mass = 1 / ball.mass;
    ball.acceleration = Vector2Zero();
    ball.velocity = Vector2Zero();

    Spring spring;
    spring.spring_start = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    spring.spring_end = ball.position;
    spring.rest_length = Vector2Distance(spring.spring_start, Vector2Scale(spring.spring_end, 1.1));
    spring.b = 1.0f;
    spring.k = 100.0f;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Spring");

    SetTargetFPS(FPS);

    float accumulator = 0;

    while(!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        Vector2 forces = Vector2Zero();

        // Vector2 gravity_force = Vector2Scale(GRAVITY, ball.mass);

        // forces = Vector2Add(forces, gravity_force);

        Vector2 spring_force;

        Vector2 D = Vector2Subtract(spring.spring_end, spring.spring_start);
        Vector2 D_norm = Vector2Normalize(D);

        spring_force = Vector2Scale(D_norm, -spring.k * (Vector2Length(D) - spring.rest_length));
        spring_force = Vector2Subtract(spring_force, Vector2Scale(ball.velocity, spring.b));
        forces = Vector2Add(forces, spring_force);

        if(CheckCollisionPointCircle(GetMousePosition(), ball.position, ball.radius)) {
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                ball.position = GetMousePosition();
                forces = Vector2Zero();
            }
        }

        ball.acceleration = Vector2Scale(forces, ball.inverse_mass);

        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {
            ball.velocity = Vector2Add(ball.velocity, Vector2Scale(ball.acceleration, TIMESTEP));
            ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, TIMESTEP));

            spring.spring_end = ball.position;
            accumulator -= TIMESTEP;
        }

        float spring_thick = 5;

        if(Vector2Length(D) > spring.rest_length) {
            spring_thick -= (Vector2Length(D) / 100);
        }
        else if (Vector2Length(D) < spring.rest_length) {
            spring_thick += (Vector2Length(D) / 100);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawCircleV(ball.position, ball.radius, ball.color);
        DrawLineEx(spring.spring_start, ball.position, spring_thick, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}