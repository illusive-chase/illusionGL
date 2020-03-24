#version 330 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_coords;

out vec3 f_normal;
out vec3 f_pos;
out vec2 f_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	f_pos = vec3(model * vec4(v_pos, 1.0));
    gl_Position = projection * view * model * vec4(v_pos, 1.0);
	f_normal = v_normal;
	f_coords = v_coords;
}
