// ==================================================
// AI Racing Evolution — Neuroevolution Training
//
// Training entry point for autonomous racing cars.
// The Evolution object manages the entire pipeline:
// population creation, simulation, fitness evaluation,
// selection, mutation, and multi-map progression.
//
// The main loop calls evolution.update() (simulates
// all cars and evolves when a generation ends) and
// evolution.draw() (visualizes the current generation)
// at 60 FPS.
//
// When training completes (all maps cleared), the 5
// best neural networks are saved to final_brains.bin.
// To test the trained brains, run final_test instead.
// ==================================================

#include "raylib.h"
#include "Evolution.h"


int main()
{
    const int screenWidth = 1200;
    const int screenHeight = 800;


    InitWindow(
        screenWidth,
        screenHeight,
        "AI Racing Evolution"
    );

    SetTargetFPS(60);


    Evolution evolution;


    while (!WindowShouldClose())
    {
        evolution.update();

        evolution.draw();
    }


    CloseWindow();

    return 0;
}