#ifndef UNIVERSE_HPP
#define UNIVERSE_HPP

/*
	Defines a system of interacting bodies. 
	Handles all calculations on them. 
*/
class universe {
private: 
//Private fields. 
	unsigned t = 0; //Time elapsed since start of simulation. 
	std::vector<body> bodies; //List of all bodies in the simulation. 
	double m; //Mass of the universe. 
	//Fundamental constants. 
	double G; //Gravitational constant. 
//Private methods. 
	//Compute mass of this universe. 
	void compute_mass_properties() {
		double m1 = 0.0; 
		for(size_t i = 0; i < bodies.size(); i++) m1 += bodies[i].mass(); 
		m = m1; 
	}
public: 
//Constructors. 
	universe(double G0) {
		G = G0; 
	}
//Methods. 
	//Time (internal ticks) elapsed since beginning of simulation. 
	double ticks() {
		return (double) t; 
	}
	
	//Advance this universe by one timestep. 
	void tick(unsigned threads) {
		if(threads <= 1) { //Single threaded method. 
			std::vector<body> bodies_tmp = bodies; //Temporary copy of bodies, for local ticking. 
			for(size_t i = 0; i < bodies_tmp.size(); i++) {
				bodies_tmp[i].tick(&bodies_tmp, i, G); //First compute all forces. 
				if(bodies_tmp[i].flagged()) { //Remove objects flagged for absorbtion. 
					bodies_tmp.erase(bodies_tmp.begin() + i); //AN: This operation RESIZES 'bodies', which is dangerous in a concurrent application. 
					i--; 
				}
			}
			bodies = bodies_tmp; //Replace true bodies vector with temporary local copy. 
			//Then perform all movements & uniformly translate bodies if scheduled. 
			for(size_t i = 0; i < bodies.size(); i++) {
				bodies[i].move(); //Move bodies. 
			}
			//Increment elapsed time. 
			t++; 
		} else { //Multithreaded method (expirimental). 
			//...
		}
	}
	//Inflate the scale of distances in this universe. 
	void inflate(double s) {
		for(size_t i = 0; i < bodies.size(); i++) bodies[i].set(ktw::scale(s, bodies[i].position())); 
	}
	//Add body to this universe. 
	void add(body b) {
		bodies.push_back(b); 
		compute_mass_properties(); 
	}
	//Remove a body from this universe. 
	void erase(size_t i) {
		bodies.erase(bodies.begin() + i); 
		compute_mass_properties(); 
	}
	//Return the current state of the bodies of this universe. 
	std::vector<body> contents() {
		return bodies; 
	}
	//Clear this universe of all bodies. 
	void clear() {
		t = 0; //Reset time. 
		bodies.clear(); 
		compute_mass_properties(); 
	}
	//Retrieve mass of this universe. 
	double mass() {
		return m; 
	}
	//Count of distinct bodies in simulation. 
	size_t count() {
		return bodies.size(); 
	}
	//Draw this universe to screen. 
	void draw(sf::RenderWindow* w, double s, double cx, double cy) {
		std::vector<body> b = bodies; //Get local snapshot of 'bodies' that won't be changed due to concurrency. 
		//Draw all bodies. 
		double x_dist, y_dist, mouse_distance; 
		const double offset = 20; 
		for(size_t i = 0; i < b.size(); i++) {
			if(b[i].flagged()) continue; //Do not attempt to draw if flagged. 
			//Draw it's bounding sphere (circle). 
			b[i].draw(w, s, cx, cy); 
			//If mouse is hovering over this body, draw diagnostic tooltips. 
			x_dist = (mx()-cx)/s - b[i].position()[0]; 
			y_dist = -(my()-cy)/s - b[i].position()[1]; 
			mouse_distance = sqrt(x_dist*x_dist + y_dist*y_dist); 
			if(mouse_distance < b[i].radius()) {
				draw_string(b[i].getname(), mx() + offset, my(), b[i].col()); 
				draw_string(" - " + ktw::str<double>(b[i].mass()) + " kg", mx() + offset, my() + offset, b[i].col()); 
				draw_string(" - " + ktw::str<double>(b[i].speed()) + " m/s", mx() + offset, my() + 2*offset, b[i].col()); 
				draw_string(" - " + ktw::str<unsigned>(b[i].absorbtions()) + " collisions", mx() + offset, my() + 3*offset, b[i].col()); 
			}
			//Draw line from this body to the other body who had the greatest effect on it this tick. 
			/*
				Implement me? 
			*/
		}
		//Draw origin point. 
		draw_x(cx, cy, 10, sf::Color::White, w); 
	}
}; 

#endif