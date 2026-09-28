// ==================================================
// Game.h — Racing Environment Simulation
//
// Game represents a single racing environment for one car controlled
// by one neural network.
//
// Each Game instance manages: track geometry, car physics, collision
// detection, sensors, and fitness calculation.
//
// During training, 100 Game instances run simultaneously (one per
// neural network in the population).
//
// The Game acts as the interface between the neural network and the
// simulation: sensors provide the NN's input, and applyAction() sets
// the NN's chosen action.
// ==================================================

#pragma once

#include "raylib.h"
#include <vector>

class Game
{
private:

    // ==================================================
    // TRACK
    // ==================================================

    // Number of centerline points defining the track shape.
    static const int trackPointCount = 18;

    // Track centerline coordinates (Vector2). These are NOT outer boundaries —
    // the road extends ±(trackWidth/2) from the centerline. Set via setMap().
    Vector2 trackPoints[trackPointCount];

    // Total road width. Currently 80.0f. To change, edit in the Game constructor
    // in Game.cpp (affects drivable road width and boundary collision margins).
    float trackWidth;


    // ==================================================
    // CAR
    // ==================================================

    Vector2 carPosition;

    float carSpeed;
    float carAngle;

    // Define the capsule collision shape. The car is modeled as a capsule
    // (line segment with rounded ends) for collision purposes.
    // Set in the Game constructor in Game.cpp; affects hitboxes and collision detection.
    float carRadius;
    float carLength;


    // ==================================================
    // CAMERA
    // ==================================================

    Camera2D camera;

    int screenWidth;
    int screenHeight;


    // ==================================================
    // RACE
    // ==================================================

    double raceTime;

    bool raceFinished;
    bool hasLeftStart;
    bool crashed;

    // Ensures the car traverses at least half the track (reaches point 10)
    // before finishing. Prevents exploiting short-circuit paths.
    bool reachedCheckpoint10;

    // 30-second time limit per simulation (checked in update()).
    bool timeLimitReached;


    // ==================================================
    // ML
    // ==================================================

    // Action index (0–6) set by the neural network each frame.
    int currentAction;

    // Total distance moved by the car, used in fitness calculation.
    float distanceTravelled;

    // Stuck detection: tracks distance moved within a 1-second window.
    // If the car moves less than 5 pixels in 1 second, it's treated as crashed.
    float distanceSinceLastCheck;

    float crashCheckTimer;


    // ==================================================
    // COLLISION
    // ==================================================

    float PointToSegmentDistanceSquared(
        Vector2 point,
        Vector2 start,
        Vector2 end
    );

    float SegmentToSegmentDistanceSquared(
        Vector2 a,
        Vector2 b,
        Vector2 c,
        Vector2 d
    );

    bool CarOnTrack(Vector2 position);

    int FindClosestTrackSegment(Vector2 position);


    // ==================================================
    // SENSORS
    // ==================================================

    float CastSensor(
        Vector2 origin,
        Vector2 direction
    );


public:

    Game(int width, int height);

    void reset();

    void update();

    void draw();

    void drawWorld(
        Camera2D view,
        bool drawTrack,
        bool drawSensors
    );


    // ==================================================
    // MAP
    // ==================================================

    void setMap(
        const Vector2* newMap
    );


    // ==================================================
    // ML INTERFACE
    // ==================================================

    std::vector<float> getSensors();

    void applyAction(int action);


    // ==================================================
    // GAME STATE
    // ==================================================

    bool isFinished();

    bool hasCrashed();

    bool isSimulationOver();

    float getSpeed();

    double getRaceTime();

    float getDistanceTravelled();

    Vector2 getCarPosition();

    float getCarAngle();

    float getFitness();
};