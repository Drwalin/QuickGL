
// This file is part of Drwalin Libraries project
// Copyright (C) 2019 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef NOISE_CPP
#define NOISE_CPP

#include "Noise.h"

#include <algorithm>

uint64_t Noise::GetDirectRandomValue()
{
	return this->distribution( this->generator );
}

uint64_t Noise::Rot( uint64_t value, uint64_t t )
{
	return ( value << (t&63) ) | ( value >> (64-(t&63)) );
}

uint64_t Noise::GetRandom64( uint64_t x, uint64_t y )
{
	uint64_t X = 0, Y = 0;
	for( int i = 0; i < 8; ++i )
	{
		X += x;
		X ^= this->randX[(x>>(i*8))&255];
		X ^= Rot(y,randY[X&255]);
		
		Y += y;
		Y ^= this->randY[(y>>(i*8))&255];
		Y ^= Rot(x,randX[Y&255]);
	}
	
	uint64_t s = X ^ Y;
	return s;
}

uint32_t Noise::GetRandom32( uint64_t x, uint64_t y )
{
	uint64_t t = GetRandom64( x, y );
	uint32_t r = uint32_t(t >> 32) ^ uint32_t(t);
	return r;
}

double Noise::GetRandomFloat( uint64_t x, uint64_t y )
{
	return (double(GetRandom32(x,y)) / double(~uint32_t(0)));
}

double Noise::GetRandomFloat( uint64_t x, uint64_t y, double min, double max )
{
	double value = ( double(GetRandom32(x,y)) * (max-min) ) / double(~uint32_t(0));
	return value + min;
}

double Noise::GetRandomFloat( int64_t x, int64_t y, double min, double max )
{
	double value = ( double(GetRandom32(uint64_t(x),uint64_t(y))) * (max-min) ) / double(~uint32_t(0));
	return value + min;
}

float Noise::Get( float x, float y, float min, float max )
{
	return GetRandomFloat( int64_t(x*this->floatToIntScale+0.5f), int64_t(y*this->floatToIntScale+0.5f), (double)min, (double)max );
}

void Noise::Seed( uint64_t seed, float floatToIntScale )
{
	this->seed = seed;
	this->floatToIntScale = floatToIntScale;
	
	this->generator = std::default_random_engine( this->seed );
	this->distribution = std::uniform_int_distribution< uint64_t >( 0, ~uint64_t(0) );
	
	for( int i = 0; i < Noise::elems; ++i )
	{
		this->randX[i] = this->GetDirectRandomValue();
		this->randX[i] |= 1;
		this->randY[i] = this->GetDirectRandomValue();
		this->randY[i] |= 1;
	}
}


Noise::Noise( uint64_t seed, float floatToIntScale )
{
	this->Seed( seed, floatToIntScale );
}

Noise::Noise()
{
	this->floatToIntScale = 1.0f;
}

#endif

