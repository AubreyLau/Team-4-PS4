#pragma once

#include <vector>

#include "Assets.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <fstream>
#include <string>
using std::vector;


//	using namespace Maths;
	class Light
	{

		//Light() {};
		Light( NCL::Vector3 position, NCL::Vector4  colour, float radius) {
			this->position = position;
			this->colour = colour;
			this->radius = radius;

		}
		~Light(void) {};
		NCL::Vector3  GetPosition() const { return position; }
		void SetPosition(NCL::Vector3  val) { position = val; }
		float GetRadius() const { return radius; }
		void SetRadius(float val) { radius = val; }
		NCL::Vector4  GetColour() const { return colour; }
		void SetColour(NCL::Vector4  val) { colour = val; }
	protected:
		NCL::Vector3  position;
		NCL::Vector4  colour;
		float radius;






	};
