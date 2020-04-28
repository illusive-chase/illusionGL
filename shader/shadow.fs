#version 330 core
in vec4 f_pos;

struct Light {
	vec3 pos;
};

uniform Light light;
uniform float far_plane;

void main() {
    gl_FragDepth = length(f_pos.xyz - light.pos) / far_plane;
}