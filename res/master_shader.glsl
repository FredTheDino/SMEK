const int MAX_JOINTS = 50;

uniform float t;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform sampler2D tex;

uniform int num_bones;
uniform mat4 bones[MAX_JOINTS];

#ifdef VERT
layout(location=0) in vec3 pos;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 norm;
layout(location=3) in vec2 weight1;
layout(location=4) in vec2 weight2;
layout(location=5) in vec2 weight3;

out vec2 pass_uv;
out vec3 pass_norm;
void main() {
    ivec3 bone_indicies;
    bone_indicies[0] = int(weight1.x);
    bone_indicies[1] = int(weight2.x);
    bone_indicies[2] = int(weight3.x);
    vec3 bone_weights;
    bone_weights[0] = weight1.y;
    bone_weights[1] = weight2.y;
    bone_weights[2] = weight3.y;

    vec4 final_norm = vec4(0.0);
    vec4 final_pos = vec4(0.0);
    for (int i = 0; i < 3; i++) {
        mat4 bone_trans = bones[bone_indicies[i]];

        vec4 p = bone_trans * vec4(pos, 1.0);
        final_pos += p * bone_weights[i];

        vec4 n = bone_trans * vec4(norm, 0.0);
        final_norm += n * bone_weights[i];
    }

    gl_Position = proj * view * model * final_pos;
    pass_norm = final_norm.xyz;
    pass_uv = uv;
    
}

#else

out vec4 color;

in vec2 pass_uv;
in vec3 pass_norm;
void main() {
    color = texture(tex, pass_uv) * max(0, dot(normalize(vec3(1, 1, 0)), pass_norm));
    color = vec4(pass_norm, 1.0);
}

#endif

