#version 130

in vec3 pos;
in vec3 normal;
in mat3 TBN;

uniform float farPlane;
uniform bool drawNormal;
uniform bool drawDepth;
uniform sampler2D normalTexture;
uniform float reflectance;

out vec4 out_data;

void main() {
    out_data = vec4(0, 0, 0, 0);

    vec3 normNormal;

    // Normal for textured scenes (by normal mapping)
    if (textureSize(normalTexture, 0).x > 1) {
        vec3 modifiedNormal = (texture2D(normalTexture, gl_TexCoord[0].st).rgb * 2.0 - 1.0) * TBN;
        normNormal = normalize(modifiedNormal);
    }

    // Normal for untextured scenes
    else
        normNormal = normalize(normal);

    // Material's reflectivity property
    if (reflectance > 0)
        normNormal = min(normNormal * reflectance, 1.0);

    vec3 normPosition = normalize(-pos);

    float linearDepth = sqrt(pos.z * pos.z + pos.x * pos.x + pos.y * pos.y);
    linearDepth = linearDepth / farPlane;

    if (!(linearDepth > 1)) {
        if (drawNormal){
            float value = dot(normPosition, normNormal);
            out_data.zw = vec2( abs(value), 1.0);
        }
        if (drawDepth)
            out_data.yw = vec2(linearDepth, 1.0);
    }

    gl_FragDepth = linearDepth;
}
