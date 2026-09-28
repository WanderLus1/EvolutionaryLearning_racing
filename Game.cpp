#include "Game.h"
#include <cmath>
#include <limits>


// ==================================================
// CONSTRUCTOR
//
// CAR / TRACK TUNING PARAMETERS
// Change these values to adjust the simulation.
// ==================================================

Game::Game(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    // Total road width, measured as ±(trackWidth/2) from
    // the track centerline. Affects drivable area and
    // collision margins.
    trackWidth = 80.0f;

    // Capsule collision shape dimensions.
    // carRadius = half-width of the capsule's rounded ends.
    // carLength = tip-to-tip length of the capsule.
    carRadius = 10.0f;
    carLength = 34.0f;

    // Default action: 2 = coast/no-input (car starts idle).
    currentAction = 2;


    // ==================================================
    // DEFAULT MAP
    // ==================================================

    trackPoints[0]  = {650, 240};
    trackPoints[1]  = {850, 240};
    trackPoints[2]  = {1020, 150};
    trackPoints[3]  = {1180, 260};
    trackPoints[4]  = {1180, 420};
    trackPoints[5]  = {1020, 530};
    trackPoints[6]  = {850, 440};
    trackPoints[7]  = {750, 440};
    trackPoints[8]  = {650, 440};
    trackPoints[9]  = {550, 440};
    trackPoints[10] = {280, 530};
    trackPoints[11] = {120, 420};
    trackPoints[12] = {120, 260};
    trackPoints[13] = {280, 150};
    trackPoints[14] = {450, 240};
    trackPoints[15] = {550, 240};
    trackPoints[16] = {600, 240};
    trackPoints[17] = {630, 240};


    // ==================================================
    // CAMERA
    // ==================================================

    camera = {0};

    camera.target = trackPoints[0];

    camera.offset =
    {
        screenWidth / 2.0f,
        screenHeight / 2.0f
    };

    camera.zoom = 1.0f;


    reset();
}


// ==================================================
// SET MAP
// ==================================================

void Game::setMap(
    const Vector2* newMap)
{
    for (int i = 0;
         i < trackPointCount;
         i++)
    {
        trackPoints[i] =
            newMap[i];
    }

    reset();
}


// ==================================================
// RESET
// ==================================================

void Game::reset()
{
    carPosition = trackPoints[0];

    carSpeed = 0.0f;
    carAngle = 90.0f;

    raceTime = 0.0;

    raceFinished = false;
    hasLeftStart = false;
    crashed = false;
    reachedCheckpoint10 = false;
    timeLimitReached = false;

    currentAction = 2;

    distanceTravelled = 0.0f;

    distanceSinceLastCheck = 0.0f;
    crashCheckTimer = 0.0f;

    camera.target = carPosition;
}


// ==================================================
// POINT → SEGMENT DISTANCE
// ==================================================

float Game::PointToSegmentDistanceSquared(
    Vector2 point,
    Vector2 start,
    Vector2 end)
{
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    float lengthSquared =
        dx * dx + dy * dy;

    if (lengthSquared == 0.0f)
    {
        float px = point.x - start.x;
        float py = point.y - start.y;

        return px * px + py * py;
    }

    float t =
        ((point.x - start.x) * dx +
         (point.y - start.y) * dy) /
        lengthSquared;

    if (t < 0.0f)
        t = 0.0f;

    if (t > 1.0f)
        t = 1.0f;

    float closestX =
        start.x + t * dx;

    float closestY =
        start.y + t * dy;

    float diffX =
        point.x - closestX;

    float diffY =
        point.y - closestY;

    return diffX * diffX +
           diffY * diffY;
}


// ==================================================
// SEGMENT → SEGMENT DISTANCE
// ==================================================

float Game::SegmentToSegmentDistanceSquared(
    Vector2 a,
    Vector2 b,
    Vector2 c,
    Vector2 d)
{
    float abx = b.x - a.x;
    float aby = b.y - a.y;

    float cdx = d.x - c.x;
    float cdy = d.y - c.y;

    float cross =
        abx * cdy -
        aby * cdx;

    float acx = c.x - a.x;
    float acy = c.y - a.y;

    if (fabs(cross) > 0.00001f)
    {
        float t =
            (acx * cdy -
             acy * cdx) /
            cross;

        float u =
            (acx * aby -
             acy * abx) /
            cross;

        if (t >= 0.0f && t <= 1.0f &&
            u >= 0.0f && u <= 1.0f)
        {
            return 0.0f;
        }
    }

    float distance1 =
        PointToSegmentDistanceSquared(
            a, c, d
        );

    float distance2 =
        PointToSegmentDistanceSquared(
            b, c, d
        );

    float distance3 =
        PointToSegmentDistanceSquared(
            c, a, b
        );

    float distance4 =
        PointToSegmentDistanceSquared(
            d, a, b
        );

    float result = distance1;

    if (distance2 < result)
        result = distance2;

    if (distance3 < result)
        result = distance3;

    if (distance4 < result)
        result = distance4;

    return result;
}


// ==================================================
// CAPSULE CAR COLLISION
//
// Determines whether the car is still on the drivable
// road surface. The car is modeled as a capsule (a line
// segment from back to front, with rounded ends of
// carRadius). The track is modeled as a series of line
// segments (the centerline) with uniform half-width
// (trackWidth / 2).
//
// Returns true if the capsule is entirely within the
// road surface. Returns false if any part extends
// beyond the track boundary.
//
// Experimental note: collision is treated as an
// immediate crash (see update()). Earlier experiments
// with accumulating collision penalties did not produce
// effective selection pressure — a binary crash signal
// gives evolution a much cleaner distinction between
// safe and unsafe driving.
// ==================================================

bool Game::CarOnTrack(Vector2 position)
{
    float radians =
        carAngle * DEG2RAD;

    Vector2 direction =
    {
        sinf(radians),
        -cosf(radians)
    };

    float halfLength =
        carLength / 2.0f -
        carRadius;

    Vector2 front =
    {
        position.x +
        direction.x * halfLength,

        position.y +
        direction.y * halfLength
    };

    Vector2 back =
    {
        position.x -
        direction.x * halfLength,

        position.y -
        direction.y * halfLength
    };

    float minimumDistanceSquared =
        INFINITY;

    for (int i = 0;
         i < trackPointCount;
         i++)
    {
        int next =
            (i + 1) % trackPointCount;

        float distance =
            SegmentToSegmentDistanceSquared(
                back,
                front,
                trackPoints[i],
                trackPoints[next]
            );

        if (distance <
            minimumDistanceSquared)
        {
            minimumDistanceSquared =
                distance;
        }
    }

    float allowedDistance =
        trackWidth / 2.0f -
        carRadius;

    return minimumDistanceSquared <=
           allowedDistance * allowedDistance;
}


// ==================================================
// FIND CLOSEST TRACK SEGMENT
// ==================================================

int Game::FindClosestTrackSegment(
    Vector2 position)
{
    float closestDistance =
        INFINITY;

    int closestSegment = 0;

    for (int i = 0;
         i < trackPointCount;
         i++)
    {
        int next =
            (i + 1) % trackPointCount;

        float distance =
            PointToSegmentDistanceSquared(
                position,
                trackPoints[i],
                trackPoints[next]
            );

        if (distance <
            closestDistance)
        {
            closestDistance =
                distance;

            closestSegment =
                i;
        }
    }

    return closestSegment;
}


// ==================================================
// SENSOR RAY
//
// Casts a ray from the car in a given direction and
// returns the distance to the nearest track boundary.
// Steps along the ray in 2-pixel increments, checking
// whether each sample point is still on the road.
//
// Returns the distance in pixels (0 to maxDistance).
// maxDistance = 500 pixels. If no boundary is hit
// within range, returns maxDistance.
//
// This is used by getSensors() to build the neural
// network's input vector. The ray directions are
// relative to the car's facing angle.
// ==================================================

float Game::CastSensor(
    Vector2 origin,
    Vector2 direction)
{
    const float maxDistance = 500.0f;
    const float step = 2.0f;

    for (float distance = 0.0f;
         distance <= maxDistance;
         distance += step)
    {
        Vector2 point =
        {
            origin.x +
            direction.x * distance,

            origin.y +
            direction.y * distance
        };

        int segment =
            FindClosestTrackSegment(point);

        int next =
            (segment + 1) %
            trackPointCount;

        float wallDistance =
            sqrtf(
                PointToSegmentDistanceSquared(
                    point,
                    trackPoints[segment],
                    trackPoints[next]
                )
            );

        if (wallDistance >
            trackWidth / 2.0f)
        {
            return distance;
        }
    }

    return maxDistance;
}


// ==================================================
// GET SENSORS — Neural Network Input Vector
//
// Builds the 6-element input vector that the neural
// network uses to perceive the world. The network has
// NO other information about the game state.
//
// Sensor layout (all normalized to [0, 1]):
//   [0] Forward-left 45°   — distance / 500
//   [1] Left 90°           — distance / 500
//   [2] Forward             — distance / 500
//   [3] Right 90°          — distance / 500
//   [4] Forward-right 45°  — distance / 500
//   [5] Car speed           — speed / 6.0 (max speed)
//
// The 0.7071 factor is sqrt(2)/2, used to construct
// 45-degree diagonal directions from the forward and
// lateral unit vectors.
//
// To add more sensors, increase the input count here
// AND update NeuralNetwork's layer_1 dimensions.
// ==================================================

std::vector<float> Game::getSensors()
{
    std::vector<float> sensors;

    float radians =
        carAngle * DEG2RAD;

    Vector2 forward =
    {
        sinf(radians),
        -cosf(radians)
    };

    Vector2 left =
    {
        -cosf(radians),
        -sinf(radians)
    };

    Vector2 right =
    {
        cosf(radians),
        sinf(radians)
    };

    Vector2 directions[5] =
    {
        {
            forward.x * 0.7071f +
            left.x * 0.7071f,

            forward.y * 0.7071f +
            left.y * 0.7071f
        },

        left,

        forward,

        right,

        {
            forward.x * 0.7071f +
            right.x * 0.7071f,

            forward.y * 0.7071f +
            right.y * 0.7071f
        }
    };

    for (int i = 0;
         i < 5;
         i++)
    {
        float distance =
            CastSensor(
                carPosition,
                directions[i]
            );

        sensors.push_back(
            distance / 500.0f
        );
    }

    sensors.push_back(
        carSpeed / 6.0f
    );

    return sensors;
}


// ==================================================
// APPLY AI ACTION
//
// Sets the current driving action chosen by the neural
// network. The action is an index (0–6) selected via
// argmax over the NN's 7 output activations.
// ==================================================

void Game::applyAction(int action)
{
    currentAction = action;
}


// ==================================================
// UPDATE
// ==================================================

void Game::update()
{
    if (IsKeyPressed(KEY_R))
    {
        reset();
    }

    if (raceFinished ||
        crashed ||
        timeLimitReached)
    {
        return;
    }


    // ==================================================
    // TIMER
    // ==================================================

    raceTime += GetFrameTime();


    // ==================================================
    // TIME LIMIT (30 seconds per simulation)
    // ==================================================

    if (raceTime >= 30.0)
    {
        timeLimitReached = true;
        carSpeed = 0.0f;

        return;
    }


    // ==================================================
    // ACTION TABLE
    //
    // 0 = Forward         (accelerate)
    // 1 = Reverse         (DISABLED — see below)
    // 2 = Coast           (no input / idle)
    // 3 = Forward + Right
    // 4 = Forward + Left
    // 5 = Turn Right      (no throttle)
    // 6 = Turn Left       (no throttle)
    //
    // Reverse (action 1) is intentionally disabled.
    // When it was enabled, evolution discovered that
    // moving backward could accumulate distance/fitness,
    // exploiting the reward function instead of learning
    // to drive the track in the intended direction.
    // The AI optimizes the reward, not the developer's
    // intention — so the exploit must be removed.
    //
    // To change the number of actions, update this
    // switch AND NeuralNetwork's output_layer dimensions.
    // ==================================================

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    switch (currentAction)
    {
        case 0:
            forward = true;
            break;

        case 1:
            // Reverse disabled
            break;

        case 2:
            break;

        case 3:
            forward = true;
            right = true;
            break;

        case 4:
            forward = true;
            left = true;
            break;

        case 5:
            right = true;
            break;

        case 6:
            left = true;
            break;
    }


    // ==================================================
    // ACCELERATION
    //
    // Physics tuning parameters:
    //   Acceleration:  +0.15 per frame when throttle is on.
    //   Friction:      ×0.98 decay per frame (always applied).
    //   Max speed:     6.0 (clamped).
    //   Min speed:     0.0 (no reverse movement allowed).
    // ==================================================

    if (forward)
        carSpeed += 0.15f;

    carSpeed *= 0.98f;


    if (carSpeed > 6.0f)
        carSpeed = 6.0f;

    if (carSpeed < 0.0f)
        carSpeed = 0.0f;


    // ==================================================
    // STEERING
    //
    // Steering rate: ±2.5 degrees per frame.
    // ==================================================

    if (left)
        carAngle -= 2.5f;

    if (right)
        carAngle += 2.5f;


    // ==================================================
    // MOVEMENT
    // ==================================================

    float radians =
        carAngle * DEG2RAD;

    Vector2 velocity =
    {
        sinf(radians) * carSpeed,
        -cosf(radians) * carSpeed
    };

    Vector2 newPosition =
    {
        carPosition.x +
        velocity.x,

        carPosition.y +
        velocity.y
    };


    float oldX =
        carPosition.x;

    float oldY =
        carPosition.y;


    // ==================================================
    // COLLISION = CRASH
    //
    // If the car leaves the track boundary, the
    // simulation ends immediately. This is intentionally
    // a hard crash rather than a gradual penalty —
    // experiments showed that accumulating small collision
    // penalties did not provide strong enough evolutionary
    // pressure. A binary crash gives evolution a clean
    // signal: stay on the road or die.
    // ==================================================

    if (CarOnTrack(newPosition))
    {
        carPosition =
            newPosition;
    }
    else
    {
        crashed = true;
        carSpeed = 0.0f;
    }


    // ==================================================
    // DISTANCE TRAVELLED
    // ==================================================

    float dx =
        carPosition.x - oldX;

    float dy =
        carPosition.y - oldY;

    float movement =
        sqrtf(
            dx * dx +
            dy * dy
        );

    distanceTravelled +=
        movement;

    distanceSinceLastCheck +=
        movement;


    // ==================================================
    // CHECKPOINT 10 — Direction Guard
    //
    // The car must pass within 50 pixels of track
    // point 10 (roughly the halfway mark) before it
    // can finish. This prevents evolution from
    // discovering that a car can "finish" by staying
    // near the start or circling back without
    // traversing the full track.
    // ==================================================

    float checkpointDX =
        carPosition.x -
        trackPoints[10].x;

    float checkpointDY =
        carPosition.y -
        trackPoints[10].y;

    float checkpointDistance =
        sqrtf(
            checkpointDX * checkpointDX +
            checkpointDY * checkpointDY
        );

    if (checkpointDistance < 50.0f)
    {
        reachedCheckpoint10 = true;
    }


    // ==================================================
    // CRASH / STUCK DETECTION
    //
    // If the car moves less than 5 pixels in any
    // 1-second window, it is marked as crashed. This
    // prevents evolution from exploiting stationary
    // or slow-spinning behaviors that could accumulate
    // time-based fitness without making track progress.
    // ==================================================

    crashCheckTimer +=
        GetFrameTime();

    if (crashCheckTimer >= 1.0f)
    {
        if (distanceSinceLastCheck < 5.0f)
        {
            crashed = true;
            carSpeed = 0.0f;
        }

        distanceSinceLastCheck = 0.0f;
        crashCheckTimer = 0.0f;
    }


    // ==================================================
    // FINISH DETECTION
    //
    // A lap is completed when ALL three conditions are met:
    //   1. hasLeftStart: car moved > 100px from start
    //      (prevents "finishing" without leaving).
    //   2. reachedCheckpoint10: car passed near point 10
    //      (ensures correct traversal direction).
    //   3. distanceFromStart < 40px: car returned to
    //      the start/finish area.
    //
    // This combination ensures the car actually drove
    // around the track in the intended direction rather
    // than exploiting a shortcut or staying near start.
    // ==================================================

    float distanceFromStart =
        sqrtf(
            (carPosition.x -
             trackPoints[0].x) *
            (carPosition.x -
             trackPoints[0].x) +

            (carPosition.y -
             trackPoints[0].y) *
            (carPosition.y -
             trackPoints[0].y)
        );


    if (distanceFromStart > 100.0f)
    {
        hasLeftStart = true;
    }


    if (!crashed &&
        !timeLimitReached &&
        hasLeftStart &&
        reachedCheckpoint10 &&
        distanceFromStart < 40.0f)
    {
        raceFinished = true;
        carSpeed = 0.0f;
    }


    // ==================================================
    // CAMERA
    // ==================================================

    camera.target.x +=
        (carPosition.x -
         camera.target.x) * 0.10f;

    camera.target.y +=
        (carPosition.y -
         camera.target.y) * 0.10f;
}


// ==================================================
// DRAW WORLD
//
// Renders the game world (track + car) using a
// provided camera. Flags control what is drawn:
//   drawTrack = true   → render the road surface.
//   drawSensors = true → render sensor ray lines.
//
// During training, Evolution calls this with
// drawTrack=true once (from game[0]) and then calls
// it with drawTrack=false for all 100 cars, so the
// track is drawn once and cars are overlaid.
//
// The simulation itself runs independently of
// rendering — this function only visualizes state.
// ==================================================

void Game::drawWorld(
    Camera2D view,
    bool drawTrack,
    bool drawSensors)
{
    BeginMode2D(view);


    // ==================================================
    // TRACK
    // ==================================================

    if (drawTrack)
    {
        for (int i = 0;
             i < trackPointCount;
             i++)
        {
            int next =
                (i + 1) % trackPointCount;

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
             i < trackPointCount;
             i++)
        {
            DrawCircleV(
                trackPoints[i],
                5,
                YELLOW
            );
        }


        // ==================================================
        // START
        // ==================================================

        DrawCircleV(
            trackPoints[0],
            20,
            WHITE
        );

        DrawCircleV(
            trackPoints[0],
            12,
            DARKGRAY
        );
    }


    // ==================================================
    // SENSOR VISUALIZATION
    // ==================================================

    if (drawSensors)
    {
        float radians =
            carAngle * DEG2RAD;

        Vector2 forward =
        {
            sinf(radians),
            -cosf(radians)
        };

        Vector2 left =
        {
            -cosf(radians),
            -sinf(radians)
        };

        Vector2 right =
        {
            cosf(radians),
            sinf(radians)
        };

        Vector2 directions[5] =
        {
            {
                forward.x * 0.7071f +
                left.x * 0.7071f,

                forward.y * 0.7071f +
                left.y * 0.7071f
            },

            left,

            forward,

            right,

            {
                forward.x * 0.7071f +
                right.x * 0.7071f,

                forward.y * 0.7071f +
                right.y * 0.7071f
            }
        };


        for (int i = 0;
             i < 5;
             i++)
        {
            float distance =
                CastSensor(
                    carPosition,
                    directions[i]
                );

            Vector2 end =
            {
                carPosition.x +
                directions[i].x *
                distance,

                carPosition.y +
                directions[i].y *
                distance
            };

            DrawLineEx(
                carPosition,
                end,
                2.0f,
                YELLOW
            );
        }
    }


    // ==================================================
    // CAR
    // ==================================================

    float radians =
        carAngle * DEG2RAD;

    Vector2 carDirection =
    {
        sinf(radians),
        -cosf(radians)
    };

    float halfLength =
        carLength / 2.0f -
        carRadius;

    Vector2 front =
    {
        carPosition.x +
        carDirection.x *
        halfLength,

        carPosition.y +
        carDirection.y *
        halfLength
    };

    Vector2 back =
    {
        carPosition.x -
        carDirection.x *
        halfLength,

        carPosition.y -
        carDirection.y *
        halfLength
    };


    DrawLineEx(
        back,
        front,
        carRadius * 2.0f,
        RED
    );

    DrawCircleV(
        back,
        carRadius,
        RED
    );

    DrawCircleV(
        front,
        carRadius,
        RED
    );


    EndMode2D();
}


// ==================================================
// DRAW
// ==================================================

void Game::draw()
{
    BeginDrawing();

    ClearBackground(GREEN);

    drawWorld(
        camera,
        true,
        true
    );


    // ==================================================
    // UI
    // ==================================================

    DrawText(
        TextFormat(
            "Speed: %.1f",
            carSpeed
        ),
        20,
        20,
        20,
        WHITE
    );

    DrawText(
        TextFormat(
            "Time: %.2f s",
            raceTime
        ),
        20,
        50,
        20,
        WHITE
    );

    DrawText(
        TextFormat(
            "Distance: %.1f",
            distanceTravelled
        ),
        20,
        80,
        20,
        WHITE
    );

    if (reachedCheckpoint10)
    {
        DrawText(
            "CHECKPOINT 10 REACHED",
            20,
            110,
            20,
            YELLOW
        );
    }

    if (crashed)
    {
        DrawText(
            "CRASHED",
            20,
            140,
            25,
            RED
        );
    }

    if (timeLimitReached)
    {
        DrawText(
            "TIME LIMIT REACHED",
            20,
            170,
            25,
            YELLOW
        );
    }


    // ==================================================
    // FINISH SCREEN
    // ==================================================

    if (raceFinished)
    {
        DrawRectangle(
            screenWidth / 2 - 230,
            screenHeight / 2 - 100,
            460,
            200,
            BLACK
        );

        DrawText(
            "FINISHED!",
            screenWidth / 2 - 105,
            screenHeight / 2 - 70,
            40,
            YELLOW
        );

        DrawText(
            TextFormat(
                "Final Time: %.2f seconds",
                raceTime
            ),
            screenWidth / 2 - 145,
            screenHeight / 2 - 15,
            25,
            WHITE
        );

        DrawText(
            "Press R to race again",
            screenWidth / 2 - 120,
            screenHeight / 2 + 35,
            20,
            WHITE
        );
    }

    EndDrawing();
}


// ==================================================
// GAME STATE
// ==================================================

bool Game::isFinished()
{
    return raceFinished;
}


bool Game::hasCrashed()
{
    return crashed;
}


bool Game::isSimulationOver()
{
    return raceFinished ||
           crashed ||
           timeLimitReached;
}


float Game::getSpeed()
{
    return carSpeed;
}


double Game::getRaceTime()
{
    return raceTime;
}


float Game::getDistanceTravelled()
{
    return distanceTravelled;
}


Vector2 Game::getCarPosition()
{
    return carPosition;
}


float Game::getCarAngle()
{
    return carAngle;
}


// ==================================================
// FITNESS — Evolutionary Reward Signal
//
// This function determines what behavior evolution
// selects for. The AI learns whatever produces the
// highest fitness — not what the developer intends.
// If the formula rewards speed too strongly, the AI
// will optimize speed even at the cost of crashing
// on difficult sections ("speedster" behavior).
//
// Current formula:
//   baseFitness = distance³ / (raceTime + 0.01)
//
// Design philosophy:
//   PRIMARY: Drive farther / make track progress.
//     Distance is CUBED to make progress dominant.
//   SECONDARY: Do it efficiently / quickly.
//     Time is a linear divisor (secondary factor).
//   FAILURE: Crash → baseFitness - 100,000
//     (strong penalty discourages wall-hitting).
//   SUCCESS: Finish → baseFitness + 1,000,000
//     (strong bonus rewards completing the lap).
//
// The 0.01 prevents division by zero at time = 0.
//
// Earlier formulations that squared distance (instead
// of cubing) weighted speed too heavily and produced
// networks that were fast but could not handle bends.
// Cubing distance ensures that making it further
// around the track always dominates over going faster
// on a shorter segment.
//
// KEY LESSON: Whenever the AI behaves unexpectedly,
// first ask "What is the fitness telling it to do?"
// rather than assuming the neural network is broken.
// ==================================================

float Game::getFitness()
{
    float baseFitness =
        (distanceTravelled *
         distanceTravelled) * distanceTravelled/
        (float)(raceTime + 0.01);


    if (crashed)
    {
        return baseFitness - 100000.0f;
    }


    if (raceFinished)
    {
        return baseFitness + 1000000.0f;
    }


    return baseFitness;
}