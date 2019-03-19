#include "ExampleRenderer.h"
#include "RenderObject.h"
#include "../Plugins/PlayStation4/PS4Shader.h"
#include "RenderObject.h"



using namespace NCL;
using namespace NCL::PS4;

ExampleRenderer::ExampleRenderer(PS4Window* window) : PS4RendererBase(window)
{
	//rotation = 0.0f;
	////defaultMesh = MeshGeometry("");
	//
	//defaultObject[0] = new RenderObject((MeshGeometry*)setMesh("/app0/RobotHead.msh"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);


	//defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/sphere2.msh"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	//defaultObject[1]->SetBasicTex("/app0/test.gnf");
	//defaultObject[1]->SetBumpTex("/app0/test.gnf");
	//defaultObject[0]->SetBumpTex("/app0/test.gnf");


	//l->loadOBJ("/app0/bunny.obj");
	//l->setAttrib(*msh);
	
//	defaultObject[0] = new RenderObject((MeshGeometry*)msh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);


//	defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/sphere2.msh"), (ShaderBase*)defaultShader, (TextureBase*)SetTexture("/app0/test.gnf"));
	skybox = new RenderObject(skyboxMeshBack, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test1 = new RenderObject(skyboxMeshBack, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test2 = new RenderObject(skyboxMeshLeft, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test3 = new RenderObject(skyboxMeshRight, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test4= new RenderObject(skyboxMeshFront, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test5 = new RenderObject(skyboxMeshUp, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	test6 = new RenderObject(skyboxMeshDown, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);

}

ExampleRenderer::~ExampleRenderer()
{
	/*delete defaultObject[0];
	delete defaultObject[1];*/
	
}
Maths::Vector3 obj1Trans = Maths::Vector3(0.4, 0, -0.3);
void ExampleRenderer::Update(float dt, float x, float y) {
	/*rotation += dt;

	defaultObject[0]->SetLocalTransform(Matrix4::Scale(Vector3(0.0000001, 0.0000001, 0.0000001))*Matrix4::Translation(Vector3(-0.4, 0, 0.3)) );

	obj1Trans = obj1Trans + Vector3(0.01*x, -0.01*y, 0);
	defaultObject[1]->SetLocalTransform(Matrix4::Scale(Vector3(1, 1,1))*Matrix4::Translation(obj1Trans));*/

	//defaultObject[0]->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(Vector3(-10.4, 0, 0)) * Matrix4::Rotation(rotation, Vector3(0, 0, 1)));

	//defaultObject[1]->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))*Matrix4::Translation(Vector3(-10.4, 0, 0)));
//	skybox->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//viewProjPos
	
	//test1->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test2->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test3->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test4->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test5->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test6->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	//test1->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//test2->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//test3->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//test4->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//test5->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	//test6->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5)));
	test1->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	test2->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	test3->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	test4->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	test5->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));
	test6->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(CameraPos));

}

void ExampleRenderer::RenderActiveScene() {
//	DrawRenderObject(defaultObject[0]);
//	DrawRenderObject(defaultObject[1]);
//	DrawRenderObject(skybox);
	DrawRenderObject(test1);
	DrawRenderObject(test2);
	DrawRenderObject(test3);
	DrawRenderObject(test4);
	DrawRenderObject(test5);
	DrawRenderObject(test6);


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
