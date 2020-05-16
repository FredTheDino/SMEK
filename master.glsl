#ifdef VERT
in vec3 pos;

void main() {
    gl_Position = vec4(pos, 1.0);
}

#else

out vec4 color;

void main() {
    color = vec4(0.1, 1, 0.3, 1);
}

#endif

