in vec3 position_in_eye_space;
in vec3 normal_in_eye_space;

out vec4 color;

uniform vec3 light_in_eye_space;
uniform vec4 Ia, Id, Is;

uniform vec4 ka, kd, ks;
uniform float s;


void main() {
	vec3 v = position_in_eye_space;
    
    vec3 l = normalize(light_in_eye_space);
    
    vec3 e = normalize(-v);
    
    vec3 n = normalize(normal_in_eye_space);
    
    vec3 h = normalize(l + e);
    
    color = ka * Ia + kd * Id * max(dot(n, l), 0) + ks * Is * pow(max(dot(h, n), 0), s);
}
