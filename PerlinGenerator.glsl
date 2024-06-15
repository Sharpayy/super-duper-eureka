
#version 450 core

layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;



layout(std430, binding = 5) buffer BIOMES
{
    int d[];
};

layout(std140, binding = 1) uniform PERLIN_DATA
{
    int map_x;
    int map_y;
    double persistence;
    double frequency;
    double amplitude;
    int octaves;
    int randomseed;
};

/*
#define map_x 10000
#define map_y 10000

#define persistence 1.0
#define frequency 2.5
#define amplitude 1.0
#define octaves 7
#define randomseed 21324
*/

#define MOUNTAIN 0
#define HILL 1
#define FOREST 2
#define TAIGA 3
#define TUNDRA 4
#define SAVANNA 5
#define GRASSLAND 6
#define PLAINS 7
#define SWAMP 8
#define BEACH 9
#define JUNGLE 10
#define DESERT 11
#define SNOW 12
#define WATER 13
#define LAKE 14
#define RIVER 15
#define STREAM 16
#define BROOK 17
#define POND 18
#define CREEK 19

struct Biome {
	int type;
	float thr;
	float c1;
	float c2;
	float c3;
};

Biome biomes[10] = Biome[10](
	Biome( SNOW, 200.0, 255, 250, 250 ),
    Biome( MOUNTAIN, 191.25, 139, 137, 137 ),
	Biome( HILL, 155.5, 100, 100, 100 ),
	Biome( TUNDRA, 125.5, 0, 90, 0 ),
	Biome( GRASSLAND, 76.5, 34, 139, 34 ),
	Biome( SAVANNA, 100.5, 0, 110, 0 ),
	Biome( DESERT, 51.0, 237, 201, 175 ),
	Biome( BEACH, 38.25, 238, 214, 175 ),
	Biome( SWAMP, 25.5, 47, 79, 79 ),
	Biome( WATER, 0.0, 0, 105, 148 )
);

layout(binding = 2, rgba8) uniform image2D WRITE_TEXTURE;

double Noise(int x, int y)
{
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - double(t) * 0.931322574615478515625e-9;/// 1073741824.0);
}

double Interpolate(double x, double y, double a)
{
    double negA = 1.0 - a;
    double negASqr = negA * negA;
    double fac1 = 3.0 * (negASqr)-2.0 * (negASqr * negA);
    double aSqr = a * a;
    double fac2 = 3.0 * aSqr - 2.0 * (aSqr * a);

    return x * fac1 + y * fac2; //add the weighted factors
}

double GetValue(double x, double y)
{
    int Xint = int(x);
    int Yint = int(y);
    double Xfrac = x - Xint;
    double Yfrac = y - Yint;

    //noise values
    double n01 = Noise(Xint - 1, Yint - 1);
    double n02 = Noise(Xint + 1, Yint - 1);
    double n03 = Noise(Xint - 1, Yint + 1);
    double n04 = Noise(Xint + 1, Yint + 1);
    double n05 = Noise(Xint - 1, Yint);
    double n06 = Noise(Xint + 1, Yint);
    double n07 = Noise(Xint, Yint - 1);
    double n08 = Noise(Xint, Yint + 1);
    double n09 = Noise(Xint, Yint);

    double n12 = Noise(Xint + 2, Yint - 1);
    double n14 = Noise(Xint + 2, Yint + 1);
    double n16 = Noise(Xint + 2, Yint);

    double n23 = Noise(Xint - 1, Yint + 2);
    double n24 = Noise(Xint + 1, Yint + 2);
    double n28 = Noise(Xint, Yint + 2);

    double n34 = Noise(Xint + 2, Yint + 2);

    //find the noise values of the four corners
    double x0y0 = 0.0625 * (n01 + n02 + n03 + n04) + 0.125 * (n05 + n06 + n07 + n08) + 0.25 * (n09);
    double x1y0 = 0.0625 * (n07 + n12 + n08 + n14) + 0.125 * (n09 + n16 + n02 + n04) + 0.25 * (n06);
    double x0y1 = 0.0625 * (n05 + n06 + n23 + n24) + 0.125 * (n03 + n04 + n09 + n28) + 0.25 * (n08);
    double x1y1 = 0.0625 * (n09 + n16 + n28 + n34) + 0.125 * (n08 + n14 + n06 + n24) + 0.25 * (n04);

    //interpolate between those values according to the x and y fractions
    double v1 = Interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
    double v2 = Interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
    double fin = Interpolate(v1, v2, Yfrac);  //interpolate in y direction

    return fin;
}

double Total(double i, double j)
{
    //properties of one octave (changing each loop)
    double t = 0.0f;
    double _amplitude = 1;
    double freq = frequency;

    for (int k = 0; k < octaves; k++)
    {
        t += GetValue(j * freq + randomseed, i * freq + randomseed) * _amplitude;
        _amplitude *= persistence;
        freq *= 2;
    }

    return t;
}

double GetHeight(double x, double y)
{
    return amplitude * Total(x, y);
}

void main()
{
    double val = (GetHeight(double(gl_GlobalInvocationID.x) / map_x, double(gl_GlobalInvocationID.y) / map_y) * 0.5f + 0.5f) * 255.0;
    uint d_idx = gl_GlobalInvocationID.y * map_y + gl_GlobalInvocationID.x;
    vec3 color = vec3(0.2, 0.3, 0.5);
    for (int i = 0; i < 10; i++) {
		if (val >= biomes[i].thr) {
			color = vec3(biomes[i].c1, biomes[i].c2, biomes[i].c3);
            d[d_idx] = (biomes[i].type << 16) + int(biomes[i].thr);
            
            break;
		}
	}
    imageStore(WRITE_TEXTURE, ivec2(gl_GlobalInvocationID.xy), vec4(color / 255.0, 1.0));
}
