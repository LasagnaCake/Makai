#include "../glapiloader.cc"

#include "uniform.hpp"

using namespace Makai; using namespace Makai::Graph;

Uniform::Uniform(String const& _name, uint32 const _id):
	name(_name),
	id(_id),
	location(glGetUniformLocation(_id, _name.cstr()))
{}

void Uniform::set(bool const value, usize const offset) const {
	this->offset = 0;
	glUniform1i(getUniform() + offset, (int)value);
	++this->offset;
}

void Uniform::set(int const value, usize const offset) const {
	this->offset = 0;
	glUniform1i(getUniform() + offset, value);
	++this->offset;
}

void Uniform::set(uint32 const value, usize const offset) const {
	this->offset = 0;
	glUniform1ui(getUniform() + offset, value);
	++this->offset;
}

void Uniform::set(float const value, usize const offset) const {
	this->offset = 0;
	glUniform1f(getUniform() + offset, value);
}

void Uniform::set(double const value, usize const offset) const {
	this->offset = 0;
	glUniform1d(getUniform() + offset, value);
}

void Uniform::set(Vector2 const& value, usize const offset) const {
	this->offset = 0;
	glUniform2f(getUniform() + offset, value.x, value.y);
	++this->offset;
}

void Uniform::set(Vector3 const& value, usize const offset) const {
	this->offset = 0;
	glUniform3f(getUniform() + offset, value.x, value.y, value.z);
	++this->offset;
}

void Uniform::set(Vector4 const& value, usize const offset) const {
	this->offset = 0;
	glUniform4f(getUniform() + offset, value.x, value.y, value.z, value.w);
	++this->offset;
}

void Uniform::set(Matrix3x3 const& value, usize const offset) const {
	this->offset = 0;
	glUniformMatrix3fv(getUniform() + offset, 1, GL_FALSE, value.begin().raw());
	++this->offset;
}

void Uniform::set(Matrix4x4 const& value, usize const offset) const {
	this->offset = 0;
	glUniformMatrix4fv(getUniform() + offset, 1, GL_FALSE, value.begin().raw());
}

void Uniform::setArray(int const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform1iv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(uint const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform1uiv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(float const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform1fv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(double const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform1dv(getUniform() + offset, count, values);
	this->offset = count;
}

void Uniform::setArray(Vector2 const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform2fv(getUniform() + offset, count, (float const*)values);
	this->offset = count;
}

void Uniform::setArray(Vector3 const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform3fv(getUniform() + offset, count, (float const*)values);
	this->offset = count;
}

void Uniform::setArray(Vector4 const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniform4fv(getUniform() + offset, count, (float const*)values);
	this->offset = count;
}

void Uniform::setArray(Matrix4x4 const* const values, usize const count, usize const offset) const {
	this->offset = 0;
	glUniformMatrix4fv(getUniform() + offset, count, GL_FALSE, (float const*)values);
	this->offset = count;
}

uint32 Uniform::getUniformArray(usize const offset) const {
	return location + offset + this->offset;
}

uint32 Uniform::getUniform() const {
	return location + offset;
}

uint32 Uniform::getUniform(String const& append) const {
	return glGetUniformLocation(id, (name + append).cstr());
}

Uniform Uniform::operator[](String const& member) const {
	return Uniform(name + "." + member, id);
}
