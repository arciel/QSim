// Program.cpp : Defines the entry point for the console application.

//#include "stdafx.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include "VParticle.h"
#include <vector>

/* Bug: 
 *		1. Breaks if + and - charges entered at the exact same place.
 *
 * To-do: 
 *		1. Fix bugs (if any).
 *		2. Implement cutoff distance.
 */

double time, delta;         // Current time and dt (step size).
float nx_q;				    // Charge and ...
int nx_s;				    // ... sign of next particle to be added.
int fixed;			        // Whether or not the charge is fixed in place.
float cutOffDistance;		// Any object beyond this distance from the screen center will be deleted.

Vector2D screenCentre(400, 300);

std::vector<VParticle *> List;        

void verlet_integrate();
void rk4_integrate();

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "Enter cutoff distance: ";
	std::cin >> cutOffDistance;
	nx_q = 1.0;
	nx_s = 1;
	puts("Init SDL...\n");
	SDL_Window *w = nullptr;
	SDL_Renderer *r = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	w = SDL_CreateWindow("q-sim.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
	r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

	bool quit = false;
	time = 0;					// Set the universe's time to zero and begin simulation.
	delta = 1.0 / 60.0;			// Each frame advances the time by delta amount.
	
	printf("NumDrv=%d\n", SDL_GetNumRenderDrivers());
	SDL_RendererInfo inf;                            
	SDL_GetRendererInfo(r, &inf);                    
	                                                 
	printf("Current Renderer = %s\n", inf.name);     
	printf("Current Video Driver = %s\n", SDL_GetCurrentVideoDriver());

	while (!quit)	// GUI Handling.
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) quit = true;
			if (e.type == SDL_MOUSEWHEEL)						// Mouse wheel controls charge magnitude. 
			{
				nx_q += e.wheel.y;
				if (nx_q < 1) nx_q = 1;
				printf("Next charge = %f\n", nx_q);
			}
			if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT)			// Left click, add a new +ve particle to the sim.
				{
					printf("Add charge +%fq at %d %d\n", nx_q, e.button.x, e.button.y);
					List.push_back(new VParticle(List.size(), e.button.x, e.button.y, e.button.x, e.button.y, -nx_q, fixed));
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)   // Right click, add -ve particle to the sim.
				{
					printf("Add charge -%fq at %d %d\n", nx_q, e.button.x, e.button.y);
					List.push_back(new VParticle(List.size(), e.button.x, e.button.y, e.button.x, e.button.y, nx_q, fixed));
				}
				else if (e.button.button == SDL_BUTTON_MIDDLE) // Middle-click toggles fixed masses.
				{
					fixed = 1 - fixed;
					printf("Fixed status is now %d\n", fixed);
				}
			}
		}								
		rk4_integrate();												   // Physics calculations.
		SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);        // Clear screen to white color.
		SDL_RenderClear(r);
		SDL_Rect pos;
		VParticle *p = nullptr;
		for (int i = 0; i < List.size(); i++)
		{   
			if (List[i] != nullptr) 
			{
				p = List[i];
				pos = { p->getR()[0] - 1 * SDL_abs(p->getQ()), p->getR()[1] - 1 * SDL_abs(p->getQ()), 2 * SDL_abs(p->getQ()), 2 * SDL_abs(p->getQ()) };
				(p->getQ() > 0) ? SDL_SetRenderDrawColor(r, 255, 0, 0, SDL_ALPHA_OPAQUE) : SDL_SetRenderDrawColor(r, 0, 0, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(r, &pos);
			}
		}									//^^Depending on the charge's magnitude and polarity, render a box centered at the x,y coordinates.
		SDL_RenderPresent(r);				// Render to the screen.
	}
	SDL_DestroyRenderer(r);					// Clean up allocated SDL resources.
	SDL_DestroyWindow(w);
	SDL_Quit();
	return 0;
}

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

Vector2D rk4_accl(Vector2D r, Vector2D v, float dt, int id, float q)
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

void rk4_integrate()
{
	for (auto &vp : List) //mad cpp11 features! Loop over every particle in the simulation... (Isn't this an STL feature?)
	{
		if (vp->getFixed() == 1) continue; //this charge is fixed in space.

		Vector2D r1 = vp->getR();
		Vector2D v1 = vp->getV();
		Vector2D a1 = rk4_accl(r1, v1, delta, vp->getID(), vp->getQ());

		Vector2D r2 = r1 + v1*(float)(0.5*delta);
		Vector2D v2 = v1 + a1*(float)(0.5*delta);
		Vector2D a2 = rk4_accl(r2, v2, delta / 2, vp->getID(),vp->getQ());
		
		Vector2D r3 = r1 + v2*(float)(0.5*delta);
		Vector2D v3 = v1 + a2*(float)(0.5*delta);
		Vector2D a3 = rk4_accl(r3, v3, delta / 2, vp->getID(), vp->getQ());

		Vector2D r4 = r1 + v3*(float)delta;
		Vector2D v4 = v1 + a3*(float)delta;
		Vector2D a4 = rk4_accl(r4, v4, delta, vp->getID(), vp->getQ());

		Vector2D rf = r1 + (v1 + v2*2.0f + v3*2.0f + v4)*(float)(delta / 6.0f);
		Vector2D vf = v1 + (a1 + a2*2.0f + a3*2.0f + a4)*(float)(delta / 6.0f);

		vp->setR_(rf);
		vp->setV_(vf);
	}
	//Once the simulation has updated for every particle,
	//Update R_ -> R and V_ -> V

	for (auto &update : List)
	{
		update->setR(update->getR_());
		update->setV(update->getV_());
	}
	// And it is done.
	// I have achieved nirvana. - arciel.
}
