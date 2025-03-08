#include <makai/makai.hpp>

struct TestApp: Makai::App {
	constexpr static usize cubeGrid		= 8;
	constexpr static usize cubeCount	= cubeGrid*cubeGrid;

	Makai::Graph::Renderable cubes[cubeCount];

	Makai::Graph::Camera3D& camera = Makai::Graph::Global::camera;

	Makai::Graph::Vertex const vertices[8] = {
		//					  X,  Y,  Z, U, V, R, G, B, A, NX, NY, NZ
		Makai::Graph::Vertex(+1, +1, +1, 0, 0, 1, 1, 1, 1, +1, +1, +1),	// 0
		Makai::Graph::Vertex(+1, +1, -1, 0, 0, 1, 1, 0, 1, +1, +1, -1),	// 1
		Makai::Graph::Vertex(+1, -1, +1, 0, 0, 1, 0, 1, 1, +1, -1, +1),	// 2
		Makai::Graph::Vertex(+1, -1, -1, 0, 0, 1, 0, 0, 1, +1, -1, -1),	// 3
		Makai::Graph::Vertex(-1, +1, +1, 0, 0, 0, 1, 1, 1, -1, +1, +1),	// 4
		Makai::Graph::Vertex(-1, +1, -1, 0, 0, 0, 1, 0, 1, -1, +1, -1),	// 5
		Makai::Graph::Vertex(-1, -1, +1, 0, 0, 0, 0, 1, 1, -1, -1, +1),	// 6
		Makai::Graph::Vertex(-1, -1, -1, 0, 0, 0, 0, 0, 1, -1, -1, -1)	// 7
	};

	virtual ~TestApp() {}

	TestApp(): Makai::App(Makai::Config::App{{600, 400, "Test 03", false}}) {
		DEBUGLN("Loading shaders...");
		loadDefaultShaders();
		DEBUGLN("Creating cubes...");
		for (usize j = 0; j < cubeGrid; ++j)
			for (usize i = 0; i < cubeGrid; ++i) {
				const usize idx		= (i*cubeGrid+j);
				constexpr float off	= (float(cubeGrid-1))/2;
				cubes[idx].material.culling = Makai::Graph::CullMode::OCM_FRONT;
				DEBUGLN("Cube [", idx,"]");
				//cubes[idx].material.shaded = true;
				cubes[idx].trans.scale = .125;
				cubes[idx].triangles = {
					// Face +X
					Makai::Graph::Triangle{{vertices[2], vertices[1], vertices[0]}},
					Makai::Graph::Triangle{{vertices[1], vertices[2], vertices[3]}},
					// Face -X
					Makai::Graph::Triangle{{vertices[4], vertices[5], vertices[6]}},
					Makai::Graph::Triangle{{vertices[7], vertices[6], vertices[5]}},
					// Face +Y
					Makai::Graph::Triangle{{vertices[0], vertices[1], vertices[4]}},
					Makai::Graph::Triangle{{vertices[5], vertices[4], vertices[1]}},
					// Face -Y
					Makai::Graph::Triangle{{vertices[6], vertices[3], vertices[2]}},
					Makai::Graph::Triangle{{vertices[3], vertices[6], vertices[7]}},
					// Face +Z
					Makai::Graph::Triangle{{vertices[4], vertices[2], vertices[0]}},
					Makai::Graph::Triangle{{vertices[2], vertices[4], vertices[6]}},
					// Face -Z
					Makai::Graph::Triangle{{vertices[1], vertices[3], vertices[5]}},
					Makai::Graph::Triangle{{vertices[7], vertices[5], vertices[3]}}
				};
				cubes[idx].trans.position = Makai::Vec3(j-off, 0, i-off) * .5;
				cubes[idx].trans.rotation = Makai::Vec3((float(i)/cubeGrid)*TAU, (float(j)/cubeGrid)*TAU);
				cubes[idx].setRenderLayer(0);
			}
		DEBUGLN("Done!");
	}

	void onOpen() override {
		camera.eye	= Makai::Vec3(0, 0, -3);
		//camera.at	= Makai::Vec3(0, -2, 3);
		camera.zFar	= 1000;
		getFrameBuffer().material.background = Makai::Graph::Color::GRAY;
		//Makai::Graph::API::toggle(Makai::Graph::API::Facility::GAF_DEBUG, true);
		camera.relativeToEye = true;
	}

	constexpr static usize MAX_FRCOUNT = 10;

	usize frcount = 0;

	float framerate[MAX_FRCOUNT];

	Makai::Vec3 crot = 0;

	void onUpdate(float delta) {
		if (frcount < MAX_FRCOUNT)
			framerate[frcount++] = 1000.0 / getFrameDelta();
		else {
			float fravg = 0;
			for(float& f: framerate) fravg += f;
			fravg *= (1.0 / (float)MAX_FRCOUNT);
			fravg = Makai::Math::clamp<float>(fravg, 0, maxFrameRate);
			DEBUGLN("Framerate: ", Makai::Format::prettify(Makai::Math::round(fravg, 2), 2, 0));
			frcount = 0;
			DEBUGLN(crot.y);
		}
		if (input.isButtonJustPressed(Makai::Input::KeyCode::KC_ESCAPE))
			close();
		camera.eye.z += (
			input.isButtonDown(Makai::Input::KeyCode::KC_W)
			- input.isButtonDown(Makai::Input::KeyCode::KC_S)
		) / 30.0;
		camera.eye.x += (
			input.isButtonDown(Makai::Input::KeyCode::KC_A)
			- input.isButtonDown(Makai::Input::KeyCode::KC_D)
		) / 30.0;
		camera.eye.y -= (
			input.isButtonDown(Makai::Input::KeyCode::KC_LEFT_SHIFT)
			- input.isButtonDown(Makai::Input::KeyCode::KC_SPACE)
		) / 30.0;
		crot += Makai::Vec3(
			(
				input.isButtonDown(Makai::Input::KeyCode::KC_UP)
				- input.isButtonDown(Makai::Input::KeyCode::KC_DOWN)
			) / 60.0,
			(
				input.isButtonDown(Makai::Input::KeyCode::KC_RIGHT)
				- input.isButtonDown(Makai::Input::KeyCode::KC_LEFT)
			) / 60.0
		);
		crot.x = Makai::Math::clamp<float>(crot.x, -2, 2);
		float cs, cc;
		Makai::Math::sincos(crot.y, cs, cc);
		camera.at = Makai::Vec3(cs, crot.x, cc);
		//camera.at = Makai::Math::angleV3(crot, Makai::Math::RotationAxis::POS_Z);
		//camera.at = Vec3(0, 0, 1);
	}
};

int main() {
	try {
		TestApp app;
		app.maxFrameRate = 60.0;
		app.run();
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.what());
	}
	return 0;
}
