/*
	Copyright 2013 David White <davewx7@gmail.com>

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include <memory>

#include "color.hpp"
#include "engine.hpp"
#include "geometry.hpp"
#include "node.hpp"

namespace graphics
{
	class DrawPrimitive;
	typedef std::shared_ptr<DrawPrimitive> DrawPrimitivePtr;

	class DrawPrimitive
	{
	public:
		explicit DrawPrimitive(const node& n);
		static DrawPrimitivePtr factory(const node& n);
		void draw(const engine& eng, const point& cam) const;
	private:
		virtual void handleDraw(const engine& eng, const point& cam) const = 0;
		DrawPrimitive();
		DrawPrimitive(const DrawPrimitive&);
		void operator=(const DrawPrimitive&);
	};

	class ArrowPrimitive : public DrawPrimitive
	{
	public:
		explicit ArrowPrimitive(const node& n);
	private:
		void handleDraw(const engine& eng, const point& cam) const override;

		void init();
		void setPoints(const node& points);
		void curve(const point& p1, const point& p2, const point& p3, std::vector<pointf>* out) const;

		std::vector<point> points_;
		float granularity_;
		float width_base_;
		float width_head_;
		float arrow_head_length_;
		float arrow_head_width_;
		int fade_in_length_;

		color color_;

		std::vector<SDL_Rect> rects_;
	};
}
