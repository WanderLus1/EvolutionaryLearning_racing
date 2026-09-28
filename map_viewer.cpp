// ==================================================
// Map Viewer — Standalone Track Preview Utility
//
// Standalone track preview utility. Paste track
// centerline coordinates into trackPoints[] and run
// this program to visually inspect the track before
// using it for training or testing.
//
// Each point is numbered on-screen. The start/finish
// is marked with a white circle at point 0.
//
// trackWidth controls the rendered road width (should
// match the value used in Game).
// ==================================================

#include "raylib.h"

#include <cmath>


// ==================================================
// CONFIGURATION
// ==================================================

static const int screenWidth = 1200;
static const int screenHeight = 800;

static const int pointCount = 18;

static const float trackWidth = 80.0f;


// ==================================================
// PUT YOUR MAP HERE
// ==================================================

Vector2 trackPoints[pointCount] =
{
           
    
        {100, 80},
    {100, 180},
    {250, 280},
    {500, 280},
    {350, 450},
    {100, 420},
    {180, 580},
    {600, 520},
    {580, 380},
    {750, 450},
    {680, 580},
    {1150, 580},
    {950, 400},
    {1100, 320},
    {900, 240},
    {1000, 100},
    {750, 160},
    {550, 70}
};


// ==================================================
// MAIN
// ==================================================

int main()
{
    InitWindow(
        screenWidth,
        screenHeight,
        "Track Viewer"
    );

    SetTargetFPS(60);


    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(GREEN);


        // ==================================================
        // TRACK
        // ==================================================

        for (int i = 0;
             i < pointCount;
             i++)
        {
            int next =
                (i + 1) % pointCount;


            Vector2 start =
                trackPoints[i];

            Vector2 end =
                trackPoints[next];


            float dx =
                end.x - start.x;

            float dy =
                end.y - start.y;


            float length =
                sqrtf(
                    dx * dx +
                    dy * dy
                );


            float angle =
                atan2f(
                    dy,
                    dx
                ) * RAD2DEG;


            Vector2 midpoint =
            {
                (start.x + end.x) / 2.0f,
                (start.y + end.y) / 2.0f
            };


            Rectangle road =
            {
                midpoint.x,
                midpoint.y,
                length,
                trackWidth
            };


            DrawRectanglePro(
                road,

                {
                    road.width / 2.0f,
                    road.height / 2.0f
                },

                angle,

                DARKGRAY
            );


            DrawCircleV(
                start,
                trackWidth / 2.0f,
                DARKGRAY
            );
        }


        // ==================================================
        // TRACK POINTS
        // ==================================================

        for (int i = 0;
             i < pointCount;
             i++)
        {
            DrawCircleV(
                trackPoints[i],
                6.0f,
                YELLOW
            );


            DrawText(
                TextFormat(
                    "%d",
                    i
                ),
                trackPoints[i].x + 8,
                trackPoints[i].y - 8,
                18,
                BLACK
            );
        }


        // ==================================================
        // START / FINISH
        // ==================================================

        DrawCircleV(
            trackPoints[0],
            20.0f,
            WHITE
        );

        DrawCircleV(
            trackPoints[0],
            12.0f,
            DARKGRAY
        );


        DrawText(
            "START / FINISH",
            trackPoints[0].x + 25,
            trackPoints[0].y - 10,
            20,
            WHITE
        );


        // ==================================================
        // TITLE
        // ==================================================

        DrawText(
            "MAP VIEWER",
            20,
            20,
            30,
            WHITE
        );


        EndDrawing();
    }


    CloseWindow();

    return 0;
}