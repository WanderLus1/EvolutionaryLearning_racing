#include <iostream>
#include <random>
#include <cmath>
#include <ostream>
#include <istream>

#include "NeuralNetwork.h"


// ==================================================
// CONSTRUCTOR
//
// Randomly initializes all weights and biases in [-1.0, 1.0]
// using a uniform real distribution.
// ==================================================

NeuralNetwork::NeuralNetwork()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<float> dist(
        -1.0f,
        1.0f
    );


    // ==================================================
    // BIASES
    // ==================================================

    for (int i = 0; i < 16; i++)
        layer_1_bias[i] = dist(gen);

    for (int i = 0; i < 8; i++)
        layer_2_bias[i] = dist(gen);

    for (int i = 0; i < 7; i++)
        output_bias[i] = dist(gen);


    // ==================================================
    // LAYER 1
    // ==================================================

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            layer_1[i][j] = dist(gen);
        }
    }


    // ==================================================
    // LAYER 2
    // ==================================================

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            layer_2[i][j] = dist(gen);
        }
    }


    // ==================================================
    // OUTPUT
    // ==================================================

    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            output_layer[i][j] = dist(gen);
        }
    }
}


// ==================================================
// PREDICT
//
// Performs a forward pass through all three layers.
// Each layer computes z = bias + Σ(weight × input),
// then applies sigmoid activation σ(z) = 1/(1+exp(-z)).
// Returns a 7-element vector of activations.
// ==================================================

std::vector<float> NeuralNetwork::predict(
    const std::vector<float>& input)
{
    float l1_out_vec[16];
    float l2_out_vec[8];

    std::vector<float> output_vec(7);


    // ==================================================
    // LAYER 1
    // ==================================================

    for (int i = 0; i < 16; i++)
    {
        float z =
            layer_1_bias[i];

        for (int j = 0; j < 6; j++)
        {
            z +=
                layer_1[i][j] *
                input[j];
        }

        float g_z =
            1.0f /
            (1.0f + std::exp(-z));

        l1_out_vec[i] =
            g_z;
    }


    // ==================================================
    // LAYER 2
    // ==================================================

    for (int i = 0; i < 8; i++)
    {
        float z =
            layer_2_bias[i];

        for (int j = 0; j < 16; j++)
        {
            z +=
                layer_2[i][j] *
                l1_out_vec[j];
        }

        float g_z =
            1.0f /
            (1.0f + std::exp(-z));

        l2_out_vec[i] =
            g_z;
    }


    // ==================================================
    // OUTPUT
    // ==================================================

    for (int i = 0; i < 7; i++)
    {
        float z =
            output_bias[i];

        for (int j = 0; j < 8; j++)
        {
            z +=
                output_layer[i][j] *
                l2_out_vec[j];
        }

        float g_z =
            1.0f /
            (1.0f + std::exp(-z));

        output_vec[i] =
            g_z;
    }


    return output_vec;
}


// ==================================================
// MUTATE
//
// Sole mechanism for exploring the weight space (no
// backpropagation is used). Perturbs every weight and bias
// by adding a uniform random value in [-mutationRange, +mutationRange].
//
// mutationRange is a critical hyperparameter:
//   - Too large destroys learned behavior.
//   - Too small causes evolutionary stagnation.
// The range is typically set per-map in Evolution.cpp
// (MapConfig::mutationRanges).
// ==================================================

void NeuralNetwork::mutate(
    float mutationRange)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<float>
        mutation_dist(
            -mutationRange,
            mutationRange
        );


    // ==================================================
    // LAYER 1 WEIGHTS
    // ==================================================

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            layer_1[i][j] +=
                mutation_dist(gen);
        }
    }


    // ==================================================
    // LAYER 1 BIASES
    // ==================================================

    for (int i = 0; i < 16; i++)
    {
        layer_1_bias[i] +=
            mutation_dist(gen);
    }


    // ==================================================
    // LAYER 2 WEIGHTS
    // ==================================================

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            layer_2[i][j] +=
                mutation_dist(gen);
        }
    }


    // ==================================================
    // LAYER 2 BIASES
    // ==================================================

    for (int i = 0; i < 8; i++)
    {
        layer_2_bias[i] +=
            mutation_dist(gen);
    }


    // ==================================================
    // OUTPUT WEIGHTS
    // ==================================================

    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            output_layer[i][j] +=
                mutation_dist(gen);
        }
    }


    // ==================================================
    // OUTPUT BIASES
    // ==================================================

    for (int i = 0; i < 7; i++)
    {
        output_bias[i] +=
            mutation_dist(gen);
    }
}


// ==================================================
// SAVE
//
// Serializes network weights and biases as a raw binary dump
// in declaration order. Platform-dependent (endianness, float size).
// The Evolution system saves 5 trained networks to final_brains.bin.
// ==================================================

bool NeuralNetwork::save(
    std::ostream& out) const
{
    out.write(
        reinterpret_cast<const char*>(
            layer_1
        ),
        sizeof(layer_1)
    );

    out.write(
        reinterpret_cast<const char*>(
            layer_1_bias
        ),
        sizeof(layer_1_bias)
    );

    out.write(
        reinterpret_cast<const char*>(
            layer_2
        ),
        sizeof(layer_2)
    );

    out.write(
        reinterpret_cast<const char*>(
            layer_2_bias
        ),
        sizeof(layer_2_bias)
    );

    out.write(
        reinterpret_cast<const char*>(
            output_layer
        ),
        sizeof(output_layer)
    );

    out.write(
        reinterpret_cast<const char*>(
            output_bias
        ),
        sizeof(output_bias)
    );

    return out.good();
}


// ==================================================
// LOAD
//
// Deserializes network weights and biases from a raw binary dump
// in declaration order. Platform-dependent (endianness, float size).
// Reads networks saved by the Evolution system (e.g., final_brains.bin).
// ==================================================

bool NeuralNetwork::load(
    std::istream& in)
{
    in.read(
        reinterpret_cast<char*>(
            layer_1
        ),
        sizeof(layer_1)
    );

    in.read(
        reinterpret_cast<char*>(
            layer_1_bias
        ),
        sizeof(layer_1_bias)
    );

    in.read(
        reinterpret_cast<char*>(
            layer_2
        ),
        sizeof(layer_2)
    );

    in.read(
        reinterpret_cast<char*>(
            layer_2_bias
        ),
        sizeof(layer_2_bias)
    );

    in.read(
        reinterpret_cast<char*>(
            output_layer
        ),
        sizeof(output_layer)
    );

    in.read(
        reinterpret_cast<char*>(
            output_bias
        ),
        sizeof(output_bias)
    );

    return in.good();
}