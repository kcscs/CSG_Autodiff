#version 410

out vec4 fs_out_col;
in vec3 vs_out_col;

void main()
{
	fs_out_col = vec4(vs_out_col,1);
}