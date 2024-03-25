#ifndef BODY_HPP
#define BODY_HPP

/*
	Massive body class. 
	Defines an object which can exert a gravitational force on its neighbors. 
	Can also absorb mass of other bodies. 
*/
struct body {
private:
	//Body properties. 
	std::string name = "body"; //Name of this body. 
	double m = 1, d = 1; //Mass, density. 
	std::vector<double> x = {0, 0}, dx = {0, 0}; //Position, velocity. 
	bool remove = false; //Is this body flagged for removal? 
	double cr = 1, cg = 1, cb = 1; //Proportions of colour components. 
	unsigned absorbed = 0; //How mony bodies has this body absorbed? 
public:
//Constructors. 
	body(double m0, double d0, std::vector<double> x0, std::vector<double> dx0, std::vector<double> colcompon0, std::string name0) {
		//Handle potential errors. 
		if(x0.size() != 2) x0 = {0.0,0.0}; 
		if(dx0.size() != 2) dx0 = {0.0,0.0}; 
		if(colcompon0.size() != 3) colcompon0 = {1.0,1.0,1.0}; 
		m=m0;  d=d0; x=x0;  dx=dx0; //cr = ic.r;  cg = ic.g;  cb = ic.b; 
		cr = colcompon0[0]; cg = colcompon0[1]; cb = colcompon0[2]; 
		name = name0; 
	}
//Methods. 
	//Properties of this body. 
	std::string getname() { return name; }
	double radius() { return m / d; } 
	double mass() { return m; } 
	double density() { return d; }
	std::vector<double> velocity() { return dx; } 
	double speed() { return sqrt(dx[0]*dx[0] + dx[1]*dx[1]); } //Magnitude of velocity. 
	std::vector<double> position() { return x; } 
	bool flagged() { return remove; } //Is this body flagged for removal? 
	void flag() { remove = true; } //Flag this body for removal. 
	unsigned absorbtions() { return absorbed; }
	//Colour of this body. 
	sf::Color col() {
		//Proportions of colour components. 
		double rprop = cr/(cr+cg+cb); 
		double gprop = cg/(cr+cg+cb); 
		double bprop = cb/(cr+cg+cb); 
		//Determine largest. 
		double max_prop = rprop; 
		if(gprop > max_prop) max_prop = gprop; 
		if(bprop > max_prop) max_prop = bprop; 
		//Scale all such that max_prop is at 1.0. 
		rprop *= 1.0/max_prop; 
		gprop *= 1.0/max_prop; 
		bprop *= 1.0/max_prop; 
		return sf::Color(255 * rprop, 255 * gprop, 255 * bprop, 255); 
	} 
	//Handle collisions, if applicable. 
	void absorb(body* b, double d) {
		if(b->flagged() || d > radius() + b->radius() || b->mass() > m) return; //If not colliding or invalid, do not absorb. 
		b->flag(); //Schedule that object for removal on next tick. 
		cr += b->col().r;  cg += b->col().g;  cb += b->col().b; //Add to proportions of colour components. 
		dx = ktw::scale(1/(m + b->mass()),ktw::sum(ktw::scale(m, dx),ktw::scale(b->mass(),b->velocity()))); //Perform a perfectly inelastic collision. 
		m += b->mass(); //Add the smaller object's mass to the larger object's mass. 
		absorbed++; //Increment absorbtions for this body. 
	}
	//Perform physics calculations on this body. 
	void tick(std::vector<body>* bs, size_t thisindex, double G) {
		for(size_t i = 0; i < (*bs).size(); i++) { //Tick over all other bodies to perform physics. 
			if(i == thisindex) continue; //Do not compute dynamics with self. 
			double distance = ktw::distance(x, (*bs)[i].position()); 
			//Handle collisions. 
			absorb(&(*bs)[i], distance); 
			if((*bs)[i].flagged()) continue; 
			//If no collision, proceed with gravitation. 
			double magnitude = G*(*bs)[i].mass()/(distance*distance); 
			dx = ktw::sum(dx, ktw::scale(-magnitude, ktw::hat(ktw::sum(x, ktw::scale(-1.0, (*bs)[i].position()))))); 
		}
	}
	//Move this body. 
	void move() {
		for(size_t i = 0; i < x.size(); i++) x[i] += dx[i]; 
	}
	//Set the position of this body. 
	void set(std::vector<double> nx) {
		for(size_t i = 0; i < x.size(); i++) x[i] = nx[i]; 	
	}
	//Draw this body. 
	void draw(sf::RenderWindow* w, double s, double cx, double cy) {
		if(radius() * s < 0.5) {
			//draw_cross(cx + s*x[0], cy - s*x[1], 5, col(), w); 
			draw_circle(cx + s*x[0], cy - s*x[1], 1, col(), w); 
		} else {
			draw_circle(cx + s*x[0], cy - s*x[1], s*radius(), col(), w); 
		}
		if(vel) { //If requested, draw velocity arrow. 
			double arrow_scale = 25.0; 
			draw_arrow(cx + s*x[0], cy - s*x[1], cx + s*(x[0] + arrow_scale*dx[0]), cy - s*(x[1] + arrow_scale*dx[1]), col()); 
		}
	}
}; 

#endif