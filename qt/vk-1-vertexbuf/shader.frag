/**
 * fragment shader
 * @author Tobias Weber
 * @date Feb-2021
 * @license: see 'LICENSE.GPL' file
 *
 * @see https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language
 */

#version 450


layout(location = 0) in vec4 fragcolor;
layout(location = 1) in vec2 fragtexcoords;

layout(location = 0) out vec4 outcolor;


void main()
{
	outcolor = fragcolor;
}
