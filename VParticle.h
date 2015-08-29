#ifndef VPARTICLE_H
#define VPARTICLE_H

#include "Vector2D.h"

class VParticle
{
	int id;		            // Unique ID
	float q;	            // Charge, signed
	int fixed;	            // is the charge fixed in place? = 0 no.
	Vector2D r;	            // Position vector at current time
	Vector2D r_;	        // Position vector at t-1 for verlet_integrate()
	Vector2D r_t;           // Stores new calculated position temporarily
	Vector2D v;	            // Velocity vector
	Vector2D v_;	        // Updated velocity
	Vector2D f;	            // Force vector

public:
	VParticle() { }
	VParticle(int id, float x, float y, float x_, float y_, float charge, int fixed)
	{
		this->id = id;
		this->r.setX(x); this->r.setY(y);
		this->r_.setX(x_); this->r_.setY(y_);
		this->q = charge;
		this->fixed = fixed;
	}
	VParticle(int id, float charge, Vector2D r, Vector2D v) //I am ashamed.
	{
		this->id = id;
		this->q = charge;
		this->r = r;
		this->v = v;
	}
	~VParticle() { }
	int getID() 
	{
		return this->id;
	}
	float getQ() 
	{
		return this->q;
	}
	Vector2D getR() 
	{
		return this->r;
	}
	Vector2D getR_()
	{
		return this->r_;
	}
	Vector2D getR_t() 
	{
		return this->r_t;
	}
	Vector2D getV() 
	{
		return this->v;
	}
	Vector2D getV_() 
	{
		return this->v_;
	}
	Vector2D getF() 
	{
		return this->f;
	}
	int getFixed()
	{
		return this->fixed;
	}
	void setID(int ID)
	{
		this->id = ID;
	}
	void setQ(float charge)
	{
		this->q = charge;
	}
	void setR(Vector2D R)
	{
		this->r = R;
	}
	void setR_(Vector2D R_)
	{
		this->r_ = R_;
	}
	void setR_t(Vector2D R_t)
	{
		this->r_t = R_t;
	}
	void setV(Vector2D V) 
	{
		this->v = V;
	}
	void setV_(Vector2D V_) 
	{
		this->v_ = V_;
	}
	void setF(Vector2D F) 
	{
		this->f = F;
	}
	int setFixed() 
	{
		return this->fixed;
	}
};

#endif