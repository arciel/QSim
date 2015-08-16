// Program.cpp : Defines the entry point for the console application.


#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <SDL.h>

#include "VParticle.h"
#include <vector>


#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib,"SDL2.lib")


/* Bugs: 
 *		1. Breaks if + and - charges entered at the exact same place.
 *
 * To-do List: 
 *		1. Fix bugs (if any).
		   Draw cartesian grid.
		   Implement particle "tracking", i.e. show particle stats on screen.
		   
 *		2. Custom Properties.
 *      3. Custom, user defined force function.
 */

int SC_WIDTH = 800;
int SC_HEIGHT = 600;

double delta; // Current time and dt (step size).
float nx_q;	  // Charge and ...
int nx_s;	  // ... sign of next particle to be added.
int fixed;	  // Whether or not the charge is fixed in place.


std::vector<VParticle *> List;        

void rk4_integrate();
void load_config_file();

int _tmain(int argc, _TCHAR* argv[])
{

	nx_q = 1.0;
	nx_s = 1;
	puts("Init SDL...\n");
	SDL_Window *w = nullptr;
	SDL_Renderer *r = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	w = SDL_CreateWindow("q-sim.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SC_WIDTH, SC_HEIGHT, 0);
	r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

	bool quit = false;
	
	delta = 1.0 / 60.0;			// Each frame advances the time by delta amount.

	load_config_file();


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
			/*
			SDL's origin is at the top-left of the onscreen window. X increments to the right and Y increments downwards.
			We want our origin to be at the centre of the screen, so we shift the X and Y coords of particles as they get added.
			*/
			if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT)			// Left click, add a new +ve particle to the sim.
				{
					printf("Add charge +%fq at %d %d\n", nx_q, e.button.x - (SC_WIDTH/2), e.button.y - (SC_HEIGHT/2));
					List.push_back(new VParticle(List.size(), e.button.x - (SC_WIDTH / 2), e.button.y - (SC_HEIGHT / 2), e.button.x - (SC_WIDTH / 2), e.button.y - (SC_HEIGHT / 2), nx_q, fixed));
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)   // Right click, add -ve particle to the sim.
				{
					printf("Add charge -%fq at %d %d\n", nx_q, e.button.x - (SC_WIDTH / 2), e.button.y - (SC_HEIGHT / 2));
					List.push_back(new VParticle(List.size(), e.button.x - (SC_WIDTH / 2), e.button.y - (SC_HEIGHT / 2), e.button.x - (SC_WIDTH / 2), e.button.y - (SC_HEIGHT / 2), -nx_q, fixed));
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
		
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(r, 0, 0, 0, 50); //Render the lines at lower alpha value, so they are less distracting.
		for (int x = 0; x < SC_WIDTH; x += 25)
			SDL_RenderDrawLine(r, x, 0, x, SC_HEIGHT);
		for (int y = 0; y < SC_HEIGHT; y += 25)
			SDL_RenderDrawLine(r, 0, y, SC_WIDTH, y);
		//^^Draw a basic cartesian coordinate grid.
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
		SDL_Rect pos;
		for (auto &it : List)
		{
			pos.x = (it->getR()[0] + (SC_WIDTH / 2)) - (it->getQ()/2);  //Shift particle coordinates back, so that SDL can draw them
			pos.y = it->getR()[1] + (SC_HEIGHT / 2) - (it->getQ()/2); //Particles are rendered as squares, with width = |charge|
			pos.w = SDL_abs(it->getQ());
			pos.h = SDL_abs(it->getQ());
			
			SDL_SetRenderDrawColor(r, it->getQ() > 0 ? 255 : 0, 0, it->getQ() > 0 ? 0 : 255,SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(r, &pos);
			
			//^^Depending on the charge's magnitude and polarity, render a square centered at the x,y coordinates.
		}
		
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
	for (auto &vp : List) //mad cpp11 features! Loop over every particle in the simulation...
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
}

void load_config_file()
{
	//qsim.cfg format

	/*
	width=800
	height=600
	n=3
	//p=q x y v_x v_y fixed
	*/
	FILE *fp;
	fopen_s(&fp,"qsim.cfg", "r");
	fscanf_s(fp, "width=%d\n", &SC_WIDTH);
	fscanf_s(fp, "height=%d\n", &SC_HEIGHT);
	int n;
	fscanf_s(fp, "n=%d\n", &n);
	printf("Config File:\n Width=%d\nHeight=%d\nnPart=%d\n", SC_WIDTH, SC_HEIGHT, n);
	float q, x, y, vx, vy;
	int fixed;
	VParticle *vp = nullptr;
	for (int i = 0; i < n; i++)
	{
		fscanf_s(fp, "p=%f %f %f %f %f %d\n", &q, &x, &y, &vx, &vy, &fixed);
		printf("Loaded Particle : %f %f %f %f %f %d\n", q, x, y, vx, vy, fixed);
		vp = new VParticle(List.size(), x, y, x, y, q, fixed);
		vp->setV({ vx, vy });
		List.push_back(vp);
	}
	fclose(fp);
}