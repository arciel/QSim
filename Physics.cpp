/*
	Physics manager class.
	Provides various methods (RK4, Parallel RK4,..)
	for solving the force ODE and calculating particle positions.

	The template parameter `n` corresponds to the following
	0 -> RK4
	1 -> Parallel RK4
	2 -> Explicit Euler
	
*/

#include "Physics.h"


template <int n> class PhysMan
{
	void integrate(std::vector<VParticle*> &particles)
	{
		std::cout << "Unimplemented integrator!" << endl;
		std::cout << "See Physics.cpp" << endl;
		cin.get();
		exit(-1);
	}
};

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

template <> class PhysMan < 0 > //RK4
{
private:
	double delta = 1.0 / 60.0;// Each frame advances the time by delta amount.
	Vector2D rk4_accl(std::vector<VParticle*>&List,Vector2D r, Vector2D v, float dt, int id, float q)
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

public:
	void integrate( std::vector<VParticle*> &List)
	{
		for (auto &vp : List) //mad cpp11 features! Loop over every particle in the simulation...
		{
			if (vp->getFixed() == 1) continue; //this charge is fixed in space.

			Vector2D r1 = vp->getR();
			Vector2D v1 = vp->getV();
			Vector2D a1 = rk4_accl(List,r1, v1, delta, vp->getID(), vp->getQ());

			Vector2D r2 = r1 + v1*(float)(0.5*delta);
			Vector2D v2 = v1 + a1*(float)(0.5*delta);
			Vector2D a2 = rk4_accl(List,r2, v2, delta / 2, vp->getID(), vp->getQ());

			Vector2D r3 = r1 + v2*(float)(0.5*delta);
			Vector2D v3 = v1 + a2*(float)(0.5*delta);
			Vector2D a3 = rk4_accl(List,r3, v3, delta / 2, vp->getID(), vp->getQ());

			Vector2D r4 = r1 + v3*(float)delta;
			Vector2D v4 = v1 + a3*(float)delta;
			Vector2D a4 = rk4_accl(List, r4, v4, delta, vp->getID(), vp->getQ());

			Vector2D rf = r1 + (v1 + v2*2.0f + v3*2.0f + v4)*(float)(delta / 6.0f);
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

/*
	Parallel RK4. Uses C++ OpenMP to split the loops among
	multiple threads. The regular method is O(4N^2 + N) and I'm not
	quite sure exactly which loop to parallelize.
*/

template <> class PhysMan < 1 > //RK4
{
private:
		double delta = 1.0 / 60.0;
		Vector2D rk4_accl(std::vector<VParticle*>&List, Vector2D r, Vector2D v, float dt, int id, float q)
		{
			VParticle vpf(id, q, r, v);
			vpf.setR(vpf.getR() + vpf.getV()*dt); // x = x0 + vdt.

			Vector2D netE(0, 0);	// Initial electric field. 
			VParticle *it;
			//#pragma omp parallel for shared(netE)
			for (int i = 0; i<List.size();i++)
			{
				it = List[i];
				if (it->getID() == vpf.getID()) continue;
			//#pragma omp atomic
				netE = netE + (vpf.getR() - it->getR()) * (it->getQ() / vpf.getR().distance2(it->getR()));
			}
			Vector2D f = netE * vpf.getQ();
			return f; // mass = 1 => acc = force
		}

public:
		void integrate(std::vector<VParticle*> &List)
		{
			VParticle *vp;
			#pragma omp parallel for private(vp) shared(List)
			for (int i = 0; i < List.size();i++)
			{
				vp = List[i];
				if (vp->getFixed() == 1) continue; //this charge is fixed in space.

				Vector2D r1 = vp->getR();
				Vector2D v1 = vp->getV();
				Vector2D a1 = rk4_accl(List, r1, v1, delta, vp->getID(), vp->getQ());

				Vector2D r2 = r1 + v1*(float)(0.5*delta);
				Vector2D v2 = v1 + a1*(float)(0.5*delta);
				Vector2D a2 = rk4_accl(List, r2, v2, delta / 2, vp->getID(), vp->getQ());

				Vector2D r3 = r1 + v2*(float)(0.5*delta);
				Vector2D v3 = v1 + a2*(float)(0.5*delta);
				Vector2D a3 = rk4_accl(List, r3, v3, delta / 2, vp->getID(), vp->getQ());

				Vector2D r4 = r1 + v3*(float)delta;
				Vector2D v4 = v1 + a3*(float)delta;
				Vector2D a4 = rk4_accl(List, r4, v4, delta, vp->getID(), vp->getQ());

				Vector2D rf = r1 + (v1 + v2*2.0f + v3*2.0f + v4)*(float)(delta / 6.0f);
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





















