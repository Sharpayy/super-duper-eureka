#include "AManager.h"

//SDL_Point mousePos;
/*

CollisionDrawer::CollisionDrawer(uint32_t cnt, uint32_t maxObjects)
{
	bufferCount = cnt;
	buffers = (Buffer<GL_ARRAY_BUFFER>*)new uint32_t[cnt];
	buffersSize = new uint32_t[cnt];
	currentArray = new float[maxObjects];
	currentArraySize = maxObjects;

	glGenBuffers(cnt, (uint32_t*)buffers);
}

void CollisionDrawer::UpdateSingleData(uint32_t type, float c, uint32_t idx)
{
	buffers[type].bind();
	glBufferSubData(GL_ARRAY_BUFFER, idx * 4, 4, &c);
}

void CollisionDrawer::AddCollisionBuffer(uint32_t type, uint32_t amount, VertexBuffer vao)
{
	vao.bind();
	buffers[type].bind();
	buffers[type].data(max(amount, currentArraySize) * 4, 0, GL_DYNAMIC_DRAW);
	buffersSize[type] = max(amount, currentArraySize);

	AddCollisionBuffera(vao, buffers[type]);
}

void CollisionDrawer::FetchCollisionBuffer(uint32_t type)
{
	buffers[type].bind();
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, max(buffersSize[type], currentArraySize), currentArray);
}

void CollisionDrawer::SendCollisionBuffer(uint32_t type)
{
	buffers[type].bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, max(buffersSize[type], currentArraySize), currentArray);
}

*/

void AddCollisionBuffera(VertexBuffer vao, Buffer<GL_ARRAY_BUFFER> obj, uint32_t first)
{
	vao.bind();
	obj.bind();

	vao.addAttrib(GL_FLOAT, 2, 1, 4, 0);
	vao.enableAttrib(2);

	glVertexAttribDivisor(2, 1);
}

CollisionDrawer::CollisionDrawer(uint32_t objs)
{
	buffer = Buffer<GL_ARRAY_BUFFER>(objs * 4, 0, GL_DYNAMIC_DRAW);
	mappedArray = new float[objs];
	offset = 0;
	size = objs;

}

void CollisionDrawer::AddCollisionBuffer(uint32_t amount, VertexBuffer vao)
{
	vao.bind();
	buffer.bind();

	vao.addAttrib(GL_FLOAT, 2, 1, 4, offset * 4);
	vao.enableAttrib(2);
	glVertexAttribDivisor(2, 1);
	offset += amount;
}

void CollisionDrawer::FetchCollisionBuffer()
{
	return;
}

void CollisionDrawer::SendCollisionBuffer()
{
	buffer.bind();
	//buffer.data(size * 4, mappedArray, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size * 4, mappedArray);
}

void CollisionDrawer::UpdateSingleData(uint32_t offset, float c, uint32_t idx)
{
	if (mappedArray == nullptr)
		return;

	mappedArray[offset + idx] = c;
}