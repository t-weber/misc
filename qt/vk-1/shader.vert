/**
 * vertex shader
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 *
 * @see https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language
 */

#version 450


layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 vertexcolor;
layout(location = 3) in vec2 texcoords;

layout(location = 0) out vec4 fragcolor;
layout(location = 1) out vec2 fragtexcoords;

layout(binding = 0) uniform Trafos
{
	mat4 proj;
	mat4 cam;
};

vec3 light_dir = vec3(2, 2, -1);


float lighting(vec3 lightdir)
{
	float I = dot(vec3(cam*normal), normalize(lightdir));
	if(I < 0) I = 0;
	return I;
}


void main()
{
	gl_Position = proj * cam * vertex;

	float I = lighting(light_dir);
	fragcolor = vertexcolor * I;
	fragcolor[3] = 1;

	fragtexcoords = texcoords;
}
