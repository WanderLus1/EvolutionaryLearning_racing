// ==================================================
// NEURAL NETWORK
//
// Fixed-topology feed-forward neural network — the
// "brain" controlling each car.
//
// Architecture:
//   6 inputs -> 16 hidden neurons -> 8 hidden neurons -> 7 outputs
//   All layers use sigmoid activation: sigma(z) = 1 / (1 + exp(-z)).
//
// Inputs (6):
//   5 normalized distance sensors (forward-left 45°,
//   left 90°, forward, right 90°, forward-right 45°)
//   plus normalized car speed.
//
// Outputs (7):
//   Corresponds to driving actions; the action with
//   the highest activation is selected (argmax).
//
// Training / Optimization:
//   Weights are evolved via mutation (neuroevolution),
//   NOT gradient descent / backpropagation.
//
// Modifying Architecture:
//   To change the architecture, modify the array dimensions
//   in the private members below and update predict(),
//   mutate(), save(), and load() accordingly in NeuralNetwork.cpp.
// ==================================================

#pragma once

#include <vector>
#include <iosfwd>

class NeuralNetwork
{
private:

    float layer_1[16][6];       // 16 neurons, each with 6 input weights
    float layer_1_bias[16];

    float layer_2[8][16];       // 8 neurons, each with 16 input weights (from layer 1)
    float layer_2_bias[8];

    float output_layer[7][8];   // 7 output neurons, each with 8 input weights (from layer 2)
    float output_bias[7];


public:

    NeuralNetwork();

    std::vector<float> predict(
        const std::vector<float>& input
    );

    void mutate(
        float mutationRange
    );

    bool save(
        std::ostream& out
    ) const;

    bool load(
        std::istream& in
    );
};