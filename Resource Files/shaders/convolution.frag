#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float kernel[9];
uniform float offset;

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2(0.0f,  offset), vec2(offset,  offset),
        vec2(-offset,  0.0f),   vec2(0.0f,  0.0f),   vec2(offset,  0.0f),
        vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
        sampleTex[i] = texture(screenTexture, TexCoords + offsets[i]).rgb;

    vec3 resultColor = vec3(0.0);
    for (int i = 0; i < 9; i++)
        resultColor += sampleTex[i] * kernel[i];

    FragColor = vec4(resultColor, 1.0);
}
