#include <makai/makai.hpp>
#include <makai-ex/makai-ex.hpp>

// Basic stuff

namespace Danmaku = Makai::Ex::Game::Danmaku;

constexpr Makai::Vector2 gamearea = Makai::Vector2(64 * (4.0/3.0), 64) / 2;

Danmaku::GameArea
	board		= {gamearea * Makai::Vector2(1, -1), (gamearea)},
	playfield	= {gamearea * Makai::Vector2(1, -1), (gamearea * 1.5)}
;

struct DoubleMeshHolder {
	MkGraph::Renderable m, gm;

	DoubleMeshHolder(usize const layer) {
		m.setRenderLayer(layer);
		gm.setRenderLayer(layer+1);
		gm.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
	}
};

struct GlowMeshHolder {
	MkGraph::Renderable gm;

	GlowMeshHolder(usize const layer) {
		gm.setRenderLayer(layer);
		gm.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
	}
};

// Item stuff

constexpr Danmaku::ItemServerInstanceConfig ITEM_CFG = {};

using BaseItemServer = Danmaku::ItemServer<>;

struct TestItemServer: DoubleMeshHolder, BaseItemServer {
	TestItemServer(usize const layer):
		DoubleMeshHolder(layer),
		BaseItemServer({256, m, gm, ::board, ::playfield, ITEM_CFG}) {}
};

// Bullet stuff

using BaseBulletServer = Danmaku::BulletServer<>;

struct TestBulletServer: DoubleMeshHolder, BaseBulletServer {
	TestBulletServer(usize const layer, Danmaku::BulletServerInstanceConfig const& cfg):
		DoubleMeshHolder(layer),
		BaseBulletServer({cfg.capacity, m, gm, ::board, ::playfield, cfg}) {}
};

constexpr Danmaku::BulletServerInstanceConfig ENEMY_BULLET_SERVER_CFG = {
	2048,
	Danmaku::BulletCollisionConfig{}
};

constexpr Danmaku::BulletServerInstanceConfig PLAYER_BULLET_SERVER_CFG = {
	256,
	Danmaku::BulletCollisionConfig{
		{Danmaku::Collision::Layer::PLAYER_BULLET, Danmaku::Collision::Tag::FOR_PLAYER_1},
		{Danmaku::Collision::Mask::PLAYER_BULLET, 0},
		{}
	}
};

// Laser stuff

using BaseLaserServer = Danmaku::LaserServer<>;

struct TestLaserServer: GlowMeshHolder, BaseLaserServer {
	TestLaserServer(usize const layer, Danmaku::LaserServerInstanceConfig const& cfg):
		GlowMeshHolder(layer),
		BaseLaserServer({cfg.capacity, gm, ::board, ::playfield, cfg}) {}
};

constexpr Danmaku::LaserServerInstanceConfig ENEMY_LASER_SERVER_CFG = {
	64,
	Danmaku::LaserCollisionConfig{}
};

constexpr Danmaku::LaserServerInstanceConfig PLAYER_LASER_SERVER_CFG = {
	16,
	Danmaku::LaserCollisionConfig{
		{Danmaku::Collision::Layer::PLAYER_LASER, Danmaku::Collision::Tag::FOR_PLAYER_1},
		{Danmaku::Collision::Mask::PLAYER_LASER},
		{}
	}
};

// Boss stuff

struct TestBoss;

using TestBossRegistry = Makai::Ex::Game::Registry<TestBoss>;

struct TestBoss: Danmaku::ABoss, TestBossRegistry::Member {
	struct TestAct: AAct {
		usize const id = 0;

		TestBulletServer& bulletServer;

		TestAct(TestBoss& boss, usize const id, TestBulletServer& bulletServer):
			AAct(boss), id(id), bulletServer(bulletServer) {}

		void createShots(float const stride = 10) {
			DEBUGLN("Firing shots...");
			for (usize i = 0; i < 5; ++i) {
				auto bullet = bulletServer.acquire().as<Danmaku::Bullet>();
				if (!bullet) return;
				float const crot = (TAU / 20) * (i + (Makai::App::current()->getCurrentCycle() / stride * 0.5));
				bullet->trans.position = boss.trans.position;
				bullet->velocity = {
					-30,
					true,
					-30,
					30,
					.03
				};
				bullet->rotation = {
					crot,
					true,
					crot,
					crot + static_cast<float>(TAU),
					.02,
					Makai::Math::Ease::InOut::back
				};
				bullet->spawn();
			}
		}

		PromiseType task() override {
			while (true) {
				createShots();
				co_yield 10;
			}
		}

		usize next() const override {
			return id;
		}
	};

	Makai::Random::Generator rng;
	
	Makai::Graph::Renderable mesh;

	Makai::Ex::Game::SpriteInstance sprite;

	TestBulletServer&	bulletServer;
	TestLaserServer&	laserServer;
	TestItemServer&		itemServer;

	TestBoss(TestBulletServer& bulletServer, TestLaserServer& laserServer, TestItemServer& itemServer):
	ABoss({::board, ::playfield}),
	bulletServer(bulletServer),
	laserServer(laserServer),
	itemServer(itemServer) {
		rng.setSeed(Makai::OS::Time::now());
		sprite = mesh.createReference<Makai::Ex::Game::Sprite>();
		mesh.setRenderLayer(Danmaku::Render::Layer::ENEMY1_LAYER);
		setHealth(1000, 1000);
		movement
		.setInterpolation(trans.position, board.center * Makai::Vec2(1, 0.5), 60, Makai::Math::Ease::Out::cubic)
		.onCompleted = [&] {
			DEBUGLN("Collider: [ ", collider->position.x, ", ", collider->position.y, " ]");
			beginBattle();
		};
		movement.setManual();
		trans.scale = 4;
		collision()->canCollide = false;
		collision()->shape = collider.as<Danmaku::C2D::IBound2D>();
		healthBar.setRenderLayer(Danmaku::Render::Layer::INGAME_OVERLAY_BOTTOM_LAYER);
		healthBar.size = 2.0;
		healthBar.setBlendEquation(Makai::Graph::BlendEquation::BE_ADD);
		healthBar.material.color = Makai::Graph::Color::MAGENTA * Makai::Graph::Color::alpha(0.5);
	}

	void spawnLasers(float const offset) {
		for (ssize i: {1, 2, 3, 4}) {
			auto laser = laserServer.acquire().as<Danmaku::Laser>();
			if (!laser) return;
			laser->trans.position = Makai::Vec2(-10, (-i) * (64.0 / 5.0) + offset);
			laser->rotation = {0};
			laser->length = {128};
			laser->spawn();
			laser->spawnTime	= 60 + 30 * i;
			laser->toggleTime	= 60;
			laser->onAction = [=] (auto& object, auto action) {
				Danmaku::Laser& l = (Danmaku::Laser&)object;
				if (action == Danmaku::AServerObject::Action::SOA_SPAWN_END) {
					l.toggle(true);
					l.onAction.clear();
				}
			};
		}
	}

	usize counter = 0;

	void onUpdate(float delta) override {
		ABoss::onUpdate(delta);
		mesh.trans.position		= trans.position;
		mesh.trans.rotation.z	= trans.rotation;
		mesh.trans.scale		= trans.scale;
		collider->position		= trans.position;
		movement.onUpdate(1);
		trans.position = movement.value();
	}

	void moveRandom() {
		movement
		.setInterpolation(
			trans.position,
			board.center * Makai::Vec2(1, 0.75) + rpos(32, 16), 60,
			Makai::Math::Ease::Out::cubic
		)
		.onCompleted.clear();
	}

	void createItems() {
		if (auto item = itemServer.acquire().as<Danmaku::Item>()) {
			item->trans.position = trans.position + rpos(6, 6);
			item->gravity = Danmaku::Property<Makai::Vec2>{
				.interpolate = true,
				.start = Makai::Vec2(0, 1),
				.stop = Makai::Vec2(0, -1),
				.speed = 0.025
			};
			item->radius = {1};
			item->terminalVelocity = {Makai::Vec2(0, 20)};
			item->spawnTime = 30;
			item->spawn();
		}
	}

	Makai::Vec2 rpos(float const rangeX, float const rangeY) {
		return Makai::Vec2(
			rng.number<float>(-rangeX, rangeX),
			rng.number<float>(-rangeY, rangeY)
		);
	}

	void onBattleBegin() override {
		collision()->canCollide = true;
		doAct(0);
	}
	
	void onBattleEnd() override	{
		for (usize i = 0; i < 20; ++i)
			createItems();
		collision()->canCollide = false;
		bulletServer.despawnAll();
		laserServer.despawnAll();
		queueDestroy();
	}

	Makai::Unique<AAct> onAct(usize const act) override {
		bulletServer.despawnAll();
		DEBUGLN("Act: [", act, "]");
		if (act > 0)
			for (usize i = 0; i < 10; ++i)
				createItems();
		if (act == CTL::Limit::MAX<usize>) {
			endBattle();
			return {};
		}
		moveRandom();
		if (act == 1) spawnLasers(0);
		if (act == 2) spawnLasers(6.4);
		setHealth(1000, 1000);
		return ActInstanceType::create<TestAct>(*this, act < 2 ? act+1 : -1, bulletServer);
	}
	
	TestBoss& spawn() override				{return *this;		}
	TestBoss& despawn() override			{return *this;		}

	Makai::Tween<Makai::Vec2> movement;

	Makai::Instance<Danmaku::C2D::Circle> collider = new Danmaku::C2D::Circle(0, 4);
};

// Player stuff

Danmaku::PlayerConfig cfg = {board, playfield};

struct TestPlayer: Danmaku::APlayer {
	MkGraph::Renderable body;
	
	Makai::Ex::Game::SpriteInstance sprite;

	TestBulletServer& server;

	TestPlayer(TestBulletServer& server):
		APlayer(cfg),
		sprite(body.createReference<Makai::Ex::Game::Sprite>()),
		server(server) {
		body.setRenderLayer(Danmaku::Render::Layer::PLAYER1_LAYER);
		trans.position = board.center * Makai::Vector2(1, 1.5);
		input.binds["player/up"] 	= {Makai::Input::KeyCode::KC_UP};
		input.binds["player/down"]	= {Makai::Input::KeyCode::KC_DOWN};
		input.binds["player/left"] 	= {Makai::Input::KeyCode::KC_LEFT};
		input.binds["player/right"]	= {Makai::Input::KeyCode::KC_RIGHT};
		input.binds["player/focus"]	= {Makai::Input::KeyCode::KC_LEFT_SHIFT};
		input.binds["player/shot"] 	= {Makai::Input::KeyCode::KC_Z};
		input.binds["player/bomb"] 	= {Makai::Input::KeyCode::KC_X};
		velocity = {20, 10};
		collision()->canCollide	= true;
		collision()->shape		= collider.as<Danmaku::C2D::IBound2D>();
		getGrazebox()->shape	= grazebox.as<Danmaku::C2D::IBound2D>();
		getItembox()->shape		= itembox.as<Danmaku::C2D::IBound2D>();
	}

	virtual ~TestPlayer() {}

	void onUpdate(float delta) override {
		if (!active) return;
		APlayer::onUpdate(delta);
		if (paused()) return;
		body.trans.position		= trans.position;
		body.trans.rotation.z	= trans.rotation;
		body.trans.scale		= trans.scale;
		itembox->position		=
		grazebox->position		=
		collider->position		= trans.position;
		if (shot > 0) --shot;
	}

	void onUpdate(float delta, Makai::App& app) override {
		if (!active) return;
		APlayer::onUpdate(delta, app);
		if (paused()) return;
	}

	void createShots() {
		if (shot) return;
		shot = 5;
		if (auto bullet = server.acquire().as<Danmaku::Bullet>()) {
			//DEBUGLN("Shots fired!");
			bullet->damage = {50};
			bullet->trans.position = trans.position + ((!focusing()) ? Makai::Vec2(-3, 2) : Makai::Vec2(-1.5, 6));
			bullet->rotation = {-HPI};
			bullet->velocity = Danmaku::Property<float>{
				.interpolate	= true,
				.start			= -40,
				.stop			= 60,
				.speed			= 0.05
			};
			bullet->radius = {1};
			bullet->spawn();
		}
		if (auto bullet = server.acquire().as<Danmaku::Bullet>()) {
			//DEBUGLN("Shots fired!");
			bullet->damage = {50};
			bullet->trans.position = trans.position + ((!focusing()) ? Makai::Vec2(3, 2) : Makai::Vec2(1.5, 6));
			bullet->rotation = {-HPI};
			bullet->velocity = Danmaku::Property<float>{
				.interpolate	= true,
				.start			= -40,
				.stop			= 60,
				.speed			= 0.05
			};
			bullet->radius = {1};
			bullet->spawn();
		}
	}

	virtual void onItem(Makai::Reference<Danmaku::Item> const& item) override				{				}
	virtual void onGraze(Makai::Reference<Danmaku::AServerObject> const& object) override	{				}
	virtual void onBomb() override															{				}
	virtual void onShot() override															{createShots();	}
	virtual TestPlayer& spawn() override													{return *this;	}
	virtual TestPlayer& despawn() override													{return *this;	}

	virtual TestPlayer& takeDamage(Makai::Reference<Danmaku::AGameObject> const& obj, CollisionMask const&) override {
		if (obj.as<Danmaku::AServerObject>()->isFree()) return *this;
		makeInvincible(120);
		trans.position = board.center * Makai::Vector2(1, 1.5);
		return *this;	
	}

	virtual TestPlayer& takeDamage(float const damage) override {
		makeInvincible(120);
		trans.position = board.center * Makai::Vector2(1, 1.5);
		return *this;
	}

	usize shot = 5;

	Makai::Instance<Danmaku::C2D::Circle> collider	= new Danmaku::C2D::Circle(0, 0.1);
	Makai::Instance<Danmaku::C2D::Circle> grazebox	= new Danmaku::C2D::Circle(0, 2);
	Makai::Instance<Danmaku::C2D::Circle> itembox	= new Danmaku::C2D::Circle(0, 10);
};

// Application

struct TestApp: Makai::Ex::Game::App {
	Makai::Random::SecureGenerator rng;

	TestBossRegistry::Object	boss;
	Makai::Instance<TestPlayer>	player;

	TestBulletServer	enemyBullet		= TestBulletServer(Danmaku::Render::Layer::ENEMY1_BULLET_LAYER, ENEMY_BULLET_SERVER_CFG);
	TestBulletServer	playerBullet	= TestBulletServer(Danmaku::Render::Layer::PLAYER1_BULLET_LAYER, PLAYER_BULLET_SERVER_CFG);
	TestLaserServer		enemyLaser		= TestLaserServer(Danmaku::Render::Layer::ENEMY1_LASER_LAYER, ENEMY_LASER_SERVER_CFG);
	TestItemServer		item			= TestItemServer(Danmaku::Render::Layer::PLAYER1_ITEM_LAYER);

	TestApp(): App(Makai::Config::App{{800, 600, "Test 07", false}}) {
		loadDefaultShaders();
		camera.cam2D = Makai::Graph::Camera3D::from2D(64, Makai::Vector2(4, 3) / 3.0);
		boss	= TestBossRegistry::create<TestBoss>(enemyBullet, enemyLaser, item);
		player	= new TestPlayer(playerBullet);
		DEBUGLN("<c2d:layers>");
		for (usize i = 0; i < Danmaku::C2D::Server::MAX_LAYERS; ++i) {
			DEBUGLN("<c2d:layer id=", i, " count=", Danmaku::C2D::Server::layers[i].getColliderCount(), " />");
			for (usize j = i; j < Danmaku::C2D::Server::MAX_LAYERS; ++j) {
				if (Danmaku::C2D::Server::layers[i].affects & Danmaku::C2D::Server::layers[j].affectedBy)
					DEBUGLN (
						"<c2d:overlap from='", 
						Danmaku::Collision::Layer::asName(i),
						"' to='",
						Danmaku::Collision::Layer::asName(j),
						"'/>"
					);
				if (Danmaku::C2D::Server::layers[j].affects & Danmaku::C2D::Server::layers[i].affectedBy)
					DEBUGLN (
						"<c2d:overlap from='",
						Danmaku::Collision::Layer::asName(j),
						"' to='",
						Danmaku::Collision::Layer::asName(i),
						"'/>"
					);
			}
		}
		DEBUGLN("</c2d:layers>");
	}

	void onLayerDrawBegin(usize const layerID) override {
		camera.use(layerID >= Danmaku::Render::Layer::BOSS1_SPELL_BG_BOTTOM_LAYER);
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

	void onUpdate(float delta) override {
		TestBossRegistry::destroyQueued();
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getCycleDelta();
		else {
			float fravg = 0;
			for(float& f: framerate) fravg += f;
			fravg *= (1.0 / (float)MAX_FRCOUNT);
			fravg = Makai::Math::clamp<float>(fravg, 0, maxFrameRate);
			DEBUGLN("Framerate: ", Makai::Format::prettify(Makai::Math::round(fravg, 2), 2, 0));
			frcount = 0;
		}
	}

	void onClose() override {
		DEBUGLN("Boss still exists (somehow)? ", ((boss.exists()) ? "YES" : "NO"));
	}
};

int main() {
	try {
		TestApp app;
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}