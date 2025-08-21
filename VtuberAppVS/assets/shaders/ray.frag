#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;

uniform float uOffset;
uniform sampler2D myTexture;

void main()
{
   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}