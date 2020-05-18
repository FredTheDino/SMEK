
uniform float t;
uniform mat4 view;
uniform mat4 model;
uniform sampler2D tex;

#ifdef VERT
in vec3 pos;

out vec2 pass_uv;
void main() {
    gl_Position = vec4((pos * 0.05), 1.0) * model * view;
    // gl_Position = vec4((pos * 0.5), 1.0);
    if (gl_VertexID % 3 == 0)
        pass_uv = vec2(1, 0);
    if (gl_VertexID % 3 == 1)
        pass_uv = vec2(0, 1);
    if (gl_VertexID % 3 == 2)
        pass_uv = vec2(0, 0);
}

#else

out vec4 color;

in vec2 pass_uv;
void main() {
    color = texture(tex, pass_uv);
}

#endif

