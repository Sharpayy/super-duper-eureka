#include "iRendered.h"
#include "iAllocators.h"

uint32_t POOLPAGE_MEDIUM_NEW_ALLOCATE = 8;
POOLMEDIUMPAGES poolAllocator = POOLMEDIUMPAGES(64, 32, sizeof(RenderModel));
PerformanceTimer RenderGlobalTimer = GetEmptyTimer();


void iinit()
{
	glewInit();
	InitEmptyTimer(&RenderGlobalTimer);
}

void ApplyTextureStdFilters(uint32_t id)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ApplyTextureStdFilters(Texture2D tex)
{
	tex.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ApplyBindedTextureStdFilters()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void BindExternalTex2dLd0(uint32_t tex)
{
	BindExternalTexture2d(GL_TEXTURE0, tex);
}

void BindExternalTexture2d(uint32_t ld, uint32_t tx)
{
	glActiveTexture(ld);
	glBindTexture(GL_TEXTURE_2D, tx);
}

void BindSampler(const char* name, int32_t value, uint32_t prgm)
{
	glUniform1i(glGetUniformLocation(prgm, name), value);
}

uint32_t CalcFps(CPUPerformanceTimer cpt, LoopStatistics* ls)
{
	uint32_t t = ls->ticks;

	ls->tick_time_left -= cpt.GetElapsed();
	if (ls->tick_time_left <= 0)
	{
		ls->tick_time_left += CLOCKS_PER_SEC;
		ls->ticks = 1;
		return t;
	}
	ls->ticks++;
	return 0;
}

PerformanceTimer GetEmptyTimer()
{
	return PerformanceTimer(0);
}

void InitEmptyTimer(PerformanceTimer* tm)
{
	glGenQueries(1, &tm->id);
}

void InitGlobalTimer()
{
	RenderGlobalTimer = PerformanceTimer();
}

void BoolSwap(bool* a, bool* b)
{
	bool t = *b;
	*b = *a;
	*a = t;
}

VertexBuffer::VertexBuffer()
{
	glGenVertexArrays(1, &id);
	//glBindVertexArray(id);
}

void VertexBuffer::bind()
{
	glBindVertexArray(id);
}

void VertexBuffer::addAttrib(GLenum type, uint32_t index, size_t size, uint32_t stride, uint32_t first)
{
	glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)first);
}

void VertexBuffer::enableAttrib(uint32_t index)
{
	glEnableVertexAttribArray(index);
}

void VertexBuffer::disableAttrib(uint32_t index)
{
	glDisableVertexAttribArray(index);
}

UniformBufferObject::UniformBufferObject()
{
	glGenBuffers(1, &id);
}

void UniformBufferObject::data(size_t size, void* data, GLenum usage)
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
}

void UniformBufferObject::bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
}

void UniformBufferObject::subdata(size_t offset, size_t size, void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void UniformBufferObject::bindBase(uint32_t index)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, index, id);
}

Program::Program()
{
	id = glCreateProgram();
}

void Program::programCompile()
{
	glLinkProgram(id);
}

void Program::programAddShader(uint32_t shdr)
{
	glAttachShader(id, shdr);
}

int Program::programGetDebugInfo(char* bf, size_t bf_size)
{
	int v;
	glGetProgramiv(id, GL_LINK_STATUS, &v);

	if (v == 1)
		return v;

	int wr;
	glGetProgramInfoLog(id, bf_size, &wr, bf);
	return v;
}

void Program::use()
{
	glUseProgram(id);
}


ShaderStorageBufferObject::ShaderStorageBufferObject()
{
	glGenBuffers(1, &id);
}

void ShaderStorageBufferObject::data(size_t size, void* data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void ShaderStorageBufferObject::bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
}

void ShaderStorageBufferObject::bindBase(uint32_t index)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
}

void ShaderStorageBufferObject::subdata(size_t offset, size_t size, void* data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

RenderGL::RenderGL(uint32_t model_amount)
{
	models = DynamicList<RenderModel*>(model_amount);
	events = STACK<RenderEvent>(128);
	idTranslator = DynamicList<uint32_t>(model_amount);

	md = nullptr;
	mdi = 0;

	POOLPAGE_MEDIUM_NEW_ALLOCATE = 8;

	MVP = InShaderMVP_DATA{ glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) };

	uMVP = UniformBufferObject();
	uMVP.data(64 * 3, &MVP, GL_DYNAMIC_COPY);

	uMVP.bindBase(RENDERER_SHADER_MPV_DATA_LOCATION);
	/*
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
	*/

}

void RenderGL::setCameraMatrix(glm::mat4 cam)
{
	MVP.matCamera = cam;
	MVP.matProjCamera = MVP.matProj * MVP.matCamera;
}

void RenderGL::setProjectionMatrix(glm::mat4 proj)
{
	MVP.matProj = proj;
	MVP.matProjCamera = MVP.matProj * MVP.matCamera;
}

void RenderGL::UpdateShaderDataProjection()
{
	uMVP.subdata(0, SIZEOF_MAT4, &MVP.matProj);
	uMVP.subdata(SIZEOF_MAT4 * 2, SIZEOF_MAT4, &MVP.matProjCamera);
}

void RenderGL::UpdateShaderData()
{
	uMVP.subdata(0, SIZEOF_MAT4 * 3, &MVP);
}

void RenderGL::UpdateShaderDataCamera()
{
	uMVP.subdata(SIZEOF_MAT4, SIZEOF_MAT4, &MVP.matCamera);
	uMVP.subdata(SIZEOF_MAT4 * 2, SIZEOF_MAT4, &MVP.matProjCamera);
}

glm::mat4* RenderGL::GetShaderCameraMatrixLocation()
{
	return &MVP.matCamera;
}

RenderModel* RenderGL::getModel(uint32_t id_model)
{
	return models.at(idTranslator.at(id_model));
}

RenderModelDetails* RenderGL::getModelDetails(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	RenderModel* md_ = models.at(idTranslator.at(m_id));
	return md_->objects.at(md_->dlIndexSpace.at(md_->objectsId.at(o_id)));
}

void RenderGL::newModel(uint32_t id_model, VertexBuffer vao, Program prgm, uint32_t vertices, GLenum rtype, Texture2D texture, uint32_t o_amount)
{
	RenderModel* md = (RenderModel*)poolAllocator.allocAligned(sizeof(RenderModel));
	*(idTranslator.base_ptr + id_model) = models.c_size;

	md->vao = vao;
	md->std_prgm = prgm;
	md->vertices = vertices;
	md->renedrPrimitive = rtype;
	md->std_texture2d = texture;

	md->matrixSSBO = ShaderStorageBufferObject();
	md->matrixSSBO.data(o_amount * 64, NULL);
	md->matrixSSBO.bindBase(RENDERER_SHADER_MATRIX_SPACE_LOCATION);
	md->objectsId = DynamicList<uint32_t>(o_amount);
	md->stFreeIdSpace = STACK<DoubleInt32>(o_amount);

	md->idSpaceSSBO = ShaderStorageBufferObject();
	md->idSpaceSSBO.data(o_amount * 4, NULL);
	md->matrixSSBO.bindBase(RENDERER_SHADER_IDX_SPACE_LOCATION);
	md->dlIndexSpace = DynamicList<uint32_t>(o_amount);

	md->objAmount = 0;
	md->activeObjects = 0;
	md->objects = DynamicList<RenderModelDetails*>(o_amount);

	models.push_back(md);
}

RENDER_OBJECT_ID RenderGL::newObject(uint32_t id_model, glm::mat4 s_mat)
{
	if (mdi != id_model)
		BindActiveModel(id_model);

	RenderModelDetails* rmd = (RenderModelDetails*)poolAllocator.allocAligned(sizeof(RenderModelDetails));
	uint32_t _object_id = -1;

	DoubleInt32 spaceIdIdx = GetFreeMdIdSpaceIndex();


	rmd->matrixId = spaceIdIdx.b;
	rmd->model = s_mat;
	rmd->render = true;
	rmd->objectId = spaceIdIdx.a;

	//md->objects.push_back(rmd);
	*md->objects.atp(spaceIdIdx.a) = rmd;
	*(md->objectsId.base_ptr + spaceIdIdx.a) = md->objAmount;
	md->objAmount++;
	md->activeObjects++;

	SetIdxSpaceValue(md->objAmount - 1, spaceIdIdx.b);
	*(md->dlIndexSpace.base_ptr + md->objAmount - 1) = spaceIdIdx.b;
	SetObjectMatrixMem(spaceIdIdx.b, s_mat);

	_object_id = spaceIdIdx.a;

	if (md->activeObjects != md->objAmount)
		EnableObjectInternal(_object_id);

	return _object_id;
}

RENDER_OBJECT_ID RenderGL::newObject(uint32_t id_model, glm::mat4 s_mat, uint64_t* longId)
{
	if (mdi != id_model)
		BindActiveModel(id_model);

	RenderModelDetails* rmd = (RenderModelDetails*)poolAllocator.allocAligned(sizeof(RenderModelDetails));
	uint32_t _object_id = -1;

	DoubleInt32 spaceIdIdx = GetFreeMdIdSpaceIndex();

	rmd->matrixId = spaceIdIdx.b;
	rmd->model = s_mat;
	rmd->render = true;
	rmd->objectId = spaceIdIdx.a;

	//md->objects.push_back(rmd);
	*md->objects.atp(spaceIdIdx.a) = rmd;
	*(md->objectsId.base_ptr + spaceIdIdx.a) = md->objAmount;
	md->objAmount++;
	md->activeObjects++;

	SetIdxSpaceValue(md->objAmount - 1, spaceIdIdx.b);
	*(md->dlIndexSpace.base_ptr + md->objAmount - 1) = spaceIdIdx.b;
	SetObjectMatrixMem(spaceIdIdx.b, s_mat);

	_object_id = spaceIdIdx.a;

	if (md->activeObjects != md->objAmount)
		EnableObjectInternal(_object_id);

	((RENDER_LONG_ID*)longId)->ModelId = mdi;
	((RENDER_LONG_ID*)longId)->ObjectId = _object_id;
	return _object_id;
}

void RenderGL::RenderSelectedModel(uint32_t id_model)
{
	RenderModel* md = models.at(idTranslator.at(id_model));

	if (md->activeObjects == 0)
		return;

	md->std_prgm.use();
	md->vao.bind();
	md->matrixSSBO.bindBase(RENDERER_SHADER_MATRIX_SPACE_LOCATION);
	md->idSpaceSSBO.bindBase(RENDERER_SHADER_IDX_SPACE_LOCATION);
	md->std_texture2d.bind();

	glDrawElementsInstanced(md->renedrPrimitive, md->vertices, GL_UNSIGNED_INT, NULL, md->activeObjects);

}

void RenderGL::RenderAll() // *
{
	for (int i = 0; i < models.c_size; i++)
	{
		RenderSelectedModel(i);
	}
}

void RenderGL::deleteObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);

	uint32_t lastObjIdx = md->objAmount - 1;
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	RenderModelDetails* obj = GetMdObject(o_id);

	NotifyFreeIdx(o_id, obj->matrixId);
	//md->objects.del_last();
	md->objAmount--;
	//poolAllocator.freeAlignedMemory(obj);

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void RenderGL::SyncObjectMatrix(RENDER_MODEL_ID o_id)
{
	RenderModelDetails* obj = GetMdObject(MapObjectToSpaceIdx(MapToObjectIdx(o_id)));
	md->matrixSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, SIZEOF_MAT4 * obj->matrixId, SIZEOF_MAT4, &obj->model[0].x);
}

void RenderGL::SetObjectMatrix(RENDER_MODEL_ID o_id, glm::mat4 mat, bool just_in_vram = false)
{
	RenderModelDetails* obj = GetMdObject(o_id);
	if (just_in_vram == 0)
		obj->model = mat;

	if (obj->render == true)
	{
		md->matrixSSBO.bind();
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, obj->matrixId * SIZEOF_MAT4, SIZEOF_MAT4, &mat[0].x);
	}
}

glm::mat4 RenderGL::GetObjectMatrix(RENDER_MODEL_ID o_id)
{
	return GetMdObject(o_id)->model;
}

void RenderGL::EnableObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);
	EnableObject(o_id);
}

void RenderGL::EnableObject(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd() + 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == true)
		return;

	md->activeObjects++;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void RenderGL::EnableObjectInternal(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd());
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void RenderGL::DisableObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);
	DisableObject(o_id);
}

void RenderGL::DisableObject(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetIdFirstMdInactiveObject() - 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == false)
		return;

	md->activeObjects--;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void RenderGL::EnableObjectNoSync(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd() + 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == true)
		return;

	md->activeObjects++;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrderNoSync(cObjectIdx, lastObjIdx);
}

void RenderGL::DisableObjectNoSync(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetIdFirstMdInactiveObject() - 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == false)
		return;

	md->activeObjects--;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrderNoSync(cObjectIdx, lastObjIdx);
}

uint32_t RenderGL::MapObjectToSpaceIdx(uint32_t obj)
{
	return md->dlIndexSpace.at(obj);
}

bool RenderGL::IsObjectActive(RENDER_OBJECT_ID o_id)
{
	uint32_t ObjIdx = MapToObjectIdx(o_id);
	return (ObjIdx < md->activeObjects);
}

void RenderGL::MakeModelRef(uint32_t new_m_id, uint32_t ref_m_id)
{
	RenderModel* copy_model = GetRenderModelFromIndex(ref_m_id);

	*(idTranslator.base_ptr + new_m_id) = models.c_size;
	models.push_back(copy_model);
}

void RenderGL::SetModelShader(Program shdr)
{
	md->std_prgm = shdr;
}

void RenderGL::SetModelVertexArray(VertexBuffer vtx, uint32_t vtx_cnt)
{
	md->vao = vtx;
	md->vertices = vtx_cnt;
}

void RenderGL::SetModelTexture(Texture2D tex)
{
	md->std_texture2d = tex;
}

VertexBuffer RenderGL::GetModelVertexArray()
{
	return md->vao;
}

uint32_t RenderGL::GetModelObjectCapacity()
{
	return md->objects.reserved;
}

void RenderGL::UpdateShaderIdSpace(uint32_t m_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);

	md->idSpaceSSBO.bind();
	md->idSpaceSSBO.subdata(0, md->objAmount * 4, md->dlIndexSpace.base_ptr);
}

void RenderGL::DisableObjectL(uint64_t longId)
{
	RENDER_LONG_ID ld = *((RENDER_LONG_ID*)&longId);
	if (mdi != ld.ModelId)
		BindActiveModel(ld.ModelId);

	DisableObject(ld.ObjectId);
}

void RenderGL::EnableObjectL(uint64_t longId)
{
	RENDER_LONG_ID ld = *((RENDER_LONG_ID*)&longId);
	if (mdi != ld.ModelId)
		BindActiveModel(ld.ModelId);

	EnableObject(ld.ObjectId);
}

DoubleInt32 RenderGL::GetFreeIdSpaceIndex(uint32_t m_id)
{
	RenderModel* md = GetRenderModelFromIndex(GetTranslatedModelIndex(m_id));

	if (md->stFreeIdSpace.c_size == 0)
	{
		md->objectsId.push_back(-1);
		return DoubleInt32{ md->objectsId.c_size - 1, md->objectsId.c_size - 1 };
	}

	DoubleInt32 value_ = md->stFreeIdSpace.fget();
	md->stFreeIdSpace.pop();
	return value_;
}

uint32_t RenderGL::GetTranslatedModelIndex(uint32_t m_id)
{
	return idTranslator.at(m_id);
}

RenderModel* RenderGL::GetRenderModelFromIndex(uint32_t m_idx)
{
	return models.at(m_idx);
}

uint32_t RenderGL::GetActiveObjectsInModel(uint32_t m_id)
{
	return GetRenderModelFromIndex(GetTranslatedModelIndex(m_id))->activeObjects;
}

void RenderGL::BindActiveModel(uint32_t m_id)
{
	md = GetRenderModelFromIndex(GetTranslatedModelIndex(m_id));
	mdi = m_id;
}

uint32_t RenderGL::GetActiveObjectsInMd()
{
	return md->activeObjects - 1;
}

DoubleInt32 RenderGL::GetFreeMdIdSpaceIndex()
{
	if (md->stFreeIdSpace.c_size == 0)
	{
		md->objectsId.push_back(-1);
		return DoubleInt32{ md->objectsId.c_size - 1, md->objectsId.c_size - 1 };
	}

	DoubleInt32 value_ = md->stFreeIdSpace.fget();
	md->stFreeIdSpace.pop();
	return value_;
}

uint32_t RenderGL::GetIdFirstMdInactiveObject()
{
	return md->activeObjects;
}

void RenderGL::SwapObjectOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<RenderModelDetails>(GetMdObject(idx0), GetMdObject(idx1));
}

void inline RenderGL::SwapObjectIdxOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->objectsId.atp(idx0), md->objectsId.atp(idx1));
}

void inline RenderGL::SwapInBufferIdxOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->dlIndexSpace.atp(idx0), md->dlIndexSpace.atp(idx1));

	//uint32_t value0, value1;
	md->idSpaceSSBO.bind();
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx0 * 4, 4, &value0);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx1 * 4, 4, &value1);

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx0 * 4, 4, md->dlIndexSpace.atp(idx0));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx1 * 4, 4, md->dlIndexSpace.atp(idx1));
}

void RenderGL::SwapInBufferIdxOrderNoSync(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->dlIndexSpace.atp(idx0), md->dlIndexSpace.atp(idx1));
}

RenderModelDetails* RenderGL::GetMdObject(uint32_t idx)
{
	return md->objects.at(idx);
}

void RenderGL::SetObjectMatrixMem(uint32_t idx, glm::mat4 mat)
{
	md->matrixSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * SIZEOF_MAT4, SIZEOF_MAT4, &mat[0].x);
}

uint32_t RenderGL::MapToObjectIdx(uint32_t id)
{
	return md->objectsId.at(id);
	//return md->dlIndexSpace.at(id);
}

uint32_t RenderGL::MapToIndexSpace(uint32_t idx)
{
	uint32_t value;
	md->idSpaceSSBO.bind();
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * 4, 4, &value);
	return value;
}

void RenderGL::SetIdxSpaceValue(uint32_t idx, uint32_t value)
{
	md->idSpaceSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * 4, 4, &value);
}

void RenderGL::NotifyFreeIdx(uint32_t idx, uint32_t mat_)
{
	md->stFreeIdSpace.put(DoubleInt32{ idx, mat_ });
}

Texture2D::Texture2D()
{
	id = 0;
}

void RenderGL::BindMVP()
{
	uMVP.bind();
	uMVP.bindBase(RENDERER_SHADER_MPV_DATA_LOCATION);
}

Texture2D::Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, data_int, x, y, 0, data_type, GL_UNSIGNED_BYTE, data);
}

Texture2D::Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int, uint32_t bind_location)
{
	glGenTextures(1, &id);
	glActiveTexture(bind_location);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, data_int, x, y, 0, data_type, GL_UNSIGNED_BYTE, data);
}

void Texture2D::genMipmap()
{
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::bind()
{
	glBindTexture(GL_TEXTURE_2D, id);
}

AtomicCounter::AtomicCounter(uint32_t svalue, uint32_t bp)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);

	value = svalue;
	bpoint = bp;

	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &value, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bpoint, id);
}

void AtomicCounter::bind()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
}

void AtomicCounter::bindbase()
{
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bpoint, id);
}

void AtomicCounter::setBindingPoint(uint32_t bp)
{
	bpoint = bp;
}

void* AtomicCounter::map()
{
	return glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
}

void AtomicCounter::unmap()
{
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void AtomicCounter::sync(ATOMIC_COUNTER_OPERATION op)
{
	if (op == ATC_SYNC_SHADER)
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
		void* mp = map();
		value = *(uint32_t*)mp;
		unmap();
		return;
	}

	if (op == ATC_SYNC_CPU)
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, 4, &value);
		return;
	}
}

void AtomicCounter::zero()
{
	uint32_t zero = 1;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, 4, &zero);
}

void AtomicCounter::bindBaseCustom(uint32_t bp)
{
	bind();
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bp, id);
}

Framebuffer::Framebuffer()
{
	glGenFramebuffers(1, &id);
}

void Framebuffer::attachTextureId(uint16_t plc, uint32_t tex, GLenum attachTo)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachTo, GL_TEXTURE_2D, tex, 0);
	*(texture_id + plc) = tex;
}

void Framebuffer::attachTexture(uint16_t plc, Texture2D tex, GLenum attachTo)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachTo, GL_TEXTURE_2D, tex.id, 0);
	*(texture_id + plc) = tex.id;
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

uint32_t Framebuffer::getAttachment(uint16_t plc)
{
	return *(texture_id + plc);
}

void Framebuffer::clear(GLenum comp)
{
	bind();
	glClear(comp);
}

PerformanceTimer::PerformanceTimer()
{
	glGenQueries(1, &id);
	v = -1;
}

PerformanceTimer::PerformanceTimer(uint32_t qid)
{
	id = qid;
	v = -1;
}

void PerformanceTimer::TimeStart()
{
	glBeginQuery(GL_TIME_ELAPSED, id);
}

void PerformanceTimer::TimeEnd()
{
	glFinish();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(id, GL_QUERY_RESULT, (GLuint64*)&v);
}

void PerformanceTimer::Reset()
{
	v = -1;
}

int64_t PerformanceTimer::GetElapsedTime()
{
	if (v == -1)
		return 0;
	return v;
}

CPUPerformanceTimer::CPUPerformanceTimer()
{
	s_time = 0;
	e_time = 0;
}

void CPUPerformanceTimer::Reset()
{
	s_time = 0;
	e_time = 0;
}

void CPUPerformanceTimer::Start()
{
	s_time = clock();
}

void CPUPerformanceTimer::End()
{
	e_time = clock();
}

clock_t CPUPerformanceTimer::GetElapsed()
{
	return e_time - s_time;
}

LoopStatistics::LoopStatistics()
{
	tick_time_left = CLOCKS_PER_SEC;
	ticks = 0;
}
