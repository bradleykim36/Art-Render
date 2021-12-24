uniform mat4 model_view_matrix;
uniform mat4 normal_matrix;
uniform mat4 proj_matrix;
uniform float thickness;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 left_normal;
layout(location = 3) in vec3 right_normal;


void main() {
    
    vec3 vertex1 = vertex;

    if (dot((model_view_matrix * vec4(vertex,1)).xyz, (normal_matrix * vec4(left_normal,1)).xyz) * dot((model_view_matrix * vec4(vertex,1)).xyz, (normal_matrix * vec4(right_normal,1)).xyz) < 0.0) {
        vertex1 = vertex + thickness * normal;
    }
    
    gl_Position = proj_matrix * model_view_matrix * vec4(vertex1,1);
}
