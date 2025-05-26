#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

vec3 rgb2ycbcr(vec3 rgb) {
    float y  = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    float cb = -0.169 * rgb.r - 0.331 * rgb.g + 0.500 * rgb.b + 0.5;
    float cr = 0.500 * rgb.r - 0.419 * rgb.g - 0.081 * rgb.b + 0.5;
    return vec3(y, cb, cr);
}

void main()
{
    vec4 color = texture(screenTexture, TexCoords);
    vec3 ycbcr = rgb2ycbcr(color.rgb);

    float cb = ycbcr.g;
    float cr = ycbcr.b;

    vec3 diff = abs(color.rgb - vec3(0.0, 1.0, 0.0)); // puur groen
    if (diff.r + diff.b < 1.0) discard; // tolerantie op rood+blauw


    FragColor = color;
}
