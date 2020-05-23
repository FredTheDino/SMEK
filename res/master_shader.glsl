
uniform float t;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform sampler2D tex;

#ifdef VERT
in vec3 pos;
in vec2 uv;
in vec3 norm;

out vec2 pass_uv;
out vec3 pass_norm;
void main() {
    gl_Position = proj * view * model * vec4(pos, 1.0);
    pass_uv = uv;
    pass_norm = norm;
}

#else

out vec4 color;

in vec2 pass_uv;
in vec3 pass_norm;
void main() {
    color = texture(tex, pass_uv) * max(0, dot(normalize(vec3(1, 1, 0)), pass_norm));
}

#endif

