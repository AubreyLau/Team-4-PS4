#include "RenderObject.h"
#include "../Common/ShaderBase.h"
#include "../Common/TextureBase.h"

using namespace NCL;
using namespace NCL::Rendering;
using namespace NCL::PS4;

RenderObject::RenderObject()
{
	mesh	= nullptr;
	shader	= nullptr;

	for (int i = 0; i < TEXTURE_COUNT; ++i) {
		textures[i] = nullptr;
	}
}

RenderObject::~RenderObject()
{

}

RenderObject::RenderObject(MeshGeometry* m, NCL::Rendering::ShaderBase* s, NCL::Rendering::TextureBase* t,NCL::Rendering::TextureBase* t1,NCL::Rendering::TextureBase* t2) {
	mesh		= m;
	shader		= s;
	textures[0] = t;
	if (t1) {
	textures[1] = t1;
	}
	else textures[1] = nullptr;
	if (t2) {
	textures[2] = t2;
	}
	else textures[2] = nullptr;

}

const Maths::Matrix4& RenderObject::GetLocalTransform() const {
	return localTransform;
}

void	RenderObject::SetLocalTransform(const Maths::Matrix4& mat) {
	localTransform = mat;
}


//C:\Program Files (x86)\SCE\ORBIS SDKs\6.000\target\samples\sample_code\graphics\hdr_sample