#include <cmath>
#include <array>
#include <vector>

#include "VParticle.h"

struct Physics
{
	float delta = 1.f / 60.f;
	std::array<std::vector<float>, 2> force(const ParticleSystem& ps)
	{
		std::vector<float> fx (ps.q.size());
		std::vector<float> fy (ps.q.size());
		for(int i = 0; i != ps.q.size(); i++)
		{
			for(int j = 0; j != ps.q.size(); j++)
			{
				if (i==j) continue;
				float f = ps.q[i] * ps.q[j] / (std::pow(ps.px[i] - ps.px[j],2) + std::pow(ps.py[i] - ps.py[j], 2));
				fx[i] += f * (ps.px[i] - ps.px[j]);
				fy[i] += f * (ps.py[i] - ps.py[j]);
			}
		}
		return {fx, fy};
	}
	void step(ParticleSystem& ps)
	{
		std::array<std::vector<float>, 2> fxy = force(ps);
		for(int i = 0; i != ps.q.size(); i++)
		{
			ps.vx[i] += fxy[0][i] * delta;
			ps.vy[i] += fxy[1][i] * delta;
			ps.px[i] += ps.vx[i] * delta;
			ps.py[i] += ps.vy[i] * delta;
		}
	}
};

#if 0

// Quick and Dirty RK4 implementation.
// Followed GoG's blog, Wikipedia, CodeFlow and Doswa.
// TODO : Refactor, refactor and refactor!
// Eventually, setup a physics manager class that 
// can be initialized with a choice of integrator (RK4, RK4 Adaptive, Verlet, RK2, etc)
// and provides generic functions for the main simulation to call.


// As far as I understand,
// RK4 will require an acceleration function, something which returns the acceleration of
// of a particle slightly into the future, given its position, velocity and a time delta 
// i.e. it wants the accn at t = t + dt. Obviously, impossible to do since accn in the 
// future depends on positions of other particles in the future. To calculate those would require 
// using RK4 itself, which leads to problems.
// So I cheat by assuming that in dt time the other particles haven't really moved.
// Yeah, it's a hack. Sue me.
// --arciel

struct PhysMan //RK4
{
	double delta = 1.0 / 60.0;// Each frame advances the time by delta amount.
	Vector2D rk4_accl(ParticleSystem& List,Vector2D r, Vector2D v, float dt, int id, float q)
	{
		VParticle vpf(id, q, r, v);
		vpf.setR(vpf.getR() + vpf.getV()*dt); // x = x0 + vdt.

		Vector2D netE(0, 0);	// Initial electric field. 
		for (auto &it : List)
		{
			if (it->getID() == vpf.getID()) continue;
			netE = netE + (vpf.getR() - it->getR()) * (it->getQ() / vpf.getR().distance2(it->getR()));
		}
		Vector2D f = netE * vpf.getQ();
		return f; // mass = 1 => acc = force
	}

	void integrate(ParticleSystem& List)
	{
		for (auto &vp : List) //mad cpp11 features! Loop over every particle in the simulation...
		{
			if (vp->getFixed() == 1) continue; //this charge is fixed in space.

			//Vector2D r1 = vp->getR();
			Vector2D v1 = vp->getV();
			Vector2D a1 = rk4_accl(List,r1, v1, delta, vp->getID(), vp->getQ());

			//Vector2D r2 = r1 + v1*(float)(0.5*delta);
			Vector2D v2 = v1 + a1*(float)(0.5*delta);
			Vector2D a2 = rk4_accl(List,r2, v2, delta / 2, vp->getID(), vp->getQ());

			//Vector2D r3 = r1 + v2*(float)(0.5*delta);
			Vector2D v3 = v1 + a2*(float)(0.5*delta);
			Vector2D a3 = rk4_accl(List,r3, v3, delta / 2, vp->getID(), vp->getQ());

			//Vector2D r4 = r1 + v3*(float)delta;
			Vector2D v4 = v1 + a3*(float)delta;
			Vector2D a4 = rk4_accl(List, r4, v4, delta, vp->getID(), vp->getQ());

			//Vector2D rf = r1 + (v1 + v2*2.0f + v3*2.0f + v4)*(float)(delta / 6.0f);
			Vector2D vf = v1 + (a1 + a2*2.0f + a3*2.0f + a4)*(float)(delta / 6.0f);

			vp->setR_(rf);
			vp->setV_(vf);
		}

		for (auto &update : List)
		{
			update->setR(update->getR_());
			update->setV(update->getV_());
		}
	}

};

#endif



















