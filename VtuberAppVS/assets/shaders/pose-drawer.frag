#version 330 core
out vec4 FragColor;

void main()
{   
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0; // Transform to -1.0 to 1.0 range
    if (dot(circCoord, circCoord) > 1.0) {
        discard; // Discard fragments outside the circle
    }
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // red
}