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


// hard-coded for the moment, no uniforms yet
mat4 proj = transpose(mat4(
	0.75,  0,       0,         0,
	0.,   -1,       0,         0, 
	0.,    0, -1.0001, -0.010001,
	0.,    0,      -1,         0));

//mat4 cam = mat4(1);
mat4 cam = transpose(mat4(
	1, 0, 0,  0,
	0, 1, 0,  0,
	0, 0, 1, -3,
	0, 0, 0,  1));

mat4 rot = mat4(
	0.7, 0.0, -0.7, 0.0,
	0.0, 1.0,  0.0, 0.0,
	0.7, 0.0,  0.7, 0.0,
	0.0, 0.0,  0.0, 1.0);


vec3 light_dir = vec3(2, 2, -1);


float lighting(vec3 lightdir)
{
	float I = dot(vec3(cam*normal), normalize(lightdir));
	if(I < 0) I = 0;
	return I;
}


void main()
{
	gl_Position = proj * cam * rot * vertex;

	float I = lighting(light_dir);
	fragcolor = vertexcolor * I;
	fragcolor[3] = 1;

	fragtexcoords = texcoords;
}
