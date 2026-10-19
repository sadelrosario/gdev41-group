#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
// Struct to encapsulate our UI library
struct UiLibrary
{
    // ID of the currently hot widget
    int hot = -1;
    // ID of the currently active widget
    int active = -1;
    // Creates a button with the specified text and bounds
    // Returns true if this button was clicked in this frame
    bool Button(int id, const std::string& text, const Rectangle& bounds)
    {
    bool result = false;
    // If this button is the currently active widget, that means
    // the user is currently interacting with this widget
    if (id == active)
    {
        // If the user released the mouse button while we are active,
        // register as a click
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
        // Make sure that we are still the hot widget
        // (User might have dragged mouse to another widget, in which case
        // we will not count as a button click)
            if (id == hot)
            {
                result = true;
            }
            // Set ourselves to not be active anymore
            active = -1;
        }
    }
    // If we are currently the hot widget
    if (id == hot)
    {
    // If the user pressed the left mouse button, that means the user
    // started
    // interacting with this widget, so we set this widget as active
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
    active = id;
    }
    }
    // If the mouse cursor is hovering within our boundaries
    if (CheckCollisionPointRec(GetMousePosition(), bounds))
    {
    // Set this widget to be the hot widget
    hot = id;
    }
    // If the mouse cursor is not on top of this widget, and this widget
    // was previously the hot widget, set the hot widget to -1
    // We check if this widget was the previously hot widget since there is a
    //possibility that
    // the hot widget is now a different widget, and we don't want to overwrite
    //that with -1
    else if (hot == id)
    {
    hot = -1;
    }
    if (id == active) {
        // button click
        DrawRectangleRec(bounds, VIOLET);
        DrawText(text.c_str(), bounds.x, bounds.y, 14, WHITE);
    } else if (id == hot) {
        // button hover
        DrawRectangleRec(bounds, GREEN);
        DrawText(text.c_str(), bounds.x, bounds.y, 14, BLACK);
    } else {
        // Draw our button regardless of what happens
        DrawRectangleRec(bounds, GRAY);
        DrawText(text.c_str(), bounds.x, bounds.y, 14, BLACK);
    }
    return result;
    }

    bool Checkbox (int id, const std::string& text, const Rectangle& bounds, bool* value)
    {
        
        bool result = false;

        //if the checkbox is active
        if(id == active){
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
            // Make sure that we are still the hot widget
            // (User might have dragged mouse to another widget, in which case
            // we will not count as a button click)
                if (id == hot)
                {
                    result = true;
                    *value = !(*value);
                }
                // Set ourselves to not be active anymore
                active = -1;
            }
        }

        if(id == hot){
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                active = id;
            }
        }

        if(CheckCollisionPointRec(GetMousePosition(), bounds)){
            hot = id;
        }
        else if (hot == id)
        {
            hot = -1;
        }

        DrawRectangleLines(bounds.x, bounds.y, 20, 20, BLACK);
        if(*value){
            DrawRectangle(bounds.x+3, bounds.y+3, 14, 14, GRAY);
        }

        DrawText(text.c_str(), bounds.x + 40, bounds.y, 14, BLACK);
        return result;
    };

};
int main()
{
    int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Immediate Mode");
    SetTargetFPS(60);
    UiLibrary uiLibrary;
    bool lockScreen = false;

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);
        BeginDrawing();
        if (uiLibrary.Button(0, "800x600", { 10, 10, 80, 40 }))
        {
            if(!lockScreen){
                SetWindowSize(800,600);
            }
        }
        
        if (uiLibrary.Button(1, "1280x720", { 100, 10, 80, 40 }))
        {
            if(!lockScreen){
                SetWindowSize(1280, 720);
            }
        }

        if (uiLibrary.Button(2, "1366x768", { 190, 10, 80, 40 }))
        {
            if(!lockScreen){
                SetWindowSize(1366, 768);
            }
        }

        if (uiLibrary.Checkbox(3, "Lock Screen", { 10, 50, 100, 40 }, &lockScreen));
        EndDrawing();
    }
    CloseWindow();
    return 0;
}