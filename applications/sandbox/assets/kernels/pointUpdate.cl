
const float _distThresh=16.0f;

const float _speed =1.0f;
__kernel void Main
(
    __global const float* positions,
    const float4 camPos,
    const float deltaTime,
    __global float4* colors
)
{
    int n = get_global_id(0);
    int positionIndex = n*3;


    float a = positions[positionIndex];
    float b = positions[positionIndex];
    float c = positions[positionIndex];

    float4 pos = (float4)(a,b,c,0);
    float dist = length(pos-camPos);

    float4 newPositions = positions[n];


    float4 inputColor = colors[n];
    if(dist>_distThresh)
    colors[n] = (float4)(inputColor.x, inputColor.y,inputColor.z, inputColor.w- deltaTime*_speed);


}