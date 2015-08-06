#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <SDL.h>

class Vector2D
{
	float x, y;
public:
	Vector2D()
	{
		x = 0, y = 0;
	}
	Vector2D(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	float getX()
	{
		return this->x;
	}
	float getY()
	{
		return this->y;
	}
	void setX(float x)
	{
		this->x = x;
	}
	void setY(float y)
	{
		this->y = y;
	}
  	float operator[](int a)					               // SUBSCRIPT access
	{
		if (a == 0)return this->x;
		else if (a == 1)return this->y;
		else return -1;
	}
	float operator*(Vector2D &b)			               // DOT multiplication
	{
		return this->x*b[0] + this->y * b[1];
	}
	Vector2D operator*(float c)                            // SCALAR multiplication
	{
		Vector2D r;
		r.x = c*this->x;
		r.y = c*this->y;
		return r;
	}
	Vector2D operator*(int c)                              // SCALAR multiplication
	{
		Vector2D r;
		r.x = c*this->x;
		r.y = c*this->y;
		return r;
	}
	void operator=(Vector2D &b)
	{
		this->x = b.x;
		this->y = b.y;
	}
	Vector2D operator+(Vector2D &b)
	{
		Vector2D r;
		r.x = this->x + b.x;
		r.y = this->y + b.y;
		return r;
	}

	Vector2D operator-(Vector2D &b)
	{
		Vector2D r;
		r.x = this->x - b.x;
		r.y = this->y - b.y;
		return r;
	}
	float norm()
	{
		return SDL_pow(SDL_pow(this->x, 2) + SDL_pow(this->y, 2), 1 / 2);
	}
	float squaredNorm() {
		return SDL_pow(this->x, 2) + SDL_pow(this->y, 2);
	}
	float distance(Vector2D &b)
	{
		return SDL_pow(((this->x - b.x)*(this->x - b.x)) + ((this->y - b.y)*(this->y - b.y)), 0.5);
	}
	float distance2(Vector2D &b)
	{
		return ((this->x - b.x)*(this->x - b.x)) + ((this->y - b.y)*(this->y - b.y));
	}
};

#endif