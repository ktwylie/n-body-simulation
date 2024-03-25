/*
<header>

AUTHOR: Kyle T. Wylie
EST: <date>
*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <array>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <iomanip>
#include <functional>
#include <complex>
#include <thread>
#include <regex>
#include <mutex>

#include "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwutil.hpp"
#include "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwgen.hpp"
#include "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwmath.hpp"

#include "utils.hpp"

//Parameters. 
bool next_tick = true; //Advance to the next tick? 
bool screensaver = true; //Running in "screensaver" mode? 
bool trails = true; //Draw trails? 
bool vel = false; //Draw velocity arrows? 

#include "entities/body.hpp"
#include "entities/universe.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Pseudo-random number generator & initial seed. 
const int rng_seed = (uint64_t)time(0)+((uint64_t)time(0)<<32); 
//ktw::LLCAPRNG_B01357_S02468 rng(rng_seed); 
ktw::llcaprng2 rng(rng_seed); 

std::mutex simutex; 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Drawing parameters. 
const double s0 = 0.2, cx0 = width/2, cy0 = height/2; 
const double ds = 1.1, dx = 10, dy = 10; 
double s = s0, cx = cx0, cy = cy0; 

//Generation constants. 
const double gen_r = 5000, max_mass = 10, max_vel = 4.5; 

//Is an object being placed into the scene and how? 
bool placing = false; 
double place_x1 = 0.0, place_y1 = 0.0; 

//Universe. 
universe u(10); 

//Coordinate conversions. 
double window_to_uni(double x, double s, double c) {
	return (x - c) / s; 
}
double uni_to_window(double x, double s, double c) {
	return s*x + c; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Setup, run once at start of program. 
void init() {
	trails = !trails; 
	u.clear(); 
	double r, g, b; 
	for(unsigned i = 0; i < 100; i++) { //Add smaller bodies. 
		r = 10.0 * fabs(rng.next<double>()); 
		g = 10.0 * fabs(rng.next<double>()); 
		b = 10.0 * fabs(rng.next<double>()); 
		u.add(body(fabs(rng.next<double>())*max_mass, 1, {rng.next<double>()*gen_r,rng.next<double>()*gen_r}, {rng.next<double>()*max_vel,rng.next<double>()*max_vel}, {r,g,b}, "Asteroid " + ktw::str(i))); 
	}
	for(unsigned i = 0; i < 20; i++) { //Add larger bodies. 
		r = 10.0 * fabs(rng.next<double>()); 
		g = 10.0 * fabs(rng.next<double>()); 
		b = 10.0 * fabs(rng.next<double>()); 
		u.add(body(5.0*fabs(rng.next<double>())*max_mass + 2.5*max_mass, 2, {rng.next<double>()*gen_r,rng.next<double>()*gen_r}, {rng.next<double>()*max_vel,rng.next<double>()*max_vel}, {r,g,b}, "Planet " + ktw::str(i))); 
	}
	u.add(body(1000, 5, {0,0}, {0,0}, {10000.0,10000.0,10000.0}, "Main Star")); //Add "sun". 
	trails = !trails; 
}

//Perform these actions each tick. 
void tick(sf::RenderWindow* w) { //AN: Optional performance mode which used distance or mass to rule out insignificant bodies? 
	simutex.lock(); //AN<15 Oct. '21>: This is probably the easiest way to fix the concurrency issues we were having. Finally got around to it. 

	if(next_tick) {
		u.tick(1); 
		//u.inflate(1.00001); 
		if(screensaver && t % (unsigned long long) (40 * (tps + 1)) == 0) init(); 
	}

	simutex.unlock(); 
}

//Draw a single frame. 
void frame(sf::RenderWindow* w) {
	simutex.lock(); 

	u.draw(w, s, cx, cy); 
	//Draw placement arrow if applicable. 
	if(placing) draw_arrow(mx(), my(), place_x1, place_y1, sf::Color::Green); 
	//Draw diagnostic codes. 
	if(!next_tick) {
		draw_string("Paused (p)", 10, 30, sf::Color::Green); 
	} else {
		draw_string("Paused (p)", 10, 30, sf::Color::Red); 
	}
	if(screensaver) {
		draw_string("Screensaver (s)", 10, 50, sf::Color::Green); 
	} else {
		draw_string("Screensaver (s)", 10, 50, sf::Color::Red); 
	}
	if(trails) {
		draw_string("Trails (t)", 10, 70, sf::Color::Green); 
	} else {
		draw_string("Trails (t)", 10, 70, sf::Color::Red); 
	}
	if(vel) {
		draw_string("Velocity Vectors (v)", 10, 90, sf::Color::Green); 
	} else {
		draw_string("Velocity Vectors (v)", 10, 90, sf::Color::Red); 
	}
	//Draw diagnostic information. 
	draw_string(ktw::str(u.mass()) + " kg", 10, height - 30, sf::Color::White); 
	draw_string(ktw::str(u.count()) + " bodies", 10, height - 50, sf::Color::White); 
	draw_string(ktw::str(u.ticks()) + " elapsed seconds", 10, height - 70, sf::Color::White); 

	simutex.unlock(); 
}

//Event handling function.
void eventhandle(sf::RenderWindow* w, sf::Event event) {
	while(w->pollEvent(event)) {
		switch(event.type) {
			case sf::Event::Closed:
				w->close();
				break;
			case sf::Event::KeyPressed:
				if(event.key.code == sf::Keyboard::Up) {
					cy += dy; 
				} else if(event.key.code == sf::Keyboard::Down) {
					cy -= dy; 
				} else if(event.key.code == sf::Keyboard::Left) {
					cx += dx; 
				} else if(event.key.code == sf::Keyboard::Right) {
					cx -= dx;
				} else if(event.key.code == sf::Keyboard::Equal) { //"+"
					s *= ds; 
				} else if(event.key.code == sf::Keyboard::Hyphen) { //"-"
					s /= ds; 
				} else if(event.key.code == sf::Keyboard::P) { //Pause. 
					next_tick = !next_tick; 
				} else if(event.key.code == sf::Keyboard::C) { //Clear & reset. 
					u.clear(); 
				} else if(event.key.code == sf::Keyboard::R) { //Randomise again. 
					init(); 
				} else if(event.key.code == sf::Keyboard::S) { //Toggle "screensaver mode". 
					screensaver = !screensaver; 
				} else if(event.key.code == sf::Keyboard::Num0) { //Reset scale & center. 
					s = s0; 
					cx = cx0;
					cy = cy0; 
				} else if(event.key.code == sf::Keyboard::Num9) { //Reset scale and recenter at barycenter. 
					//AN: How to implement this, this doesn't seem to work...
					//s = s0; 
					//cx = s*u.center_of_mass()[0] + cx; 
					//cy = -s*u.center_of_mass()[1] + cy; 
				} else if(event.key.code == sf::Keyboard::T) { //Toggle trails. 
					trails = !trails; 
				} else if(event.key.code == sf::Keyboard::V) { //Toggle velocity arrows. 
					vel = !vel; 
				}
				break; 
			case sf::Event::MouseButtonPressed:
				if(event.mouseButton.button == sf::Mouse::Left) { //Handle placement of bodies. 
					if(!placing) {
						placing = true; 
						place_x1 = mx(); 
						place_y1 = my(); 
					} else {
						placing = false; 
						double r = 10.0 * fabs(rng.next<double>()), g = 10.0 * fabs(rng.next<double>()), b = 10.0 * fabs(rng.next<double>()); 
						if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { //Spawn a bigger body. 
							u.add(body(5.0*fabs(rng.next<double>())*max_mass + 2.5*max_mass, 2, {window_to_uni(place_x1, s, cx), -window_to_uni(place_y1, s, cy)}, {0.1*(place_x1 - mx()), -0.1*(place_y1 - my())}, {r,g,b}, "User-Planet")); 
						} else if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { //Spawn a normal "star". 
							u.add(body(750*fabs(rng.next<double>()) + 500, 5, {window_to_uni(place_x1, s, cx), -window_to_uni(place_y1, s, cy)}, {0.1*(place_x1 - mx()), -0.1*(place_y1 - my())}, {1000*r,1000*g,1000*b}, "User-Star")); 
						} else { //Spawn a smaller body. 
							u.add(body(fabs(rng.next<double>())*max_mass, 1, {window_to_uni(place_x1, s, cx), -window_to_uni(place_y1, s, cy)}, {0.1*(place_x1 - mx()), -0.1*(place_y1 - my())}, {r,g,b}, "User-Asteroid")); 
						}
					}
				} else if(event.mouseButton.button == sf::Mouse::Right) { //Handle erasure of bodies.  
					double mxu = window_to_uni(mx(), s, cx); 
					double myu = -window_to_uni(my(), s, cy); 
					std::vector<body> b = u.contents(); 
					for(size_t i = 0; i < b.size(); i++) {
						double distance = sqrt((mxu - b[i].position()[0])*(mxu - b[i].position()[0]) + (myu - b[i].position()[1])*(myu - b[i].position()[1])); 
						if(distance < b[i].radius()) u.erase(i); 
					}
				}
				break; 
			default:
				break;
		}; 
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Graphical rendering thread.
void renderthread(sf::RenderWindow* w) {
	font.loadFromFile("tnr.ttf");
	text.setFont(font);
	text.setCharacterSize(fontsize);

	//"Trails" semi-transparent "clear" rectangle. 
	sf::RectangleShape trails_clear; 
	trails_clear.setFillColor(sf::Color(0, 0, 0, 20)); 
	trails_clear.setPosition(0, 0); 
	trails_clear.setSize({(float) width, (float) height}); 

	while(w->isOpen()) {
		//Draw trails or don't. 
		if(trails) {
			w->draw(trails_clear); 
		} else {
			w->clear(sf::Color::Black); //Clear. 
		}
		//Draw all buttons. 
		for(size_t i = 0; i < buttons.size(); i++) buttons[i].draw(w); 

		//Draw frame. 
		frame(w); 

		//Draw FPS. 
		draw_string(ktw::str((int) fps) + " fps, " + ktw::str((int) tps) + " tps", 10, 10, sf::Color::White, w); 
		//Initiate frame-draw. 
		w->display(); 
		//Wait some time per frame.
		ktw::wait(framedelay); 
		frames_since_last++; 
		f++; 
	}
}

//Main program entry point.
int main() {
	srand(time(NULL));
	sf::RenderWindow w(sf::VideoMode(width, height), "Wisps 3", sf::Style::Default);
	mw = &w; 
	w.setActive(false);

	init(); //Run any initial setup that must be done. 

	sf::Thread rt(&renderthread, &w);
	rt.launch();
	std::clock_t fps_t0 = std::clock();  //Timestamp for FPS. 
	while(w.isOpen()) {
		//Handle misc events.
		sf::Event event;
		eventhandle(&w, event);
		//Check if any buttons are being clicked. 
		for(size_t i = 0; i < buttons.size(); i++) buttons[i].click(&w); 

		//Game events. 
		tick(&w); 

		//Calculate frames-per-second & ticks-per-second. 
		if(ktw::dur(fps_t0) >= fps_calc_delay) { //Every so often. 
			fps = (long double) frames_since_last / ktw::dur(fps_t0); //Compute FPS. 
			tps = (long double) ticks_since_last / ktw::dur(fps_t0); //Compute TPS. 
			frames_since_last = 0; //Reset frames-since-last check. 
			ticks_since_last = 0; 
			fps_t0 = std::clock(); //Reset timer. 
		}

		//Wait some time per tick. 
		ktw::wait(tickdelay); 
		ticks_since_last++; 
		t++; 
	}

	//Clean up and report normal exit. 
	return 0;
}