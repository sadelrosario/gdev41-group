#include <raylib.h>
#include <raymath.h>
#include <cstdlib> // for rand()
#include "entt.hpp"
#include <iostream>
#include <chrono>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS;
const float screenCenterX = WINDOW_WIDTH / 2;
const float screenCenterY = WINDOW_HEIGHT / 2;


// Basic Components
struct PositionComponent {
    Vector2 position;
};

struct ColorComponent {
    Color color;
};

struct CircleComponent {
    float radius;
    bool isActive;
    // bool isSplit;
    // int spawn_count;
};

struct VelocityComponent {
    Vector2 velocity;
};

// Chance Behaviors
struct MouseAttractComponent{
    float strength;
};

struct MouseRepelComponent{
    float strength;
};

struct LifeTimeComponent{
    // float life_time;
    float time_left;
};

struct SplitOnDestroyComponent{
    int spawn_count;
};

// struct CountDownComponent{
//     String timerString;
// }

float GetRandomFloat(float min, float max) {
    float randomVal = min + (max - min) * ((float)GetRandomValue(0, 10000) / 10000.0f);
    return randomVal;
}

//Function To Spawn Asteroid
entt::entity SpawnAsteroid(entt::registry& registry, Vector2 position){
    entt::entity asteroid = registry.create();
    
    //Base Component
    PositionComponent& pos_comp = registry.emplace<PositionComponent>(asteroid);
    pos_comp.position = position;

    //Circle Compoonent
    CircleComponent& circ_comp = registry.emplace<CircleComponent>(asteroid);
    circ_comp.radius = GetRandomValue(20, 50);
    circ_comp.isActive = false;

    //Direction and Speed
    Vector2 dir = Vector2{GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)};
    float speed = GetRandomValue(50, 100);
    
    //Add Compnent Here for Velocity and Emplace it
    VelocityComponent& vel_comp = registry.emplace<VelocityComponent>(asteroid);
    vel_comp.velocity = Vector2Scale(dir, speed);


    //Put Mouse Behaviour of Repel or Attract
    int behavior = rand() % 100 + 1;
    ColorComponent& color_comp = registry.emplace<ColorComponent>(asteroid);
    // 25% chance to be attracted
    if (behavior <= 25) {
        MouseAttractComponent& attraction_comp = registry.emplace<MouseAttractComponent>(asteroid);
        attraction_comp.strength = 100;
        color_comp.color = BLUE;
    }
    // 25% chance to be repel
    else if (behavior <= 50) {
        MouseRepelComponent& repel_comp = registry.emplace<MouseRepelComponent>(asteroid);
        repel_comp.strength = 100;
        color_comp.color = RED;
    }
    // 50% asteroid is normal (no repel or attraction)
    else {
        color_comp.color = GRAY;
    }

    //Chance for Lifetime Here
    int self_destruct = rand() % 100 + 1;
    if (self_destruct <= 35) {
        LifeTimeComponent& countdown = registry.emplace<LifeTimeComponent>(asteroid);
        countdown.time_left = GetRandomValue(2, 5);
        // countdown.life_time = 0;
    }

    //Chance for Split
    int split = rand() % 100 + 1;
    if (split <= 10) {
        SplitOnDestroyComponent& splitComp = registry.emplace<SplitOnDestroyComponent>(asteroid);
        // circ_comp.isSplit = true;
        splitComp.spawn_count = GetRandomValue(2 , 5);
    } 

    // if (registry.valid(asteroid)) {
    //     std::cout<<"valid registry"<<std::endl;
    // }
    
    return asteroid;
}


void DestroyAsteroid(entt::registry& registry, entt::entity entity){
    std::cout<<"IM DELETING\n";
    if(!registry.valid(entity)) return;

    if (registry.all_of<SplitOnDestroyComponent>(entity)){
        Vector2 pos = registry.get<PositionComponent>(entity).position;
        int spawnCount = registry.get<SplitOnDestroyComponent>(entity).spawn_count;
        for (int i = 0; i < spawnCount; i++){
            entt::entity asteroid = SpawnAsteroid(registry, pos);
            CircleComponent& circle = registry.get<CircleComponent>(asteroid);
            circle.isActive = true;
        }
    }

    registry.destroy(entity);
}


int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ECS Homework");
    SetTargetFPS(60);

    
    entt::registry registry;
    float timePassed = 0.0f;
    float accumulator = 0.0f;
    float rate = 1.0f;
    int asteroidCount = 10;
    
    // spawn asteroid
    for (int i = 0; i < asteroidCount; i++) {
        entt::entity asteroid = SpawnAsteroid(registry, Vector2{screenCenterX, screenCenterY});
        // std::cout<<"made asteroid"<<std::endl;
    }
    
    while(!WindowShouldClose()) {        
        std::vector<entt::entity> to_destroy;

        // timer
        float delta_time = GetFrameTime();

        // get all asteroids with base components
        auto all_asteroids = registry.view<PositionComponent, CircleComponent, ColorComponent, VelocityComponent>();

        // get all asteroids with extra components
        auto all_attracting = registry.view<PositionComponent, MouseAttractComponent>();
        auto all_repeling = registry.view<PositionComponent, MouseRepelComponent>();
        auto all_lifetime = registry.view<PositionComponent, LifeTimeComponent, CircleComponent>();
        auto all_splitting = registry.view<PositionComponent, CircleComponent, SplitOnDestroyComponent>();
        
        ; 

        // actually spawn the asteroids
        float interval = 1/rate;
        timePassed += delta_time;
        while (timePassed >= interval) {

            SpawnAsteroid(registry, Vector2{screenCenterX, screenCenterY});


            for (auto entity : all_asteroids) {
                CircleComponent& circle = all_asteroids.get<CircleComponent>(entity);
                PositionComponent& pos = all_asteroids.get<PositionComponent>(entity);
                if (!circle.isActive) {
                    pos.position = Vector2{screenCenterX, screenCenterY};
                    circle.isActive = true;
                    break;
                }
            }
            timePassed -= interval;
        }

        // Physics step
        
            // std::cout<<"enter physics step"<<std::endl;
            
            for (auto entity : all_asteroids) {
                // std::cout<<"checking all asteroids"<<std::endl;

                PositionComponent& pos = all_asteroids.get<PositionComponent>(entity);
                VelocityComponent& vel = all_asteroids.get<VelocityComponent>(entity);
                CircleComponent& circle = all_asteroids.get<CircleComponent>(entity);

                // // CHECK IF ASTEROID IS KILLED ON CLICK
                if (CheckCollisionPointCircle(GetMousePosition(), pos.position, circle.radius)) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){                         
                        std::cout<<"KILL ASTEROID"<<std::endl;
                        // to_destroy.push_back(entity);
                        circle.isActive = false;
                        to_destroy.push_back(entity);
                    }

                }

                // UPDATE VELOCITY
                // entity.velocity = Vector2Add(entity.velocity, Vector2Scale(entity.acceleration, TIMESTEP));
                pos.position = Vector2Add(pos.position, Vector2Scale(vel.velocity, TIMESTEP));
                
            }

            // SPLIT ATTRIBUTE
            for (auto split_asteroid : all_splitting) {
                    SplitOnDestroyComponent& split = all_splitting.get<SplitOnDestroyComponent>(split_asteroid);
                    PositionComponent& split_pos = all_splitting.get<PositionComponent>(split_asteroid);  
                    CircleComponent& split_circle = all_splitting.get<CircleComponent>(split_asteroid);  
                    if (CheckCollisionPointCircle(GetMousePosition(), split_pos.position, split_circle.radius)) {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                            std::cout<<"SPLIT ON CLICK"<<std::endl;
                            to_destroy.push_back(split_asteroid);
                        }
                        break;
                    } 
                }

            // WALL COLLISION
            auto bouncing = registry.view<PositionComponent, VelocityComponent, CircleComponent>();
            for (auto entity : bouncing){
                PositionComponent& pos = all_asteroids.get<PositionComponent>(entity);
                VelocityComponent& vel = all_asteroids.get<VelocityComponent>(entity);
                CircleComponent& circle = all_asteroids.get<CircleComponent>(entity);
                
                if(pos.position.x + circle.radius > WINDOW_WIDTH) {
                    pos.position.x = WINDOW_WIDTH - circle.radius;
                    vel.velocity.x *= -1;
                } else if ( pos.position.x - circle.radius < 0){
                    pos.position.x = circle.radius;
                    vel.velocity.x *= -1;
                }

                if(pos.position.y + circle.radius > WINDOW_HEIGHT) {
                    pos.position.y = WINDOW_HEIGHT - circle.radius;
                    vel.velocity.y *= -1;
                } else if ( pos.position.y - circle.radius < 0){
                    pos.position.y = circle.radius;
                    vel.velocity.y *= -1;
                }
            }
            
            // ATTRACTION
            for (auto entity : all_attracting) {
                MouseAttractComponent& attraction = all_attracting.get<MouseAttractComponent>(entity);
                auto& pos = all_attracting.get<PositionComponent>(entity);
                if (IsCursorOnScreen()) {   
                    Vector2 dir = Vector2Normalize(Vector2Subtract(GetMousePosition(), pos.position));
                    pos.position = Vector2Add(pos.position, Vector2Scale(dir, attraction.strength * delta_time));
                }
            }
            
            // REPEL
            for (auto entity : all_repeling) {
                MouseRepelComponent& repel = all_repeling.get<MouseRepelComponent>(entity);
                auto& pos = all_repeling.get<PositionComponent>(entity);
                if (IsCursorOnScreen()) {
                    Vector2 dir = Vector2Normalize(Vector2Subtract(GetMousePosition(), pos.position));
                    pos.position = Vector2Subtract(pos.position, Vector2Scale(dir, repel.strength * delta_time));
                }
            }

            for (auto entity : all_lifetime) {
                CircleComponent& circle = all_lifetime.get<CircleComponent>(entity);
                LifeTimeComponent& countdown = all_lifetime.get<LifeTimeComponent>(entity);
                if (circle.isActive) {
                    countdown.time_left -= delta_time;
                    if (countdown.time_left <= 0) {
                        // self-destruct
                        circle.isActive = false;
                        to_destroy.push_back(entity);
                    }
                }
            }

        
        for (auto entity : to_destroy) {
           DestroyAsteroid(registry, entity);
        }
        

        BeginDrawing();
        ClearBackground(WHITE);

        for (auto entity : all_asteroids) {
            PositionComponent& pos = all_asteroids.get<PositionComponent>(entity);
            CircleComponent& circle = all_asteroids.get<CircleComponent>(entity);
            ColorComponent& color = all_asteroids.get<ColorComponent>(entity);
            if(circle.isActive) {
                DrawCircleV(pos.position, circle.radius, color.color);
                // std::cout<<"DRAW ASTEROID"<<std::endl;
                // std::cout<< pos.position.x << "    " << pos.position.y <<std::endl;
                bool has_all = registry.all_of<LifeTimeComponent>(entity);
                if (has_all){
                    LifeTimeComponent& lftComp = registry.get<LifeTimeComponent>(entity);
                    DrawText(TextFormat("%.2f",lftComp.time_left), pos.position.x, pos.position.y, 20, WHITE);
                    // DrawText(std::format("{:.2f}", lftComp.time_left), pos.position.x, pos.position.y, 20, WHITE);
                }   
            }
            
        }
        
        EndDrawing();
        
    }
    
    CloseWindow();

    return 0;
}