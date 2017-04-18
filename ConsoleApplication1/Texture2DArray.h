#pragma once

#include "ppng.h"
#include "util.h"
#include "Texture.h"

//Generic type for 2D array textures. 
class Texture2DArray : public Texture {
public:
	int w, h;
	int numslices;

	Texture2DArray() : Texture(GL_TEXTURE_2D_ARRAY) {}

};

