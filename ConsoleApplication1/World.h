#pragma once

#include "Cube.h"
#include "math3d.h"
#include "Texture.h"
#include "Robot.h"

class World {
public:

	int numrows, numcols;
	vector<string> lines;
	vector<Robot*> robots;
	Cube* cube;
	ImageTexture* bricks;
	ImageTexture* bricks_s;
	ImageTexture* bricks_n;
	ImageTexture* floortex;
	ImageTexture* floor_n;
	//ImageTexture* floor_s;
	ImageTexture* ceiltex;
	ImageTexture* ceil_n;
	ImageTexture* ceil_s;
	float currFrame;

	World() {
		currFrame = 1.f;
		ifstream fp("map.txt");
		if (!fp)
			throw runtime_error("Cannot load map.txt");

		while (true) {
			string s;
			getline(fp, s);
			if (fp.fail())
				break;
			lines.push_back(s);
		}
		numrows = int(lines.size());
		numcols = int(lines[0].size());

		for (unsigned i = 0; i<lines.size(); ++i) {
			for (unsigned j = 0; j<lines[i].size(); ++j) {
				if (lines[i][j] == 'R') {
					lines[i][j] = ' ';
					robots.push_back(new Robot(vec3(float(j * 2), 0, float(i * 2))));
				}
			}
		}
		cube = new Cube();
		bricks = new ImageTexture("assets/T_Brick_Clay_New_D.png");
		bricks_s = new ImageTexture("assets/T_Brick_Clay_Beveled_S.png");
		bricks_n = new ImageTexture("assets/T_Brick_Clay_Beveled_N.png");
		floortex = new ImageTexture("assets/T_CobbleStone_Rough_D.PNG");
		floor_n = new ImageTexture("assets/T_CobbleStone_Rough_N.PNG");
		ceiltex = new ImageTexture("assets/T_Wood_Floor_Walnut_D.PNG");
		ceil_n = new ImageTexture("assets/T_Wood_Floor_Walnut_N.PNG");
		ceil_s = new ImageTexture("assets/T_Wood_Floor_Walnut_M.PNG");
		//bricks = new ImageTexture("bricks2.png");
		//bricks_s = new ImageTexture("bricks2s.png");
		//bricks_n = new ImageTexture("bricks_n.png");
		//floortex = new ImageTexture("floor.png");
		//floortex_s = new ImageTexture("")
		//ceiltex = new ImageTexture("ceiling.png");
	}

	void update(int elapsed) {
		for (auto R : robots)
			R->update(elapsed);
	}

	void draw(Program* prog) {
		
		//prog->setUniform("etex", black);
		////FIXME: We could make this more efficient by building one big mesh...
		prog->setUniform("roughness", 1.0f);
		prog->setUniform("tex", ceiltex);
		prog->setUniform("stex", ceil_s);
		prog->setUniform("ntex", ceil_n);
		for (int r = 0; r<numrows; ++r) {
			for (int c = 0; c<numcols; ++c) {
				prog->setUniform("worldMatrix",
					translation(vec3(float(c * 2), 3, float(r * 2))));
				cube->draw(prog);
			}
		}
		prog->setUniform("tex", floortex);
		prog->setUniform("ntex", floor_n);
		//prog->setUniform("worldMatrix", mat4::identity());
		for (int r = 0; r<numrows; ++r) {
			for (int c = 0; c<numcols; ++c) {
				prog->setUniform("worldMatrix",
					translation(vec3(float(c * 2), -1, float(r * 2))));
				cube->draw(prog);
			}
		}
		prog->setUniform("roughness", 2.0f);
		prog->setUniform("tex", this->bricks);
		prog->setUniform("stex", this->bricks_s);
		prog->setUniform("ntex", this->bricks_n);
		for (int r = 0; r<(int)this->lines.size(); ++r) {
			string& L = this->lines[r];
			for (int c = 0; c<(int)L.size(); ++c) {
				if (L[c] == '*') {
					prog->setUniform("worldMatrix",
						translation(vec3(float(c * 2), 1.0f, float(r * 2))));
					cube->draw(prog);
				}
			}
		}
	}

	void robotDraw(Program* prog) {
		prog->setUniform("ntex", this->bricks_n);
		prog->setUniform("frame", currFrame);
		for (auto R : robots) {
			R->draw(prog);
		}
	}

	void robotUpdate(int elapsed) {
		/*currFrame += 0.8f;
		if (currFrame > 40.f)
			currFrame = 0.f;
		for (auto R : robots) {
			R->update(elapsed);*/
		//}
	}
};