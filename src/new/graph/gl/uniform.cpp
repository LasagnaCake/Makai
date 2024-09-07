#include <GL/gl3w.h>
#include <GL/gl.h>

#include "uniform.hpp"

using namespace Makai::Graph;

Uniform::Uniform(String const& _name, uint const& _id):
	name(_name),
	id(_id),
	location(glGetUniformLocation(_id, _name.c_str()))
{}

void Uniform::set(bool const& value, usize const& offset) const {
	this->offset = 0;
	glUniform1i(getUniform() + offset, (int)value);
	++this->offset;
}

void Uniform::set(int const& value, usize const& offset) const {
	this->offset = 0;
	glUniform1i(getUniform() + offset, value);
	++this->offset;
}

void Uniform::set(uint const& value, usize const& offset) const {
	this->offset = 0;
	glUniform1ui(getUniform() + offset, value);
	++this->offset;
}

void Uniform::set(float const& value, usize const& offset) const {
	this->offset = 0;
	glUniform1f(getUniform() + offset, value);
}

void Uniform::set(Vector2 const& value, usize const& offset) const {
	this->offset = 0;
	glUniform2f(getUniform() + offset, value.x, value.y);
	++this->offset;
}

void Uniform::set(Vector3 const& value, usize const& offset) const {
	this->offset = 0;
	glUniform3f(getUniform() + offset, value.x, value.y, value.z);
	++this->offset;
}

void Uniform::set(Vector4 const& value, usize const& offset) const {
	this->offset = 0;
	glUniform4f(getUniform() + offset, value.x, value.y, value.z, value.w);
	++this->offset;
}

void Uniform::set(Matrix3x3 const& value, usize const& offset) const {
	this->offset = 0;
	glUniformMatrix3fv(getUniform() + offset, 1, GL_FALSE, value.begin());
	++this->offset;
}

void Uniform::set(Matrix4x4 const& value, usize const& offset) const {
	this->offset = 0;
	glUniformMatrix4fv(getUniform() + offset, 1, GL_FALSE, value.begin());
}

void Uniform::setArray(int* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	glUniform1iv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(uint* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	glUniform1uiv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(float* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	glUniform1fv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(Vector2* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	for SRANGE(i, 0, count)
		glUniform2f(getUniformArray(i) + offset, values[i].x, values[i].y);
	this->offset = count;
}

void Uniform::setArray(Vector3* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	for SRANGE(i, 0, count)
		glUniform3f(getUniformArray(i)+ offset, values[i].x, values[i].y, values[i].z);
	this->offset = count;
}

void Uniform::setArray(Vector4* const& values, usize const& count, usize const& offset) const {
	this->offset = 0;
	for SRANGE(i, 0, count)
		glUniform4f(getUniformArray(i)+ offset, values[i].x, values[i].y, values[i].z, values[i].w);
	this->offset = count;
}

uint Uniform::getUniformArray(usize const& offset) const {
	return location + offset + this->offset;
}

uint Uniform::getUniform() const {
	return location + offset;
}

uint Uniform::getUniform(String const& append) const {
	return glGetUniformLocation(id, (name + append).c_str());
}
