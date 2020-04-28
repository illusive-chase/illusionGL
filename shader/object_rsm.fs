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

uniform samplerCube depth_map;
uniform samplerCube indirect_map;
uniform samplerCube pos_map;
uniform samplerCube normal_map;
uniform float far_plane;

#define SAMPLE_NUM 15


float rand(int x, int y){
	return fract(sin(x * 12.9898 + y * 78.233) * 43758.5453);
}

float calc_shadow(vec3 normal) {
	vec3 frag_to_light = f_pos - point_lights[0].pos;
	float closest_depth = texture(depth_map, frag_to_light).r * far_plane;
	float current_depth = length(frag_to_light);
	float bias = max(0.05 * (1.0 - dot(normal, -normalize(frag_to_light))), 0.005);
    float shadow = (current_depth - bias > closest_depth) ? 1.0 : 0.0;
    return shadow;
}

vec3 indirect_light() {
	vec3 frag_to_light = normalize(f_pos - point_lights[0].pos);
	vec3 ret = vec3(0);
	float step = 1.0 / SAMPLE_NUM;
	float weight_sum = 0.0;
	for (int i = 0; i <= SAMPLE_NUM; i++) {
		for (int j = 0; j <= SAMPLE_NUM; j++) {
			vec3 frag_to_light_noised = frag_to_light + normalize(vec3(2*rand(i,j)-1, 2*rand(i*2,j)-1, 2*rand(i,j*2)-1));
			vec3 psai = texture(indirect_map, frag_to_light_noised).rgb;
			vec3 pos = (texture(pos_map, frag_to_light_noised).rgb - 0.5) * 2.0;
			vec3 normal = (texture(normal_map, frag_to_light_noised).rgb - 0.5) * 2.0;
			vec3 distance = pos - f_pos;
			float distance_square = dot(distance, distance);
			float weight = 1.0 / (2.5 + dot(frag_to_light, normalize(frag_to_light_noised)));
			weight_sum += weight;
			ret += psai * weight * max(0, dot(normalize(f_normal), distance)) * max(0, dot(normal, -distance)) / (distance_square * distance_square);
		}
	}
	ret /= weight_sum;
	if (ret.x > 1.0) ret.x = 1.0;
	if (ret.y > 1.0) ret.y = 1.0;
	if (ret.z > 1.0) ret.z = 1.0;
	return 0.6 * ret;
}

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_dir, float shadow_factor) {
	vec3 light_dir = normalize(-light.dir);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_coords));
	return ambient + (diffuse + specular) * (1.0 - shadow_factor) + indirect_light();
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 view_dir, float shadow_factor) {
	//return indirect_light();
	vec3 light_dir = normalize(light.pos - f_pos);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    
	float distance    = length(light.pos - f_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_coords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
	return ambient + (diffuse + specular) * (1.0 - shadow_factor) + indirect_light();
}

vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 view_dir, float shadow_factor) {
	vec3 light_dir = normalize(light.pos - f_pos);

	float theta = dot(light_dir, normalize(-light.dir));
	float epsilon   = light.cut_off - light.outer_cut_off;
	float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
	
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

	vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_coords));
	vec3 diffuse  = intensity * light.diffuse  * diff * vec3(texture(material.diffuse, f_coords));
	vec3 specular = intensity * light.specular * spec * vec3(texture(material.specular, f_coords));

	return ambient + (diffuse + specular) * (1.0 - shadow_factor) + indirect_light();
}

void main() {

	vec3 norm = normalize(f_normal);
	vec3 view_dir = normalize(view_pos - f_pos);
	
	float shadow_factor = calc_shadow(norm);

	vec3 result = vec3(0, 0, 0);
	if (dir_light_num > 0) result += calc_dir_light(dir_light, norm, view_dir, shadow_factor);
    for(int i = 0; i < point_light_num; i++) result += calc_point_light(point_lights[i], norm, view_dir, shadow_factor);
    for(int i = 0; i < spot_light_num; i++) result += calc_spot_light(spot_lights[i], norm, view_dir, shadow_factor);    

    o_color = vec4(result, 1.0);
}
