#include "bingus.h"
#include <unordered_map>

static VertBuffer spriteBuffer;
static Shader spriteShader;
static SpriteSheet spriteSheet;

//static std::unordered_map<u32, 

Scene::Scene()
{
	spriteBuffer = VertBuffer({ VERTEX_POS, VERTEX_UV });

	//Load shader and sprite atlas
	//World and HUD sprites use same atlas for now
	spriteShader = Shader("world.vert", "sprite.frag");
	spriteShader.EnableUniforms(SHADER_MAIN_TEX | SHADER_COLOR);
	spriteShader.SetUniformVec4(SHADER_COLOR, vec4(1, 1, 1, 1));

	spriteSheet = SpriteSheet("ui.png");
	spriteSheet.sequences["debug"] = SpriteSequence(vec2(0), vec2(128, 128), 4, 0.f);
}

void Scene::Draw()
{
	SpriteBatch batch;
	batch.buffer = spriteBuffer;
	batch.shader = spriteShader;
	batch.sheet = &spriteSheet;
	batch.texture = spriteSheet.texture;

	for (Entity e : entities)
	{
		e.Draw(&batch);
	}

	batch.Draw();
}

void TestEntity::Tick()
{
	
}

void TestEntity::Draw(SpriteBatch* batch)
{
	//Sprite sprite Sprite()
	//batch->PushSprite();
}
