
//constant values
//exit condition for poission sampling
const int K=30;
//predefined values
#define maxPointsPerTri 500
#define radius 0.33f
#define cellSize (radius)/1.41421356237f
#define depth (int)ceil(1/ cellSize)
#define gridDimension  depth *depth

//seed state
uint state;

//returns random uint (range 0-255?)
uint Rand()
{
   state = state * 1664525 + 1013904223;
   return state >> 24;
}
//uses Rand to generate a random float (range 0-1)
float RandomValue()
{
   return (Rand() / (float)(UINT_MAX)) * FLT_MAX;
}

//includes upper bound
uint RandomUpperRange(uint upper)
{
    uint r = Rand();
    return r %(upper+1);
}
//usese barycentric coordinates to sample a new point
float4 SampleTriangle(float2 coordinates, float4 a, float4 b, float4 c)
{
    return mix(a, b, coordinates.x) + coordinates.y * (c-a);
}

float2 SampleUVs(float2 coordinates, float2 a, float2 b, float2 c)
{
    return mix(a, b, coordinates.x) + coordinates.y * (c-a);
}

float sampleLight(float2 coordinates, float2 centerInterpolators, float4 items)
{
    return mix(items.w, mix(items.x, items.y, coordinates.x) + coordinates.y * (items.z - items.x), (centerInterpolators.x + centerInterpolators.y)*0.5f);
}
//checks validity of point for poission sampling
bool CheckPoint(float2 newPoint, float2* points, int* grid)
{
    //check if point lies within triangle range else just return false
    if(newPoint.x> 0 && newPoint.x <1 && newPoint.y>  0 && newPoint.y <1 && newPoint.x + newPoint.y < 1)
    {
        //check point validity
        //get grid cell
        int currentX = (int) newPoint.x / cellSize;
        int currentY = (int) newPoint.y / cellSize;
        //iterate neighbour cells in a radius of 2
        int startX = max(0, currentX - 2);
        int endX = min(currentX + 2, maxPointsPerTri- 1);
        int startY = max(0, currentY - 2);
        int endY = min(currentY + 2, maxPointsPerTri-1);
        for(int x = startX; x < endX; x++)
        {
            for(int y = startY; y<endY; y++)
            {
                //get other cell, if valid, check distance is larger than radius
                int otherCellIndex = grid[x*y+x]-1;
                if(otherCellIndex>=0)
                {
                    float2 delta = (newPoint - points[otherCellIndex]);
                    float dist = length(delta);
                    if(dist< radius * radius) return false;
                }
            }
        }
        return true;
    }
    return false;
}
//sampling technique checkout:
//http://www.cemyuksel.com/cyCodeBase/soln/poisson_disk_sampling.html
//to understand some more.
void PoissionSampling(__local float2* outputPoints, int samplePerTri)
{
    //init output
    int outPutIndex=0;

    //create cell grid
    __local int grid[maxPointsPerTri];

    __local float2 spawnPoints[maxPointsPerTri];
    //add center point as starting point
    spawnPoints[0] = (float2)(0.25f,0.25f);
    int spawnPointAmount=1;
    while(spawnPointAmount>0)
    {
       // get random point from spawn points && its position
        int index = RandomUpperRange(spawnPointAmount);
        float2 spawnCenter = spawnPoints[index];

        bool isAccepteed=false;
        // try generating a valid point until k is reached
        for(int i=0; i <K; i++)
        {
            //generate offset and new point
            float angle = RandomValue() * 2 * M_PI;
            float2 direction = (float2)(sin(angle), cos(angle));
            float2 newPoint = spawnCenter + normalize(direction) * radius;
            // check point
            if(CheckPoint(newPoint,outputPoints, grid ))
            {
                outputPoints[outPutIndex] = newPoint;
                outPutIndex++;
                spawnPoints[spawnPointAmount] = newPoint;
                spawnPointAmount++;
                isAccepteed=true;

                grid[(int)(newPoint.x / cellSize) *(int)(newPoint.y/cellSize)] = outPutIndex;
                if(outPutIndex> samplePerTri)
                    return;
            }
        }
        if(!isAccepteed)
        {
            spawnPoints[index] = 0;
            spawnPointAmount--;
        }
    }

}


float2 sampleUniformly(__local float2* output, uint samplesPerTri, uint sampleWidth, float costheta)
{
    float offset = 1.0f / (float)(clamp(sampleWidth-1u, 1u, sampleWidth));
    int index = 0;
    for(int x = 0; x < sampleWidth; x++)
    {
        for(int y = 0; y < sampleWidth - x; y++)
        {
            //float angle = fmod(RandomValue(), (float)(M_PI) * 0.5f);
            //float magnitude = fmod(RandomValue(), offset) - offset * 0.5f;

            //float2 direction = (float2)(sin(angle), cos(angle)) * magnitude;

            output[index] = clamp((float2)(offset * (x), offset * (y)), 0.f, 1.f);
            index++;
        }
    }
}

constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float sampleHeight(__read_only image2d_t texture, float2 uvs)
{
    return read_imagef(texture, sampler, uvs).x;
}
float4 sampleColor(__read_only image2d_t texture, float2 uvs)
{
    return read_imagef(texture, sampler, uvs);
}


bool intersection_MT97(float4 rayOrigin, float4 rayDir,float4 vertA,float4 vertB,float4 vertC)
{
    float4 edge1 = vertB-vertA;
    float4 edge2 = vertC-vertA;

    float4 pVec = cross(rayDir,edge2);

    float det = dot(edge1,pVec);
    //no intersection is found
    if(det<0.0001f ) return false;

    float inv_det = 1.0f/det;
    float4 tVec = rayOrigin-vertA;

    float u = dot(tVec,pVec) * inv_det;
    //no intersection
    //
    if(u<0.0f|| u >1.0f)
    return false;

    float4 qvec = cross(tVec,edge1);

    float v = dot(rayDir, qvec)*inv_det;

    if(v< 0.0f ||u + v >1.0f)
        return false;


    float t = inv_det * dot(edge2, qvec);
    if(t<0.0f) return false;
    //no intersection found


    //end reached intersection was found
    return true;

}

float4 IsOccludedFromDirLight(float4 pointA, float4 pointB, float4 pointC, float4 center, int n, const float* vertices, const uint* indices, float4 lightDir , int triangleIndex)
{
    //get the linverted light direction
    float4 direction = -lightDir;
    direction.w = 0.f;

    bool a = true;
    bool b = true;
    bool c = true;
    bool d = true;

    for(int i = 0; i < n*3; i += 3)
    {
        if(i==triangleIndex)
            continue;

        float x = vertices[indices[i]*3];
        float y = vertices[indices[i]*3+1];
        float z = vertices[indices[i]*3+2];
        float4 vertA = (float4)(x, y, z,0.0f);

        //vert2
        x = vertices[indices[i+1]*3];
        y = vertices[indices[i+1]*3+1];
        z = vertices[indices[i+1]*3+2];
        float4 vertB = (float4)(x, y, z,0.0f);

        //vert3
        x = vertices[indices[i+2]*3];
        y = vertices[indices[i+2]*3+1];
        z = vertices[indices[i+2]*3+2];
        float4 vertC = (float4)(x, y, z,0.0f);

        //if there is intersection, point is occluded, return true
        if(a)
            if(intersection_MT97(pointA, direction, vertA, vertB, vertC))
                a = false;
        if(b)
            if(intersection_MT97(pointB, direction, vertA, vertB, vertC))
                b = false;
        if(c)
            if(intersection_MT97(pointC, direction, vertA, vertB, vertC))
                c = false;
        if(d)
            if(intersection_MT97(center, direction, vertA, vertB, vertC))
                d = false;

        if(!a && !b && !c && !d)
            return (float4)(0.f);
    }


    return (float4)((float)(a), (float)(b), (float)(c), (float)(d));
}

__kernel void Main
(
    __global const float* vertices,
    __global const uint* indices,
    __global const float* normalsIn,
    __global const float2* uvs,
    __global const uint* samples,
    __global const float4* lightDir,
    __read_only image2d_t albedoMap,
    __read_only image2d_t emissionMap,
    const uint seed,
    const uint triangleCount,
    __global float4* points,
    __global float4* colors,
    __global float4* emission,
    __global float4* normals
)
{
    //init indices and rand state
    int n = get_global_id(0)*3;
    state = get_global_id(0) + seed;
//    int resultIndex = get_global_id(0)*samplePerTri;

    //get vertex values and corrosponding uvs
    //vertA
    float v1a = vertices[indices[n]*3];
    float v1b = vertices[indices[n]*3+1];
    float v1c = vertices[indices[n]*3+2];
    float4 vertA = (float4)(v1a,v1b,v1c,0.0f);
    float2 uvA =uvs[indices[n]];
    //normal2
    float normal1x = normalsIn[indices[n]*3];
    float normal1y = normalsIn[indices[n]*3+1];
    float normal1z = normalsIn[indices[n]*3+2];
    float4 normalA = (float4)(normal1x,normal1y,normal1z,0.0f);


    //vert2
    float v2a = vertices[indices[n+1]*3];
    float v2b = vertices[indices[n+1]*3+1];
    float v2c = vertices[indices[n+1]*3+2];
    float4 vertB = (float4)(v2a,v2b,v2c,0.0f);
    float2 uvB =uvs[indices[n+1]];
    //normal2
    float normal2x = normalsIn[indices[n+1]*3];
    float normal2y = normalsIn[indices[n+1]*3+1];
    float normal2z = normalsIn[indices[n+1]*3+2];
    float4 normalB = (float4)(normal2x,normal2y,normal2z,0.0f);


    //vert3
    float v3a = vertices[indices[n+2]*3];
    float v3b = vertices[indices[n+2]*3+1];
    float v3c = vertices[indices[n+2]*3+2];
    float4 vertC = (float4)(v3a,v3b,v3c,0.0f);
    float2 uvC =uvs[indices[n+2]];
    //normal3
    float normal3x = normalsIn[indices[n+2]*3];
    float normal3y = normalsIn[indices[n+2]*3+1];
    float normal3z = normalsIn[indices[n+2]*3+2];
    float4 normalC = (float4)(normal3x,normal3y,normal3z,0.0f);



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
            float2 tempuv = uvA;
            uvA = uvB;
            uvB = tempuv;
        }
        else
        {
            float4 temp = vertA;
            vertA = vertC;
            vertC = temp;
            float2 tempuv = uvA;
            uvA = uvC;
            uvC = tempuv;
        }
    }
    else if(gamma < alpha)
    {
        float4 temp = vertA;
        vertA = vertC;
        vertC = temp;
        float2 tempuv = uvA;
        uvA = uvC;
        uvC = tempuv;
    }

    uint newSampleCount = samples[get_global_id(0)];
    uint resultIndex=0;
    //calculate new result ID, accumulate previous sample counts
    for(int i=0; i< get_global_id(0); i++)
    {
        resultIndex += samples[i];
    }

    //calculate the sample width for the newly generated sample count
    uint currentIt=0;
    uint sum = 0;
    while(sum < newSampleCount)
    {
        currentIt++;
        sum += currentIt;
    }
    uint sampleWidth = currentIt;//clamp(currentIt - 4u, 0u, currentIt);

    //generate normal && scale by strength
    //float4 normal = normalize(cross(vertB-vertA,vertC-vertA));
    //generate average of normals stored in vertices
    float4 normal =(normalA+normalB +normalC)/3.0f;
    float4 centerPoint = (vertA + vertB + vertC) / 3.f;

    //intSeed = centerPoint.x * 234 * intRand() + centerPoint.y * 57 * intRand() + centerPoint.z * 113 * intRand() ;

    //normal*=normalStrength;

    //generate samples
    __local float2 uniformOutput[maxPointsPerTri];
    //sampleUniformly(uniformOutput, newSampleCount, sampleWidth, dot(vertB-vertA, vertC-vertA));
    //__local float2 poissonOutput[maxPointsPerTri];
    PoissionSampling(uniformOutput, maxPointsPerTri);
    float4 islit = IsOccludedFromDirLight(vertA + normal * 0.1f, vertB + normal * 0.1f, vertC + normal * 0.1f, centerPoint + normal * 0.1f, triangleCount, vertices, indices, lightDir[0], n);

    float angle = acos(dot(normalize(vertB - vertA), normalize(vertC - vertA)));
    float2 centercoords = (float2)(mix(0.5f, 0.f, smoothstep(0.f, (float)(M_PI), angle)));

    //uint center = IsOccludedFromDirLight(centerPoint, triangleCount, vertices, indices, lightDir[0], n);
    //bool trianlgeIsLit = IntersectTriangles_MT97()

    //store generated samples
    for(int i = 0; i < newSampleCount; i++)
    {
        int index= resultIndex + i;
        //sample point position
        float4 newPoint = SampleTriangle(uniformOutput[i], vertA, vertB, vertC);

        //get uvs
        float2 uvCoordinates = SampleUVs(uniformOutput[i], uvA, uvB, uvC);
        //sample height based on uvs
        float4 emissionColor = sampleColor(emissionMap, uvCoordinates);
        float4 Color = sampleColor(albedoMap, uvCoordinates);


        float2 centerInterpolators = fabs(centercoords - uniformOutput[i]);
        centerInterpolators.x /= (centercoords.x > uniformOutput[i].x ? 1.f -  uniformOutput[i].x :  uniformOutput[i].x);
        centerInterpolators.y /= (centercoords.y > uniformOutput[i].y ? 1.f -  uniformOutput[i].y :  uniformOutput[i].y);

        emissionColor.w = sampleLight(uniformOutput[i], centerInterpolators, islit);

        if(Color.w <= 0.5f)
            Color.w = 60.f;
        else
            Color.w = 0.f;

        //Color = (float4)(uvCoordinates.x, uvCoordinates.y, 0, 1);
        //Color = (float4)(1,0,0,1);
        //scale normal by the height & add it to the point
        //newPoint+= normal*heightOffset;
      //  float l = sampleLight(uniformOutput[i],vertA,vertB,vertC);
       // Color= (float4)(l,l,l,0);
       // Color=normal;
        colors[index] = Color;
        points[index] = newPoint;
        emission[index] = emissionColor;
        normals[index] = normal;
    }
}

