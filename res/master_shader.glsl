
uniform float t;
uniform mat4 view;
uniform mat4 model;

#ifdef VERT
in vec3 pos;

out vec3 pass_color;
void main() {
    gl_Position = vec4((pos * 0.05), 1.0) * model * view;
    // gl_Position = vec4((pos * 0.5), 1.0);
    if (gl_VertexID % 3 == 0)
        pass_color = vec3(1, 0, 0);
    if (gl_VertexID % 3 == 1)
        pass_color = vec3(0, 1, 0);
    if (gl_VertexID % 3 == 2)
        pass_color = vec3(0, 0, 1);
}

#else

out vec4 color;

in vec3 pass_color;
void main() {
    color = vec4(pass_color, 1);
}

#endif

