@ctype vec4 hmm_vec4
@ctype mat4 hmm_mat4

@vs vs
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color_v;

in vec4 inst_mat_x;
in vec4 inst_mat_y;
in vec4 inst_mat_z;
in vec4 inst_mat_w;

uniform vs_params {
    mat4 texture_matrix;
    mat4 projection;
};

out vec4 out_texcoord;
out vec3 out_normal;
out vec4 out_color;

mat4 make_matrix(vec4 x, vec4 y, vec4 z, vec4 w) {
    mat4 m;
    m[0][0] = x.x;
    m[1][0] = x.y;
    m[2][0] = x.z;
    m[3][0] = x.w;
    m[0][1] = y.x;
    m[1][1] = y.y;
    m[2][1] = y.z;
    m[3][1] = y.w;
    m[0][2] = z.x;
    m[1][2] = z.y;
    m[2][2] = z.z;
    m[3][2] = z.w;
    m[0][3] = w.x;
    m[1][3] = w.y;
    m[2][3] = w.z;
    m[3][3] = w.w;
    return m;
}

void main() {
    mat4 modelview = make_matrix(inst_mat_x, inst_mat_y, inst_mat_z, inst_mat_w);
    gl_Position = (projection * modelview) * position;
    out_texcoord = texture_matrix * vec4(texcoord, 0.0, 1.0);
    out_normal = normal;
    out_color = color_v;
}
@end

@fs fs
uniform texture2D texture_v;
uniform sampler sampler_v;

in vec4 out_texcoord;
in vec3 out_normal;
in vec4 out_color;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(texture_v, sampler_v), out_texcoord.xy) * color;
}
@end

@program sim vs fs
