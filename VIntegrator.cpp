// VIntegrator.cpp : Defines the entry point for the console application.

#define _CRT_ARGUMENT_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include "VParticle.h"
#include <vector>

/* Bug: 
 *		1. Breaks if + and - charges entered at the exact same place.
 *		2. Asymmetry observed (charge 1 gets attracted but 2 does not).
 *
 * To-do: 
 *		1. Fix bugs (if any).
 */

double time, delta;         // Current time and dt (step size).
float nx_q;				    // Charge and ...
int nx_s;				    // ... sign of next particle to be added.
int fixed;			        // Whether or not the charge is fixed in place.
float cutOffDistance;		// Any object beyond this distance from the screen center will be deleted.

Vector2D screenCentre(400, 300);

std::vector<VParticle *> List;        

void verlet_integrate();

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
		verlet_integrate();											// Physics calculations.
		SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE); // Clear screen to white color.
		SDL_RenderClear(r);
		SDL_Rect pos;
		VParticle *p = nullptr;
		for (int i = 0; i < List.size(); i++)
		{   
			if (List[i] != nullptr) {
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

void verlet_integrate()
{
	VParticle *vp = nullptr;
	for (int i = 0; i < List.size(); i++)							// For each particle in the simulation..
	{
		if (List[i] != nullptr) {
			vp = List[i];
			if (vp->getFixed() == 1) continue;
			Vector2D netE(0, 0);
			for (int j = 0; j < List.size(); j++) { 				// Calculate the net E field at the particle coords.
				if (List[j] != nullptr) {
					if (vp->getID() == List[j]->getID()) continue;	// Do not include ourselves in the calculation for net E-field.
					netE = netE + (vp->getR() - List[j]->getR()) * (List[j]->getQ() / vp->getR().distance2(List[j]->getR()));
				}
			}
			vp->setF(netE * vp->getQ()); //The net force on the particle, F=qE (Doesn't this lool completely non-intimidating?).
			// Here, the accn = force as mass = 1 and gravitational effects are not considered.

			vp->setR_t((vp->getR()) * 2 - vp->getR_() + vp->getF()*((float)delta*(float)delta / SDL_abs(vp->getQ())));

			if (vp->getR_t().squaredDistance(screenCentre) >= cutOffDistance*cutOffDistance) {
				delete List[i];
				List[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < List.size(); i++)
	{
		if (List[i] != nullptr) {
			vp = List[i];
			if (vp->getFixed() == 1) {}
			else {
				vp->setR_(vp->getR());
				vp->setR(vp->getR_t());
			}
		}
	}
}