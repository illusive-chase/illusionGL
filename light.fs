#version 330 core
out vec4 o_color;

in vec3 f_normal;
in vec3 f_pos;
in vec2 f_coords;

uniform vec3 light_color;

void main() {
    o_color = vec4(light_color, 1.0);
}
