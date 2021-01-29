
const float _distThresh=3.0f;

const float _speed =1.0f;
__kernel void Main
(
    __global const float* positions,
    const float4 camPos,
    const float deltaTime,
    __global bool* isAnimating,
    __global float4* colors
)
{
    //get indices
    int n = get_global_id(0);
    int positionIndex = n*3;

    //get input color and check if already animating
    float4 inputColor = colors[n];
    bool animate = isAnimating[n];
    //if animating keep animating
    if(animate)
    {
        colors[n] = (float4)(inputColor.x,inputColor.y,inputColor.z, inputColor.w+deltaTime*_speed);
    }
    //else check if we should start to animate
    else
    {

    float a = positions[positionIndex];
    float b = positions[positionIndex];
    float c = positions[positionIndex];

    float4 pos = (float4)(a,b,c,0);
    float dist = length(pos-camPos);

    //we are below the defined threshold, animate and set bool
    if(dist<_distThresh)
    {       
        isAnimating[n]=true;
        colors[n] = (float4)(inputColor.x,inputColor.y,inputColor.z, inputColor.w+deltaTime*_speed);
    }

    }
}