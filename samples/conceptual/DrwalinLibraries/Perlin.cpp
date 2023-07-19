
// This file is part of Drwalin Libraries project
// Copyright (C) 2019 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef PERLIN_CPP
#define PERLIN_CPP

#include "Noise.h"

#include "Perlin.h"

#include <cmath>

Noise & Perlin::GetNoise()
{
	return this->noise;
}

inline float Perlin::FractionPositive( float value )
{
	return value - floor(value);
}

inline float Perlin::Interpolate( float t, float a, float b )
{
	const float PI = 3.141592f;
	float f = (sin((t-0.5f)*PI)+1.0f)*0.5f;
	return (a*f) + (b*(1.0f-f));
}

inline float Perlin::Interpolate( float tx, float ty, float a, float b, float c, float d )
{
	return Perlin::Interpolate( tx, Interpolate(ty,a,b), Interpolate(ty,c,d) );
}

float Perlin::GetScale() const
{
	return float( 1 << (this->octaves) );
}

float Perlin::Value( float _x, float _y )
{
	float value = 0.0f;
	
	int64_t x, y;
	float mult = 1.0f;
	float _exp = 0.7f;
	float sum_mult = 0;
	
	for( int i = 0; i < this->octaves; ++i, mult *= _exp, _exp *= 0.97f )
	{
		x = (int64_t)floor( _x * float(1<<i) );
		y = (int64_t)floor( _y * float(1<<i) );
		
		float a, b, c, d;
		a = this->noise.GetRandomFloat( (x + this->octaveOffsetX[i]), (y + this->octaveOffsetY[i]), -mult, mult );
		b = this->noise.GetRandomFloat( (x + this->octaveOffsetX[i]), (y+1 + this->octaveOffsetY[i]), -mult, mult );
		c = this->noise.GetRandomFloat( (x+1 + this->octaveOffsetX[i]), (y + this->octaveOffsetY[i]), -mult, mult );
		d = this->noise.GetRandomFloat( (x+1 + this->octaveOffsetX[i]), (y+1 + this->octaveOffsetY[i]), -mult, mult );
		
		sum_mult += mult;
		
		value += Perlin::Interpolate( Perlin::FractionPositive(_x*float(1<<i)), Perlin::FractionPositive(_y*float(1<<i)), d, c, b, a );
	}
	
	return value/sum_mult;
}

void Perlin::Seed( uint64_t seed, int octaves )
{
	this->octaves = octaves;
	this->noise.Seed( seed, this->GetScale() );
	if( this->octaves >= Perlin::maxOctaves )
		this->octaves = Perlin::maxOctaves-1;
	if( this->octaves < 1 )
		this->octaves = 1;
	for( int i = 0; i < Perlin::maxOctaves; ++i )
	{
		this->octaveOffsetX[i] = this->noise.GetDirectRandomValue();
		this->octaveOffsetY[i] = this->noise.GetDirectRandomValue();
	}
}

Perlin::Perlin( uint64_t seed, int octaves )
{
	this->Seed( seed, octaves );
}

Perlin::Perlin()
{
	this->Seed( 8528939847, 7 );
}

#endif

