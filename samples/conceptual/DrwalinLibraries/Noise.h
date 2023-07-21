
// This file is part of Drwalin Libraries project
// Copyright (C) 2019 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef NOISE_H
#define NOISE_H

#include <cinttypes>

#include <random>

class Noise
{
private:
	
	std::default_random_engine generator;
	std::uniform_int_distribution < uint64_t > distribution;
	
	const static uint64_t elems = 256;
	uint64_t randX[Noise::elems];
	uint64_t randY[Noise::elems];
	
	uint64_t seed;
	float floatToIntScale;
	
public:
	
	uint64_t GetDirectRandomValue();
	
	uint64_t Rot( uint64_t value, uint64_t t );
	
	uint64_t GetRandom64( uint64_t x, uint64_t y );
	uint32_t GetRandom32( uint64_t x, uint64_t y );
	
	double GetRandomFloat( uint64_t x, uint64_t y );
	double GetRandomFloat( uint64_t x, uint64_t y, double min, double max );
	double GetRandomFloat( int64_t x, int64_t y, double min, double max );
	float Get( float x, float y, float min, float max );
	
	void Seed( uint64_t seed, float floatToIntScale );
	
	Noise( uint64_t seed, float floatToIntScale );
	Noise();
};

#endif

