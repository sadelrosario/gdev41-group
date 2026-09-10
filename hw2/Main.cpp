#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <fstream>

struct Particle {
    bool isActive; //is active or nah
    Vector2 position; //particle position
    Vector2 direction; // where the particle is going
    float speed; //how fast is moving pixel per second
    float lifetime; //how long particle is active in seconds
    float maxLifetime; //how long particle is active in seconds
    Color color; // particle color

    void initialize(Vector2 pos, Vector2 dir, float spd, float life, Color col) {
        isActive = true;
        position = pos;
        direction = dir;
        speed = spd;
        lifetime = life;
        maxLifetime = life;
        color = col;
    }

    void bounce() {
        if (position.x <= 0) { direction.x = -direction.x; }
        if (position.x >= 800) { direction.x = -direction.x; }
        if (position.y <= 0) { direction.y = -direction.y; }
        if (position.y >= 600) { direction.y = -direction.y; }
    }


};

Color randomColor() {
    unsigned char r = GetRandomValue(0, 255);
    unsigned char g = GetRandomValue(0, 255);
    unsigned char b = GetRandomValue(0, 255);
    Color randomColor = {r, g, b, 1};
    return randomColor;
}

float GetRandomFloat(float min, float max) {
    float randomVal = min + (max - min) * ((float)GetRandomValue(0, 10000) / 10000.0f);
    return randomVal;
}

int main(){
	InitWindow(800, 600, "Homework 02");
    int particleCount = 1000;
    float rateX = 50.0f;
    float rateY = 50.0f;
    // float timer = 1.0f;
    float mouseTimePassed = 0.0f;
    float bottomTimePassed = 0.0f;
    int particleBirthed = 0;

    int add_rateX, minus_rateX, add_rateY, minus_rateY, emit_particles_bottom, emit_particles_cursor;

    Particle* particles = new Particle[particleCount]; 
    for (int i = 0; i < particleCount; i++) {
        particles[i].isActive = false;
        particles[i].lifetime = 0;
    }

    // configuration of keys
    std::ifstream file("config.ini");
    file >> add_rateX;
    file.ignore(256, '\n');
    file >> minus_rateX;
    file.ignore(256, '\n');
    file >> add_rateY;
    file.ignore(256, '\n');
    file >> minus_rateY;
    file.ignore(256, '\n');
    file >> emit_particles_bottom;
    file.ignore(256, '\n');
    file >> emit_particles_cursor;
    file.ignore(256, '\n');
    

	while(!WindowShouldClose()){
		float deltaTime = GetFrameTime();

    
        // Input Handling for Emission Rates etc. etc.
        if (IsKeyPressed(minus_rateX)) {
            rateX--;
            if (rateX <= 0) { rateX = 1; }
        }
        else if (IsKeyPressed(add_rateX)) {
            rateX++;
            if (rateX >= 50) { rateX = 50; }
        }
        else if (IsKeyPressed(minus_rateY)) {
            rateY--;
            if (rateY <= 0) { rateY = 1; }
        }
        else if (IsKeyPressed(add_rateY)) {
            rateY++;
            if (rateY >= 50) { rateY = 50; }
        }
        
        // interval between each particle spawn = 1 / rate
        // then we use the interval as a Timer? kind of similar to sleep

        float intervalX = 1.0f/rateX;
        float intervalY = 1.0f/rateY;    
        // Initialize particles based on #6
        if (IsKeyDown(emit_particles_bottom)) {
            bottomTimePassed += deltaTime;
            while (bottomTimePassed >= intervalX) {
                for (int j = 0; j < particleCount; j++) {
                    if (!particles[j].isActive) {
                        particles[j].initialize(Vector2{400, 600}, Vector2{GetRandomFloat(-1.0f, 1.0f),(float)-1}, GetRandomValue(50, 100), GetRandomValue(2.0f, 5.0f), randomColor());
                        particleBirthed++;
                        // std::cout << particleBirthed << std::endl;
                        break;
                    }
                }
                bottomTimePassed -= intervalX;
            }         
        }

        // Initialize particles based on #7
        if (IsMouseButtonDown(emit_particles_cursor)) {
            mouseTimePassed += deltaTime;
            while (mouseTimePassed >= intervalY) {
                for (int j = 0; j < particleCount; j++) {
                    if (!particles[j].isActive) {
                        particles[j].initialize(GetMousePosition(), Vector2{GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)}, GetRandomValue(50, 100), GetRandomValue(2.0f, 5.0f), randomColor());
                        particleBirthed++;
                        break;
                    }
                }
                mouseTimePassed -= intervalY;
            }
            
        }
        
        
        //Update and draw particles
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].isActive) {
                particles[i].position.x += particles[i].direction.x * particles[i].speed * deltaTime;
                particles[i].position.y += particles[i].direction.y * particles[i].speed * deltaTime;
                particles[i].lifetime -= deltaTime;

                float lifeRatio = particles[i].lifetime / particles[i].maxLifetime;
                particles[i].color.a = (lifeRatio * 255); // Fade
                particles[i].bounce();
                
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
		ClearBackground(WHITE);
        
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].isActive) {
                DrawCircle(particles[i].position.x, particles[i].position.y, 5, particles[i].color);
            }
        }

		EndDrawing();
	
	}

    delete[] particles;
	CloseWindow();
	return 0;
}