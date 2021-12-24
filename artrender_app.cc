#include "artrender_app.h"
#include "config.h"

#include <iostream>
#include <sstream>



using namespace std;

ArtRenderApp::ArtRenderApp() : GraphicsApp(1024,768, "Art Render"),
    shader_style_(0), current_model_(0), light_pos_(1.5, 1.5, 1.5),
    diffuse_ramp_(GL_CLAMP_TO_EDGE), specular_ramp_(GL_CLAMP_TO_EDGE)
{
    // Define a search path for finding data files (images and shaders)
    search_path_.push_back(".");
    search_path_.push_back("./data");
    search_path_.push_back("./shaders");
    search_path_.push_back(DATA_DIR_INSTALL);
    search_path_.push_back(DATA_DIR_BUILD);
    search_path_.push_back(SHADERS_DIR_INSTALL);
    search_path_.push_back(SHADERS_DIR_BUILD);
    
    // NOTE: YOU CAN COMMENT OUT SOME OF THESE IF THE APP IS LOADING TOO SLOWLY
    // THE MODEL_FILES ARRAY JUST NEEDS TO HOLD AT LEAST ONE MODEL.
	// Also, compiling in Release mode will optimize the loading of text files a lot.
    model_files_.push_back("bunny.obj");
    model_files_.push_back("chamferedCube.obj");
    model_files_.push_back("cow.obj");
    model_files_.push_back("hippo.obj");
    model_files_.push_back("maxplanck.obj");
    model_files_.push_back("sphere.obj");
    model_files_.push_back("teapot.obj");
}


ArtRenderApp::~ArtRenderApp() {
}


void ArtRenderApp::InitNanoGUI() {
	// Setup the GUI window
	nanogui::Window* window = new nanogui::Window(screen(), "Shading Style");
	window->setPosition(Eigen::Vector2i(10, 10));
	window->setSize(Eigen::Vector2i(200, 100));
	window->setLayout(new nanogui::GroupLayout());

	new nanogui::Label(window, "Dynamically Reload", "sans-bold");

	nanogui::Button* btnReload = new nanogui::Button(window, "Reload Shaders and Textures");
	btnReload->setCallback(std::bind(&ArtRenderApp::OnReloadBtnPressed, this));


	new nanogui::Label(window, "Rendering Style", "sans-bold");

	nanogui::Button* btnGouraud = new nanogui::Button(window, "Gouraud Shading");
	btnGouraud->setCallback(std::bind(&ArtRenderApp::OnGouraudBtnPressed, this));

	nanogui::Button* btnPhong = new nanogui::Button(window, "Phong Shading");
	btnPhong->setCallback(std::bind(&ArtRenderApp::OnPhongBtnPressed, this));

	nanogui::Button* btnArtsy = new nanogui::Button(window, "Artsy Shading");
	btnArtsy->setCallback(std::bind(&ArtRenderApp::OnArtsyBtnPressed, this));


	new nanogui::Label(window, "Model", "sans-bold");

	for (int i = 0; i < model_files_.size(); i++) {
		nanogui::Button* btn = new nanogui::Button(window, model_files_[i]);
		btn->setCallback([this, i] { this->current_model_ = i; });
		Mesh m;
		m.LoadFromOBJ(Platform::FindFile(model_files_[i], search_path_));
		meshes_.push_back(m);
		EdgeMesh em;
		em.CreateFromMesh(m);
		edge_meshes_.push_back(em);
	}
	screen()->performLayout();
}


void ArtRenderApp::OnLeftMouseDown(const Point2 &pos) {
    Point2 normalizedMousePos = PixelsToNormalizedDeviceCoords(pos);
    float mouseZ = ReadZValueAtPixel(pos);
    uni_cam_.OnButtonDown(normalizedMousePos, mouseZ);
}


void ArtRenderApp::OnLeftMouseDrag(const Point2 &pos, const Vector2 &delta) {
    Point2 normalizedMousePos = PixelsToNormalizedDeviceCoords(pos);
    uni_cam_.OnDrag(normalizedMousePos);
}


void ArtRenderApp::OnLeftMouseUp(const Point2 &pos) {
    Point2 normalizedMousePos = PixelsToNormalizedDeviceCoords(pos);
    uni_cam_.OnButtonUp(normalizedMousePos);
}


void ArtRenderApp::UpdateSimulation(double dt)  {
    uni_cam_.AdvanceAnimation(dt);
}


void ArtRenderApp::OnReloadBtnPressed() {
	LoadShadersAndTextures();
}

void ArtRenderApp::OnGouraudBtnPressed() {
	shader_style_ = 0;
}

void ArtRenderApp::OnPhongBtnPressed() {
	shader_style_ = 1;
}

void ArtRenderApp::OnArtsyBtnPressed() {
	shader_style_ = 2;
}



void ArtRenderApp::InitOpenGL() {
    // Set up the camera in a good position to see the model
	proj_matrix_ = Matrix4::Perspective(30, aspect_ratio(), 0.1f, 50.0f);
    uni_cam_.set_view_matrix(Matrix4::LookAt(Point3(0,0,3), Point3(0,0,0), Vector3(0,1,0)));
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	// Customize the lighting used for the quick shapes default shader to make the
	// ambient really bright since we're using it to draw the yellow "light bulb" 
	// in the scene.
	DefaultShader::LightProperties qs_light;
	qs_light.ambient_intensity = Color(0.8f, 0.8f, 0.8f);
	qs_light.diffuse_intensity = Color(0.4f, 0.4f, 0.4f);
	qs_light.specular_intensity = Color(0.0f, 0.0f, 0.0f);
	quick_shapes_.default_shader()->SetLight(0, qs_light);

	LoadShadersAndTextures();
}


void ArtRenderApp::LoadShadersAndTextures() {
	// (Re)loads and (re)compiles all of the shader programs text files.  And, (re)loads the
	// textures from file as well.

	gouraud_shaderprog_.AddVertexShaderFromFile(Platform::FindFile("gouraud.vert", search_path_));
	gouraud_shaderprog_.AddFragmentShaderFromFile(Platform::FindFile("gouraud.frag", search_path_));
	gouraud_shaderprog_.LinkProgram();

	phong_shaderprog_.AddVertexShaderFromFile(Platform::FindFile("phong.vert", search_path_));
	phong_shaderprog_.AddFragmentShaderFromFile(Platform::FindFile("phong.frag", search_path_));
	phong_shaderprog_.LinkProgram();

	artsy_shaderprog_.AddVertexShaderFromFile(Platform::FindFile("artsy.vert", search_path_));
	artsy_shaderprog_.AddFragmentShaderFromFile(Platform::FindFile("artsy.frag", search_path_));
	artsy_shaderprog_.LinkProgram();

	outline_shaderprog_.AddVertexShaderFromFile(Platform::FindFile("outline.vert", search_path_));
	outline_shaderprog_.AddFragmentShaderFromFile(Platform::FindFile("outline.frag", search_path_));
	outline_shaderprog_.LinkProgram();

	// To try out different shading styles, you can replace toonDiffuse.png and toonSpecular.png with
	// some of the other texture files in the data/ directory.
	diffuse_ramp_.InitFromFile(Platform::FindFile("toonDiffuse.png", search_path_));
	diffuse_ramp_.set_wrap_mode(GL_CLAMP_TO_EDGE);

	specular_ramp_.InitFromFile(Platform::FindFile("toonSpecular.png", search_path_));
	specular_ramp_.set_wrap_mode(GL_CLAMP_TO_EDGE);
}


void ArtRenderApp::DrawUsingOpenGL() {
    // Just the identity matrix
    Matrix4 model_matrix;
    
    // Lighting parameters
    static const Color Ia(0.3f, 0.3f, 0.3f, 1.0f);
    static const Color Id(0.7f, 0.7f, 0.7f, 1.0f);
    static const Color Is(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Material parameters
    static const Color ka(1.0f, 0.4f, 0.4f, 1.0f);
    static const Color kd(1.0f, 0.4f, 0.4f, 1.0f);
    static const Color ks(0.6f, 0.6f, 0.6f, 1.0f);
    static const float s = 50.0f;
    
    // Precompute items needed in the shader
    
    // Light positions are usually defined in world space.  For lighting calculations
    // we need the position of the light in view space (a.k.a. eye space).
    Point3 light_in_eye_space = uni_cam_.view_matrix() * light_pos_;
    
    // The shader also needs these matrices
    Matrix4 model_view_matrix = uni_cam_.view_matrix()*model_matrix;
    Matrix4 normal_matrix = model_view_matrix.Inverse().Transpose();
    
    // Make sure the default option to only draw front facing triangles is set
    glEnable(GL_CULL_FACE);
    
	if (shader_style_ == 0) {

		// Render the current model's mesh using the Gouraud shader program
		gouraud_shaderprog_.UseProgram();
		gouraud_shaderprog_.SetUniform("model_view_matrix", model_view_matrix);
		gouraud_shaderprog_.SetUniform("normal_matrix", normal_matrix);
		gouraud_shaderprog_.SetUniform("proj_matrix", proj_matrix_);
		gouraud_shaderprog_.SetUniform("ka", ka);
		gouraud_shaderprog_.SetUniform("kd", kd);
		gouraud_shaderprog_.SetUniform("ks", ks);
		gouraud_shaderprog_.SetUniform("s", s);
		gouraud_shaderprog_.SetUniform("light_in_eye_space", light_in_eye_space);
		gouraud_shaderprog_.SetUniform("Ia", Ia);
		gouraud_shaderprog_.SetUniform("Id", Id);
		gouraud_shaderprog_.SetUniform("Is", Is);
		meshes_[current_model_].Draw();
		gouraud_shaderprog_.StopProgram();

	}
	if (shader_style_ == 1) {

		// Render the current model's mesh using the Phong shader program
		phong_shaderprog_.UseProgram();
		phong_shaderprog_.SetUniform("model_view_matrix", model_view_matrix);
		phong_shaderprog_.SetUniform("normal_matrix", normal_matrix);
		phong_shaderprog_.SetUniform("proj_matrix", proj_matrix_);
		phong_shaderprog_.SetUniform("ka", ka);
		phong_shaderprog_.SetUniform("kd", kd);
		phong_shaderprog_.SetUniform("ks", ks);
		phong_shaderprog_.SetUniform("s", s);
		phong_shaderprog_.SetUniform("light_in_eye_space", light_in_eye_space);
		phong_shaderprog_.SetUniform("Ia", Ia);
		phong_shaderprog_.SetUniform("Id", Id);
		phong_shaderprog_.SetUniform("Is", Is);
		meshes_[current_model_].Draw();
		phong_shaderprog_.StopProgram();

	}
	else if (shader_style_ == 2) {

		// Rendering using the Artsy shader programs

		// Step 1: Use the toon shader to draw the object's mesh
		artsy_shaderprog_.UseProgram();
		artsy_shaderprog_.SetUniform("model_view_matrix", model_view_matrix);
		artsy_shaderprog_.SetUniform("normal_matrix", normal_matrix);
		artsy_shaderprog_.SetUniform("proj_matrix", proj_matrix_);
		artsy_shaderprog_.SetUniform("ka", ka);
		artsy_shaderprog_.SetUniform("kd", kd);
		artsy_shaderprog_.SetUniform("ks", ks);
		artsy_shaderprog_.SetUniform("s", s);
		artsy_shaderprog_.SetUniform("light_in_eye_space", light_in_eye_space);
		artsy_shaderprog_.SetUniform("Ia", Ia);
		artsy_shaderprog_.SetUniform("Id", Id);
		artsy_shaderprog_.SetUniform("Is", Is);
		artsy_shaderprog_.BindTexture("diffuse_ramp", diffuse_ramp_);
		artsy_shaderprog_.BindTexture("specular_ramp", specular_ramp_);
		meshes_[current_model_].Draw();
		artsy_shaderprog_.StopProgram();


		// Step 2: Draw the silhouette edge using the edge mesh and outline shader

		// Disable back face culling so OpenGL will draw both front and back facing triangles
		glDisable(GL_CULL_FACE);

		// Set the OpenGL polygon offset so it will draw triangles even if they
		// exactly on top of another triangle
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1, 1);

		static const float thickness = 0.01f;

		// Draw edge mesh
		outline_shaderprog_.UseProgram();
		outline_shaderprog_.SetUniform("model_view_matrix", model_view_matrix);
		outline_shaderprog_.SetUniform("normal_matrix", normal_matrix);
		outline_shaderprog_.SetUniform("proj_matrix", proj_matrix_);
		outline_shaderprog_.SetUniform("thickness", thickness);
		edge_meshes_[current_model_].Draw();
		outline_shaderprog_.StopProgram();

	}

	// Draw a little yellow sphere at the location of the light source
	Matrix4 light_model = Matrix4::Translation(light_pos_ - Point3(0, 0, 0)) *
		Matrix4::Scale(Vector3(0.1f, 0.1f, 0.1f));
	quick_shapes_.DrawSphere(light_model, uni_cam_.view_matrix(), proj_matrix_, Color(1, 1, 0));

    // Draw the UniCam widget when in rotation mode
    uni_cam_.Draw(proj_matrix_);
}
