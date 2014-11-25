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

#include "asserts.hpp"
#include "draw_primitives.hpp"

namespace graphics
{
	DrawPrimitive::DrawPrimitive(const node& n)
	{
	}

	DrawPrimitivePtr DrawPrimitive::factory(const node& n)
	{
		auto& type = n["type"].as_string();
		if(type == "arrow") {
			return std::make_shared<ArrowPrimitive>(n);
		}
		ASSERT_LOG(false, "Unrecognised 'type' attribute: '" << type << "'");
		return DrawPrimitivePtr();
	}

	void DrawPrimitive::draw(const engine& eng, const point& cam) const
	{
		handleDraw(eng, cam);
	}

	ArrowPrimitive::ArrowPrimitive(const node& n)
		: DrawPrimitive(n),
		  granularity_(static_cast<float>(n["granularity"].as_float(0.005f))),
		  width_base_(static_cast<float>(n["width_base"].as_float(12.0))),
		  width_head_(static_cast<float>(n["width_head"].as_float(5))),
		  arrow_head_length_(n["arrow_head_length"].as_float(10)),
		  arrow_head_width_(static_cast<float>(n["arrow_head_width"].as_float(2.0f))),
		  fade_in_length_(n["fade_in_length"].as_int32(50)),
		  color_(255,255,255,255)
	{
		if(n.has_key("color")) {			
			int crgba[4] = {255,255,255,255};
			int index = 0;
			for(auto& c : n["color"].as_list()) {
				crgba[index++] = c.as_int32();
			}
			color_ = color(crgba[0],crgba[1],crgba[2],crgba[3]);
		}

		setPoints(n["points"]);
		init();
	}

	void ArrowPrimitive::handleDraw(const engine& eng, const point& cam) const
	{
		if(points_.size() < 3) {
			return;
		}

		SDL_RenderFillRects(const_cast<SDL_Renderer*>(eng.get_renderer()), &rects_[0], rects_.size());
	}
	
	void ArrowPrimitive::init()
	{
		if(points_.size() < 3) {
			return;
		}
		std::vector<pointf> path;

		for(unsigned n = 1; n < points_.size()-1; ++n) {
			std::vector<pointf> new_path;
			curve(points_[n-1], points_[n], points_[n+1], &new_path);

			if(path.empty()) {
				path.swap(new_path);
			} else {
				ASSERT_LOG(path.size() >= new_path.size(), "path.size() < new_path.size() : " << path.size() << " < " << new_path.size());
				const int overlap = path.size()/2;
				for(int n = 0; n != overlap; ++n) {
					const float ratio = static_cast<float>(n)/static_cast<float>(overlap);
					pointf& value = path[(path.size() - overlap) + n];
					pointf new_value = new_path[n];
					value[0] = value[0]*(1.0f-ratio) + new_value[0]*ratio;
					value[1] = value[1]*(1.0f-ratio) + new_value[1]*ratio;
				}

				path.insert(path.end(), new_path.begin() + overlap, new_path.end());
			}
		}

		const float PathLength = static_cast<float>(path.size()-1);

		std::vector<std::pair<pointf,pointf>> lr_path;
		for(unsigned n = 0; n < path.size()-1; ++n) {
			const pointf& p = path[n];
			const pointf& next = path[n+1];

			pointf direction(next.x - p.x, next.y - p.y);
			pointf unit_direction = geometry::normalize(direction);
		
			pointf normal_direction_left, normal_direction_right;
			normal_direction_left.x = -unit_direction.y;
			normal_direction_left.y = unit_direction.x;
			normal_direction_right.x = unit_direction.y;
			normal_direction_right.y = -unit_direction.x;

			const float ratio = n/PathLength;

			float arrow_width = width_base_ - (width_base_-width_head_)*ratio;

			const int time_until_end = path.size()-2 - n;
			if(time_until_end < arrow_head_length_) {
				arrow_width = arrow_head_width_*time_until_end;
			}

			lr_path.emplace_back(std::make_pair(pointf(p.x + normal_direction_left.x*arrow_width, p.y + normal_direction_left.y*arrow_width),
				pointf(p.x + normal_direction_right.x*arrow_width, p.y + normal_direction_right.y*arrow_width));
		}

		for(auto& p : lr_path) {
			rects_.emplace_back(p.first.x, p.second.x, p.first.y, p.second.y);
		}
	}

	void ArrowPrimitive::setPoints(const node& points)
	{
		ASSERT_LOG(points.is_list(), "arrow points is not a list: " << points.type_as_string());

		points_.clear();

		for(auto& p : points.as_list()) {
			ASSERT_LOG(p.is_list() && p.num_elements() == 2, "arrow points in invalid format: " << p.type_as_string() << " : " << p);
			points_.emplace_back(p[0].as_int32(), p[1].as_int32());
		}
	}

	void ArrowPrimitive::curve(const point& p0, const point& p1, const point& p2, std::vector<pointf>* out) const
	{
		for(float t = 0.0f; t < 1.0f - granularity_; t += granularity_) {
			//formula for a bezier curve.
			out->emplace_back((1.0f-t)*(1.0f-t)*p0.x + 2.0f*(1.0f-t)*t*p1.x + t*t*p2.x, 
				(1.0f-t)*(1.0f-t)*p0.y + 2.0f*(1.0f-t)*t*p1.y + t*t*p2.y);
		}
	}
}
