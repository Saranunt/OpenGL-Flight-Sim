#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main()
{
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(p, p);
    if (r2 > 1.0) discard;
    float falloff = 1.0 - smoothstep(0.0, 1.0, r2);
    FragColor = vec4(vColor.rgb, vColor.a * falloff);
}

