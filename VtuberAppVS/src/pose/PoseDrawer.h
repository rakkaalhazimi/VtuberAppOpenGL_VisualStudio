#pragma once

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "pose/BlazePose.h"
#include "shader.h"



class PoseDrawer
{
	public:
		GLuint VAO, VBO;

		PoseDrawer();
		void Draw(Shader &shader, Landmark& landmark, int windowWidth, int windowHeight);
};