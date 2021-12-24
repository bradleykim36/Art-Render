in vec3 position_in_eye_space;
in vec3 normal_in_eye_space;

out vec4 color;

uniform vec3 light_in_eye_space;
uniform vec4 Ia, Id, Is;

uniform vec4 ka, kd, ks;
uniform float s;

uniform sampler2D diffuse_ramp;
uniform sampler2D specular_ramp;


void main() {
    vec3 v = position_in_eye_space;

    vec3 l = normalize(light_in_eye_space);
    
    vec3 e = normalize(vec3(0,0,0) - v);
    
    vec3 n = normalize(normal_in_eye_space);
    
    vec3 h = normalize(l + e);
    
    vec2 d_t = vec2(((dot(n,l) + 1) / 2), 1.0 - ((dot(n,l) + 1) / 2));
    vec2 s_t = vec2(pow(max(dot(n, h), 0), s), 1.0 - pow(max(dot(n, h), 0), s));

    color = (ka * Ia) + kd * Id * texture(diffuse_ramp, d_t) + texture(specular_ramp, s_t);
}
