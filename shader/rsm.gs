#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadow_matrices[6];

out vec3 f_normal;
out vec4 f_pos;
out vec2 f_coords;

in VsOut {
	vec3 normal;
	vec2 coords;
} gs_in[];

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) {
            f_pos = gl_in[i].gl_Position;
			f_normal = gs_in[i].normal;
			f_coords = gs_in[i].coords;
            gl_Position = shadow_matrices[face] * f_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
