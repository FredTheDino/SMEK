const int MAX_JOINTS = 50;
const int MAX_LIGHTS = 10;

uniform float t;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform sampler2D tex;

uniform int num_bones;
uniform mat4 bones[MAX_JOINTS];

uniform int num_lights;
uniform vec3 light_positions[MAX_LIGHTS];
uniform vec3 light_colors[MAX_LIGHTS];

uniform vec3 sun_dir;
uniform vec3 sun_color;
uniform vec3 ambient_color;

#ifdef VERT
layout(location=0) in vec3 pos;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 norm;
layout(location=3) in vec2 weight1;
layout(location=4) in vec2 weight2;
layout(location=5) in vec2 weight3;

out vec2 pass_uv;
out vec3 pass_norm;
out vec3 pass_pos;
void main() {
    ivec3 bone_indicies;
    bone_indicies[0] = int(weight1.x);
    bone_indicies[1] = int(weight2.x);
    bone_indicies[2] = int(weight3.x);
    vec3 bone_weights;
    bone_weights[0] = weight1.y;
    bone_weights[1] = weight2.y;
    bone_weights[2] = weight3.y;

    vec4 final_norm;
    vec4 final_pos;
    if (num_bones != 0) {
        final_norm = vec4(0.0);
        final_pos = vec4(0.0);
        for (int i = 0; i < 3; i++) {
            mat4 bone_trans = bones[bone_indicies[i]];

            vec4 p = bone_trans * vec4(pos, 1.0);
            final_pos += p * bone_weights[i];

            vec4 n = normalize(bone_trans * vec4(norm, 0.0));
            final_norm += n * bone_weights[i];
        }
    } else {
        final_pos = vec4(pos, 1.0);
        final_norm = vec4(norm, 0.0);
    }

    gl_Position = proj * view * model * final_pos;
    pass_norm = normalize((model * final_norm).xyz);
    pass_pos = (model * final_pos).xyz;
    pass_uv = uv;
}

#else

out vec4 color;

in vec2 pass_uv;
in vec3 pass_norm;
in vec3 pass_pos;
void main() {
    float sun_lightness = max(0, dot(sun_dir, pass_norm));
    vec4 albedo = texture(tex, pass_uv);
    albedo = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 light_color = sun_lightness * (sun_lightness * vec4(sun_color, 1.0) + vec4(ambient_color, 1.0));

    for (int i = 0; i < num_lights; i++) {
        vec3 p = light_positions[i];
        vec3 c = light_colors[i];
        vec3 delta_p = p - pass_pos;
        vec3 light_dir = normalize(delta_p);
        float effect = max(0, dot(light_dir, pass_norm)) / pow(length(delta_p), 3);
        light_color += vec4(c, 1.0) * effect;
    }

    color = albedo * light_color / 3.0;
}

#endif

