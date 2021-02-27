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

layout(binding = 0) uniform sampler2D img;

// cursor position
layout(binding = 0) uniform Cursor
{
	uniform vec2 fragCurUV;
};


void main()
{
	outcolor = texture(img, fragtexcoords);
	outcolor *= fragcolor;

	// paint cursor position
	if(length(fragtexcoords - fragCurUV) < 0.01)
		outcolor = vec4(1,1,1,1);
}
