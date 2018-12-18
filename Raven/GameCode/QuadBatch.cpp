#include "QuadBatch.hpp"

QuadBatch::QuadBatch(GLuint program, size_t batchSize, bool uvBatch, bool staticBatch): RealSpriteBatch(program, batchSize, uvBatch, staticBatch)
{
	(decltype(this->spriteData)(0u)).swap(this->spriteData);
	quads.reserve(batchSize);
	spriteQuads.reserve(batchSize);
	this->batchMaxSize = this->batchMaxCount * sizeof(SpriteQuad);
}

GLuint QuadBatch::initializeVAO(GLuint VAO)
{
	if (VAO != 0)
	{
		this->VAO = VAO;
	}
	else
	{
		constexpr GLuint posAttrib = 0;
		constexpr GLuint scaleAttrib = 1;
		constexpr GLuint rotAttrib = 2;
		constexpr GLuint layerAttrib = 3;
		constexpr GLuint quadVertices = 4;
		glGenVertexArrays(1, &this->VAO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->SBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO);
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, false, sizeof(SpriteQuad), reinterpret_cast<GLvoid*>(offsetof(SpriteQuad, s) + offsetof(SGE::Sprite, position)));
		glVertexAttribDivisor(posAttrib, 1u);
		glEnableVertexAttribArray(scaleAttrib);
		glVertexAttribPointer(scaleAttrib, 2, GL_FLOAT, false, sizeof(SpriteQuad), reinterpret_cast<GLvoid*>(offsetof(SpriteQuad, s) + offsetof(SGE::Sprite, scale)));
		glVertexAttribDivisor(scaleAttrib, 1u);
		glEnableVertexAttribArray(rotAttrib);
		glVertexAttribPointer(rotAttrib, 1, GL_FLOAT, false, sizeof(SpriteQuad), reinterpret_cast<GLvoid*>(offsetof(SpriteQuad, s) + offsetof(SGE::Sprite, rotation)));
		glVertexAttribDivisor(rotAttrib, 1u);
		glEnableVertexAttribArray(layerAttrib);
		glVertexAttribPointer(layerAttrib, 1, GL_FLOAT, false, sizeof(SpriteQuad), reinterpret_cast<GLvoid*>(offsetof(SpriteQuad, s) + offsetof(SGE::Sprite, layer)));
		glVertexAttribDivisor(layerAttrib, 1u);
		auto var1 = (offsetof(SpriteQuad, s) + offsetof(SGE::Sprite, scale));
		for (GLuint i = 0u; i < 4u; ++i)
		{
			glEnableVertexAttribArray(quadVertices + i);
			auto var = (offsetof(SpriteQuad, q) + i * sizeof(glm::vec2));
			glVertexAttribPointer(quadVertices + i, 2, GL_FLOAT, false, sizeof(SpriteQuad), reinterpret_cast<GLvoid*>(offsetof(SpriteQuad, q) + i * sizeof(glm::vec2)));
			glVertexAttribDivisor(quadVertices + i, 1u);
		}

		this->samplerLocation = glGetUniformLocation(this->program, "textureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture.id);
		glBindSampler(0, this->sampler);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindSampler(0, 0);
	}
	return this->VAO;
}

void QuadBatch::prepareBatch()
{
	if (staticBatch && batched)
		return;
	if (!this->initialized)
	{
		this->initializeBatch();
	}
	this->spriteQuads.clear();
	this->uvData.clear();

	const size_t count = this->batchedObjects.size();
	SGE::Object* o = nullptr;

	for (size_t i = 0u; i < count; ++i)
	{
		o = batchedObjects[i];
		if (!o->getDrawable() || !o->getVisible())
			continue;
		this->spriteQuads.emplace_back(SGE::Sprite(o->getPositionGLM(), { 1.f, 1.f }, o->getOrientation(), o->getLayer()), this->quads[i]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->SBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->spriteQuads.size() * sizeof(SpriteQuad), this->spriteQuads.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->batched = true;
}

void QuadBatch::renderBatch() const
{
	//Assume program, texture are bound
	using namespace SGE::Const;
	glBindVertexArray(this->VAO);
	glUniform1i(this->samplerLocation, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, MUBB, this->MUBO);
	glBindSampler(0, this->sampler);
	//glDrawElements(GL_TRIANGLES, this->spriteData.size() * 6u, GL_UNSIGNED_SHORT, nullptr);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, GLsizei(this->spriteQuads.size()));

	glBindVertexArray(0);
	glBindBufferBase(GL_UNIFORM_BUFFER, MUBB, 0);
	glBindSampler(0, 0);
}

void QuadBatch::addObject(SGE::Object* o)
{
	throw std::runtime_error("QuadBatch requires extended addObject()!");
}

void QuadBatch::addObject(SGE::Object* o, const Quad& q)
{
	this->batchedObjects.push_back(o);
	this->quads.push_back(q);
}

void QuadBatch::removeObject(SGE::Object* o)
{
	//Assumes same order
	auto it = std::find(this->batchedObjects.begin(), this->batchedObjects.end(), o);
	auto index = std::distance(this->batchedObjects.begin(), it);
	this->quads.erase(this->quads.begin() + index);
	this->batchedObjects.erase(it);
}
