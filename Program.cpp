// Program.cpp : Defines the entry point for the console application.

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <SDL.h>

#include "VParticle.h"
#include "Vector2D.h"
#include "Physics.cpp"



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

float nx_q;	  // Charge and ...
int nx_s;	  // ... sign of next particle to be added.
int fixed;	  // Whether or not the charge is fixed in place.

std::vector<VParticle *> List;        

void load_config_file();

int _tmain(int argc, _TCHAR* argv[])
{
	nx_q = 1.0;
	nx_s = 1;
	std::cout << "Init SDL...\n";
	SDL_Window *w = nullptr;
	SDL_Renderer *r = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	w = SDL_CreateWindow("q-sim.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SC_WIDTH, SC_HEIGHT, 0);
	r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

	bool quit = false;
	
	load_config_file();

	PhysMan<1> p;

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
		p.integrate(List);									   // Physics calculations.
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


void load_config_file()
{
	/*  qsim.cfg format
	*	width=800
	*	height=600
	*	n=3
	*	p=q x y v_x v_y fixed
	*/
	std::fstream f;
	f.open("qsim.cfg");
	std::string line;
	int n;
	while (getline(f, line, '\n')) 
	{
		int found = line.find("width=");
		if (found != std::string::npos) 
		{
			SC_WIDTH = atoi(line.substr(found + 6).c_str());
			continue;
		}
		found = line.find("height=");
		if (found != std::string::npos) 
		{
			SC_HEIGHT = atoi(line.substr(found + 7).c_str());
			continue;
		}
		found = line.find("n=");
		if (found != std::string::npos) 
		{
			n = atoi(line.substr(found + 2).c_str());
			std::stringstream ss;
			float q, x, y, v_x, v_y;
			int fixed;
			for (int i = 0; i < n; i++) 
			{
				VParticle *vp = nullptr;
				getline(f, line, '\n');
				found = line.find("p=");
				ss << line.substr(found + 2);
				ss >> q >> x >> y >> v_x >> v_y >> fixed;
				vp = new VParticle(List.size(), x, y, x, y, q, fixed);
				vp->setV({ v_x, v_y });
				List.push_back(vp);
				ss.str("");
				ss.clear();
			}
			break;
		}
	}
	f.close();
}
