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

#include <vector>

#include "asserts.hpp"
#include "shaders.hpp"

namespace shader
{
	shader::shader(GLenum type, const std::string& name, const std::string& code)
		: type_(type), shader_(0), name_(name)
	{
		ASSERT_LOG(compile(code), "Error compiling shader for " << name_);
	}

	bool shader::compile(const std::string& code)
	{
		GLint compiled;
		if(shader_) {
			glDeleteShader(shader_);
			shader_ = 0;
		}

		ASSERT_LOG(glCreateShader != NULL, "Something bad happened with Glew shader not initialised.");
		shader_ = glCreateShader(type_);
		if(shader_ == 0) {
			std::cerr << "Enable to create shader." << std::endl;
			return false;
		}
		const char* shader_code = code.c_str();
		glShaderSource(shader_, 1, &shader_code, NULL);
		glCompileShader(shader_);
		glGetShaderiv(shader_, GL_COMPILE_STATUS, &compiled);
		if(!compiled) {
			GLint info_len = 0;
			glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetShaderInfoLog(shader_, info_log.capacity(), NULL, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				std::cerr << "Error compiling shader: " << s << std::endl;
			}
			glDeleteShader(shader_);
			shader_ = 0;
			return false;
		}
		return true;
	}

	program_object::program_object()
	{
	}

	program_object::program_object(const std::string& name, const shader& vs, const shader& fs)
	{
		init(name, vs, fs);
	}

	void program_object::init(const std::string& name, const shader& vs, const shader& fs)
	{
		name_ = name;
		vs_ = vs;
		fs_ = fs;
		ASSERT_LOG(link(), "Error linking program: " << name_);
	}

	GLuint program_object::get_attribute(const std::string& attr) const
	{
		const_actives_map_iterator it = attribs_.find(attr);
		ASSERT_LOG(it != attribs_.end(), "Attribute \"" << attr << "\" not found in list.");
		return it->second.location;
	}

	GLuint program_object::get_uniform(const std::string& attr) const
	{
		const_actives_map_iterator it = uniforms_.find(attr);
		ASSERT_LOG(it != uniforms_.end(), "Uniform \"" << attr << "\" not found in list.");
		return it->second.location;
	}

	const_actives_map_iterator program_object::get_attribute_iterator(const std::string& attr) const
	{
		const_actives_map_iterator it = attribs_.find(attr);
		ASSERT_LOG(it != attribs_.end(), "Attribute \"" << attr << "\" not found in list.");
		return it;
	}

	const_actives_map_iterator program_object::get_uniform_iterator(const std::string& attr) const
	{
		std::map<std::string, actives>::const_iterator it = uniforms_.find(attr);
		ASSERT_LOG(it != uniforms_.end(), "Uniform \"" << attr << "\" not found in list.");
		return it;
	}

	bool program_object::link()
	{
		if(object_) {
			glDeleteProgram(object_);
			object_ = 0;
		}
		object_ = glCreateProgram();
		ASSERT_LOG(object_ != 0, "Unable to create program object.");
		glAttachShader(object_, vs_.get());
		glAttachShader(object_, fs_.get());
		glLinkProgram(object_);
		GLint linked = 0;
		glGetProgramiv(object_, GL_LINK_STATUS, &linked);
		if(!linked) {
			GLint info_len = 0;
			glGetProgramiv(object_, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetProgramInfoLog(object_, info_log.capacity(), NULL, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				std::cerr << "Error linking object: " << s << std::endl;
			}
			glDeleteProgram(object_);
			object_ = 0;
			return false;
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		return queryUniforms() && queryAttributes();
	}

	bool program_object::queryUniforms()
	{
		GLint active_uniforms;
		glGetProgramiv(object_, GL_ACTIVE_UNIFORMS, &active_uniforms);
		GLint uniform_max_len;
		glGetProgramiv(object_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_max_len);
		std::vector<char> name;
		name.resize(uniform_max_len+1);
		for(int i = 0; i < active_uniforms; i++) {
			actives u;
			GLsizei size;
			glGetActiveUniform(object_, i, name.size(), &size, &u.num_elements, &u.type, &name[0]);
			u.name = std::string(&name[0], &name[size]);
			u.location = glGetUniformLocation(object_, u.name.c_str());
			ASSERT_LOG(u.location >= 0, "Unable to determine the location of the uniform: " << u.name);
			uniforms_[u.name] = u;
		}
		return true;
	}

	bool program_object::queryAttributes()
	{
		GLint active_attribs;
		glGetProgramiv(object_, GL_ACTIVE_ATTRIBUTES, &active_attribs);
		GLint attributes_max_len;
		glGetProgramiv(object_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributes_max_len);
		std::vector<char> name;
		name.resize(attributes_max_len+1);
		for(int i = 0; i < active_attribs; i++) {
			actives a;
			GLsizei size;
			glGetActiveAttrib(object_, i, name.size(), &size, &a.num_elements, &a.type, &name[0]);
			a.name = std::string(&name[0], &name[size]);
			a.location = glGetAttribLocation(object_, a.name.c_str());
			ASSERT_LOG(a.location >= 0, "Unable to determine the location of the attribute: " << a.name);
			attribs_[a.name] = a;
		}
		return true;
	}

	void program_object::make_active()
	{
		glUseProgram(object_);
	}

	void program_object::set_uniform(const_actives_map_iterator it, const GLint* value)
	{
		const actives& u = it->second;
		ASSERT_LOG(value != NULL, "set_uniform(): value is NULL");
		switch(u.type) {
		case GL_INT:
		case GL_BOOL:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:	
			glUniform1i(u.location, *value); 
			break;
		case GL_INT_VEC2:	
		case GL_BOOL_VEC2:	
			glUniform2i(u.location, value[0], value[1]); 
			break;
		case GL_INT_VEC3:	
		case GL_BOOL_VEC3:	
			glUniform3iv(u.location, u.num_elements, value); 
			break;
		case GL_INT_VEC4: 	
		case GL_BOOL_VEC4:
			glUniform4iv(u.location, u.num_elements, value); 
			break;
		default:
			ASSERT_LOG(false, "Unhandled uniform type: " << it->second.type);
		}
	}

	void program_object::set_uniform(const_actives_map_iterator it, const GLfloat* value)
	{
		const actives& u = it->second;
		ASSERT_LOG(value != NULL, "set_uniform(): value is NULL");
		switch(u.type) {
		case GL_FLOAT: {
			glUniform1f(u.location, *value);
			break;
		}
		case GL_FLOAT_VEC2: {
			glUniform2fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_VEC3: {
			glUniform3fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_VEC4: {
			glUniform4fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_MAT2:	{
			glUniformMatrix2fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		case GL_FLOAT_MAT3: {
			glUniformMatrix3fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		case GL_FLOAT_MAT4: {
			glUniformMatrix4fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		default:
			ASSERT_LOG(false, "Unhandled uniform type: " << it->second.type);
		}	
	}
}
