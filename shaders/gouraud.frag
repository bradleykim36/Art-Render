in vec4 color;


// All fragment shaders are required to output a vec4 color.
out vec4 final_color;


void main() {
    // For a Gouraud shader, there is nothing more to compute at this stage.  We
    // just output the input color.
    final_color = vec4(0,0,0,1);
}
