#include "ExampleRenderer.h"
#include "RenderObject.h"
#include "../Plugins/PlayStation4/PS4Shader.h"
#include "RenderObject.h"



using namespace NCL;
using namespace NCL::PS4;

ExampleRenderer::ExampleRenderer(PS4Window* window) : PS4RendererBase(window)
{
	rotation = 0.0f;
	//defaultMesh = MeshGeometry("");
	
	defaultObject[0] = new RenderObject((MeshGeometry*)setMesh("/app0/centeredcube.obj"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	//defaultObject[0] = new RenderObject((MeshGeometry*)setMesh("/app0/building1.obj"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);



	defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/bunny.obj"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	defaultObject[1]->SetBasicTex("/app0/test.gnf");
//	defaultObject[1]->SetBumpTex("/app0/test.gnf");
//	defaultObject[0]->SetBumpTex("/app0/test.gnf");


//	l->loadOBJ("/app0/bunny.obj");
//	l->setAttrib(*msh);
	
//	defaultObject[0] = new RenderObject((MeshGeometry*)msh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);


//	defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/sphere2.msh"), (ShaderBase*)defaultShader, (TextureBase*)SetTexture("/app0/test.gnf"));


}

ExampleRenderer::~ExampleRenderer()
{
	delete defaultObject[0];
	delete defaultObject[1];
	
}
Maths::Vector3 obj1Trans = Maths::Vector3(0.4, 0, -0.3);
void ExampleRenderer::Update(float dt, float x, float y) {
	rotation += dt;

	obj1Trans = obj1Trans + Vector3(0.01*x, -0.01*y, 0);
	defaultObject[1]->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5,0.5))*Matrix4::Translation(obj1Trans));

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

	//DrawMesh(myMesh);
	//DrawCube(*defaultCube);
	DrawObject(o);
}
