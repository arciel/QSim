#pragma once

#include <vector>

struct ParticleSystem
{
	/* Position, velocity and force. */
	std::vector<float> px, py;
	std::vector<float> vx, vy;
	/* Charge */
	std::vector<float> q;
	std::vector<int> is_fixed;

	void push_back(float ppx, float ppy, float pq, int pis_fixed)
	{
		px.push_back(ppx); py.push_back(ppy);
		vx.push_back(0.); vy.push_back(0.);
		q.push_back(pq); is_fixed.push_back(pis_fixed);
	}
};
