#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), gamearea},
	playfield	= {gamearea * Makai::Vector2(1, -1), gamearea}
;

using BaseBulletServer = Danmaku::BulletServer<>;

struct TestBulletServer: BaseBulletServer {
	MkGraph::Renderable mesh, glowMesh;
	Danmaku::BulletServerConfig const cfg = {1024, mesh, glowMesh, board, playfield};

	TestBulletServer(): BaseBulletServer(cfg) {}
};

int main() {
	TestBulletServer server;

	return 0;
}