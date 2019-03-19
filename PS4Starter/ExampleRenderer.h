#pragma once
#include "../Plugins/PlayStation4/PS4RendererBase.h"
#include"../Common/ObjLoader.h"
#include"../Plugins/PlayStation4/PS4Mesh.h"

namespace NCL {
	namespace PS4 {
		class ExampleRenderer :
			public PS4RendererBase
		{
		public:
			ExampleRenderer(PS4Window* window);
			~ExampleRenderer();

			//void Update(float dt)	 override;
			void Update(float dt, float x, float y);

		protected:
			void DrawRenderObject(RenderObject* o);
			//void RenderFrame()		 override;
			void RenderActiveScene() override;

			ObjLoader* l;

			PS4Mesh* msh;
			float rotation;
			RenderObject* defaultObject[2];



			/*Mo test*/
			RenderObject* skybox;
			RenderObject* test1;
			RenderObject* test2;
			RenderObject* test3;
			RenderObject* test4;
			RenderObject* test5;
			RenderObject* test6;

		};
	}
}

