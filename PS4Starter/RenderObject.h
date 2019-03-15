#pragma once
#include "../Common/Matrix4.h"
#include "../Common/MeshGeometry.h"
#include"../Plugins/PlayStation4/PS4Texture.h"
class ShaderBase;
class TextureBase;

namespace NCL {
	namespace Rendering {
		class ShaderBase;
		class TextureBase;
	}
	namespace PS4 {
		class RenderObject	{
		public:
			RenderObject();
			~RenderObject();

			RenderObject(MeshGeometry* m, Rendering::ShaderBase* shader, Rendering::TextureBase* tex, Rendering::TextureBase* bumpTex=nullptr, Rendering::TextureBase* heightTex=nullptr);

			const Maths::Matrix4& GetLocalTransform() const;
			void	SetLocalTransform(const Maths::Matrix4& mat);

			Rendering::ShaderBase* GetShader() const {
				return shader;
			}
			MeshGeometry* GetMesh() const {
				return mesh;
			}

			//Texture setters
			
			void SetBasicTex(const std::string&Tex) {
				textures[0] = PS4Texture::LoadTextureFromFile(Tex);
			}
			void SetBumpTex(const std::string&Tex) {
				textures[1] = PS4Texture::LoadTextureFromFile(Tex);
			}
			void SetHeightMap(const std::string&Tex) {
				textures[2] = PS4Texture::LoadTextureFromFile(Tex);
			}
			
			//Texture getters
			Rendering::TextureBase* getBasicTex() {
				return textures[0];
			}
			Rendering::TextureBase* getBumpTex() {
				if (textures[1]) {
					return textures[1];
				}
				else return nullptr;
			
			}
			Rendering::TextureBase* getHeightMap() {
				if (textures[2]) {
					return textures[2];
				}
				else return nullptr;
			}

		protected:
			MeshGeometry*	mesh;
			Rendering::ShaderBase*		shader;

			static const int TEXTURE_COUNT = 4;
			Rendering::TextureBase* textures[TEXTURE_COUNT];

			Maths::Matrix4		localTransform;
		};
	}
}
