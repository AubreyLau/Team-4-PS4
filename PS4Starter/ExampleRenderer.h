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

			void Update(float dt)	 override;

		protected:
			void DrawRenderObject(RenderObject* o);
			//void RenderFrame()		 override;
			void RenderActiveScene() override;

			ObjLoader* l;

			PS4Mesh* msh;
			float rotation;
			RenderObject* defaultObject[2];
		};
	}
}

