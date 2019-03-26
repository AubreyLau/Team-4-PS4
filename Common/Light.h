#pragma once

#include <vector>

#include "Assets.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <fstream>
#include <string>
using std::vector;

namespace NCL {
	namespace Maths {
		class Vector2;
		class Vector3;
		class Vector4;
		class Matrix4;
	}
	using namespace Maths;
	class Light
	{

		Light() {};
		Light(  Vector3 position, Vector4 colour, float radius) {
			this->position = position;
			this->colour = colour;
			this->radius = radius;

		}
		~Light(void) {};
		Vector3 GetPosition() const { return position; }
		void SetPosition(Vector3 val) { position = val; }
		float GetRadius() const { return radius; }
		void SetRadius(float val) { radius = val; }
		Vector4 GetColour() const { return colour; }
		void SetColour(Vector4 val) { colour = val; }
	protected:
		Vector3 position;
		Vector4 colour;
		float radius;






	};
}