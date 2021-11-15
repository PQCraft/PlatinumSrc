#ifndef PSRC_GFX_SHADERS

#define PSRC_GFX_SHADERS

const char PSRC_GFX_VSHADER[] = "\
#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
void main()\n\
{\n\
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
}\
";

const char PSRC_GFX_FSHADER[] = "\
#version 330 core\n\
out vec4 FragColor;\n\
void main()\n\
{\n\
   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n\
}\
";

#endif
