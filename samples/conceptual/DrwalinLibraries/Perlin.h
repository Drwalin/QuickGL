
// This file is part of Drwalin Libraries project
// Copyright (C) 2019 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef PERLIN_H
#define PERLIN_H

#include <cinttypes>

#include "Noise.h"

class Perlin
{
private:
	
	Noise noise;
	int octaves;
	
	const static int maxOctaves = 60;
	int64_t octaveOffsetX[Perlin::maxOctaves];
	int64_t octaveOffsetY[Perlin::maxOctaves];
	
	float GetScale() const;
	
public:
	
	Noise & GetNoise();
	
	inline static float FractionPositive( float value );
	inline static float Interpolate( float t, float a, float b );
	inline static float Interpolate( float tx, float ty, float a, float b, float c, float d );
	float Value( float _x, float _y );
	
	void Seed( uint64_t seed, int octaves );
	
	Perlin( uint64_t seed, int octaves );
	Perlin();
};

#endif

