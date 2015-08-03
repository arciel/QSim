// verlet_integrator.cpp : Defines the entry point for the console application.

#include <stdio.h>
#include <tchar.h>
#include "Vector2D.h"

// Bug: Breaks if + and - charges entered at the exact same place.

class vParticle
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

double time, delta;         // Current time and dt (step size).
int npart;				    // Number of particles

float nx_q;				    // Charge and ...
int nx_s;				    // ... sign of next particle to be added.
int fixed;			        // Whether or not the charge is fixed in place.

vParticle List[128];        // 128 particles max. TODO : Replace with vector<Particle>

// Physics functions
void vector_euler();
void rk4_integrate();
void verlet_integrate();
void rk4_adaptive_integrate();

int _tmain(int argc, _TCHAR* argv[])
{
	npart = 0;
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
	time = 0;					// Set the universe's time to zero and begin simulation
	delta = 1.0 / 60.0;			// Each frame advances the time by delta amount
	
	printf("NumDrv=%d\n", SDL_GetNumRenderDrivers());

	SDL_RendererInfo inf;

	SDL_GetRendererInfo(r, &inf);

	printf("Current Renderer = %s\n", inf.name);
	printf("Current Video Driver = %s\n", SDL_GetCurrentVideoDriver());



	while (!quit)
	{
		//GUI Handling
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
					List[npart].id = npart;
					List[npart].r.setX(e.button.x);
					List[npart].r.setY(e.button.y);
					List[npart].r_.setX(e.button.x);
					List[npart].r_.setY(e.button.y);
					List[npart].q = nx_q;
					List[npart].fixed = fixed;					// TODO
					npart++;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)   // Right click, add -ve particle to the sim.
				{
					printf("Add charge -%fq at %d %d\n", nx_q, e.button.x, e.button.y);
					List[npart].id = npart;
					List[npart].r.setX(e.button.x);
					List[npart].r.setY(e.button.y);
					List[npart].r_.setX(e.button.x);
					List[npart].r_.setY(e.button.y);
					List[npart].q = -nx_q;
					List[npart].fixed = fixed;
					//TODO
					npart++;
				}
				else if (e.button.button == SDL_BUTTON_MIDDLE) // Middle-click toggles fixed masses
				{
					fixed = 1 - fixed;
					printf("Fixed status is now %d\n",fixed);
					
				}
			}
		}

		//Physics calculations

		verlet_integrate(); //Superior

		//Render to screen

		SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE); // Clear screen to white color.
		SDL_RenderClear(r);

		//Loop over and draw the points

		SDL_Rect pos;
		vParticle *p = nullptr;

		for (int i = 0; i < npart; i++)
		{
			p = &List[i];
			pos = { p->r[0] - 1 * SDL_abs(p->q), p->r[1] - 1 * SDL_abs(p->q), 2*SDL_abs(p->q), 2*SDL_abs(p->q) };
			p->q > 0 ? SDL_SetRenderDrawColor(r,255, 0, 0, SDL_ALPHA_OPAQUE) : SDL_SetRenderDrawColor(r,0, 0, 255, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(r, &pos);
		} //^^Depending on the charge's magnitude and polarity, render a box centered at the x,y coordinates
		
		SDL_RenderPresent(r);			// Render to the screen
	}

	SDL_DestroyRenderer(r);				// Clean up allocated SDL resources
	SDL_DestroyWindow(w);
	SDL_Quit();

	return 0;
}

void vector_euler()	 // EXPLICIT EULER INTEGRATIONS
{
	vParticle *vp = nullptr;

	for (int i = 0; i < npart; i++)
	{
		vp = &List[i];

		Vector2D netE(0,0);

		for (int j = 0; j < npart; j++)
		{
			if (vp->id == List[j].id) continue;
			netE = netE + (vp->r - List[j].r) * (List[j].q / vp->r.distance2(List[j].r));
		}

		vp->f = netE * vp->q;
		vp->v = vp->v + vp->f*(float)delta;
		vp->r_ = vp->r + vp->v*(float)delta;
		
	}
	//update positions
	for (int i = 0; i < npart; i++)
	{
		vp = &List[i];
		vp->r = vp->r_;
	}
}

void verlet_integrate()
{
	vParticle *vp = nullptr;

	for (int i = 0; i < npart; i++)					// For each particle in the simulation..
	{
		vp = &List[i];

		if (vp->fixed == 1)continue;

		Vector2D netE(0, 0);

		for (int j = 0; j < npart; j++)				// Calculate the net E field at the particle coords
		{
			if (vp->id == List[j].id) continue;		// Do not include ourselves in the calculation for net E-field
			
			netE = netE + (vp->r - List[j].r) * (List[j].q / vp->r.distance2(List[j].r));
		}
		vp->f = netE * vp->q; //The net force on the particle, F=qE (Doesn't this lool completely non-intimidating?)
		//Here, the accn = force as mass = 1 and gravitational effects are not considered.
		//actually on further consideration, let mass = abs(q) makes for more interesting sims.
		vp->r_t = (vp->r) * 2 - vp->r_ + vp->f*((float)delta*(float)delta / SDL_abs(vp->q));
	}
	//Updates
	for (int i = 0; i < npart; i++)
	{
		vp = &List[i];
		if (vp->fixed == 1)continue;
		vp->r_ = vp->r;
		vp->r = vp->r_t;
	}
}