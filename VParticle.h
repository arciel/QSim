#ifndef VPARTICLE_H
#define VPARTICLE_H

#include "Vector2D.h"

class VParticle
{
public:
	int id;		            // Unique ID
	float q;	            // Charge, signed
	int fixed;	            // is the charge fixed in place? = 0 no.

	Vector2D r;	            // Position vector at current time
	Vector2D r_;	        // Position vector at t-1 for verlet int

	Vector2D r_t;           // Stores new calculated position temporarily

	Vector2D v;	            // Velocity vector
	Vector2D v_;	        // Updated velocity

	Vector2D f;	            // Force vector
};

#endif