#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
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
in vec2 f_coords;

uniform Light light;
uniform vec3 view_pos;
uniform Material material;

void main() {
	vec3 ambient = vec3(texture(material.diffuse, f_coords)) * light.ambient;

	vec3 light_dir = normalize(light.pos - f_pos);
	vec3 diffuse =  max(dot(f_normal, light_dir), 0.0) * vec3(texture(material.diffuse, f_coords)) * light.diffuse;		

	vec3 view_dir = normalize(view_pos - f_pos);
	vec3 reflect_dir = reflect(-light_dir, f_normal);
	vec3 specular = vec3(texture(material.specular, f_coords)) * pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess) * light.specular;

    o_color = vec4(ambient + diffuse + specular, 1.0);
}
