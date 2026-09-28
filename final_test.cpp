// ==================================================
// Final AI Test — Post-Training Evaluation
//
// Post-training evaluation program. Loads 5 trained
// neural networks from final_brains.bin and runs them
// on a test map without any evolution or mutation.
//
// Purpose: observe learned driving behavior with
// fixed weights.
//
// To change the test map, edit the testMap[18]
// coordinates below.
//
// This program does NOT perform training — brains run
// with the weights exactly as saved.
// ==================================================

#include "raylib.h"

#include "Game.h"
#include "NeuralNetwork.h"

#include <fstream>
#include <iostream>
#include <vector>


// ==================================================
// CONFIGURATION
// ==================================================

static const int screenWidth = 1200;
static const int screenHeight = 800;

static const int brainCount = 5;


// ==================================================
// TEST MAP
//
// CHANGE ONLY THESE 18 POINTS.
// ==================================================

// These are track centerline coordinates. The road extends ±(trackWidth/2)
// from this centerline. Edit these to test the trained brains on any track geometry.
Vector2 testMap[18] =
{
    {600, 80},
    {850, 80},
    {1080, 80},
    {1200, 160},
    {1180, 320},
    {1180, 480},
    {1050, 580},
    {800, 580},
    {550, 580},
    {350, 560},
    {300, 460},
    {500, 420},
    {750, 400},
    {750, 240},
    {500, 220},
    {300, 180},
    {350, 80},
    {450, 80}
};


// ==================================================
// MAIN
// ==================================================

int main()
{
    InitWindow(
        screenWidth,
        screenHeight,
        "Final AI Test"
    );

    SetTargetFPS(60);


    // ==================================================
    // LOAD BRAINS
    // ==================================================

    std::ifstream file(
        "final_brains.bin",
        std::ios::binary
    );


    if (!file)
    {
        std::cerr
            << "ERROR: Could not open final_brains.bin\n";

        CloseWindow();

        return 1;
    }


    int count = 0;


    file.read(
        reinterpret_cast<char*>(&count),
        sizeof(count)
    );


    if (!file || count != brainCount)
    {
        std::cerr
            << "ERROR: Expected "
            << brainCount
            << " brains, but file contains "
            << count
            << ".\n";

        CloseWindow();

        return 1;
    }


    // ==================================================
    // LOAD THE FIVE BRAINS
    // ==================================================

    std::vector<NeuralNetwork> brains;

    brains.reserve(brainCount);


    for (int i = 0;
         i < brainCount;
         i++)
    {
        NeuralNetwork brain;


        if (!brain.load(file))
        {
            std::cerr
                << "ERROR: Failed to load NN "
                << i + 1
                << ".\n";

            CloseWindow();

            return 1;
        }


        brains.push_back(
            brain
        );
    }


    file.close();


    std::cout
        << "Loaded 5 trained brains.\n";


    // ==================================================
    // CREATE FIVE GAMES
    // ==================================================

    std::vector<Game> games;

    games.reserve(brainCount);


    for (int i = 0;
         i < brainCount;
         i++)
    {
        games.emplace_back(
            screenWidth,
            screenHeight
        );


        games[i].setMap(
            testMap
        );
    }


    // ==================================================
    // FIRST FINISHER
    // ==================================================

    // Tracks which brain finishes first. Set to -1 until a brain completes the lap.
    int firstFinisher = -1;


    // ==================================================
    // MAIN LOOP
    // ==================================================

    while (!WindowShouldClose())
    {
        // ==================================================
        // UPDATE ALL FIVE
        // ==================================================

        for (int i = 0;
             i < brainCount;
             i++)
        {
            if (games[i].isSimulationOver())
            {
                continue;
            }


            // --------------------------------------------------
            // GET SENSORS
            // --------------------------------------------------

            std::vector<float> sensors =
                games[i].getSensors();


            // --------------------------------------------------
            // NN PREDICTION
            // --------------------------------------------------

            std::vector<float> output =
                brains[i].predict(
                    sensors
                );


            // --------------------------------------------------
            // CHOOSE ACTION
            // --------------------------------------------------

            int action = 0;


            for (int j = 1;
                 j < 7;
                 j++)
            {
                if (output[j] >
                    output[action])
                {
                    action = j;
                }
            }


            // --------------------------------------------------
            // APPLY ACTION
            // --------------------------------------------------

            games[i].applyAction(
                action
            );


            games[i].update();


            // --------------------------------------------------
            // FIRST FINISHER
            // --------------------------------------------------

            if (games[i].isFinished() &&
                firstFinisher == -1)
            {
                firstFinisher = i;


                std::cout
                    << "FIRST FINISHER: NN "
                    << i + 1
                    << std::endl;
            }
        }


        // ==================================================
        // DRAW
        // ==================================================

        BeginDrawing();

        ClearBackground(GREEN);


        // ==================================================
        // CAMERA
        // ==================================================

        Camera2D camera = {0};

        camera.target =
        {
            600.0f,
            410.0f
        };

        camera.offset =
        {
            600.0f,
            400.0f
        };

        camera.zoom = 1.0f;


        // ==================================================
        // TRACK
        // ==================================================

        games[0].drawWorld(
            camera,
            true,
            false
        );


        // ==================================================
        // FIVE CARS
        // ==================================================

        for (int i = 0;
             i < brainCount;
             i++)
        {
            games[i].drawWorld(
                camera,
                false,
                false
            );


            // --------------------------------------------------
            // NN NUMBER
            // --------------------------------------------------

            Vector2 position =
                games[i].getCarPosition();


            DrawText(
                TextFormat(
                    "NN %d",
                    i + 1
                ),
                (int)position.x + 12,
                (int)position.y - 10,
                18,
                WHITE
            );
        }


        // ==================================================
        // FIRST FINISHER
        // ==================================================

        if (firstFinisher != -1)
        {
            DrawText(
                TextFormat(
                    "FIRST FINISHER: NN %d",
                    firstFinisher + 1
                ),
                20,
                20,
                28,
                WHITE
            );
        }
        else
        {
            DrawText(
                "FIRST FINISHER: --",
                20,
                20,
                28,
                WHITE
            );
        }


        EndDrawing();
    }


    CloseWindow();

    return 0;
}