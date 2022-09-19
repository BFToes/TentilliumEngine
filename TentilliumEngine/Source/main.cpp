#include "Rendering/Window/AppWindow.h"
#include "Scene.h"
#include "Rendering/Resources/ShaderProgram.h"
#include "Components/Projection.h"
#include "Components/PointLight.h"

/* FINALIZE:
*
*/

/* REVIEW:
*	> localize reference counting to Resource
*	> replace current flag system with replace and a entt tag
*/

/* TODO:
*	
*	> seperate rendering functionality further for mesh/camera/rendering type
* 
*	> Tidy AppWindow functionality into something less ugly
*	> Tidy Shader functionality into something less ugly
*	> Rendering
*	
*   > bones, skinning and animation
*	> seperate Transform and Transform implementation
*/

/* RESEARCH:
*	> Morton encoding for position in a quad tree -> chunking???
*	> ANGLE a library that compiles opengl calls into directx
*/

/*
	Y  Z
	| /
	|/
	O---X
*/

std::vector<float> positions = {
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f,  0.5f
};
std::vector<float> normals = {
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f,  0.5f
};
std::vector<float> texCoords = {
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 
	1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
	0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f
};
std::vector<unsigned int> indices = {	// mesh indices
	0, 2, 1, 2, 0, 3, 		// e(0, 2)	 = ---,++-	// back
	4, 5, 6, 6, 7, 4,		// e(4, 6)   =	--+,+++	// front
	8, 9, 10, 10, 4, 8,		// e(8, 10)  = -++,---	// right
	11, 12, 2, 12, 11, 13, 	// e(11, 12) = +++,+--	// left
	10, 14, 5, 5, 4, 10,	// e(5, 10)  = +-+,---	// bottom
	2, 3, 11, 11, 3, 15		// e(3, 11)  = -+-,+++	// top
};

unsigned char Rtest[] = {
	0xff, 
	0x7f,
	0x3f, 
	0x1f, 
};
unsigned char RGtest[] = {
	0xff, 0x00,
	0x00, 0xff,
	0x00, 0xff,
	0xff, 0x00,
};
unsigned char RGBtest[] = {
	0xff, 0x00, 0xff, 
	0x00, 0x00, 0xff, 
	0x00, 0x00, 0xff, 
	0xff, 0x00, 0xff, 
};
unsigned char RGBAtest[] = {
	0xff, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0xff, 0x00, 0xff, 0xff,
};


class TestApp : public AppWindow
{
public:
	Scene scene;
	glm::vec2 cam_dir;

	entt::entity obj;
	entt::entity box1;
	entt::entity box2;
	entt::entity root;
	entt::entity light1, light2;

	float time;

	std::vector<entt::entity> lights;

	TestApp(const char* title) : AppWindow(800, 600, title)
	{
		scene.size = AppWindow::size;

		root = scene.create();

		// create camera
		{ 
			entt::entity cam = scene.create();
			scene.emplace<Projection>(cam, glm::radians(60.0f), 800.0f / 600.0f, 0.02f, 100.0f);
			scene.emplace<Position>(cam);
			scene.emplace<Rotation>(cam);
			scene.camera = cam;
		}

		// create obj
		{
			obj = scene.load("Resources/meshes/Person.fbx");
			scene.emplace<Hierarchy>(obj, root);
			scene.emplace<Position>(obj, 0, 0, -2);
			scene.emplace<Rotation>(obj);
		}

		// create box1
		{
			box1 = scene.create();
			scene.emplace<Hierarchy>(box1, obj);
			
			scene.emplace<Position>(box1, -0.5, 0, 0);
			scene.emplace<Scale>(box1, 0.5f);
			
			scene.emplace<VBO<V_Index>>(box1, indices);
			scene.emplace<VBO<V_Position>>(box1, positions);
			scene.emplace<VBO<V_Normal>>(box1, normals);
			scene.emplace<VBO<V_TexCoord>>(box1, texCoords);
			
			scene.emplace<Material>(box1, Texture("Resources/textures/pigeon.jpg"));
		}

		// create box2
		{
			box2 = scene.create();
			scene.emplace<Hierarchy>(box2, box1);
			
			scene.emplace<Position>(box2, 0.5, 0, 0);
			scene.emplace<Scale>(box2, 0.5f);
			
			scene.emplace<VBO<V_Index>>(box2, scene.get<VBO<V_Index>>(box1));
			scene.emplace<VBO<V_Position>>(box2, scene.get<VBO<V_Position>>(box1));
			scene.emplace<VBO<V_Normal>>(box2, scene.get<VBO<V_Normal>>(box1));
			scene.emplace<VBO<V_TexCoord>>(box2, scene.get<VBO<V_TexCoord>>(box1));
			
			scene.emplace<Material>(box2, Texture(RGBtest, 2, 2, Texture::Format::RGB));
		}
		
		// floor
		{
			entt::entity floor = scene.create();
			
			scene.emplace<Position>(floor, 0, -0.1f, 0);
			scene.emplace<Scale>(floor, 200, 0, 200);
			
			scene.emplace<VBO<V_Index>>(floor, scene.get<VBO<V_Index>>(box1));
			scene.emplace<VBO<V_Position>>(floor, scene.get<VBO<V_Position>>(box1));
			scene.emplace<VBO<V_Normal>>(floor, scene.get<VBO<V_Normal>>(box1));
			scene.emplace<VBO<V_TexCoord>>(floor, scene.get<VBO<V_TexCoord>>(box1));
			
			scene.emplace<Material>(floor, Texture("Resources/textures/grid.png"));
		}

		// light 1
		{
			light1 = scene.create();
			scene.emplace<PointLight>(light1, glm::vec3(0.5f, 0, 0), glm::vec3(1, 0, 0), 10.0f);
			
			scene.emplace<Position>(light1, 0.5f, 0, 0);
			scene.emplace<Scale>(light1, 0.1f);
			
			scene.emplace<VBO<V_Index>>(light1, scene.get<VBO<V_Index>>(box1));
			scene.emplace<VBO<V_Position>>(light1, scene.get<VBO<V_Position>>(box1));
			scene.emplace<VBO<V_Normal>>(light1, scene.get<VBO<V_Normal>>(box1));
			scene.emplace<VBO<V_TexCoord>>(light1, scene.get<VBO<V_TexCoord>>(box1));
			
			scene.emplace<Material>(light1, glm::vec4(1, 0, 0, 1));
		}

		// light 2
		{
			light2 = scene.create();
			scene.emplace<PointLight>(light2, glm::vec3(-0.5f, 0, 0), glm::vec3(0, 0, 1), 10.0f);
			
			scene.emplace<Position>(light2, -0.5f, 0, 0);
			scene.emplace<Scale>(light2, 0.1f);

			scene.emplace<VBO<V_Index>>(light2, scene.get<VBO<V_Index>>(box1));
			scene.emplace<VBO<V_Position>>(light2, scene.get<VBO<V_Position>>(box1));
			scene.emplace<VBO<V_Normal>>(light2, scene.get<VBO<V_Normal>>(box1));
			scene.emplace<VBO<V_TexCoord>>(light2, scene.get<VBO<V_TexCoord>>(box1));
			
			scene.emplace<Material>(light2, glm::vec4(0, 0, 1, 1));
		}
	}

	void onProcess(float delta) {
		time += delta;

		if (isPressed(Button::LEFT)) cam_dir += (glm::vec2)m_mouse.getDelta() / glm::vec2(getSize());
		

		glm::vec3 move_direction;
		if (isPressed(Key::W)) move_direction.z -= 1;
		if (isPressed(Key::S)) move_direction.z += 1;
		if (isPressed(Key::A)) move_direction.x -= 1;
		if (isPressed(Key::D)) move_direction.x += 1;
		if (isPressed(Key::F)) move_direction.y -= 1;
		if (isPressed(Key::R)) move_direction.y += 1;

		
		if (isPressed(Key::LEFT_SHIFT))		move_direction *= 10;
		if (isPressed(Key::LEFT_CONTROL))	move_direction *= 0.1;

		glm::quat camera_rotation = scene.get<Rotation>(scene.camera);
		glm::vec3 camera_position = scene.get<Position>(scene.camera);
		
		scene.replace<Position>(scene.camera, camera_rotation * move_direction * delta + camera_position); // rotate direction by camera rotation
		scene.replace<Rotation>(scene.camera, cam_dir.y, cam_dir.x, 0.0f);

		scene.replace<Position>(box1, cos(time), sin(time), 0.0f);
		scene.replace<Position>(box2, -sin(time), -cos(time), 0.0f);
		scene.replace<Rotation>(obj, 0.0, time, 0);
	}

	void onDraw() {
		scene.process();
		refresh();
	}

	void onResize(glm::ivec2 size) {
		scene.setSize(size);
	}

	void onKey(Key key, Action action, Mod mod) { }

	void onMouse(Button button, Action action, Mod mod) { }

	void onMouseMove(glm::ivec2 position) { }
};

int main(int argc, char** argv)
{
	// when the program is closed from the X in the console there are memory leaks
	auto app1 = TestApp("app 1");
	AppWindow::main({ &app1 });
}