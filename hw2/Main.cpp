#include <raylib.h>
#include <raymath.h>
#include <iostream>

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
    int rateX = 20;
    int rateY = 20;
    float timer = 1.0f;
    int particleBirthed = 0;

    Particle* particles = new Particle[particleCount]; 
    for (int i = 0; i < particleCount; i++) {
        particles[i].isActive = false;
        particles[i].lifetime = 0;
    }

	while(!WindowShouldClose()){
		float deltaTime = GetFrameTime();

        

        // Input Handling for Emission Rates etc. etc.
        if (IsKeyPressed(KEY_LEFT)) {
            rateX--;
            if (rateX <= 0) { rateX = 1; }
        }
        else if (IsKeyPressed(KEY_RIGHT)) {
            rateX++;
            if (rateX >= 50) { rateX = 50; }
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            rateY--;
            if (rateY >= 0) { rateY = 1; }
        }
        else if (IsKeyPressed(KEY_UP)) {
            rateY++;
            if (rateY >= 50) { rateY = 50; }
        }

        

        // std::cout << "rate X: " << rateX << std::endl;
        // std::cout << "rate Y: " << rateY << std::endl;
        std::cout << deltaTime%1.0f << std::endl;
        
        if (timer > 0.0f) {
            timer -= deltaTime;
            if (timer <= 0.0f) {
                particleBirthed = 0;
                timer = 1.0f;
            }
        }
        
        // Initialize particles based on #6
        if (IsKeyDown(KEY_SPACE)) {
            if (timer > 0.0f) {
                timer -= deltaTime;
                if (timer <= 0.0f) {
                    particleBirthed = 0;
                    timer = 1.0f;
                }
            }
            for (int j = 0; j < particleCount; j++) {
                if (!particles[j].isActive && particleBirthed < rateX) {
                    particles[j].initialize(Vector2{400, 600}, Vector2{GetRandomFloat(-1.0f, 1.0f),(float)-1}, GetRandomValue(50, 100), GetRandomValue(2.0f, 5.0f), randomColor());
                    particleBirthed++;
                    std::cout << particleBirthed << std::endl;
                    break;
                }
            }
            
        }
        // std::cout << particleBirthed << std::endl;
        
        

        // Initialize particles based on #7
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            for (int j = 0; j < rateY; j++) {
                if (!particles[j].isActive) {
                    particles[j].initialize(GetMousePosition(), Vector2{GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)}, GetRandomValue(50, 100), GetRandomValue(2.0f, 5.0f), randomColor());
                    particleBirthed++;
                    break;
                }
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