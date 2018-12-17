#pragma once
#include <Renderer/SpriteBatch/sge_sprite_batch.hpp>
#include <array>

class QuadBatch:
	public SGE::RealSpriteBatch
{
public:
	using Quad = std::array<glm::vec2, 4u>;
protected:
	struct SpriteQuad
	{
		SGE::Sprite s;
		Quad q;

		SpriteQuad(const SGE::Sprite& s, const Quad& q): s(s), q(q){}
		SpriteQuad(const SpriteQuad&) = default;
		~SpriteQuad() = default;
		SpriteQuad& operator=(const SpriteQuad&) = default;
	};
	std::vector<SpriteQuad> spriteQuads;
	std::vector<Quad> quads;
public:
	QuadBatch(GLuint program, size_t batchSize, bool uvBatch = false, bool staticBatch = false);

	GLuint initializeVAO(GLuint VAO = 0) override;

	void prepareBatch() override;

	void renderBatch() const override;

	void addObject(SGE::Object* o) override;

	virtual void addObject(SGE::Object* o, const Quad& q);

	void removeObject(SGE::Object* o) override;
};

