uniform float t;
uniform sampler2D tex;

#ifdef VERT
layout(location=0) in vec3 pos;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 norm;

out vec2 pass_uv;
void main() {
    gl_Position = vec4(pos * vec3(1.0, 1.0, 1.0), 1.0);
    pass_uv = uv;
}

#else

out vec4 color;

in vec2 pass_uv;
void main() {

    vec2 uv;
    if (pass_uv.x < 0.5) {
        uv = vec2(1.0 - pass_uv.x, pass_uv.y);
    }

    if (pass_uv.y < 0.5) {
        uv = vec2(uv.x, 1.0 - pass_uv.y);
    }
    color = texture(tex, uv);
}

#endif

