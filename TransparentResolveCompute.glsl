#version 450 core
#extension EXT_shader_image_load_store: enable

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(binding = 4, r32f) uniform readonly image2D LDK_ENTRY;
layout(binding = 5, rgba32f) uniform image2D IMAGE;

struct LDK_FRAG
{
	vec4 color;
	uint ptr;
	float depth;
};

layout(std430, binding = 2) buffer LK_OPC
{
	LDK_FRAG ldk[];
};

layout(binding = 3) uniform atomic_uint LDKC;

#define K 16

struct FP
{
	uint ptr;
	float depth;
};

void insertLinkedList(uint node, uint inode)
{
	uint nextPtr = ldk[node].ptr;
	ldk[node].ptr = inode;
	ldk[inode].ptr = nextPtr;
}

uint Next(uint idx)
{
	return ldk[idx].ptr;
}

float Value(uint idx)
{
	return ldk[idx].depth;
}

float GetAlpha(uint ld)
{
	 return ldk[ld].color.w;
}

vec3 GetColor(uint ld)
{
	return ldk[ld].color.xyz;
}

void main()
{

	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	float front = imageLoad(LDK_ENTRY, pos).x;
	if (front != 0.0)
	{

		uint arr[K];

		uint cNode = uint(front);
		uint nodeAmount = 0;
		//while (cNode != 0)
		for (int arrIter = 0; arrIter < K; arrIter += 1)
		{
			nodeAmount += uint(cNode != 0);
			arr[arrIter] = cNode;
			cNode = Next(cNode);
		}
		
		uint ndl;
		uint t;

		for (uint i = 0; i < nodeAmount; i += 1)
		{
			ndl = i;

			for (uint l = i + 1; l < nodeAmount; l += 1)
			{
				if (Value(arr[ndl]) < Value(arr[l]))
					ndl = l;
			}

			t = arr[i];
			arr[i] = arr[ndl];
			arr[ndl] = t;
			
		}

		//new_color = (alpha)*(foreground_color) + (1 - alpha)*(background_color)
		vec3 newColor = imageLoad(IMAGE, ivec2(gl_GlobalInvocationID)).xyz;

		uint cid;
		for (int i = 0; i < nodeAmount; i += 1)
		{
			cid = arr[i];
			newColor = GetAlpha(cid) * GetColor(cid) + (1 - GetAlpha(cid)) * newColor;

		}

		imageStore(IMAGE, ivec2(gl_GlobalInvocationID), vec4(newColor, 1.0));
		/*
		if (gl_GlobalInvocationID.xy == uvec2(400, 400))
		{
			for (int i = 0; i < K; i++)
			{
				values[i + K + 1].color = ldk[arr[i]].color;
				values[i + K + 1].ptr = ldk[arr[i]].ptr;
				values[i + K + 1].depth = ldk[arr[i]].depth;
				values[i + K + 1].lIdx = arr[i];
				values[i + K + 1].ichk = uint(i);
			}
		}
		*/
	}
}