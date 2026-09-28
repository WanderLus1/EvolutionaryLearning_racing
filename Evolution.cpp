#include "Evolution.h"

#include <algorithm>
#include <fstream>
#include <iostream>


// ============================================================
// MAP CONFIGURATION
//
// To edit maps, modify the arrays below. All three structures
// (mapCount, mutationRanges, maps) must be kept in sync.
//
// Each map is defined by 18 centerline points (Vector2).
// These are NOT outer boundaries — the road extends
// ±(trackWidth/2) from this centerline. trackWidth is set
// in Game's constructor (currently 80.0f).
//
// mutationRanges[i] controls how strongly children are
// mutated when generating the next population after map i
// is completed. A larger value means more exploration
// (weights change more), which is useful on early/easy maps.
// A smaller value means finer refinement, preserving the
// behavior learned on previous maps. Too much mutation
// can destroy successful behavior; too little can cause
// evolutionary stagnation.
//
// To add a new map:
//   1. Increase mapCount.
//   2. Add a mutation range to mutationRanges[].
//   3. Add 18 centerline points to maps[].
// ============================================================

namespace MapConfig
{
    constexpr int mapCount = 7;
    constexpr int pointCount = 18;


    // ==================================================
    // MUTATION RANGES
    //
    // mutationRanges[i] is used when creating children
    // after map i is successfully completed. Each weight
    // and bias in a child network is perturbed by a
    // uniform random value in [-range, +range].
    //
    // Early maps use larger ranges (broad exploration).
    // Later maps use smaller ranges (fine-tuning).
    // ==================================================

    float mutationRanges[mapCount] =
    {
        0.8f,   // Map 1
        0.3f,   // Map 2
        0.3f,   // Map 3
        0.15f,
        0.15f,
        0.1f,
        0.1f
    

    };


    // ==================================================
    // MAPS
    // ==================================================

    Vector2 maps[mapCount][pointCount] =
    {
        {
            // ==================================================
            // MAP 1
            // ==================================================

            {100, 100},
            {350, 100},
            {500, 60},
            {650, 100},
            {800, 140},
            {950, 100},
            {1120, 150},
            {1200, 340},
            {1120, 530},
            {950, 580},
            {800, 540},
            {650, 580},
            {500, 620},
            {350, 580},
            {180, 530},
            {100, 340},
            {100, 180},
            {100, 120}
        },


        {
            // ==================================================
            // MAP 2
            // ==================================================

            {650, 580},
            {850, 580},
            {1080, 580},
            {1180, 500},
            {1180, 420},
            {950, 420},
            {750, 400},
            {750, 220},
            {730, 80},
            {650, 60},
            {570, 80},
            {550, 220},
            {550, 400},
            {350, 420},
            {120, 420},
            {120, 500},
            {220, 580},
            {450, 580}
        },
        
            {
  {650, 80},
    {850, 80},
    {1080, 80},
    {1180, 160},
    {1180, 240},
    {950, 240},
    {750, 260},
    {750, 440},
    {730, 580},
    {650, 600},
    {570, 580},
    {550, 440},
    {550, 260},
    {350, 240},
    {120, 240},
    {120, 160},
    {220, 80},
    {450, 80}

        },

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
},


        {
            // ==================================================
            // MAP 3
            // ==================================================

            {600, 580},
            {850, 580},
            {1080, 580},
            {1200, 500},
            {1180, 340},
            {1180, 180},
            {1050, 80},
            {800, 80},
            {550, 80},
            {350, 100},
            {300, 200},
            {500, 240},
            {750, 260},
            {750, 420},
            {500, 440},
            {300, 480},
            {350, 580},
            {450, 580}
        },
        
            // Technical C-Hook Circuit (Vertically Inverted / Mirrored across Y = 330)

        

                {
                     
            // ==================================================
            // MAP 4
            // ==================================================

{100, 80},
    {500, 80},
    {650, 180},
    {800, 80},
    {1150, 80},
    {1200, 200},
    {950, 260},
    {1180, 340},
    {1180, 520},
    {950, 580},
    {800, 420},
    {650, 580},
    {500, 420},
    {350, 580},
    {120, 520},
    {200, 340},
    {80, 220},
    {100, 120}
        },


        {
            // ==================================================
            // MAP 5
            // ==================================================

{150, 80}  ,
{150, 520} , 
{215, 580} , 
{280, 520} , 
{315, 200} , 
{365, 160} , 
{415, 200} , 
{450, 520} , 
{565, 580} , 
{680, 520} , 
{715, 200} , 
{765, 160} , 
{815, 200} , 
{850, 520} , 
{925, 580} , 
{1000, 520}, 
{1050, 200},
{1000, 80}
        },





    };
}


// ==================================================
// CONSTRUCTOR
// ==================================================

Evolution::Evolution()
{
    generation = 0;

    currentMap = 0;

    finishedCars = 0;

    trainingComplete = false;


    // ==================================================
    // POPULATION
    // ==================================================

    population.reserve(
        populationSize
    );

    fitness.assign(
        populationSize,
        0.0f
    );

    evaluated.assign(
        populationSize,
        false
    );


    for (int i = 0;
         i < populationSize;
         i++)
    {
        population.emplace_back();
    }


    // ==================================================
    // GAMES
    // ==================================================

    games.reserve(
        populationSize
    );


    for (int i = 0;
         i < populationSize;
         i++)
    {
        games.emplace_back(
            1200,
            800
        );

        games[i].setMap(
            MapConfig::maps[currentMap]
        );
    }


    // ==================================================
    // CAMERA
    // ==================================================

    populationCamera = {0};

    populationCamera.target =
    {
        640.0f,
        410.0f
    };

    populationCamera.offset =
    {
        600.0f,
        400.0f
    };

    populationCamera.zoom = 1.0f;
}


// ==================================================
// UPDATE — Main per-frame evolution loop
//
// Each frame, this function:
//   1. Runs every active car: sensors → NN predict →
//      argmax action → game update.
//   2. Records fitness when a car's simulation ends.
//   3. Counts finishers in the current generation.
//   4. If >= parentCount finishers in THIS generation:
//      select top 5, save as parents, advance to next
//      map (or save final brains if on last map).
//   5. If all cars finished/crashed but < parentCount
//      finishers: evolve on the SAME map.
// ==================================================

void Evolution::update()
{
    if (trainingComplete)
    {
        return;
    }


    bool allOver = true;


    // ==================================================
    // UPDATE ALL 100 CARS
    //
    // For each car that is still running:
    //   - Read its 6 sensor values from the Game.
    //   - Feed them through the neural network.
    //   - Select the action with the highest output
    //     (argmax over 7 outputs).
    //   - Apply the action and advance the simulation.
    // ==================================================

    for (int i = 0;
         i < populationSize;
         i++)
    {
        // --------------------------------------------------
        // CAR STILL RUNNING
        // --------------------------------------------------

        if (!games[i].isSimulationOver())
        {
            allOver = false;


            std::vector<float> sensors =
                games[i].getSensors();


            std::vector<float> output =
                population[i].predict(
                    sensors
                );


            // --------------------------------------------------
            // CHOOSE HIGHEST OUTPUT (argmax)
            //
            // The NN produces 7 activation values, one per
            // driving action. The action with the highest
            // activation is selected. See Game::applyAction()
            // for the action-to-behavior mapping.
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


            games[i].applyAction(
                action
            );

            games[i].update();
        }


        // --------------------------------------------------
        // STORE FITNESS WHEN CAR ENDS
        // --------------------------------------------------

        if (games[i].isSimulationOver() &&
            !evaluated[i])
        {
            evaluated[i] = true;

            fitness[i] =
                games[i].getFitness();
        }
    }


    // ==================================================
    // COUNT FINISHERS
    // ==================================================

    finishedCars = 0;

    for (int i = 0;
         i < populationSize;
         i++)
    {
        if (games[i].isFinished())
        {
            finishedCars++;
        }
    }


    // ==================================================
    // SUCCESSFUL GENERATION
    //
    // IMPORTANT: The finisher threshold is per-generation.
    // At least parentCount (5) cars must FINISH THE TRACK
    // within this single generation — finishers are NOT
    // accumulated across multiple generations.
    //
    // When met, the top 5 finishers (by fitness) become
    // the parents for the next map's population.
    // ==================================================

    if (finishedCars >= parentCount)
    {
        std::vector<int> best =
            selectBest();


        std::vector<NeuralNetwork>
            selectedParents;


        // --------------------------------------------------
        // TOP FIVE BY FITNESS
        // --------------------------------------------------

        for (int i = 0;
             i < parentCount;
             i++)
        {
            selectedParents.push_back(
                population[best[i]]
            );
        }


        successfulParents =
            selectedParents;


        std::cout
            << "\n================================\n"
            << "MAP "
            << currentMap + 1
            << " COMPLETE\n"
            << "Generation: "
            << generation
            << "\nFinishers: "
            << finishedCars
            << "\nTop 5 brains selected.\n"
            << "================================\n";


        // --------------------------------------------------
        // FINAL MAP
        // --------------------------------------------------

        if (currentMap ==
            MapConfig::mapCount - 1)
        {
            saveFinalBrains();

            trainingComplete =
                true;

            return;
        }


        // --------------------------------------------------
        // MOVE TO NEXT MAP
        //
        // The generation counter resets. A new population
        // is built from the 5 successful parents using
        // createChildrenFromFive() — which applies the
        // PREVIOUS map's mutation range (the parents already
        // proved themselves on that map; mutation produces
        // variants to adapt to the new track geometry).
        // --------------------------------------------------

        currentMap++;

        generation = 0;

        createChildrenFromFive();


        // Reset fitness tracking for the new map.

        fitness.assign(
            populationSize,
            0.0f
        );

        evaluated.assign(
            populationSize,
            false
        );

        finishedCars = 0;


        // Load the new map geometry into all game instances.

        for (int i = 0;
             i < populationSize;
             i++)
        {
            games[i].setMap(
                MapConfig::maps[currentMap]
            );
        }


        // Parents are now embedded in the new population;
        // clear the staging vector.

        successfulParents.clear();


        std::cout
            << "\n================================\n"
            << "MOVING TO MAP "
            << currentMap + 1
            << "\nMutation range: "
            << MapConfig::mutationRanges[currentMap - 1]
            << "\n================================\n";


        return;
    }


    // ==================================================
    // GENERATION ENDED WITHOUT 5 FINISHERS
    // ==================================================

    if (allOver)
    {
        std::cout
            << "Map "
            << currentMap + 1
            << " | Generation "
            << generation
            << " ended with "
            << finishedCars
            << "/"
            << parentCount
            << " finishers."
            << std::endl;


        // Evolve again on the SAME MAP.

        startNewGeneration();
    }
}


// ==================================================
// EVALUATE
// ==================================================

void Evolution::evaluate()
{
    update();
}


// ==================================================
// SELECT BEST
// ==================================================

std::vector<int> Evolution::selectBest()
{
    std::vector<int> indices;


    for (int i = 0;
         i < populationSize;
         i++)
    {
        indices.push_back(i);
    }


    std::sort(
        indices.begin(),
        indices.end(),

        [this](int a, int b)
        {
            return fitness[a] >
                   fitness[b];
        }
    );


    indices.resize(
        parentCount
    );


    return indices;
}


// ==================================================
// START NEW GENERATION
//
// Called when a generation ends without enough finishers.
// Uses the current generation's best networks (by fitness)
// to seed the next attempt on the SAME map.
//
// Strategy:
//   1. Select the top parentCount (5) networks by fitness.
//   2. Preserve them unchanged (elitism — their behavior
//      is the best available so far on this map).
//   3. Generate 95 children by cycling through the 5
//      parents and mutating copies.
//   4. Reset all games on the same map and increment
//      the generation counter.
//
// The mutation range comes from MapConfig::mutationRanges
// for the current map.
// ==================================================

void Evolution::startNewGeneration()
{
    std::vector<int> best =
        selectBest();


    std::vector<NeuralNetwork>
        newPopulation;


    newPopulation.reserve(
        populationSize
    );


    // ==================================================
    // KEEP BEST 5 UNCHANGED (elitism)
    //
    // These parents are copied directly into the new
    // population without mutation. This guarantees the
    // best-known behavior is never lost between
    // generations.
    // ==================================================

    for (int i = 0;
         i < parentCount;
         i++)
    {
        newPopulation.push_back(
            population[best[i]]
        );
    }


    // ==================================================
    // CREATE 95 CHILDREN
    //
    // Each child is a mutated copy of one parent.
    // Parents are assigned in round-robin order
    // (child i gets parent i % 5), so each parent
    // produces ~19 children.
    // ==================================================

    float mutationRange =
        MapConfig::mutationRanges[
            currentMap
        ];


    for (int i = parentCount;
         i < populationSize;
         i++)
    {
        int parentIndex =
            i % parentCount;


        NeuralNetwork child =
            population[
                best[parentIndex]
            ];


        child.mutate(
            mutationRange
        );


        newPopulation.push_back(
            child
        );
    }


    population =
        newPopulation;


    // ==================================================
    // RESET FITNESS
    // ==================================================

    fitness.assign(
        populationSize,
        0.0f
    );


    evaluated.assign(
        populationSize,
        false
    );


    finishedCars = 0;


    // ==================================================
    // RESET GAMES ON SAME MAP
    // ==================================================

    for (int i = 0;
         i < populationSize;
         i++)
    {
        games[i].setMap(
            MapConfig::maps[currentMap]
        );
    }


    generation++;


    std::cout
        << "Retrying Map "
        << currentMap + 1
        << " with Generation "
        << generation
        << "...\n";
}


// ==================================================
// CREATE POPULATION FROM FIVE SUCCESSFUL BRAINS
//
// Used specifically for MAP TRANSITIONS (not same-map
// retries). Builds a new population of 100 from the
// 5 successfulParents:
//   - 5 parents preserved unchanged.
//   - 95 children: mutated copies, round-robin across
//     the 5 parents.
//
// The mutation range is taken from the PREVIOUS map's
// entry in MapConfig::mutationRanges (currentMap - 1),
// because this function is called after currentMap has
// already been incremented.
// ==================================================

void Evolution::createChildrenFromFive()
{
    std::vector<NeuralNetwork>
        newPopulation;


    newPopulation.reserve(
        populationSize
    );


    // ==================================================
    // FIVE PARENTS
    // ==================================================

    for (int i = 0;
         i < parentCount;
         i++)
    {
        newPopulation.push_back(
            successfulParents[i]
        );
    }


    // ==================================================
    // 95 CHILDREN
    // ==================================================

    float mutationRange =
        MapConfig::mutationRanges[
            currentMap - 1
        ];


    for (int i = parentCount;
         i < populationSize;
         i++)
    {
        int parentIndex =
            (i - parentCount) %
            parentCount;


        NeuralNetwork child =
            successfulParents[
                parentIndex
            ];


        child.mutate(
            mutationRange
        );


        newPopulation.push_back(
            child
        );
    }


    population =
        newPopulation;
}


// ==================================================
// CREATE NEXT GENERATION
// ==================================================

void Evolution::createNextGeneration()
{
    startNewGeneration();
}


// ==================================================
// NEXT GENERATION
// ==================================================

void Evolution::nextGeneration()
{
    startNewGeneration();
}


// ==================================================
// SAVE FINAL BRAINS
//
// Called once after the last map is completed.
// Writes the successfulParents to "final_brains.bin"
// in binary format:
//   - 4-byte int: number of brains saved (parentCount).
//   - For each brain: raw dump of all weight/bias arrays
//     (see NeuralNetwork::save()).
//
// To change the output path, edit the filename below.
// To test the saved brains, run final_test.
// ==================================================

void Evolution::saveFinalBrains()
{
    std::ofstream file(
        "final_brains.bin",
        std::ios::binary
    );


    if (!file)
    {
        std::cerr
            << "ERROR: Could not create "
            << "final_brains.bin\n";

        return;
    }


    int count =
        static_cast<int>(
            successfulParents.size()
        );


    file.write(
        reinterpret_cast<const char*>(
            &count
        ),
        sizeof(count)
    );


    for (int i = 0;
         i < count;
         i++)
    {
        successfulParents[i].save(
            file
        );
    }


    file.close();


    std::cout
        << "\n================================\n"
        << "TRAINING COMPLETE\n"
        << "Saved "
        << count
        << " brains to final_brains.bin\n"
        << "================================\n";
}


// ==================================================
// GET BEST NETWORK
// ==================================================

NeuralNetwork& Evolution::getBestNetwork()
{
    int bestIndex = 0;


    for (int i = 1;
         i < populationSize;
         i++)
    {
        if (fitness[i] >
            fitness[bestIndex])
        {
            bestIndex =
                i;
        }
    }


    return population[bestIndex];
}


// ==================================================
// GET BEST FITNESS
// ==================================================

float Evolution::getBestFitness()
{
    float best =
        fitness[0];


    for (int i = 1;
         i < populationSize;
         i++)
    {
        if (fitness[i] >
            best)
        {
            best =
                fitness[i];
        }
    }


    return best;
}


// ==================================================
// GET GENERATION
// ==================================================

int Evolution::getGeneration()
{
    return generation;
}


// ==================================================
// GET CURRENT MAP
// ==================================================

int Evolution::getCurrentMap()
{
    return currentMap;
}


// ==================================================
// GET FINISHED CARS
// ==================================================

int Evolution::getFinishedCars()
{
    return finishedCars;
}


// ==================================================
// TRAINING COMPLETE
// ==================================================

bool Evolution::isTrainingComplete()
{
    return trainingComplete;
}


// ==================================================
// DRAW — Population Visualization
//
// Renders the training state each frame. The track is
// drawn once (from game[0]) and then all 100 cars are
// drawn as overlays (without redrawing the track).
// The populationCamera provides a fixed overview of
// the entire track, unlike Game::draw() which follows
// a single car.
//
// A HUD displays: current map, generation number,
// alive cars, finisher count, best fitness, and the
// current map's mutation range.
// ==================================================

void Evolution::draw()
{
    BeginDrawing();

    ClearBackground(GREEN);


    // ==================================================
    // DRAW TRACK
    // ==================================================

    games[0].drawWorld(
        populationCamera,
        true,
        false
    );


    // ==================================================
    // DRAW ALL CARS
    // ==================================================

    for (int i = 0;
         i < populationSize;
         i++)
    {
        games[i].drawWorld(
            populationCamera,
            false,
            false
        );
    }


    // ==================================================
    // COUNT ALIVE CARS
    // ==================================================

    int alive = 0;


    for (int i = 0;
         i < populationSize;
         i++)
    {
        if (!games[i].isSimulationOver())
        {
            alive++;
        }
    }


    // ==================================================
    // UI
    // ==================================================

    if (trainingComplete)
    {
        DrawRectangle(
            300,
            320,
            600,
            150,
            BLACK
        );


        DrawText(
            "TRAINING COMPLETE",
            390,
            350,
            35,
            YELLOW
        );


        DrawText(
            "Saved: final_brains.bin",
            420,
            405,
            22,
            WHITE
        );
    }
    else
    {
        DrawText(
            TextFormat(
                "Map: %d / %d",
                currentMap + 1,
                MapConfig::mapCount
            ),
            20,
            20,
            25,
            WHITE
        );


        DrawText(
            TextFormat(
                "Generation: %d",
                generation
            ),
            20,
            50,
            25,
            WHITE
        );


        DrawText(
            TextFormat(
                "Cars alive: %d / %d",
                alive,
                populationSize
            ),
            20,
            80,
            20,
            WHITE
        );


        DrawText(
            TextFormat(
                "Finishers: %d / %d",
                finishedCars,
                parentCount
            ),
            20,
            110,
            20,
            WHITE
        );


        DrawText(
            TextFormat(
                "Best fitness: %.2f",
                getBestFitness()
            ),
            20,
            140,
            20,
            WHITE
        );


        DrawText(
            TextFormat(
                "Mutation: %.2f",
                MapConfig::mutationRanges[currentMap]
            ),
            20,
            170,
            20,
            WHITE
        );
    }


    EndDrawing();
}