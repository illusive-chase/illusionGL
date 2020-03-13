#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 o_color;

in vec3 f_normal;
in vec3 f_pos;

uniform Light light;
uniform vec3 view_pos;
uniform Material material;

void main() {
	vec3 ambient = material.ambient * light.ambient;

	vec3 light_dir = normalize(light.pos - f_pos);
	vec3 diffuse = material.diffuse * max(dot(f_normal, light_dir), 0.0) * light.diffuse;

	vec3 view_dir = normalize(view_pos - f_pos);
	vec3 reflect_dir = reflect(-light_dir, f_normal);
	vec3 specular = material.specular * pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess) * light.specular;

    o_color = vec4(ambient + diffuse + specular, 1.0);
}
