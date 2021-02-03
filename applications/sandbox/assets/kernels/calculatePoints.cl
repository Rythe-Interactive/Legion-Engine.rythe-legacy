const int maxSize=100;


__kernel void Main
(
    __global const float* vertices,
    __global const uint* indices,
    const uint samplesPerTri,
    __global uint* pointsCount
)
{
    //init indices and rand state
    int n=get_global_id(0)*3;
    int triangelIndex= get_global_id(0);


    //get vertex values and corrosponding uvs
    //vertA
    float v1a = vertices[indices[n]*3];
    float v1b = vertices[indices[n]*3+1];
    float v1c = vertices[indices[n]*3+2];
    float4 vertA = (float4)(v1a,v1b,v1c,1.0f);

    //vert2
    float v2a = vertices[indices[n+1]*3];
    float v2b = vertices[indices[n+1]*3+1];
    float v2c = vertices[indices[n+1]*3+2];
    float4 vertB = (float4)(v2a,v2b,v2c,1.0f);

    //vert3
    float v3a = vertices[indices[n+2]*3];
    float v3b = vertices[indices[n+2]*3+1];
    float v3c = vertices[indices[n+2]*3+2];
    float4 vertC = (float4)(v3a,v3b,v3c,1.0f);

    float alpha = fabs(dot(normalize(vertB-vertA), normalize(vertC-vertA))); // abs(cos(theta)) at vertex A
    float beta = fabs(dot(normalize(vertA-vertB), normalize(vertC-vertB))); // abs(cos(theta)) at vertex B
    float gamma = fabs(dot(normalize(vertA-vertC), normalize(vertB-vertC))); // abs(cos(theta)) at vertex C

    if(beta < alpha)
    {
        if(beta < gamma)
        {
            float4 temp = vertA;
            vertA = vertB;
            vertB = temp;
        }
        else
        {
            float4 temp = vertA;
            vertA = vertC;
            vertC = temp;
        }
    }
    else if(gamma < alpha)
    {
        float4 temp = vertA;
        vertA = vertC;
        vertC = temp;
    }

    float size = 1;// length(vertC-vertA) * length(vertB-vertA) * 0.5f;

    //get the size of the triangle
    //size is used to modify the amount of samples for the current triangle
    //large triangles should have more samples than a very small triangle
    // float lengthA = length(vertC-vertA);
    // float lengthB = length(vertB-vertA);
    // float lengthC = length(vertC-vertB);
    // float size = 1;

    int newSampleCount = ceil(size * samplesPerTri);

    uint currentIt=0;
    uint sum = 0;
    while(sum < newSampleCount)
    {
        currentIt++;
        sum += currentIt;
    }

    //newSampleCount= min(newSampleCount,maxSize);
    pointsCount[triangelIndex] = sum;
}

