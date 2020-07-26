
uniform mat4 proj;
uniform mat4 view;

#ifdef VERT
layout(location=0) in vec3 pos;
layout(location=3) in vec4 color;

out vec4 pass_color;
void main() {
    gl_Position = proj * view * vec4(pos, 1.0);
    pass_color = color;
}

#else

layout(location=0) out vec3 color;

in vec4 pass_color;
void main() {
    color = pass_color.rgb;
}

#endif

