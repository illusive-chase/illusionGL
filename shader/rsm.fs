#version 330 core

in vec3 f_normal;
in vec4 f_pos;
in vec2 f_coords;

struct PointLight {
	vec3 pos;
	float constant;
    float linear;
    float quadratic;
    vec3 diffuse;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
};

layout (location = 0) out vec3 o_pos;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out vec3 o_color;

uniform PointLight light;
uniform Material material;
uniform float far_plane;

void main() {
	vec3 light_dir = normalize(light.pos - vec3(f_pos));
	vec3 norm = normalize(f_normal);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, norm);
	float distance = length(light.pos - vec3(f_pos));
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 diffuse = light.diffuse  * diff * vec3(texture(material.diffuse, f_coords)) * attenuation;
    o_color = diffuse;
	o_pos = vec3(f_pos) * 0.5 + 0.5;
	o_normal = normalize(f_normal) * 0.5 + 0.5;
	gl_FragDepth = length(f_pos.xyz - light.pos) / far_plane;
}
