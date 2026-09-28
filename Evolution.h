// ============================================================
// Evolution.h — Evolutionary Training Controller
//
// Manages the entire neuroevolution training pipeline:
//
//   1. Create a population of neural networks.
//   2. Run each network in its own Game (racing environment).
//   3. Collect fitness scores when simulations end.
//   4. Select the best-performing networks.
//   5. Preserve top parents unchanged (elitism).
//   6. Generate children by mutating copies of parents.
//   7. Repeat on the same map until a generation produces
//      enough finishers, then advance to the next map.
//   8. After all maps are completed, save the final brains.
//
// The multi-map system progressively exposes networks to
// harder track geometries. A network that succeeds on an
// easier map is carried forward and refined on the next.
// ============================================================

#pragma once

#include "NeuralNetwork.h"
#include "Game.h"

#include <vector>


class Evolution
{
private:

    // ============================================================
    // TRAINING CONFIGURATION
    //
    // Change these values to control the evolutionary experiment.
    // ============================================================

    // Number of neural networks (and cars) evaluated simultaneously
    // per generation. Each network gets its own Game instance.
    // Larger populations explore more of the solution space per
    // generation but take longer to simulate.
    static constexpr int populationSize = 100;

    // Number of top networks preserved as parents when a generation
    // achieves enough finishers to advance to the next map.
    // Also serves as the finisher threshold: a generation must
    // produce at least this many finishers (within that single
    // generation, not accumulated across generations) to trigger
    // a map transition.
    static constexpr int parentCount = 5;

    // Alternative elite count for selection. Currently parentCount
    // is used in the active selection/mutation paths.
    static constexpr int eliteCount = 10;

    // Current generation's neural networks.
    std::vector<NeuralNetwork> population;

    // Fitness score for each network in the current generation.
    std::vector<float> fitness;

    // One Game (racing simulation) per network in the population.
    std::vector<Game> games;

    // Tracks whether each network's fitness has been recorded
    // this generation (prevents double-counting).
    std::vector<bool> evaluated;


    // ==================================================
    // SUCCESSFUL PARENTS
    //
    // When a generation produces >= parentCount finishers,
    // the top parentCount networks are stored here. These
    // are the brains carried to the next map — they proved
    // themselves on the current track geometry.
    // ==================================================

    std::vector<NeuralNetwork> successfulParents;

    // Number of cars that completed the track in the current
    // generation. Compared against parentCount to decide
    // whether to advance to the next map.
    int finishedCars;


    // ==================================================
    // MAPS
    //
    // Map count and point count are mirrored here for
    // convenience; the authoritative map data lives in
    // MapConfig (Evolution.cpp). To add/remove maps,
    // edit MapConfig::mapCount, mutationRanges[], and maps[].
    // ==================================================

    static constexpr int mapCount = 3;

    static constexpr int pointCount = 18;

    // Index of the current training map (0-based).
    int currentMap;


    // ==================================================
    // GENERATION
    // ==================================================

    // Current generation number within the current map.
    // Resets to 0 when advancing to a new map.
    int generation;

    // Set to true once the final map is completed and
    // brains have been saved to final_brains.bin.
    bool trainingComplete;


    // ==================================================
    // DRAW
    // ==================================================

    Camera2D populationCamera;


    // ==================================================
    // INTERNAL
    // ==================================================

    // Evolve the population on the SAME map when a generation
    // ends without enough finishers: select best, preserve
    // parents, generate mutated children, reset games.
    void startNewGeneration();

    // Advance currentMap and reset the generation counter.
    void moveToNextMap();

    // Build a new population of 100 from the 5 successful
    // parents: 5 parents preserved + 95 mutated children.
    // Used specifically for map transitions.
    void createChildrenFromFive();

    // Serialize the successfulParents to final_brains.bin.
    void saveFinalBrains();


public:

    Evolution();


    // ==================================================
    // EVOLUTION
    // ==================================================

    // Main per-frame entry point. Runs all active cars,
    // collects fitness, and triggers generation transitions
    // or map advancement when appropriate.
    void update();

    // Alias for update() — runs one frame of evaluation.
    void evaluate();

    // Return indices of the top parentCount networks
    // sorted by fitness (descending).
    std::vector<int> selectBest();

    // Alias for startNewGeneration().
    void createNextGeneration();

    // Alias for startNewGeneration().
    void nextGeneration();


    // ==================================================
    // DRAW
    // ==================================================

    void draw();


    // ==================================================
    // STATUS
    // ==================================================

    bool isTrainingComplete();


    // ==================================================
    // ACCESS
    // ==================================================

    NeuralNetwork& getBestNetwork();

    float getBestFitness();

    int getGeneration();

    int getCurrentMap();

    int getFinishedCars();
};