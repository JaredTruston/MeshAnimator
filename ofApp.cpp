// Robot Builder Project: This project provides implementation for
//  creating a 3D scene in the viewer which the user can add scene objects to.
// The User can create Joint scene objects and connect them to create a
//  transformable skeleton which multiple 3D models can then be mapped onto.
// The models can then be ray traced along with a floor plane to create an
//  image with smooth phong shading and shadows.
// This file provides implementation of both Mesh and ofApp methods.
// - author: Jared Bechthold 
// - starter files provided by Professor Kevin Smith

#include "ofApp.h"

//--------------------------------------------------------------
// Returns the size of the mesh in KB
int Mesh::getMeshSize()
{
	return (sizeof(verts[0]) * verts.size() + sizeof(triangles[0]) * triangles.size()) / 1000;
}

//--------------------------------------------------------------
// Draws the mesh by iterating through its list of triangles
//  and drawing them with the stored transformations of the mesh
//  applied
void Mesh::draw()
{
	// Makes drawing of mesh in viewer transparent and filled
	ofEnableAlphaBlending();
	ofSetColor(ofColor::gray, 160);
	ofFill();

	// Integer variables to hold vertices of Triangle
	int v1, v2, v3;

	// Iterate through and draw each Triangle in Mesh
	for (Triangle t : triangles) {
		// Record indices of vertices of triangle
		v1 = t.vertInd[0];
		v2 = t.vertInd[1];
		v3 = t.vertInd[2];
		// Draw triangle using recorded vertices with
		//  mesh's stored transformations applied
		ofPushMatrix();
		ofMultMatrix(this->meshTransMatrix);
		ofDrawTriangle(verts[v1], verts[v2], verts[v3]);
		ofPopMatrix();
		
	}
	ofDisableAlphaBlending();
}

//--------------------------------------------------------------
// Provides initial setup for the cameras, scene, and image instances.
void ofApp::setup() {
	// camera setup
	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;
	// sets the mainCam's initial distance from origin in Z direction
	mainCam.setDistance(10);
	mainCam.setNearClip(.1);
	// sets previewCam to renderCam's position and aim
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(renderCam.aim);
	previewCam.setNearClip(.1);
	// sets sideCam to view Origin from X direction
	sideCam.setPosition(glm::vec3(100, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);
	// setup one point light in viewer
	//  but not rendered image
	light1.enable();
	light1.setPosition(10, 5, 0);
	light1.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	light1.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	// set plane to be used as floor
	floor = new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::darkGreen);

	// adds the floor plane to the scene
	meshScene.push_back(floor);

	// adds Light instances to lights vector
	addLight(new PointLight(glm::vec3(0, 4, 0), 100, 0.1));
	addLight(new PointLight(glm::vec3(-5, 2, 2), 100, 0.1));
	addLight(new PointLight(glm::vec3(3, 5, -2), 100, 0.1));

	// initializes the image ofImage instance to be drawn by rayTrace method
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);
	image.save("newImage.png");

	// sets up the gui slider
	gui.setup();
	gui.add(power.setup("Phong Power", 20, 0, 100));
	gui.add(intensity.setup("P-Lights Intensity", 15, 0, 100));
	gui.add(smoothMesh.setup("Smooth Shading", true, 20, 20));
}

//--------------------------------------------------------------
// Update each light's intensity and the power of phong shading
//  to values shown in gui
void ofApp::update() {

	// Sets each light's intensity value to current value in the gui
	for (int i = 0; i < lights.size(); i++) {
		lights[i]->setIntensity(intensity);
	}
	// Sets phong shading power to current value on gui
	phongPower = power;
	// Sets smooth shading boolean value of all scene objects in meshScene
	for (int i = 0; i < meshScene.size(); i++) {
		meshScene[i]->smoothShading = smoothMesh;
	}
}

//--------------------------------------------------------------
// Draws the scene objects within the Camera's perspective. 
// User can also toggle to see drawing of the completed rendering
//	with the 'P' key.
void ofApp::draw() {
	// draws the SceneObjects in the 3D view if bShowImage = false
	if (!bShowImage) {
		// show gui
		ofDisableDepthTest();
		gui.draw();
		ofEnableDepthTest();
		// 3D transformation for the camera
		theCam->begin();

		// draws axis
		drawAxis();

		// Draw all of the joint instances in joints vector
		// and the plane in meshScene vector with 
		// lighting in viewer enabled
		ofEnableLighting();
		for (int i = 0; i < joints.size(); i++) {
			if (objSelected() && joints[i] == selected[0])
				ofSetColor(ofColor::yellow);	// set color of selected joint to yellow
			else ofSetColor(joints[i]->diffuseColor);
			joints[i]->draw();
		}
		meshScene[0]->draw();
		ofDisableLighting();

		// Draw all of the meshes in the meshScene vector and the 
		// referenceMesh if it is not NULL with 
		// lighting in viewer disabled
		for (int i = 1; i < meshScene.size(); i++) {
			meshScene[i]->draw();
		}
		if (referenceMesh != NULL) {
			referenceMesh->draw();
		}

		// Draw all Lights in light vector
		for (int i = 0; i < lights.size(); i++) {
			lights[i]->draw();
		}

		// draws RenderCam and RenderCam fields if true
		if (!bHide) {
			// draws the RenderCam
			ofSetColor(ofColor::white);
			ofNoFill();
			renderCam.draw();
			// draws the ViewPlane
			renderCam.view.draw();
			// draws the Frustum
			renderCam.drawFrustum();
		}

		// end 3D transformation for the camera
		theCam->end();
	}
	else { // bShowImage = true and shows preview of the rendered ofImage prevImage
		ofSetColor(ofColor::white);
		// loads the prevImage to be displayed
		prevImage.load("newImage.png");
		// draws prevImage
		prevImage.draw(ofGetWidth() / 2 - imageWidth / 2, ofGetHeight() / 2 - imageHeight / 2);
	}
}

//--------------------------------------------------------------
// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(len, 0, 0, 1)));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, len, 1)));
}

//--------------------------------------------------------------
// Print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject *obj) {
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");" << endl;
}

//--------------------------------------------------------------
// Creates script file to store current skeleton
//
void ofApp::createFile() {
	// creates and instantiates name of new joint script file
	string newFileName = "skeleton_" + std::to_string(joints.size()) + "_joints.txt";

	// output stream for new file
	ofstream outputStream;

	// creates and opens new file to write to
	outputStream.open(newFileName);

	// iterate through all joints in the joints vector
	for (int i = 0; i < joints.size(); i++) {
		if (joints[i]->getName() != "no name") {
			// add current joint's name to line
			outputStream << "create -joint " << joints[i]->getName();
			// add current joint's rotation to line
			outputStream << " -rotate <" << joints[i]->rotation.x << ", " << joints[i]->rotation.y << ", " << joints[i]->rotation.z << ">";
			// add current joint's translation to line
			outputStream << " -translate <" << joints[i]->position.x << ", " << joints[i]->position.y << ", " << joints[i]->position.z << ">";
			// add current joint's parent to line if it exists
			if (joints[i]->parent) {
				outputStream << " -parent " << joints[i]->parent->getName();
			}
			// skip outputStream to next line
			outputStream << "\n";
		}
	}

	// detaches file from output stream
	outputStream.close();

	// message showing file was saved
	cout << "Saved current skeleton to file " + newFileName + "\n" << endl;
}

//--------------------------------------------------------------
// Parses through script file and adds joints to joints vector
//  with specified name, rotation, translation, and parent
//
void ofApp::loadScriptFile(string fileName)
{
	// removes all meshes from scene
	meshScene.erase(meshScene.begin() + 1, meshScene.begin() + meshScene.size());
	// removes all joints from the joints vector
	joints.clear();
	// clear selection vector
	selected.clear();

	ifstream inputStream;		// input stream
	string read;				// reads from input stream
	string tempString;			// temporarily stores string read from input stream
	string readName;			// holds name of joint read from file
	string parentName;			// holds name of parent read from file
	glm::vec3 readRotate;		// holds the rotation read from file
	glm::vec3 readTranslate;	// holds the translation read from file
	Joint *jointToAdd;			// holds joint to be added to joints vector
	float v1, v2, v3;			// temprorarily stores values of vertices read for rotation and translation

	// attaches file to input stream
	inputStream.open(fileName);

	if (!inputStream) {	// checks if file opening failed
		cout << "File open failed" << endl;
		exit();	// special system call to abort program
	}
	else {
		// read from input stream
		while (inputStream >> read) {
			if (read == "create") {
				// create new joint and adds it to joints vector
				jointToAdd = new Joint();
				joints.push_back(jointToAdd);
			}
			else if (read == "-joint") {
				// sets the name of the new joint
				inputStream >> readName;
				jointToAdd->name = readName;
			}
			else if (read == "-rotate") {
				// gets and sets rotation of new joint
				inputStream >> tempString;
				v1 = stof(tempString.substr(1, tempString.find(",")));
				inputStream >> tempString;
				v2 = stof(tempString.substr(0, tempString.find(",")));
				inputStream >> tempString;
				v3 = stof(tempString.substr(0, tempString.find(">")));

				readRotate = glm::vec3(v1, v2, v3);
				jointToAdd->rotation = readRotate;
			}
			else if (read == "-translate") {
				// gets and sets translation of new joint
				inputStream >> tempString;
				v1 = stof(tempString.substr(1, tempString.find(",")));
				inputStream >> tempString;
				v2 = stof(tempString.substr(0, tempString.find(",")));
				inputStream >> tempString;
				v3 = stof(tempString.substr(0, tempString.find(">")));

				readTranslate = glm::vec3(v1, v2, v3);
				jointToAdd->position = readTranslate;
			}
			else if (read == "-parent") {
				// gets and sets parent of new joint (if it exists)
				inputStream >> parentName;
				for (int i = 0; i < joints.size(); i++) {
					if (joints[i]->getName() == parentName)
						joints[i]->addChild(jointToAdd);
				}
			}
		}
	}
	// detaches file from input stream
	inputStream.close();
}

//--------------------------------------------------------------
// removes the currently selected joint from the joints vector
//
void ofApp::deleteJoint() {
	SceneObject *jointToDelete;	// holds reference to joint to delete
	SceneObject *parentJoint;	// holds reference to joint's parent
	SceneObject *currentChild;	// holds reference to current child in joint's child list
	glm::vec3 resetPosition;	// holds currentChild's position in world space
	glm::vec3 resetRotation;	// holds currentChild's rotation
	string currentJointName;	// holds the joint's name
	if (selected.size() > 0) {
		// set jointToDelete to currrently selected joint
		jointToDelete = selected[0];
		// set currentJointName to currently selected joint's name
		currentJointName = jointToDelete->getName();

		// check if the current joint has a parent and remove the joint from
		//  its parent's child list if it does
		if (jointToDelete->parent) {
			// set parentJoint variable to the joint's parent
			parentJoint = jointToDelete->parent;
			// iterate through all of the parent's children
			for (int i = 0; i < parentJoint->childList.size(); i++) {
				// find the joint to be deleted in parent's child list
				if (parentJoint->childList[i]->getName() == currentJointName) {
					// remove the joint from the child list
					parentJoint->childList.erase(parentJoint->childList.begin() + i);
				}
			}
		}

		// change the parent of each of the current joint's children to
		//  the current joint's parent or NULL if it doesn't have one
		for (int i = 0; i < jointToDelete->childList.size(); i++) {
			// set currentChild to the current child in list
			currentChild = jointToDelete->childList[i];
			// Checks if currentChild has an attatched Mesh
			for (int i = 0; i < meshScene.size(); i++) {
				if (meshScene[i]->getName() == currentChild->getMeshName()) {
					// if currentChild has an attatched mesh remove it from scene
					meshScene.erase(meshScene.begin() + i);
				}
			}
			// record currentChild's position in world space
			resetPosition = currentChild->getPosition();
			// record currentChild's rotation
			resetRotation = currentChild->rotation;
			// set parent of current child to NULL if joint to be deleted has no parent
			currentChild->parent = NULL;
			// resets matrices of current child to default
			currentChild->resetMatrices();
			// add child to childlist of current joint's parent if it has one
			if (jointToDelete->parent) {
				parentJoint->addChild(currentChild);
			}
			// reset current child's position relative to new parent
			currentChild->setPosition(resetPosition);
			// reset current child's rotation back to previous value
			currentChild->rotation = resetRotation;
		}

		// Checks if selected Joint has an attatched mesh
		for (int i = 0; i < meshScene.size(); i++) {
			if (meshScene[i]->getName() == selected[0]->getMeshName()) {
				// if selected Joint has an attatched mesh remove it from scene
				meshScene.erase(meshScene.begin() + i);
			}
		}

		// remove joint to be deleted from joints vector
		for (int i = 0; i < joints.size(); i++) {
			if (joints[i]->getName() == currentJointName) {
				joints.erase(joints.begin() + i);
			}
		}
	}

	// de-select currently selected variable
	selected.clear();
}

//--------------------------------------------------------------
// iterates through the joints vector to see if any of the joint
//  names already present match the given name
// if any matches are detected, the name is changed
//
string ofApp::getNewName(string newName) {
	for (int i = 0; i < joints.size(); i++) {
		if (joints[i]->getName() == newName) {
			jointCount++;
			return getNewName("joint" + std::to_string(jointCount));
		}
	}
	return newName;
}

//--------------------------------------------------------------
// adds a joint to the joints vector
//
void ofApp::addJoint() {
	// creates pointer to new Joint to add to joints vector
	Joint *jointToAdd = new Joint();
	// instantiates the name of the new Joint
	jointCount = 0;
	jointToAdd->name = getNewName("joint" + std::to_string(jointCount));
	// add the new Joint instance to the joints vector
	joints.push_back(jointToAdd);

	// check to see if a joint is selected
	if (objSelected()) {
		// if a joint is selected, add new joint to its children
		selected[0]->addChild(jointToAdd);
	}

	// set new joint's position relative to mouse position
	glm::vec3 newPosition;
	int x = ofGetMouseX();
	int y = ofGetMouseY();
	mouseToDragPlane(x, y, newPosition);
	jointToAdd->setPosition(newPosition);
}
//--------------------------------------------------------------
// Create a mesh using specified obj file and attatches it to
//  the selected joint
void ofApp::loadObjFile(string fileName)
{
	// Create a new mesh instance
	Mesh* mesh = new Mesh();

	ifstream inputStream;			// Input stream
	string read;					// Reads from input stream
	float posV1, posV2, posV3;		// Temporarily stores position vertices of triangles
	float normV1, normV2, normV3;	// Temporarily stores normal verticies of triangles
	int iPos1, iPos2, iPos3;		// Temporarily stores indices of the triangle's position vertices
	int iNorm1, iNorm2, iNorm3;		// Temporarily stores indices of the triangle's normal verticies
	string tempString;				// Temporarily stores string read from input stream

	// Opens the obj file
	inputStream.open(fileName);

	if (!inputStream) // Check if file opening failed
	{
		cout << "File open failed";
		exit();	// Special system call to abort program
	}
	else {
		// Read from input stream
		while (inputStream >> read) {
			if (read == "v") {			// Check for a v to denote vertex
				// Reads position vertices from input stream
				inputStream >> posV1 >> posV2 >> posV3;

				// Adds position vertices to mesh's position vertices vector
				mesh->verts.push_back(glm::vec3(posV1, posV2, posV3));
			}
			else if (read == "vn") {	// Check for a vn to denote vertex normal
				// Reads normal verticies from input stream
				inputStream >> normV1 >> normV2 >> normV3;

				// Adds normal verticies to mesh's normal verticies vector
				mesh->nVerts.push_back(glm::vec3(normV1, normV2, normV3));
			}
			else if (read == "f") {		// Check for an f to denote face
				// Reads indices of triangle normal and position vertices from input stream

				// Reads first variable set of current triangle
				inputStream >> tempString;
				// Stores index of first position vertex of current triangle
				iPos1 = stoi(tempString.substr(0, tempString.find("/"))) - 1;
				// Removes characters from string to leave only vertex normal index
				tempString.erase(0, tempString.find("/") + 1);
				tempString.erase(0, tempString.find("/") + 1);
				// Stores index of first normal vertex of current triangle
				iNorm1 = stoi(tempString) - 1;

				// Reads second variable set of current triangle
				inputStream >> tempString;
				// Stores index of second position vertex of current triangle
				iPos2 = stoi(tempString.substr(0, tempString.find("/"))) - 1;
				// Removes characters from string to leave only vertex normal index
				tempString.erase(0, tempString.find("/") + 1);
				tempString.erase(0, tempString.find("/") + 1);
				// Stores index of second normal vertex of current triangle
				iNorm2 = stoi(tempString) - 1;

				// Reads third variable set of current triangle
				inputStream >> tempString;
				// Stores index of third position vertex of current triangle
				iPos3 = stoi(tempString.substr(0, tempString.find("/"))) - 1;
				// Removes characters from string to leave only vertex normal index
				tempString.erase(0, tempString.find("/") + 1);
				tempString.erase(0, tempString.find("/") + 1);
				// Stores index of third normal vertex of current triangle
				iNorm3 = stoi(tempString) - 1;

				// Adds indices of triangle's vertices to mesh's triangle vector
				mesh->triangles.push_back(Triangle(iPos1, iPos2, iPos3, iNorm1, iNorm2, iNorm3));
			}
		}
	}
	// Close file
	inputStream.close();

	// Print mesh diagnostic information
	cout << "Number of Vertices: " << mesh->verts.size() << endl;
	cout << "Total Number of Faces: " << mesh->triangles.size() << endl;
	cout << "Size of Mesh (in kB): " << mesh->getMeshSize() << "\n" << endl;

	// Iterate through all vertices of mesh to determine greatest and lowest y value
	for (int i = 0; i < mesh->verts.size(); i++) {
		if (mesh->verts[i].y < mesh->minYVal) {
			mesh->minYVal = mesh->verts[i].y;
		}
		if (mesh->verts[i].y > mesh->maxYVal) {
			mesh->maxYVal = mesh->verts[i].y;
		}
	}

	// assigns a name to the mesh
	numMeshes++;
	mesh->name = "mesh" + std::to_string(numMeshes);

	
	if (objSelected()) { // Attatches Mesh to selected joint
		// Checks if selected joint is a root
		if (selected[0]->parent == NULL) {
			// do not add mesh to scene if selected joint is a root
			cout << "The joint you selected is a root joint and a mesh cannot be attatched to it.\n" << endl;
		}
		else {
			// Checks if selected Joint already has a mesh
			for (int i = 0; i < meshScene.size(); i++) {
				if (meshScene[i]->getName() == selected[0]->getMeshName()) {
					// if selected Joint already has a mesh, delete it from scene
					meshScene.erase(meshScene.begin() + i);
				}
			}
			// set selected joint's attatchedMesh to new mesh
			selected[0]->attatchMesh(mesh);
			// add new mesh to scene
			meshScene.push_back(mesh);
		}
	}
	else { // Sets new mesh to be reference mesh if no joint is selected
		referenceMesh = mesh;
	}
}

//--------------------------------------------------------------
// Provides implementation for Keys to switch between camera
// perspectives, display different outputs in drawing method,
// and call the rayTrace method.
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'C':
	case 'c':			// enables camera movement
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'D':
	case 'd':			// deletes the reference mesh
		referenceMesh = NULL;
		break;
	case 'I':
	case 'i':			// get info on currently selected joint
		if (objSelected()) {
			// print out name of selected joint
			cout << selected[0]->getName() << ":" << endl;
			// print out all matrices of the selected joint
			printChannels(selected[0]);
			// print out position in world space of selected joint
			cout << "Selected joint's position in world space: " << selected[0]->getPosition() << endl;
			// print all children of selected joint's child list
			if (selected[0]->childList.size() > 0) {
				int count = 1;									// tracks current child
				string result = "Selected joint's children: ";	// string to contain all children
				for (int i = 0; i < selected[0]->childList.size(); i++) {
					result += "child" + std::to_string(count) + " = " +
						selected[0]->childList[i]->getName() + ", ";
					count++;
				}
				cout << result << endl;
			}
			// print parent of selected joint if it has one
			if (selected[0]->parent)
				cout << "Selected joint's parent: " + selected[0]->parent->getName() << endl;
			// skip next line
			cout << endl;
		}
		break;
	case 'J':
	case 'j':			// adds new joint to scene
		addJoint();
		break;
	case OF_KEY_DEL:	// deletes currently selected joint from scene
		deleteJoint();
		break;
	case 'S':
	case 's':			// creates script file containing current skeleton's joints
		createFile();
		break;
	case 'R':
	case 'r':			// calls the rayTrace method
		cout << "rendering..." << endl;
		rayTrace();
		cout << "done" << endl;
		break;
	case 'X':
	case 'x':			// enables rotation around x axis
		bRotateX = true;
		break;
	case 'Y':
	case 'y':			// enables rotation around y axis
		bRotateY = true;
		break;
	case 'Z':
	case 'z':			// enables rotation around z axis
		bRotateZ = true;
		break;
	case OF_KEY_F1:		// switches POV to mainCam
		theCam = &mainCam;
		break;
	case OF_KEY_F2:		// switches POV to sideCam
		theCam = &sideCam;
		break;
	case OF_KEY_F3:		// switches POV to previewCam
		theCam = &previewCam;
		break;
	case OF_KEY_ALT:
		bAltKeyDown = true;
		if (!mainCam.getMouseInputEnabled()) mainCam.enableMouseInput();
		break;
	case OF_KEY_UP:
		if (objSelected()) {	// increments position of mesh towards selected joint 0.1 in y direction
			selected[0]->yOffset += 0.1;
		}
		break;
	case OF_KEY_DOWN:
		if (objSelected()) {	// decrements position of mesh towards selected joint 0.1 in y direction
			selected[0]->yOffset -= 0.1;
		}
		break;
	case 'P':
	case 'p':			// toggles drawing of prevImage
		bShowImage = !bShowImage;
		break;
	case 'V':
	case 'v':			// toggles drawing of the RenderCam, ViewPlane, and Frustom
		bHide = !bHide;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case 'X':
	case 'x':	// disables rotation around x axis
		bRotateX = false;
		printCurrentObjRot();
		break;
	case 'Y':
	case 'y':	// disables rotation around y axis
		bRotateY = false;
		printCurrentObjRot();
		break;
	case 'Z':
	case 'z':	// disables rotation around z axis
		bRotateZ = false;
		printCurrentObjRot();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	// translates or rotates currently selected object
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}
}

//--------------------------------------------------------------
//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {
	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	// test if something selected
	//
	vector<Joint *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of joints
	//
	for (int i = 0; i < joints.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (joints[i]->isSelectable && joints[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(joints[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	Joint *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

void ofApp::printCurrentObjRot()
{
	if (selected.size() > 0) {
		cout << "Selected object's rotation: X = " + to_string(selected[0]->rotation.x)
			+ ", Y = " + to_string(selected[0]->rotation.y)
			+ ", Z = " + to_string(selected[0]->rotation.z) + "\n" << endl;
	}
}

//--------------------------------------------------------------
// Reads in files dragged into window
void ofApp::dragEvent(ofDragInfo dragInfo) {
	// Records name of file
	string fileName = dragInfo.files[0];
	// Records file type
	string fileType = fileName.substr(fileName.length() - 3, fileName.length() - 1);
	// Check which type of file was dragged in
	if (fileType == "obj") {
		// loads an obj file to create a mesh
		loadObjFile(dragInfo.files[0]);
	}
	else if (fileType == "txt"){
		// load a script file containing list of joints and reintialize 
		//  the joint vector with this new list of joints
		loadScriptFile(dragInfo.files[0]);
	}
	else {
		// no of the specified files were added
		cout << "Invalid File Type\n" << endl;
	}
}

//--------------------------------------------------------------
// Iterates through the dimensions of the image ofImage instance 
// given by imageHeight and imageWidth and draws a color at each
// pixel given by the closest SceneObject viewed by the RenderCam
// at that position
void ofApp::rayTrace()
{
	Ray ray;						// holds the current ray set by the current pixel in the iteration
	bool hit;						// determines if the ray hit a SceneObject
	float shortestDistance;			// holds the distance from the ray start position to the closest SceneObject
	float currentDistance;			// holds the distance between the ray start position at current pixel in iteration and the current SceneObject
	SceneObject *closestObject;		// refers to the object that is closest to the RenderCam where a hit occurred
	ofColor color;					// holds color of closest object after phong shading has been applied
	ofColor objColor;				// holds color of closest object before any shading has been applied

	// for each pixel in the image
	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {
			// get current pixel in u and v coordinates
			float u = (i + 0.5) / imageWidth;
			float v = (j + 0.5) / imageHeight;
			// get the current ray from renderCam to point(u, v)
			ray = renderCam.getRay(u, v);
			// hit is initially set to false
			hit = false;
			// distance is initially infinity since no object has been tested for intersection yet
			shortestDistance = std::numeric_limits<float>::infinity();
			// set closest object equal to NULL to begin since no object has been tested for intersection yet
			closestObject = NULL;
			// for each object in the scene
			for (int k = 0; k < meshScene.size(); k++) {
				// tests the current SceneObject for intersection with Ray from current pixel in iteration
				if (meshScene[k]->intersect(ray, intersectPt, intersectNormal)) {
					// gets the distance of the Ray at current pixel to Intersection Point with current SceneObject
					currentDistance = glm::distance(ray.p, intersectPt);
					// checks if the currentDistance is shortest
					if (currentDistance < shortestDistance) {
						// sets currentDistance to be shortest and sets closestObject to current SceneObject
						shortestDistance = currentDistance;
						closestObject = meshScene[k];
					}
					// sets hit to true since Ray intersected with a SceneObject
					hit = true;
				}
				if (hit) {	// if a hit occurred color current pixel with currentObject's color and shade it according to light placement
					// reset intersectPt and intersectNormal to closestObject
					closestObject->intersect(ray, intersectPt, intersectNormal);

					// assign color of closest object to objColor (use texture for plane if applied)
					objColor = closestObject->getColor(intersectPt);

					// Shades the current pixel with ambient and lambert shading
					//color = lambert(ray, intersectPt, intersectNormal, closestObject->diffuseColor);
					// Shades the current pixel with ambient, lambert and phong shading
					color = phong(ray, intersectPt, intersectNormal, objColor, ofColor::white, phongPower);

					// colors the current pixel in iteration
					image.setColor(i, imageHeight - 1 - j, color);
				}
				else {		// if hit did not occur color current pixel with background color
					image.setColor(i, imageHeight - 1 - j, ofGetBackgroundColor());
				}
			}
		}
	}
	// save changes to the image
	image.save("newImage.png");
}

//--------------------------------------------------------------
// Adds lambert shading to given pixel in the scene
ofColor ofApp::lambert(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse)
{
	// Sets ambient shading
	ofColor result = 0.25 * diffuse;			// ambient shading value to not make image completely dark
	// Variables used in checking for shadows
	glm::vec3 blockIntersectPt;					// point where ray to light from given point intersects another surface
	glm::vec3 blockIntersectNormal;				// normal where ray to light from point intersects another surface
	glm::vec3 shadowRayPt;						// point where light intersects (+ small value towards normal)
	Ray shadingRay;								// ray from	shadowRayPt to light origin
	bool blocked;								// dictates whether point is blocked from current light
	// Variables used in calculating the light
	glm::vec3 directionToCam;					// vector from point to camera
	glm::vec3 directionToLight;					// vector from point to light
	glm::vec3 norm = glm::normalize(normal);	// normal at point
	float illumination;							// light intensity/(distance to light)^2
	float dotProdNormLight;						// dot product of norm vector and directionToLight vector


	// iterates through all lights
	for (int i = 0; i < lights.size(); i++) {
		// Sets direction of ray pointing to camera from intersection point on SceneObject
		directionToCam = -glm::normalize(ray.d);
		// Sets direction of ray pointing to light from intersection point on SceneObject
		directionToLight = glm::normalize(lights[i]->position - point);

		// Determines point near surface where shadingRay begins
		shadowRayPt = point + 0.0001*norm;
		// Initializes ray fired from shadowRayPt
		shadingRay = Ray(shadowRayPt, directionToLight);
		// Checks for shadows and sets blocked to true if point is blocked from light
		blocked = shadowCheck(shadingRay, blockIntersectPt, blockIntersectNormal, lights[i]->position);

		// Only adds lambert shading to result if point is not blocked from current light
		if (blocked == false) {
			// Gets the illumination from source
			illumination = lights[i]->intensity / pow(glm::distance(lights[i]->position, point), 2);
			// Gets dot product of normal and directionToLight vectors
			dotProdNormLight = glm::dot(norm, directionToLight);
			// Adds lambert shaded color to result
			result = result + diffuse * illumination * glm::max(0.0f, dotProdNormLight);
		}
	}
	return result;
}

//--------------------------------------------------------------
// Adds phong shading to given pixel in the scene
ofColor ofApp::phong(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power)
{
	// Sets ambient shading
	ofColor result = 0.15 * (diffuse);			// ambient shading value to not make image completely dark
	// Variables used in checking for shadows
	glm::vec3 blockIntersectPt;					// point where ray to light from given point intersects another surface
	glm::vec3 blockIntersectNormal;				// normal where ray to light from point intersects another surface
	glm::vec3 shadowRayPt;						// point where light intersects (+ small value towards normal)
	Ray shadingRay;								// ray from	shadowRayPt to light origin
	bool blocked;								// dictates whether point is blocked from current light
	// Variables used in calculating the diffuse and phong shading
	glm::vec3 directionToCam;					// vector from point to camera
	glm::vec3 directionToLight;					// vector from point to light
	glm::vec3 norm = glm::normalize(normal);	// normal at point
	float illumination;							// light intensity/(distance to light)^2
	float dotProdNormLight;						// dot product of norm vector and directionToLight vector
	glm::vec3 bisectingVec;						// bisecting vector between directionToLight and directionToCam vectors
	float dotProdNormBis;						// dot product of norm vector and bisectingVec vector


	// iterates through all lights
	for (int i = 0; i < lights.size(); i++) {
		// Sets direction of ray pointing to camera from intersection point on SceneObject
		directionToCam = glm::normalize(renderCam.position - point);
		// Sets direction of ray pointing to light from intersection point on SceneObject
		directionToLight = glm::normalize(lights[i]->position - point);

		// Determines point near surface where shadingRay begins
		shadowRayPt = point + 0.0001*norm;
		// Initializes ray fired from shadowPt
		shadingRay = Ray(shadowRayPt, directionToLight);
		// Checks for shadows and sets blocked to true if point is blocked from light
		blocked = shadowCheck(shadingRay, blockIntersectPt, blockIntersectNormal, lights[i]->position);

		// Only adds lambert and phong shading to result if point is not blocked from current light
		if (blocked == false) {
			// Gets the illumination from source
			illumination = lights[i]->intensity / pow(glm::distance(lights[i]->position, point), 2);
			// Gets dot product of normal and directionToLight vectors
			dotProdNormLight = glm::dot(norm, directionToLight);
			// Calculate and add diffuse shading to result
			result += diffuse * illumination * glm::max(0.0f, dotProdNormLight);
			// Obtains the bisecting vector between vector to cam and vector to light
			bisectingVec = glm::normalize(directionToCam + directionToLight);
			// Dot product of bisecting vector and normal
			dotProdNormBis = glm::dot(norm, bisectingVec);
			// Adds phong shaded color to result
			result += specular * illumination * pow(glm::max(0.0f, dotProdNormBis), power);
		}
	}
	return result;
}

//--------------------------------------------------------------
// Checks for intersection between lights and other objects in scene
bool ofApp::shadowCheck(Ray ray, glm::vec3 intersection, glm::vec3 normal, glm::vec3 lightPosition) {
	for (int k = 0; k < meshScene.size(); k++) {
		// only return true if an intersection occurs with a surface before ray reaches the light
		if (meshScene[k]->intersect(ray, intersection, normal) && (glm::distance(ray.p, intersection) < glm::distance(ray.p, lightPosition))
			&& (glm::distance(intersection, lightPosition) < glm::distance(ray.p, lightPosition))) {
			return true;
		}
	}
	return false;
}
