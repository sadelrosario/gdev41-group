#include <raylib.h>
#include <raymath.h>

#include "entt.hpp"


// Basic Components
struct PositionComponent {
    Vector2 position;
};

struct ColorComponent {
    Color color;
};

struct CircleComponent {
    float radius;
};

struct VelocityComponent {
    Vector2 velocity;
};

// Chance Behaviors
struct MouseAttractComponent{
    float strength;
};

struct MouseRepelComponent{
    float strenght;
};

struct LifeTimeComponent{
    float time_left;
};

struct SplitOnDestroyComponent{
    int spawn_count;
}


//Function To Spawn Asteroid
entt:entity SpawnAsteroid(entt:registry& registry, Vector2 position){
    entt:entity asteroid = registry.create();
    
    //Base Component
    registry.emplace<PositionComponent>(asteroid);
    //Circle Compoonent
    
    //Direction and Speed
    //Add Compnent Here for Velocity and Emplace it

    //Put Mouse Behaviour of Repel or Attract

    //Chance for Lifetime Here

    //Chance for Split

    return asteroid
}


int main() {
    InitWindow(1280, 720, "ECS Homework");

    entt::registry registry;

    // Sample Asteroid
    

    while(!WindowShouldClose()) {
        // auto move_circle = registry.view<CircleComponent>();
        // for (auto entity : move_circle) {
        //     PositionComponent& position = registry.get<PositionComponent>(entity);
        //     CircleComponent& circle = registry.get<CircleComponent>(entity);

        //     position.position.y += 20 * GetFrameTime();
        // }

        // auto move_rect = registry.view<RectangleComponent>();
        // for (auto entity: move_rect) {
        //     PositionComponent& position = registry.get<PositionComponent>(entity);
        //     RectangleComponent& rect = registry.get<RectangleComponent>(entity);

        //     position.position.x += 20 * GetFrameTime();
        // }

        // BeginDrawing();
        // ClearBackground(WHITE);
        // auto all_squares = registry.view<PositionComponent, RectangleComponent, ColorComponent>();
        // for (auto entity: all_squares) {
        //     RectangleComponent& rectangle = registry.get<RectangleComponent>(entity);
        //     PositionComponent& position = registry.get<PositionComponent>(entity);
        //     ColorComponent& color = registry.get<ColorComponent>(entity);

        //     DrawRectangleV(position.position, {rectangle.width, rectangle.height}, color.color);
        // }

        // auto all_circles = registry.view<PositionComponent, CircleComponent, ColorComponent>();
        // for (auto entity : all_circles) {
        //     PositionComponent& pos = registry.get<PositionComponent>(entity);
        //     CircleComponent& circle = registry.get<CircleComponent>(entity);
        //     ColorComponent& color = registry.get<ColorComponent>(entity);

        //     DrawCircleV(pos.position, circle.radius, color.color);
        // }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}