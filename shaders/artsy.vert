uniform mat4 model_view_matrix;
uniform mat4 normal_matrix;
uniform mat4 proj_matrix;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

out vec3 position_in_eye_space;
out vec3 normal_in_eye_space;

void main() {
    position_in_eye_space = (model_view_matrix * vec4(vertex,1)).xyz;
    normal_in_eye_space = normalize((normal_matrix * vec4(normal,0)).xyz);
    gl_Position = proj_matrix * model_view_matrix * vec4(vertex,1);
}
