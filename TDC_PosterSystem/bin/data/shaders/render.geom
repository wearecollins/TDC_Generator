#version 120
#extension GL_EXT_geometry_shader4 : enable
#extension GL_EXT_gpu_shader4 : enable

uniform float pointSize;
uniform float pointRandomization;
uniform mat4 homography;
uniform vec2 screen;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const float PI = 3.1415926;

void main(void){
    float m = screen.x == 1.0 ? 1.0 : 20.0;
    float mult = 600.0 * m;
    float size = (pointSize + (rand(gl_PositionIn[0].xy)*pointSize * pointRandomization))/mult;
    
    float circleRes = 12.0;
    
    for ( float a = 0; a < circleRes; a++ ){
        // Angle between each side in radians
        float ang = (PI * 2.0 / circleRes) * a;
        
        vec4 offset = vec4(cos(ang) * size, -sin(ang) * size, 0.0, 0.0);
        gl_Position = gl_PositionIn[0] + offset;
        gl_FrontColor = gl_FrontColorIn[0];
        EmitVertex();
        
        ang = (PI * 2.0 / circleRes) * (a + 1);
        offset = vec4(cos(ang) * size, -sin(ang) * size, 0.0, 0.0);
        gl_Position = gl_PositionIn[0] + offset;
        gl_FrontColor = gl_FrontColorIn[0];
        EmitVertex();
        
        gl_Position = gl_PositionIn[0];
        gl_FrontColor = gl_FrontColorIn[0];
        EmitVertex();
    }
    
    EndPrimitive();
    
}