#include "ExampleRenderer.h"
#include "RenderObject.h"
#include "../Plugins/PlayStation4/PS4Shader.h"
#include "RenderObject.h"

using namespace NCL;
using namespace NCL::PS4;

ExampleRenderer::ExampleRenderer(PS4Window* window) : PS4RendererBase(window)
{
	rotation = 0.0f;
	defaultObject[0] = new RenderObject((MeshGeometry*)defaultMesh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	defaultObject[1] = new RenderObject((MeshGeometry*)defaultMesh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
}

ExampleRenderer::~ExampleRenderer()
{
	delete defaultObject[0];
	delete defaultObject[1];
}

void ExampleRenderer::Update(float dt)	{
	rotation += dt;

	defaultObject[0]->SetLocalTransform(Matrix4::Translation(Vector3(-0.4, 0, 0)) * Matrix4::Rotation(rotation, Vector3(0,0,1)));

	defaultObject[1]->SetLocalTransform(Matrix4::Translation(Vector3(0.4, 0, 0)));
}

void ExampleRenderer::RenderActiveScene() {
	DrawRenderObject(defaultObject[0]);
	DrawRenderObject(defaultObject[1]);
}

void ExampleRenderer::DrawRenderObject(RenderObject* o) {
	Matrix4* transformMat = (Matrix4*)currentGFXContext->allocateFromCommandBuffer(sizeof(Matrix4), Gnm::kEmbeddedDataAlignment4);
	*transformMat = o->GetLocalTransform();

	Gnm::Buffer constantBuffer;
	constantBuffer.initAsConstantBuffer(transformMat, sizeof(Matrix4));
	constantBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	PS4Shader* realShader = (PS4Shader*)o->GetShader();

	int objIndex = realShader->GetConstantBuffer("RenderObjectData");
	int camIndex = realShader->GetConstantBuffer("CameraData");

	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, objIndex, 1, &constantBuffer);
	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, camIndex, 1, &cameraBuffer);

	realShader->SubmitShaderSwitch(*currentGFXContext);

	DrawMesh(*defaultMesh);
}
