#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;

out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;

// Transformation matrices
uniform mat4 model = mat4(1.0f);
uniform mat4 view;
uniform mat4 proj;


void main()
{
   // crntPos = model * vec3(aPos, 1);
   // Normal = aNormal;
   color = aColor;
   texCoord = aTex;
   
   // gl_Position = model * vec4(aPos, 1.0);
   gl_Position = camMatrix * model * vec4(aPos, 1.0);
   // gl_Position = proj * view * model * vec4(aPos, 1);
   // gl_Position = vec4(aPos, 1);
}