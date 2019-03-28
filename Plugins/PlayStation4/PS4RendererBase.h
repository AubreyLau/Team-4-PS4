#pragma once
#include "../../Common/RendererBase.h"
#include "PS4MemoryAware.h"
#include"../../PS4Starter/RenderObject.h"
#include <gnm.h>
#include <gnmx\fetchshaderhelper.h>
#include <fstream>
#include <string>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\allocators.h>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\stack_allocator.h>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\toolkit.h>
#include "PS4Frame.h"
#include "PS4Texture.h"
#include"../../Common/Light.h"

using namespace sce;
using namespace Gnmx;
using namespace Gnmx::Toolkit;

namespace NCL {
	namespace Maths {
		class Matrix4;
	}
	namespace PS4 {
		enum MemoryLocation {
			GARLIC,
			ONION,
			MEMORYMAX
		};
		typedef struct {
			Vector4* _position_Radius;
			Vector4* _colours_Intensity;
			/*std::vector<NCL::Vector4>	_position_Radius;
			std::vector<NCL::Vector4>	_colours_Intensity;*/
		}_light;

		struct PS4ScreenBuffer {
			sce::Gnm::RenderTarget		colourTarget;
			sce::Gnm::DepthRenderTarget depthTarget;
		};

		class PS4Window;
		class PS4Shader;
		class PS4Mesh;

		class RenderObject;

		class PS4RendererBase :
			public RendererBase, public PS4MemoryAware
		{
		public:
			PS4RendererBase(PS4Window* window);
			~PS4RendererBase();

		protected:
			virtual void RenderActiveScene() = 0;

			void	OnWindowResize(int w, int h) override;
			void	BeginFrame()    override;
			void	RenderFrame(float x, float y)	override;
			void	EndFrame()		override;

			void	SwapScreenBuffer();
			void	SwapCommandBuffer();
			
			void	DrawMesh(PS4Mesh* mesh);
			void	DrawObject(RenderObject* object);

			void	DrawSphere(PS4Mesh& mesh);

		private:
			void	InitialiseMemoryAllocators();
			void	InitialiseVideoSystem();
			void	InitialiseGCMRendering();

			void	DestroyMemoryAllocators();
			void	DestroyVideoSystem();
			void	DestroyGCMRendering();

			void	SetRenderBuffer(PS4ScreenBuffer*buffer, bool clearColour, bool clearDepth, bool clearStencil);
			void	ClearBuffer(bool colour, bool depth, bool stencil);

			PS4ScreenBuffer* GenerateScreenBuffer(uint width, uint height, bool colour = true, bool depth = true, bool stencil = false);

		protected:
			int currentGPUBuffer;

			const int _MaxCMDBufferCount;

			//VIDEO SYSTEM VARIABLES
			int videoHandle;		//Handle to video system

		//SCREEN BUFFER VARIABLES
			const int			_bufferCount;	//How many screen buffers should we have
			int					currentScreenBuffer;
			int					prevScreenBuffer;
			PS4ScreenBuffer**	screenBuffers;	//Pointer to our screen buffers
		//Memory Allocation
			const int _GarlicMemory;
			const int _OnionMemory;

			sce::Gnmx::Toolkit::StackAllocator	stackAllocators[MEMORYMAX];

			PS4Mesh*	defaultCube;
			PS4Shader*	defaultShader;
			PS4Mesh*	defaultMesh;
			PS4Mesh*	defaultSphere;
			PS4Mesh*	myMesh;
			PS4Mesh*    setMesh(const std::string&filename);
			PS4Mesh*    setOBJ(const std::string&filename);
			PS4Texture* defaultTexture;
			PS4Texture* pink;
			PS4Texture* blue2;
			PS4Texture* blue3;
			PS4Texture* myTexture;
			PS4Texture* SetTexture(const std::string&basicTexture, const std::string bumpMap="", const std::string heightMap="");
			PS4Texture* t[3];

		//void setMesh(const std::string&filename);


			//Individual Frames
			PS4Frame*	frames;

			NCL::Maths::Matrix4*	viewProjMat;
			Gnm::Buffer	cameraBuffer;


			NCL::Maths::Vector4*	light_position_Radius;
			NCL::Maths::Vector4*	light_colours_Intensity;
			Gnm::Buffer	lightBuffer;

			int framesSubmitted;

			//Per frame pointers...
			PS4ScreenBuffer*		currentPS4Buffer;  //Pointer to whichever buffer we're currently using...
			Gnmx::GnmxGfxContext*	currentGFXContext;
			PS4Frame*				currentFrame;


			/*MO TEST*/

			PS4Mesh* skyboxMeshUp;
			PS4Mesh* skyboxMeshDown;
			PS4Mesh* skyboxMeshFront;
			PS4Mesh* skyboxMeshBack;
			PS4Mesh* skyboxMeshLeft;
			PS4Mesh* skyboxMeshRight;

			Vector3 CameraPos;
			Vector3 CameraPosChange;
			PS4Shader*	skyboxShader;

			PS4Texture* SkyboxTextureUp;
			PS4Texture* SkyboxTextureDown;
			PS4Texture* SkyboxTextureLeft;
			PS4Texture* SkyboxTextureRight;
			PS4Texture* SkyboxTextureFront;
			PS4Texture* SkyboxTextureBack;


			PS4Mesh* floorMesh;
			PS4Texture* floorTexture;

			PS4Mesh* testMesh;
		//	Light l;

			_light* _l;

			float pitch ;
			float yaw ;

			float yaw_total;
			Vector3 changePos = Vector3(0, 0, 0);
			//Vector3 BallPos;

			/*MO TEST*/
		};
	}
}

