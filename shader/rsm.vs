#version 330 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_coords;

struct PointLight {
	vec3 pos;
	float constant;
    float linear;
    float quadratic;
    vec3 diffuse;
};

uniform PointLight light;

out VsOut {
	vec3 normal;
	vec2 coords;
} vs_out;

uniform mat4 model;

void main() {
	vs_out.normal = transpose(inverse(mat3(model))) * v_normal;
	vs_out.coords = v_coords;
    gl_Position = model * vec4(v_pos, 1.0);
}
