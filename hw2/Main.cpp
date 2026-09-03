#include <raylib.h>
#include <raymath.h>

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

int main(){
	InitWindow(800,600, "Homework 02");
    int particleCount = 1000;

    Particle* particles = new Particle[particleCount]; 
    for (int i = 0; i < particleCount; i++) {
        particles[i].isActive = false;
        particles[i].lifetime = 0;
    }

	while(!WindowShouldClose()){
		float deltaTime = GetFrameTime();

        // Input Handling for Emission Rates etc. etc.

        // Initialize particles based on #6

        // Initialize particles based on #7


        //for checking onleh
        // if (IsKeyDown(KEY_SPACE)) {
        //     for (int i = 0; i < particleCount; i++) {
        //         if (!particles[i].isActive) {
        //             particles[i].initialize(Vector2{400, 300}, Vector2{(float)GetRandomValue(-2, 5),(float) -1}, GetRandomValue(5, 25), 2, RED);
        //             break;
        //         }
        //     }
        // }
        
        //Update and draw particles
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].isActive) {
                particles[i].position.x += particles[i].direction.x * particles[i].speed * deltaTime;
                particles[i].position.y += particles[i].direction.y * particles[i].speed * deltaTime;
                particles[i].lifetime -= deltaTime;

                float lifeRatio = particles[i].lifetime / particles[i].maxLifetime;
                particles[i].color.a = (lifeRatio * 255); // Fade
                
                //Clamp so it woundl't go below 0
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