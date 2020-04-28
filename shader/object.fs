#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
	vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 pos;
	float constant;
    float linear;
    float quadratic;
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
	vec3 pos;
	vec3 dir;
	float cut_off;
	float outer_cut_off;
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 o_color;

in vec3 f_normal;
in vec3 f_pos;
in vec2 f_coords;

#define MAX_MULTILIGHT_NUM 4

uniform DirLight dir_light;
uniform int dir_light_num;
uniform PointLight point_lights[MAX_MULTILIGHT_NUM];
uniform int point_light_num;
uniform SpotLight spot_lights[MAX_MULTILIGHT_NUM];
uniform int spot_light_num;
uniform vec3 view_pos;
uniform Material material;

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_dir) {
	vec3 light_dir = normalize(-light.dir);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_coords));
    return (ambient + diffuse + specular);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.pos - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    
	float distance    = length(light.pos - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_coords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	vec3 light_dir = normalize(light.pos - frag_pos);

	float theta = dot(light_dir, normalize(-light.dir));
	float epsilon   = light.cut_off - light.outer_cut_off;
	float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
	
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

	vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
	vec3 diffuse  = intensity * light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
	vec3 specular = intensity * light.specular * spec * vec3(texture(material.specular, f_coords));

	return (ambient + diffuse + specular);
}

void main() {

	vec3 norm = normalize(f_normal);
	vec3 view_dir = normalize(view_pos - f_pos);
	
	vec3 result = vec3(0, 0, 0);
	if (dir_light_num > 0) result += calc_dir_light(dir_light, norm, view_dir);
    for(int i = 0; i < point_light_num; i++) result += calc_point_light(point_lights[i], norm, f_pos, view_dir);
    for(int i = 0; i < spot_light_num; i++) result += calc_spot_light(spot_lights[i], norm, f_pos, view_dir);    

    o_color = vec4(result, 1.0);
}
