#ifndef TEXT_RENDERER_H_CLASS
#define TEXT_RENDERER_H_CLASS


#include<map>
#include<iostream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include<glad/glad.h>
#include "glm/glm.hpp"
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "shader.h"



struct Character
{
  unsigned int TextureID;  // ID handle of the glyph texture
  glm::ivec2   Size;       // Size of glyph
  glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
  long int     Advance;    // Offset to advance to next glyph
};


class TextRenderer
{
  public:
    unsigned int VAO, VBO;
    std::map<char, Character> Characters;
    
    TextRenderer(const char* fontPath);
    void type(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);
};

#endif