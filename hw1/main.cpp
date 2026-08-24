#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

void drawGrid(int width, int height, int playerPosX, int playerPosY, int enemyPosX, int enemyPosY) {
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

int main() {

    // get input from fi
    ifstream file("settings.txt");
    
    int gridWidth, gridHeight;
    int playerPosX, playerPosY;
    int enemyPosX, enemyPosY;

    file >> gridWidth >> gridHeight;
    file.ignore(256, '\n');
    file >> playerPosX >> playerPosY;
    file.ignore(256, '\n');
    file >> enemyPosX >> enemyPosY;

    // draw grid with initial settings first
    cout << "Text-Based Game\n";
    cout << "LOADING GRID...\n";
    drawGrid(gridWidth, gridHeight, playerPosX, playerPosY, enemyPosX, enemyPosY);

    // start game loop
    while(true) {
        // reading inputs
        string lineInput;
        getline(cin, lineInput);
        string input;
        stringstream ss(lineInput);
    
        // parse input line per string
        while (getline(ss, input, ' ')) {            
            if (input == "w" || input == "west" || input == "e" || input == "east") {
                // horizontal movement
                playerPosX = warpMovement(input, playerPosX, gridWidth);
            }
            else if (input == "s" || input == "south" || input == "n" || input == "north") {
                // vertical movement
                playerPosY = warpMovement(input, playerPosY, gridHeight);
            }       
            else if (input == "a" || input == "attack") {
                // attack input
                if ((playerPosX == enemyPosX) && (playerPosY == enemyPosY)) {
                    cout << "Enemy has been slayed!\n";
                    exit(1);
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

        }

        drawGrid(gridWidth, gridHeight, playerPosX, playerPosY, enemyPosX, enemyPosY);
    }
    
    // checking if we got the input settings right
    // cout << gridWidth << " " << gridHeight << endl;
    // cout << playerPosX << " " << playerPosY << endl;
    // cout << enemyPosX << " " << enemyPosY << endl; 
    // i just verified, this works na !!
    
    return 0; // i forgot to add this when testing lmao
}