#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <vector>
#include <cstdlib>
using namespace std;

struct Position {
    int x;
    int y;
};

void drawGrid(int width, int height, int playerPosX, int playerPosY, int enemyPosX, int enemyPosY, vector<Position> clearedPositions) {
    //clear the screen
    // error checker
    if (playerPosX > width || playerPosY > height || enemyPosX > width || enemyPosY > height) {
        cout << "INVALID SETTINGS\n";
        exit(1);
    }
    
    string grid = ""; 

    // add the topmost horizontal line of the grid first
    for (int x = 0; x <= (width * 3); x++) {
        grid += "-";
    }
    grid += "\n";    
    // fill in the rest of the grid
    for (int y = 0; y < height; y++) { // column height
        for (int x = 0; x < width; x++) { // 
            // check units' position
            bool posCleared = false;
            for (const Position& pos : clearedPositions) {
                if (pos.x == x && pos.y == y) {
                    posCleared = true;
                    break;
                }
            }

            if (!posCleared) {
                grid += "|\u2588\u2588";
            } 
            else{
                if ((playerPosX == x) && (playerPosY == y) && (enemyPosX == x) && (enemyPosY == y)) {
                    grid += "|PE";
                } 
                else if ((playerPosX == x) && (playerPosY == y)) {
                    grid += "|P ";
                } 
                else if ((enemyPosX == x) && (enemyPosY == y)) {
                    grid += "| E";
                }
                else {
                    grid += "|  ";
                }
            }
        }
        grid += "|\n";
        
        // draw the row divider line
        for (int x = 0; x <= (width * 3); x++) {
            grid += "-";
        }
        grid += "\n";
    }

    // print everything
    cout << grid << endl;
}

void drawBattleUI(int playerHP, int playerDmg,string playerAction, int enemyHP, int enemyDmg, string enemyAction){
    string battleUI = "";
    for (int y = 0; y < 7; y++) {

        if (y == 0 || y == 2 || y == 4 || y == 6) {
            battleUI += "--------------------\n";
        }
        else if (y == 1) {
            if (playerHP <= 0) {
                battleUI += "|   X            E   |\n";
            } else if (enemyHP <= 0) {
                battleUI += "|   P            X   |\n";
            } else {
                if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "a")) {
                    battleUI += "|   P  >      <  E   |\n";
                } else if ((playerAction == "d" || playerAction == "defend") && (enemyAction == "a")) {
                    battleUI += "|   P  |      <  E   |\n";
                } else if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "d")) {
                    battleUI += "|   P  >      |  E   |\n";
                } else if ((playerAction == "d" || playerAction == "defend") && (enemyAction == "d")) {
                    battleUI += "|   P  |      |  E   |\n";
                }else{
                    battleUI += "|   P            E   |\n";
                }
            }
        }
        else if (y == 3) {
            battleUI += "| HP: " + to_string(playerHP) + " ---- HP: " + to_string(enemyHP) + " |\n";
        }
        else if (y == 5) {
            battleUI += "|- ATK(a) - DEF(d) -|\n";
        }
    }
    cout << battleUI;
    if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "a")) {
        cout << "Both attack each other dealing " << playerDmg << " DMG and " << enemyDmg << " DMG respectively.\n";
    } else if ((playerAction == "d" || playerAction == "defend") && (enemyAction == "a")) {
        cout << "Player got hurt defending Enemy's attack!\n";
    } else if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "d")) {
        cout << "Enemy got hurt defending Player's attack!\n";
    } else if ((playerAction == "d" || playerAction == "defend") && (enemyAction == "d")) {
        cout << "Both defends as nothing happens!\n";
    }
}


int warpMovement(string direction, int position, int gridSize) { 
    // handles positive direction
    if (direction == "e" || direction == "east" || direction == "s" || direction == "south") {
        position = (position + 1) % gridSize;
    } 
    // handles negative direction
    else if (direction == "w" || direction == "west" || direction == "n" || direction == "north") {
        position = ((position - 1) % gridSize);
        if (position < 0) {
            position = gridSize - 1;
        }
    }
    return position;
}

void damageProcess(string playerAction, int playerDmg, int& playerHP, string enemyAction, int enemyDmg, int& enemyHP) {
    if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "a")) {
        enemyHP -= playerDmg;
        playerHP -= enemyDmg;
    }
    else if ((playerAction == "a" || playerAction == "attack") && (enemyAction == "d")) {
        enemyHP -= rand() % 5 + 2;
    }
    else if ((playerAction == "d" || playerAction == "defend") && (enemyAction == "a")) {
        playerHP -= rand() % 5 + 2;
    }
}

void toLower(string& str){
    for (char& c : str) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    // get input from fi
    ifstream file("settings.txt");
    
    int gridWidth, gridHeight;
    
    int playerPosX, playerPosY;
    int playerHealth = 100;
    int playerDmg = 10;
    
    int enemyPosX, enemyPosY;
    int enemyHealth = 100;
    int enemyDmg = 20;

    int movementBuffer = 0;
    bool engageBattle = false;

    vector<Position> clearedPositions;
    string inputLines[6] = {"w", "e", "s", "n", "a","d"};

    file >> gridWidth >> gridHeight;
    file.ignore(256, '\n');
    file >> playerPosX >> playerPosY;
    file.ignore(256, '\n');
    file >> enemyPosX >> enemyPosY;

    // draw grid with initial settings first
    cout << "Text-Based Game\n";
    cout << "LOADING GRID...\n";
    clearedPositions.push_back({playerPosX, playerPosY});
    drawGrid(gridWidth, gridHeight, playerPosX, playerPosY, enemyPosX, enemyPosY, clearedPositions);

    // start game loop
    while(true) {
        // reading inputs
        string lineInput;
        getline(cin, lineInput);
        string input;
        string enemyAction;
        stringstream ss(lineInput);
    
        // parse input line per string
        while (getline(ss, input, ' ')) {
            toLower(input);
            if (!engageBattle) {
                if (input == "w" || input == "west" || input == "e" || input == "east") {
                // horizontal movement
                playerPosX = warpMovement(input, playerPosX, gridWidth);
                clearedPositions.push_back({playerPosX, playerPosY});
                if (movementBuffer > 1) {
                    int rnd = rand() % 2;
                    enemyPosX = warpMovement(inputLines[rnd], enemyPosX, gridWidth); 
                    movementBuffer = 0;
                }
                
                }
                else if (input == "s" || input == "south" || input == "n" || input == "north") {
                    // vertical movement
                    playerPosY = warpMovement(input, playerPosY, gridHeight);
                    clearedPositions.push_back({playerPosX, playerPosY});
                    if (movementBuffer > 1) {
                        int rnd = rand() % 2 + 2;
                        enemyPosY = warpMovement(inputLines[rnd], enemyPosY, gridHeight); 
                        movementBuffer = 0;
                    }
                }       
                else if (input == "a" || input == "attack") {
                    // attack input
                    if ((playerPosX == enemyPosX) && (playerPosY == enemyPosY)) {
                        cout << "Engaged in Battle!\n";
                        engageBattle = true;
                        // exit(1);
                    } 
                    else {
                        cout << "There are no enemies to attack.\n";
                    }
                }
                else if (input == "exit") {
                    cout << "Player has left the game. Sayonara!\n";
                    exit(1);
                } 
                else {
                    cout << "INVALID INPUT\n";
                    break;
                }
                movementBuffer++;
            }else{
                if(input == "a" || input == "attack") {
                    enemyAction = (rand() % 2 == 0) ? "a" : "d";
                    damageProcess(input, playerDmg, playerHealth, enemyAction, enemyDmg, enemyHealth);
                } 
                else if (input == "d" || input == "defend") {
                    enemyAction = (rand() % 2 == 0) ? "a" : "d";
                    damageProcess(input, playerDmg, playerHealth, enemyAction, enemyDmg, enemyHealth);
                }else if (input == "exit") {
                    cout << "Player has left the battle. Sayonara!\n";
                    exit(1);
                } else {
                    cout << "INVALID INPUT\n";
                    break;
                }
            }
            
        }


        if (!engageBattle){
            drawGrid(gridWidth, gridHeight, playerPosX, playerPosY, enemyPosX, enemyPosY, clearedPositions);
        } else{
            if (playerHealth <= 0) {
                drawBattleUI(playerHealth, playerDmg,input, 0, enemyDmg, enemyAction);
                cout << "Player has been defeated! Game Over.\n";
                exit(1);
            } else if (enemyHealth <= 0) {
                drawBattleUI(playerHealth, playerDmg,input, 0, enemyDmg, enemyAction);
                cout << "Enemy has been defeated! You win!\n";
                exit(1);
            }
            drawBattleUI(playerHealth, playerDmg,input, enemyHealth, enemyDmg, enemyAction);
        }
    }
    
    // checking if we got the input settings right
    // cout << gridWidth << " " << gridHeight << endl;
    // cout << playerPosX << " " << playerPosY << endl;
    // cout << enemyPosX << " " << enemyPosY << endl; 
    // i just verified, this works na !!
    
    return 0; // i forgot to add this when testing lmao
}