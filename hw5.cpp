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
const float HALF_WIDTH = max(WINDOW_WIDTH, WINDOW_HEIGHT) / 2.0f; 
const float MAX_DEPTH = 4;
const Vector2 CENTER = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
const Vector2 ROOT_MIN = {CENTER.x - HALF_WIDTH, CENTER.y - HALF_WIDTH};
const Vector2 ROOT_MAX = {CENTER.x + HALF_WIDTH, CENTER.y + HALF_WIDTH};


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
struct QuadNode;

struct Ball {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float radius;
    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    bool isBigBall;
    Color color;
    QuadNode* assignedNode; // store the node info the ball was inserted to
    int nodeIndex;

    Ball(Vector2 pos, bool big) {
        position = pos;
        radius = big ? 25: GetRandomFloat(5, 10);
        color = RandomColor();
        mass = big ? 10 : 1;
        inverse_mass = 1 / mass;
        acceleration = Vector2Zero();
        velocity = {GetRandomFloat(-200, 200), GetRandomFloat(-200, 200)};
        isBigBall = big;
        assignedNode = nullptr;
        nodeIndex = 0;
    }
};


struct QuadNode {
    float halfWidth;
    Vector2 min;
    Vector2 max;

    QuadNode* parent;
    bool isChild;
    
    vector<QuadNode*> children;
    vector<Ball*> ballsInside;
};



// recursion function
void QuadDivision(QuadNode* parentNode, int depth) {
    
    // base case (MAX DEPTH)
    if (depth >= MAX_DEPTH) {
        parentNode->isChild = true;
        return;
        // do nothing; don't have children
    }

    float childHalf = parentNode->halfWidth / 2.0f;
    // center of the parent = min + halfWidth
    Vector2 center = { parentNode->min.x + parentNode->halfWidth, parentNode->min.y + parentNode->halfWidth };

    Vector2 mins[4] = {
        parentNode->min,                    // top-left
        { center.x, parentNode->min.y },         // top-right
        { parentNode->min.x, center.y },         // bottom-left
        center                             // bottom-right
    };

    for (int i = 0; i < 4; i++) {
        QuadNode* child = new QuadNode{ childHalf, mins[i], { mins[i].x + parentNode->halfWidth, mins[i].y + parentNode->halfWidth }, parentNode, true};
        parentNode->children.push_back(child);
        QuadDivision(child, depth + 1);
    }
    
}

//if inside
bool BallFitsInsideNode(const Ball& ball, const QuadNode& node) {
    float ballLeft = ball.position.x - ball.radius;
    float ballRight = ball.position.x + ball.radius;
    float ballTop = ball.position.y - ball.radius;
    float ballBottom = ball.position.y + ball.radius;

    return ballLeft >= node.min.x && ballRight <= node.max.x && ballTop >= node.min.y && ballBottom <= node.max.y;
}

//check if its intersecting
bool BallIntersectsNode(const Ball& ball, const QuadNode& node){
    float ballLeft = ball.position.x - ball.radius;
    float ballRight = ball.position.x + ball.radius;
    float ballTop = ball.position.y - ball.radius;
    float ballBottom = ball.position.y + ball.radius;

    return ballRight >= node.min.x && ballLeft <= node.max.x && ballBottom >= node.min.y && ballTop <= node.max.y;
}

void ClearBallsFromTree(QuadNode* node){
    node->ballsInside.clear();

    for(QuadNode* child: node->children){
        ClearBallsFromTree(child);
    }
}

void InsertBall(QuadNode* node, Ball* ball) {
    //if the bottom/child node
    if (node->children.empty()) {
        ball->assignedNode = node;
        ball->nodeIndex = (int)node->ballsInside.size();
        node->ballsInside.push_back(ball);
        return;
    }

    //try to find the child that completly contains the ball
    for (QuadNode* child : node->children) {
        if (BallFitsInsideNode(*ball, *child)) {
            InsertBall(child, ball);
            return;
        }
    }

    //if wala then stay kung saan sya
    ball->assignedNode = node;
    ball->nodeIndex = (int)node->ballsInside.size();
    node->ballsInside.push_back(ball);
}

//recursive function
void QueryCandidates(QuadNode* node, const Ball& target, vector<Ball*>& candidates){
    if(!BallIntersectsNode(target,*node)){
        return;
    } //exit nya

    for(Ball* ball: node->ballsInside){
        candidates.push_back(ball);
    }

    for(QuadNode* child: node->children){
        QueryCandidates(child,target,candidates);
    } //it goes down deeper
}

void InsertAllBalls(QuadNode* root, vector<Ball>& balls) {
    ClearBallsFromTree(root);

    for (Ball& ball : balls) {
        InsertBall(root, &ball);
    }
}

void CircleToCircleCollision(Ball& ball1, Ball&  ball2){

    // cout << "I AM COLLIDING \n";
    Vector2 collision_normal = Vector2Subtract(ball1.position, ball2.position);
    float distance = Vector2Length(collision_normal);
    float distanceSq = collision_normal.x *collision_normal.x + collision_normal.y *collision_normal.y;
    float radius_sum = ball1.radius + ball2.radius;
    if (distanceSq >= radius_sum * radius_sum) return; //early exit to if not touching

    Vector2 relVelA = Vector2Subtract(ball1.velocity, ball2.velocity);
    float velAlongNormal = Vector2DotProduct(relVelA, collision_normal);
    if(velAlongNormal >= 0) return; // early exit uli if they are separating from each other

    if(distance < radius_sum && velAlongNormal < 0) {
        float impulseNumerator = (1.0f + ELASTICITY_COEFFICIENT) * velAlongNormal;
        float impulseDenominator = Vector2DotProduct(collision_normal, collision_normal) * (ball1.inverse_mass + ball2.inverse_mass);
        float impulse = -(impulseNumerator / impulseDenominator);
        ball1.velocity = ball1.velocity + Vector2Scale(collision_normal, impulse * ball1.inverse_mass);
        ball2.velocity = ball2.velocity - Vector2Scale(collision_normal, impulse * ball2.inverse_mass);
    }
};


int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Exercise 5 - Uniform Grid");
    
    int ballCount = 0;

    // Initializing vectors
    vector<Ball> balls;
    // vector<QuadNode*> nodes;

    // Initializing Quadnodes
    // root
    QuadNode root = QuadNode{HALF_WIDTH, ROOT_MIN, ROOT_MAX, nullptr};
    // branching recursively from root
    QuadDivision(&root, 0);

    SetTargetFPS(FPS);

    float accumulator = 0;
    bool paused = false;
    bool gridEnabled = true;
    int clickCount = 0;
    vector<Ball*> candidates;
    
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        Vector2 forces = Vector2Zero(); // every frame set the forces to a 0 vector

        if (IsKeyPressed(KEY_SPACE)){
            clickCount++;
            if(clickCount <= 10){
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
        }
        
        if (paused){
            delta_time = 0;
        }else{
            delta_time = GetFrameTime();
        }
        // TO-DO: INSERTIONS
        // Recursively populate the children of the nodes which node contains what object

        // Physics step
        accumulator += delta_time;
        // if (accumulator > 2 * TIMESTEP) accumulator = 2 * TIMESTEP; 
        while(accumulator >= TIMESTEP) {
            // ------ SEMI-IMPLICIT EULER INTEGRATION -------
            // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt)
            // Movign the balls first
            for (Ball& ball : balls) {
                ball.velocity = Vector2Add(ball.velocity, Vector2Scale(ball.acceleration, TIMESTEP));
                ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, TIMESTEP));

                // EDGE CHECK
                // Negates the velocity at x and y if the object hits a wall. (Basic Collision Detection)
                if(ball.position.x + ball.radius >= WINDOW_WIDTH || ball.position.x - ball.radius <= 0) {
                    ball.velocity.x *= -1;
                }
                if(ball.position.y + ball.radius >= WINDOW_HEIGHT || ball.position.y - ball.radius <= 0) {
                    ball.velocity.y *= -1;
                }

            }

            //inserting the balls into the quadtree
            InsertAllBalls(&root, balls);
            
            
            //Dito lalagay ung collission ng balls
            for (Ball& targetBall : balls) {
                candidates.clear(); //reuse the memory rather than new allocation

                //recursive query outside
                QueryCandidates(&root, targetBall, candidates);
                // now do collision with candidates
                for (Ball* ball : candidates) {
                    if(ball <= &targetBall) continue;
                    CircleToCircleCollision(targetBall, *ball);
                }
            }
            accumulator -= TIMESTEP;
            
        }



        BeginDrawing();
        ClearBackground(WHITE);
        
        for (const Ball& ball : balls) {
            DrawCircleV(ball.position, ball.radius, ball.color);
        }
        
        DrawText(TextFormat("Number of Balls: %d", ballCount), 10, 10, 34, BLACK);
        DrawText("Press V to Enable/Disable Grid.", 10, 44, 34, BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}