
uniform float t;
uniform mat4 view;
uniform mat4 model;
uniform sampler2D tex;

#ifdef VERT
in vec3 pos;

out vec2 pass_uv;
void main() {
    gl_Position = vec4(pos, 1.0);
}

#else

out vec4 color;

in vec2 pass_uv;
void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif

