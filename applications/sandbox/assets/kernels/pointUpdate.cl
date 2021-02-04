const float _distThresh = 6.0f;
const float _speed = 1.0f;

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
    int positionIndex = n * 3;

    //get input color and check if already animating
    float4 inputColor = colors[n];
    bool animate = isAnimating[n];
    //if animating keep animating
    if(animate)
    {
        colors[n] = (float4)(inputColor.x, inputColor.y, inputColor.z, inputColor.w + deltaTime * _speed);
    }
    else //else check if we should start to animate
    {
        float x = positions[positionIndex + 0];
        float y = positions[positionIndex + 1];
        float z = positions[positionIndex + 2];

        float4 pos = (float4)(x, y, z, 0);
        float dist = length(pos - camPos);

        //we are below the defined threshold, animate and set bool
        if(dist < _distThresh)
        {
            isAnimating[n] = true;
            colors[n] = (float4)(inputColor.x, inputColor.y, inputColor.z, inputColor.w + deltaTime * _speed);
        }
        else if(dist < _distThresh + 0.5f)
        {
            colors[n] = (float4)(inputColor.x, inputColor.y, inputColor.z, clamp(inputColor.w + deltaTime * _speed, 0.f, (_distThresh - dist) / 5.f));
        }
    }
}
