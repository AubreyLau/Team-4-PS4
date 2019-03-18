#include "PS4RendererBase.h"
#include "PS4Window.h"
#include <video_out.h>	//Video System
#include "PS4Shader.h"
#include "PS4Mesh.h"
#include <gnmx\basegfxcontext.h>
#include <fstream>
#include <string>


#include <.\graphics\api_gnm\toolkit\allocators.h>
#include <.\graphics\api_gnm\toolkit\stack_allocator.h>

#include "../../Common/Matrix4.h"

using namespace sce::Vectormath::Scalar::Aos;

namespace SonyMath = sce::Vectormath::Scalar::Aos;

#include <iostream>

using namespace NCL;
using namespace NCL::PS4;

sce::Gnmx::Toolkit::IAllocator	oAllocator;
sce::Gnmx::Toolkit::IAllocator	gAllocator;

PS4RendererBase::PS4RendererBase(PS4Window*window)
	: RendererBase(*window),
	_MaxCMDBufferCount(3),
	  _bufferCount(3),
	  _GarlicMemory(1024 * 1024 * 512),
	  _OnionMemory( 1024 * 1024 * 256)
{
	framesSubmitted		 = 0;
	currentGPUBuffer	 = 0;
	currentScreenBuffer	 = 0;
	prevScreenBuffer	 = 0;

	std::cerr << "Starting Team4 Code!" << std::endl;

	currentGFXContext	 = nullptr;

	InitialiseMemoryAllocators();

	InitialiseGCMRendering();
	InitialiseVideoSystem();

	defaultShader = PS4Shader::GenerateShader(
		"/app0/VertexShader.sb",
		"/app0/PixelShader.sb"
	);

	defaultMesh		= PS4Mesh::GenerateTriangle();
	defaultSphere = PS4Mesh::GenerateSphere();
	myMesh = PS4Mesh::GenerateTriangle();
	defaultTexture	= PS4Texture::LoadTextureFromFile("/app0/doge.gnf");
	myTexture= PS4Texture::LoadTextureFromFile("/app0/test.gnf");

	viewProjMat		= (Matrix4*)onionAllocator->allocate(sizeof(Matrix4), Gnm::kEmbeddedDataAlignment4);
	*viewProjMat	= Matrix4();

	cameraBuffer.initAsConstantBuffer(viewProjMat, sizeof(Matrix4));
	cameraBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	EndFrame(); //always swap at least once...
}

PS4Mesh* PS4RendererBase::setMesh(const std::string&filename) {
	
	defaultSphere = PS4Mesh::GenerateSphere();
	myMesh = myMesh->GenerateMesh(filename);
	return myMesh;

}

PS4Mesh* PS4RendererBase::setOBJ(const std::string&filename) {

	myMesh = myMesh->GenerateOBJ(filename);
	return myMesh;

}



PS4Texture*  PS4RendererBase::SetTexture(const std::string&basicTexture, const std::string bumpMap, const std::string heightMap) {
	// PS4Texture* t[3];
	t[0] = PS4Texture::LoadTextureFromFile(basicTexture);
	if (bumpMap!="") {
		t[1] = PS4Texture::LoadTextureFromFile(bumpMap);
	}

	if (heightMap != "") {
		t[2] = PS4Texture::LoadTextureFromFile(heightMap);
	}

	return *t;
}

//void PS4RendererBase::setMesh(const std::string&filename) {
//	
//	defaultSphere = PS4Mesh::GenerateSphere();
//	myMesh = myMesh->GenerateMesh(filename);
//
//}

//  "$(SCE_ORBIS_SDK_DIR)\host_tools\bin\orbis-image2gnf" -i "%(FullPath)" -o "%(ProjectDir)%(Filename).gnf" -f Auto
PS4RendererBase::~PS4RendererBase()	{
	delete defaultMesh;
	delete defaultTexture;
	delete myTexture;
	delete defaultShader;
	delete defaultSphere;
	delete myMesh;

	DestroyGCMRendering();
	DestroyVideoSystem();
	DestroyMemoryAllocators();
}

void	PS4RendererBase::InitialiseVideoSystem() {
	screenBuffers = new PS4ScreenBuffer*[_bufferCount];

	for (int i = 0; i < _bufferCount; ++i) {
		screenBuffers[i] = GenerateScreenBuffer(1920, 1080);
	}

	//Now we can open up the video port
	videoHandle		= sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);

	SceVideoOutBufferAttribute attribute;
	sceVideoOutSetBufferAttribute(&attribute,
		SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
		SCE_VIDEO_OUT_TILING_MODE_TILE,
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		screenBuffers[0]->colourTarget.getWidth(),
		screenBuffers[0]->colourTarget.getHeight(),
		screenBuffers[0]->colourTarget.getPitch()
	);

	void* bufferAddresses[_bufferCount];

	for (int i = 0; i < _bufferCount; ++i) {
		bufferAddresses[i] = screenBuffers[i]->colourTarget.getBaseAddress();
	}

	sceVideoOutRegisterBuffers(videoHandle, 0, bufferAddresses, _bufferCount, &attribute);
}

void	PS4RendererBase::InitialiseGCMRendering() {
	frames = (PS4Frame*)onionAllocator->allocate(sizeof(PS4Frame) * _MaxCMDBufferCount, alignof(PS4Frame));

	for (int i = 0; i < _MaxCMDBufferCount; ++i) {
		new (&frames[i])PS4Frame();
	}

	sce::Gnmx::Toolkit::Allocators allocators = sce::Gnmx::Toolkit::Allocators(oAllocator, gAllocator, ownerHandle);
	Gnmx::Toolkit::initializeWithAllocators(&allocators);
}

void	PS4RendererBase::InitialiseMemoryAllocators() {
	stackAllocators[GARLIC].init(SCE_KERNEL_WC_GARLIC, _GarlicMemory);
	stackAllocators[ONION ].init(SCE_KERNEL_WB_ONION , _OnionMemory);

	oAllocator = Gnmx::Toolkit::GetInterface(&stackAllocators[ONION]);
	gAllocator = Gnmx::Toolkit::GetInterface(&stackAllocators[GARLIC]);

	this->garlicAllocator   = &gAllocator;
	this->onionAllocator	= &oAllocator;
	Gnm::registerOwner(&ownerHandle, "PS4RendererBase");
}

void PS4RendererBase::DestroyMemoryAllocators() {
	stackAllocators[GARLIC].deinit();
	stackAllocators[ONION ].deinit();
}

PS4ScreenBuffer*	PS4RendererBase::GenerateScreenBuffer(uint width, uint height, bool colour, bool depth, bool stencil) {
	PS4ScreenBuffer* buffer = new PS4ScreenBuffer();

	if (colour) {	
		Gnm::RenderTargetSpec spec;
		spec.init();
		spec.m_width		= width;
		spec.m_height		= height;
		spec.m_numSamples	= Gnm::kNumSamples1;
		spec.m_numFragments = Gnm::kNumFragments1;
		spec.m_colorFormat	= Gnm::kDataFormatB8G8R8A8UnormSrgb;	

		GpuAddress::computeSurfaceTileMode(Gnm::GpuMode::kGpuModeBase, &spec.m_colorTileModeHint, GpuAddress::kSurfaceTypeColorTargetDisplayable, spec.m_colorFormat, 1);

		int32_t success = buffer->colourTarget.init(&spec);

		if (success != SCE_GNM_OK) {
			bool a = true;
		}

		const Gnm::SizeAlign colourAlign = buffer->colourTarget.getColorSizeAlign();

		void *colourMemory = stackAllocators[GARLIC].allocate(colourAlign);

		Gnm::registerResource(nullptr, ownerHandle, colourMemory, colourAlign.m_size,
			"Colour", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);

		buffer->colourTarget.setAddresses(colourMemory, NULL, NULL);
	}

	if (depth) {
		Gnm::DepthRenderTargetSpec spec;
		spec.init();
		spec.m_width			= width;
		spec.m_height			= height;
		spec.m_numFragments		= Gnm::kNumFragments1;
		spec.m_zFormat			= Gnm::ZFormat::kZFormat32Float;
		spec.m_stencilFormat	= (stencil ? Gnm::kStencil8 : Gnm::kStencilInvalid);

		GpuAddress::computeSurfaceTileMode(Gnm::GpuMode::kGpuModeBase, &spec.m_tileModeHint, GpuAddress::kSurfaceTypeDepthTarget, Gnm::DataFormat::build(spec.m_zFormat), 1);

		void* stencilMemory = 0;

		int32_t success = buffer->depthTarget.init(&spec);

		if (success != SCE_GNM_OK) {
			bool a = true;
		}

		void *depthMemory = stackAllocators[GARLIC].allocate(buffer->depthTarget.getZSizeAlign());

		Gnm::registerResource(nullptr, ownerHandle, depthMemory, buffer->depthTarget.getZSizeAlign().m_size,
			"Depth", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);


		if (stencil) {
			stencilMemory = stackAllocators[GARLIC].allocate(buffer->depthTarget.getStencilSizeAlign());

			Gnm::registerResource(nullptr, ownerHandle, stencilMemory, buffer->depthTarget.getStencilSizeAlign().m_size,
				"Stencil", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);
		}

		buffer->depthTarget.setAddresses(depthMemory, stencilMemory);
	}

	return buffer;
}

void	PS4RendererBase::DestroyGCMRendering() {
	//onionAllocator->release(frames);
}

void	PS4RendererBase::DestroyVideoSystem() {
	for (int i = 0; i < _bufferCount; ++i) {
		delete screenBuffers[i];
	}
	delete[] screenBuffers;

	sceVideoOutClose(videoHandle);
}


//
Maths::Vector3 viewProjPos = Maths::Vector3(0, 0, 3);
Maths::Matrix4 viewMat = Maths::Matrix4();
Maths::Matrix4 projMat = Maths::Matrix4();


void PS4RendererBase::RenderFrame()			{
	currentFrame->StartFrame();	

	currentGFXContext->waitUntilSafeForRendering(videoHandle, currentGPUBuffer);

	SetRenderBuffer(currentPS4Buffer, true, true, true);

	defaultShader->SubmitShaderSwitch(*currentGFXContext);

	//Primitive Setup State
	Gnm::PrimitiveSetup primitiveSetup;
	primitiveSetup.init();
	primitiveSetup.setCullFace(Gnm::kPrimitiveSetupCullFaceNone);
	primitiveSetup.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCcw);
	//primitiveSetup.setPolygonMode()
	currentGFXContext->setPrimitiveSetup(primitiveSetup);

	////Screen Access State
	Gnm::DepthStencilControl dsc;
	dsc.init();
	dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLessEqual);
	dsc.setDepthEnable(true);
	currentGFXContext->setDepthStencilControl(dsc);

	Gnm::Sampler trilinearSampler;
	trilinearSampler.init();
	trilinearSampler.setMipFilterMode(Gnm::kMipFilterModeLinear);

	currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 1, &defaultTexture->GetAPITexture());


	currentGFXContext->setSamplers(Gnm::kShaderStagePs, 0, 1, &trilinearSampler);

	
	viewMat = Matrix4::BuildViewMatrix(viewProjPos, Vector3(0, 0, 0), Vector3(0, 1, 0));

	projMat = Matrix4::Perspective(1.0f, 100.0f, (float)16 / (float)9, 70.0f);

	viewProjMat->ToIdentity();
	//viewProjPos = viewProjPos + Vector3(0.01*3, -0.01*3, 0);
	//viewProjMat->SetPositionVector(viewProjPos);
	//viewProjMat->Rotation(x*90, Vector3(0, 0, 0));

	*viewProjMat = projMat * viewMat;



	RenderActiveScene();

	currentFrame->EndFrame();

	framesSubmitted++;
}

void	PS4RendererBase::OnWindowResize(int w, int h)  {

}

void	PS4RendererBase::BeginFrame()   {

}

void PS4RendererBase::EndFrame()			{
	SwapScreenBuffer();
	SwapCommandBuffer();
}

void	PS4RendererBase::SwapScreenBuffer() {
	prevScreenBuffer	= currentScreenBuffer;
	currentScreenBuffer = (currentScreenBuffer + 1) % _bufferCount;
	sceVideoOutSubmitFlip(videoHandle, prevScreenBuffer, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);

	currentPS4Buffer = screenBuffers[currentScreenBuffer];
}

void	PS4RendererBase::SwapCommandBuffer() {
	if (currentGFXContext) {	
		if (currentGFXContext->submit() != sce::Gnm::kSubmissionSuccess) {
			std::cerr << "Graphics queue submission failed?" << std::endl;
		}
		Gnm::submitDone();
	}

	currentGPUBuffer	= (currentGPUBuffer + 1) % _MaxCMDBufferCount;
	 
	currentFrame		= &frames[currentGPUBuffer]; 
	currentGFXContext	= &currentFrame->GetCommandBuffer();
}
 
void	PS4RendererBase::SetRenderBuffer(PS4ScreenBuffer*buffer, bool clearColour, bool clearDepth, bool clearStencil) {
	currentPS4Buffer = buffer;
	currentGFXContext->setRenderTargetMask(0xF);
	currentGFXContext->setRenderTarget(0, &currentPS4Buffer->colourTarget);
	currentGFXContext->setDepthRenderTarget(&currentPS4Buffer->depthTarget);

	currentGFXContext->setupScreenViewport(0, 0, currentPS4Buffer->colourTarget.getWidth(), currentPS4Buffer->colourTarget.getHeight(), 0.5f, 0.5f);
	currentGFXContext->setScreenScissor(0, 0, currentPS4Buffer->colourTarget.getWidth(), currentPS4Buffer->colourTarget.getHeight());
	currentGFXContext->setWindowScissor(0, 0, currentPS4Buffer->colourTarget.getWidth(), currentPS4Buffer->colourTarget.getHeight(), sce::Gnm::WindowOffsetMode::kWindowOffsetDisable);
	currentGFXContext->setGenericScissor(0, 0, currentPS4Buffer->colourTarget.getWidth(), currentPS4Buffer->colourTarget.getHeight(), sce::Gnm::WindowOffsetMode::kWindowOffsetDisable);

	ClearBuffer(clearColour, clearDepth, clearStencil);
}

void	PS4RendererBase::ClearBuffer(bool colour, bool depth, bool stencil) {
	if (colour) {
	//	SonyMath::Vector4 defaultClearColour(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1.0f);
		SonyMath::Vector4 defaultClearColour(0.36f, 0.52f, 0.64f, 1.0f);


		SurfaceUtil::clearRenderTarget(*currentGFXContext, &currentPS4Buffer->colourTarget, defaultClearColour);
	}

	if (depth) {
		float defaultDepth = 1.0f;
		SurfaceUtil::clearDepthTarget(*currentGFXContext, &currentPS4Buffer->depthTarget, defaultDepth);
	}

	if (stencil && currentPS4Buffer->depthTarget.getStencilReadAddress()) {
		int defaultStencil = 0;
		SurfaceUtil::clearStencilTarget(*currentGFXContext, &currentPS4Buffer->depthTarget, defaultStencil);
	}
}


void PS4RendererBase::DrawMesh(PS4Mesh *mesh) {
	mesh->SubmitDraw(*currentGFXContext, Gnm::ShaderStage::kShaderStageVs);
}

void PS4RendererBase::DrawObject(RenderObject* o) {
	
	//PS4Texture *t = (PS4Texture*)o->getBasicTex();
	////currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 1, &t[0]->GetAPITexture());
	////currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 1, &t->GetAPITexture());
	

	currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 2, &((PS4Texture*)o->getBasicTex())->GetAPITexture());
	//currentGFXContext->setTextures(Gnm::kShaderStagePs, 1, 2, &((PS4Texture*)o->getBasicTex())->GetAPITexture());

	if ((PS4Texture*)o->getBumpTex()!=nullptr) {
	
	currentGFXContext->setTextures(Gnm::kShaderStagePs, 1, 2, &((PS4Texture*)o->getBumpTex())->GetAPITexture());
	}

//	currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 1, &((PS4Texture*)o->getBasicTex())->GetAPITexture());

	
	((PS4Mesh*)o->GetMesh())->SubmitDraw(*currentGFXContext, Gnm::ShaderStage::kShaderStageVs);

}

void PS4RendererBase::DrawSphere(PS4Mesh& mesh) {
	defaultSphere->SubmitDraw(*currentGFXContext, Gnm::ShaderStage::kShaderStageVs);

	//	std::cout << "cube drawn!!!!" << std::endl;
	
}