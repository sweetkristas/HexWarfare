/*
   Copyright 2014 Kristina Simpson <sweet.kristas@gmail.com>

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

#pragma once

#include <sstream>
#include "texture.hpp"

namespace graphics
{
    template<typename N>
	using surface_pair = std::pair<N, surface_ptr>;
    template<typename N>
	using surface_pair_list = std::vector<surface_pair<N>>;

    template<typename N>
	class tex_node
    {
    public:
        tex_node(const rect& r, surface_pair<N> img=surface_pair<N>(N(),surface_ptr())) : r_(r), img_(img) {
            child_[0] = child_[1] = nullptr;
        }
        ~tex_node() {
            if(child_[0]) {
                delete child_[0];
            }
            if(child_[1]) {
                delete child_[1];
            }
        }
        std::vector<rect> split_rect_vertically(int h) const {
            std::vector<rect> res;
            res.emplace_back(rect(r_.x(), r_.y(), r_.w(), h));
            res.emplace_back(rect(r_.x(), r_.y()+h, r_.w(), r_.h()-h));
            return res;
        }
        std::vector<rect> split_rect_horizontally(int w) const {
            std::vector<rect> res;
            res.emplace_back(rect(r_.x(), r_.y(), w, r_.h()));
            res.emplace_back(rect(r_.x()+w, r_.y(), r_.w()-w, r_.h()));
            return res;
        }
        void split_node(surface_pair<N> img) {
            ASSERT_LOG(is_leaf(), "Attempt to split non-leaf");
            ASSERT_LOG(can_contain(rect(0, 0, img.second->width(), img.second->height())), "Node to small to fit image.");
            if(img.second->width() == r_.w() && img.second->height() == r_.h()) {
                img_ = img;
            } else {
                if(should_split_vertically(img.second->width(), img.second->height())) {
                    auto vr = split_rect_vertically(img.second->height());
                    child_[0] = new tex_node(vr[0]);
                    child_[1] = new tex_node(vr[1]);
                } else {
                    auto hr = split_rect_horizontally(img.second->width());
                    child_[0] = new tex_node(hr[0]);
                    child_[1] = new tex_node(hr[1]);
                }
                child_[0]->split_node(img);
            }
        }
        bool grow_node(surface_pair<N> img, int max_w, int max_h) {
            ASSERT_LOG(!is_empty_leaf(), "Attempt to grow empty leaf.");
            if(r_.w() + img.second->width() > max_w || r_.h() + img.second->height() > max_h) {
                return false;
            }
            auto tn = new tex_node(r_, img_);
            tn->child_[0] = child_[0];
            tn->child_[1] = child_[1];
            img_ = std::make_pair(N(),nullptr);
            child_[0] = tn;
            if(should_grow_vertically(img.second->width(), img.second->height())) {
                child_[1] = new tex_node(rect(r_.x(), r_.y()+r_.h(), r_.w(), img.second->height()));
                r_ = rect(r_.x(), r_.y(), r_.w(), r_.h() + img.second->height());
            } else {
                child_[1] = new tex_node(rect(r_.x()+r_.w(), r_.y(), img.second->width(), r_.h()));
                r_ = rect(r_.x(), r_.y(), r_.w() + img.second->width(), r_.h());
            }
            child_[1]->split_node(img);
            return true;
        }
        bool should_split_vertically(int w, int h) const {
            if(r_.w() == w) {
                return true;
            } else if(r_.h() == h) {
                return false;
            }
            auto vr = split_rect_vertically(h);
            auto hr = split_rect_horizontally(w);
            return vr[1].perimeter() > hr[1].perimeter();
        }
        bool should_grow_vertically(int w, int h) const {
            bool can_grow_vert = r_.w() >= w;
            bool can_grow_horz = r_.h() >= h;
            ASSERT_LOG(can_grow_vert || can_grow_horz, "Unable to grow any further.");
            if(can_grow_vert && !can_grow_horz) {
                return true;
            }
            if(!can_grow_vert && can_grow_horz) {
                return false;
            }
            return r_.h() + h < r_.w() + w;
        }
        bool is_empty_leaf() const { return is_leaf() && img_.second == nullptr; }
        bool is_leaf() const { return child_[0] == nullptr && child_[1] == nullptr; }
        bool can_contain(const rect& r) const { return r.w() <= r_.w() && r.h() <= r_.h(); }
        tex_node<N>* get_left_child() { return child_[0]; }
        tex_node<N>* get_right_child() { return child_[1]; }
        const rect& get_rect() const { return r_; }
        const rect& get_bounds() const { return bounds_; }
        void blit(surface_ptr dest, std::vector<std::pair<N,rect>>* rects) {
            if(child_[0]) {
                child_[0]->blit(dest, rects);
            }
            if(img_.second) {
                dest->blit_clipped(img_.second, rect(r_.x(), r_.y(), img_.second->width(), img_.second->height()));
                rects->emplace_back(img_.first, r_);
            }
            if(child_[1]) {
                child_[1]->blit(dest, rects);
            }
        }
    private:
        rect r_;
        rect bounds_;
        tex_node<N>* child_[2];
        surface_pair<N> img_;
    };

	template<typename N>
    inline tex_node<N>* find_empty_leaf(tex_node<N>* tn, surface_ptr img) 
    {
        if(tn->is_empty_leaf()) {
            return tn->can_contain(rect(0, 0, img->width(), img->height())) ? tn : nullptr;
        }
        if(tn->is_leaf()) {
            return nullptr;
        }
        auto leaf = find_empty_leaf(tn->get_left_child(), img);
        if(leaf) { 
            return leaf;
        }
        return find_empty_leaf(tn->get_right_child(), img);
    }

    template<typename N>
	class packer
	{
	public:
		//using texture_pair = std::pair<N, graphics::texture>;
		typedef typename std::vector<std::vector<std::pair<N, graphics::texture>>>::iterator iterator;
		typedef typename std::vector<std::vector<std::pair<N, graphics::texture>>>::const_iterator const_iterator;
        packer(const surface_pair_list<N>& inp, int max_width, int max_height)
        {
            std::vector<tex_node<N>*> root;
            for(auto& img : inp) {
                if(root.empty()) {
                    root.push_back(new tex_node<N>(rect(0,0,img.second->width(), img.second->height())));
                    root.back()->split_node(img);
                }
                auto leaf =	find_empty_leaf<N>(root.back(), img.second);
                if(leaf) {
                    leaf->split_node(img);
                } else {
                    if(!root.back()->grow_node(img, max_width, max_height)) {
                        root.push_back(new tex_node<N>(rect(0,0,img.second->width(), img.second->height())));
                        root.back()->split_node(img);
                    }
                }
            }

            // process root
            int count = 0;
            for(auto& n : root) {
                std::stringstream ss;
                surface_ptr dest = std::make_shared<graphics::surface>(n->get_rect().w(), n->get_rect().h());
                std::vector<std::pair<N,rect>> rects;
                n->blit(dest, &rects);
                ss << "images/temp/nn" << count++ << ".png";
                dest->save(ss.str());
                graphics::texture texn(dest, graphics::TextureFlags::NONE);

                std::vector<std::pair<N, graphics::texture>> texs;
                for(auto& r : rects) {
                    graphics::texture t(texn);
                    t.set_area(r.second);
                    //t.set_name(r.first);
                    texs.emplace_back(std::make_pair(r.first,t));
                }
                outp_.push_back(texs);
            }

            // delete root.
            for(auto& n : root) {
                delete n;
            }
        }
		iterator begin() { return outp_.begin(); }
		iterator end() { return outp_.end(); }
		const_iterator begin() const { return outp_.begin(); }
		const_iterator end() const { return outp_.end(); }
	private:
		std::vector<std::vector<std::pair<N, graphics::texture>>> outp_;
	};
}
