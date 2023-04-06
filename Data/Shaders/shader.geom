#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 3) out;

layout (location = 1) in vec2 texCoord[];
layout (location = 0) in vec3 FragColor[];

layout (location = 1) out vec2 fs_texCoord;
layout (location = 0) out vec3 fs_FragColor;

void main()
{
    for (int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        fs_texCoord = texCoord[i];
        fs_FragColor = FragColor[i];
        EmitVertex();
    }

    EndPrimitive();
}