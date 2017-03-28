#pragma once

#include "math3d.h"
#include "Mesh.h"

class Robot {
public:
	static Mesh* mesh;
	vec3 pos;
	vec3 vel;
	float timeleft;
	float maxtime;

	Robot(vec3 pos) {
		if (mesh == NULL)
			mesh = new Mesh("assets/robot.ms3d.mesh");
		this->pos = pos;
		this->vel = vec3(
			randrange(-0.00001, 0.00001),
			0,
			randrange(-0.00001, 0.00001)
		);
		maxtime = randrange(1000, 2000);
		timeleft = maxtime;
	}
	void update(int elapsed) {
		this->pos = this->pos + timeleft * this->vel;
		timeleft -= elapsed;
		if (timeleft < 0) {
			timeleft = maxtime;
			vel = -vel;
		}
	}
	void draw(Program* prog) {
		prog->setUniform("worldMatrix", translation(this->pos));
		Robot::mesh->draw(prog);
	}
};



