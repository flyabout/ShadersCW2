
#include "scenediffuse.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
using std::cerr;
using std::endl;

#include "defines.h"

using glm::vec3;


#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp>


namespace imat2908
{


/////////////////////////////////////////////////////////////////////////////////////////////
// Default constructor
/////////////////////////////////////////////////////////////////////////////////////////////
SceneDiffuse::SceneDiffuse() //Constructor to set base values for variables
{
	Attenuation = 1.0f;
	SpotlightAttenuation = 1.0f;
	SpotlightPos = vec3(0.0f, 20.0f, 0.0f);
	SpotlightDir = vec3(0.0f, -1.0f, 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Initialise the scene
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::initScene(QuatCamera camera)
{
    //|Compile and link the shader  
	compileAndLinkShader();

    gl::Enable(gl::DEPTH_TEST);

 	//Set up the lighting
	setLightParams(camera);


	//Create the plane to represent the ground
	plane = new VBOPlane(100.0,100.0,100,100);



	//A matrix to move the teapot lid upwards
	glm::mat4 lid = glm::mat4(1.0);
	//lid *= glm::translate(vec3(0.0,1.0,0.0));

	//Create the teapot with translated lid
	teapot = new VBOTeapot(16,lid);


}

/////////////////////////////////////////////////////////////////////////////////////////////
//Update not used at present
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::update( float t )
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
// Set up the lighting variables in the shader
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::setLightParams(QuatCamera camera)
{

	vec3 worldLight = vec3(10.0f,10.0f,10.0f);
   
	
	prog.setUniform("Ld", 0.9f, 0.9f, 0.9f);//what elements does Ld have?
	prog.setUniform("Ls", 0.3f, 0.3f, 0.3f);
	prog.setUniform("La", 0.3f, 0.3f, 0.3f);
	
	prog.setUniform("LightPosition", worldLight );
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Render the scene
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::render(QuatCamera camera)
{
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	//Pass values into the fragment shader
	prog.setUniform("cameraPos", camera.position());
	prog.setUniform("lightCutoff", glm::cos(glm::radians(12.5f)));
	prog.setUniform("OuterlightCutoff", glm::cos(glm::radians(17.5f)));
	prog.setUniform("Attenuation", Attenuation);
	prog.setUniform("SpotlightAttenuation", SpotlightAttenuation);
	prog.setUniform("SpotlightDir", SpotlightDir);
	//First deal with the plane to represent the ground

	//Initialise the model matrix for the plane
	model = mat4(1.0f);
	//Set the matrices for the plane although it is only the model matrix that changes so could be made more efficient
    setMatrices(camera);
	//Set the plane's material properties in the shader and render
	prog.setUniform("Kd", 0.51f, 1.0f, 0.49f); // What elements does Kd have?
	prog.setUniform("Ks", 0.1f, 0.1f, 0.1f);
	prog.setUniform("Ka", 0.51f, 1.0f, 0.49f);
	prog.setUniform("n", 0.5f);
	plane->render();// what does it do?



	//Now set up the teapot 
	 model = mat4(1.0f);
	 setMatrices(camera);
	 //Set the Teapot material properties in the shader and render
	 prog.setUniform("Kd", 0.46f, 0.29f, 0.0f); // What elements does Kd have?
	 prog.setUniform("Ks", 0.29f, 0.29f, 0.29f);
	 prog.setUniform("Ka", 0.46f, 0.29f, 0.0f);
	 prog.setUniform("n", 0.5f);
	 teapot->render(); // what does it do?
	
}



/////////////////////////////////////////////////////////////////////////////////////////////
//Send the MVP matrices to the GPU
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::setMatrices(QuatCamera camera)
{

    mat4 mv = camera.view() * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) ));
    prog.setUniform("MVP", camera.projection() * mv);
	mat3 normMat = glm::transpose(glm::inverse(mat3(model)));// What does this line do?
	prog.setUniform("M", model);
	prog.setUniform("V", camera.view() ); //pass camera?!?!?
	prog.setUniform("P", camera.projection() );

	prog.setUniform("SpotLight", SpotlightPos); // Pass spotlight position to vertex shader
	
}

/////////////////////////////////////////////////////////////////////////////////////////////
// resize the viewport
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::resize(QuatCamera camera,int w, int h)
{
    gl::Viewport(0,0,w,h);
    width = w;
    height = h;
	camera.setAspectRatio((float)w/h);

}

/////////////////////////////////////////////////////////////////////////////////////////////
// Compile and link the shader
/////////////////////////////////////////////////////////////////////////////////////////////
void SceneDiffuse::compileAndLinkShader()
{
   
	try {
    	prog.compileShader("Shaders/phong.vert");
    	prog.compileShader("Shaders/phong.frag");
    	prog.link();
    	prog.validate();
    	prog.use();
    } catch(GLSLProgramException & e) {
 		cerr << e.what() << endl;
 		exit( EXIT_FAILURE );
    }
}

void SceneDiffuse::KeyProcess(GLFWwindow * window) //Function to process key inputs and change private variables
{
	if (glfwGetKey(window, GLFW_KEY_UP))
		Attenuation += 0.05;
	if (glfwGetKey(window, GLFW_KEY_DOWN))
		Attenuation -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_RIGHT))
		SpotlightAttenuation -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_LEFT))
		SpotlightAttenuation += 0.05;
	if (glfwGetKey(window, GLFW_KEY_W))
		SpotlightPos.z -= 0.5;
	if (glfwGetKey(window, GLFW_KEY_S))
		SpotlightPos.z += 0.5;
	if (glfwGetKey(window, GLFW_KEY_A))
		SpotlightPos.x -= 0.5;
	if (glfwGetKey(window, GLFW_KEY_D))
		SpotlightPos.x += 0.5;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		SpotlightPos.y += 0.5;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
		SpotlightPos.y -= 0.5;
	if (glfwGetKey(window, GLFW_KEY_I))
		SpotlightDir.z -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_K))
		SpotlightDir.z += 0.05;
	if (glfwGetKey(window, GLFW_KEY_J))
		SpotlightDir.x -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_L))
		SpotlightDir.x += 0.05;
}


}
