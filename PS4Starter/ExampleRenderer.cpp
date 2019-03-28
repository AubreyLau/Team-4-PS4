#include "ExampleRenderer.h"
#include "RenderObject.h"
#include "../Plugins/PlayStation4/PS4Shader.h"
#include "RenderObject.h"



using namespace NCL;
using namespace NCL::PS4;

ExampleRenderer::ExampleRenderer(PS4Window* window) : PS4RendererBase(window)
{
	/*Mo test- skybox*/
	skybox[0] = new RenderObject(skyboxMeshBack, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureBack);
	skybox[1] = new RenderObject(skyboxMeshLeft, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureLeft);
	skybox[2] = new RenderObject(skyboxMeshRight, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureRight);
	skybox[3] = new RenderObject(skyboxMeshFront, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureFront);
	skybox[4] = new RenderObject(skyboxMeshUp, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureUp);
	skybox[5] = new RenderObject(skyboxMeshDown, (ShaderBase*)skyboxShader, (TextureBase*)SkyboxTextureDown);
	testbunny = new RenderObject((MeshGeometry*)setMesh("/app0/polyBunny.obj"), (ShaderBase*)defaultShader, (TextureBase*)pink); //Now we replace msh by obj. try sphere.obj/ cube.obj/ building10.obj /star3.obj .
	
	building = new RenderObject((MeshGeometry*)setMesh("/app0/building10.obj"), (ShaderBase*)defaultShader, (TextureBase*)blue3, (TextureBase*)myTexture); //Now we replace msh by obj. try sphere.obj/ cube.obj/ building10.obj /star3.obj .
	//ball = new RenderObject((MeshGeometry*)defaultCube, (ShaderBase*)skyboxShader, (TextureBase*)myTexture); //Now we replace msh by obj. try sphere.obj/ cube.obj/ building10.obj /star3.obj .
	//floor = new RenderObject(floorMesh, (ShaderBase*)skyboxShader, (TextureBase*)myTexture);
	/*changePos = Vector3(0,0,0);*/
	//BallPos = Vector3(0, 0, 0);

	//test = new RenderObject((MeshGeometry*)setMesh("/app0/sphere.obj"), (ShaderBase*)skyboxShader, (TextureBase*)defaultTexture);
	/*Mo test- skybox*/
//	rotation = 0.0f;
	//defaultMesh = MeshGeometry("");
	
//	defaultObject[0] = new RenderObject((MeshGeometry*)setMesh("/app0/RobotHead.msh"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);


//	defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/cube.obj"), (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
//	defaultObject[1]->SetBasicTex("/app0/test.gnf");
//	defaultObject[1]->SetBumpTex("/app0/test.gnf");
//	defaultObject[0]->SetBumpTex("/app0/test.gnf");


//	l->loadOBJ("/app0/bunny.obj");
//	l->setAttrib(*msh);
	
//	defaultObject[0] = new RenderObject((MeshGeometry*)msh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);


//	defaultObject[1] = new RenderObject((MeshGeometry*)setMesh("/app0/sphere2.msh"), (ShaderBase*)defaultShader, (TextureBase*)SetTexture("/app0/test.gnf"));


}

ExampleRenderer::~ExampleRenderer()
{
	//delete defaultObject[0];
	//delete defaultObject[1];
	delete skybox[0];
	delete skybox[1];
	delete skybox[2];
	delete skybox[3];
	delete skybox[4];
	delete skybox[5];
	delete building;
	delete floor;
	delete testbunny;
	
}
void ExampleRenderer::Update(float dt, float x, float y) {

	/*rotation += dt;

	defaultObject[0]->SetLocalTransform(Matrix4::Scale(Vector3(0.0000001, 0.0000001, 0.0000001))*Matrix4::Translation(Vector3(-0.4, 0, 0.3)) * Matrix4::Rotation(rotation, Vector3(0,0,1)));

	obj1Trans = obj1Trans + Vector3(0.01*x, -0.01*y, 0);
	defaultObject[1]->SetLocalTransform(Matrix4::Scale(Vector3(1, 1,1))*Matrix4::Translation(obj1Trans));*/

	//defaultObject[0]->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))* Matrix4::Translation(Vector3(-10.4, 0, 0)) * Matrix4::Rotation(rotation, Vector3(0, 0, 1)));

	//defaultObject[1]->SetLocalTransform(Matrix4::Scale(Vector3(0.5, 0.5, 0.5))*Matrix4::Translation(Vector3(-10.4, 0, 0)));


	/*Mo test*/
	changePos= changePos+Vector3(0.08*x, 0, 0.08*y);
	//BallPos = BallPos + Vector3(0.1*x, 0, 0.1*y);

//	ball->SetLocalTransform(Matrix4::Scale(Vector3(5.0f,5.0f,5.0f))*Matrix4::Translation(changePos));
	building->SetLocalTransform(Matrix4::Scale(Vector3(0.2f, 0.2f, 0.2f)));
	testbunny->SetLocalTransform(Matrix4::Scale(Vector3(3.0f, 3.0f, 3.0f))*Matrix4::Translation(changePos));
	//floor->SetLocalTransform(Matrix4::Translation(Vector3(0, 10000.0f, 0)));
	//ball->SetLocalTransform(Matrix4::Translation(BallPos));
	//test->SetLocalTransform(Matrix4::Translation(changePos));
}

RenderObject* ExampleRenderer::addFloorToWorld(NCL::Vector3 position, NCL::TextureBase* tex, NCL::Vector3 size) {
	RenderObject*  floor =  new RenderObject((MeshGeometry*)defaultCube, (ShaderBase*)defaultShader, tex); //Now we replace msh by obj. try sphere.obj/ cube.obj/ building10.obj /star3.obj .
	floor->SetLocalTransform(Matrix4::Scale(size)*Matrix4::Translation(position)* Matrix4::Rotation(90, Vector3(0, 1, 0)));
	return floor;
}


void ExampleRenderer::RenderActiveScene() {
	/*DrawRenderObject(defaultObject[0]);
	DrawRenderObject(defaultObject[1]);*/
	DrawRenderObject(skybox[0]);
	DrawRenderObject(skybox[1]);
	DrawRenderObject(skybox[2]);
	DrawRenderObject(skybox[3]);
	DrawRenderObject(skybox[4]);
	DrawRenderObject(skybox[5]);
	DrawRenderObject(building);

//	DrawRenderObject(addFloorToWorld(Vector3(1, 0, 1), (TextureBase*)myTexture, Vector3(2, 2, 0.1)));
	DrawRenderObject(addFloorToWorld(Vector3(0, -0.2, 0), (TextureBase*)blue3, Vector3(20,0.1,20)));
//	DrawRenderObject(floor);
	DrawRenderObject(testbunny);
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
	int lightIndex = realShader->GetConstantBuffer("LightData"); //


	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, objIndex, 1, &constantBuffer);
	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, camIndex, 1, &cameraBuffer);
	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, lightIndex, 1, &lightBuffer);//


	realShader->SubmitShaderSwitch(*currentGFXContext);

	//DrawMesh(myMesh);
	//DrawCube(*defaultCube);
	DrawObject(o);
}
