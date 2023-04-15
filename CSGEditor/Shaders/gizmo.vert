#version 410

in vec3 vs_in_pos;
in vec3 vs_in_col;
out vec3 vs_out_col;

uniform mat4 view_proj;
uniform vec2 shift;

void main()
{
	vec4 pos = view_proj * vec4(vs_in_pos, 1);

	// shift image on screen
	pos.x = shift.x * pos.w + pos.x; // multiplay with w so it cancels out during homogene division
	pos.y = shift.y * pos.w + pos.y;
	vs_out_col = vs_in_col;
	gl_Position = pos;
}