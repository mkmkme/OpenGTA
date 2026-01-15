#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D spriteTexture;
uniform float alphaThreshold;

void main()
{
    vec4 texColor = texture(spriteTexture, TexCoord);
    if (texColor.a <= alphaThreshold)
        discard;
    FragColor = texColor;
}
