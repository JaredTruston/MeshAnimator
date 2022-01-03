// This file provides definitions for Light, PointLight, Triangle,
//  Joint, and Mesh classes in addition to declarations of variables
//  and methods utilized in the ofApp class.
// - author: Jared Bechthold 
// - starter files provided by Professor Kevin Smith

#include "ofMain.h"
#include "ofxGui.h"
#include "SceneObjects.h"
#include <glm/gtx/intersect.hpp>

// Base Light class
//
class Light : public SceneObject {
public:
	// Sets intensity of light
	void setIntensity(float newIntensity) {
		intensity = newIntensity;
	}

	// Tracks light intensity
	float intensity;
};

// Point Light class
//
class PointLight : public Light {
public:
	// PointLight constructor
	PointLight(glm::vec3 position, float intensity, float radius, ofColor diffuse = ofColor::white) {
		this->position = position;
		this->intensity = intensity;
		this->radius = radius;
		this->diffuseColor = diffuse;
	}

	// Draws the Sphere representing the light
	void draw() {
		ofNoFill();
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
	}

	// Tracks size of drawble light
	float radius;
};

// Triangle class
//
class Triangle {
public:
	// Defined constructor for the Triangle class
	Triangle(int i1, int i2, int i3, int in1, int in2, int in3) {
		// adds indices of Triangle's position verticies to vector
		vertInd[0] = i1;
		vertInd[1] = i2;
		vertInd[2] = i3;

		// adds indices of Triangle's normal verticies to vector
		nVertInd[0] = in1;
		nVertInd[1] = in2;
		nVertInd[2] = in3;
	}

	// Default constructor of the Triangel class
	Triangle() { }

	// Fields of the Triangle class
	int vertInd[3];		// holds the three position vertices of triangle
	int nVertInd[3];	// holds the three normal verticies of triangel
};

//  Mesh class
//  
class Mesh : public SceneObject {
public:
	// Mesh Default Constructor
	Mesh() {}

	// Methods of Mesh class
	
	// Detects intersection between mesh and ray
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		// distance between closest intersection point and camera
		float shortestDistance = std::numeric_limits<float>::infinity();
		float currentDistance = 0;	// distance between current intersection point and camera	
		glm::vec3 v0, v1, v2;		// current position vertices of current triangle
		glm::vec3 nV0, nV1, nV2;	// current normal verticies of current triangle 
		bool hit = false;			// tracks whether ray hits the mesh
		glm::vec2 baryCenter;		// position of intersect point on triangle in barycentric coordinates
		glm::vec3 v0v1;				// vector from triangle's vertex0 to vertex1
		glm::vec3 v0v2;				// vector from triangle's vertex0 to vertex2
		Ray r = ray;				// ray passed into the method

		// iterate through all of the triangles of the mesh
		for (int i = 0; i < triangles.size(); i++) {
			// initializes transformed position vertices of current triangle
			v0 = meshTransMatrix * glm::vec4(verts[triangles[i].vertInd[0]], 1);
			v1 = meshTransMatrix * glm::vec4(verts[triangles[i].vertInd[1]], 1);
			v2 = meshTransMatrix * glm::vec4(verts[triangles[i].vertInd[2]], 1);

			// initializes transformed normal verticies of current triangle
			nV0 = glm::normalize(meshTransMatrix * glm::vec4(nVerts[triangles[i].nVertInd[0]], 1) 
				- meshTransMatrix * glm::vec4(0, 0, 0, 1));
			nV1 = glm::normalize(meshTransMatrix * glm::vec4(nVerts[triangles[i].nVertInd[1]], 1)
				- meshTransMatrix * glm::vec4(0, 0, 0, 1));
			nV2 = glm::normalize(meshTransMatrix * glm::vec4(nVerts[triangles[i].nVertInd[2]], 1)
				- meshTransMatrix * glm::vec4(0, 0, 0, 1));

			// check for intersection
			if (glm::intersectRayTriangle(ray.p, ray.d, v0, v1, v2, baryCenter, currentDistance)) {
				hit = true;	// set hit to true if intersection occurs

				// record fields of closest triangle
				if (currentDistance < shortestDistance) {
					shortestDistance = currentDistance;
					point = r.evalPoint(shortestDistance);
					if (smoothShading) {
						// calculates the average point normal using barycentric coordinates
						normal = glm::normalize((1 - baryCenter.x - baryCenter.y)*nV0
							+ baryCenter.x * nV1 + baryCenter.y * nV2);
					}
					else {
						// calculate vectors from vertex0 to both vertex1 and vertex2
						v0v1 = glm::normalize(v1 - v0);
						v0v2 = glm::normalize(v2 - v0);
						// calculates the surface normal using cross product of triangle's vectors
						normal = glm::cross(v0v1, v0v2);
					}
				}
			}
		}
		// return result
		return hit;
	}

	// Returns name of the mesh
	string getName() { return name; }

	int getMeshSize();											// returns size of mesh in KB
	void draw();												// draws all the triangles of the mesh
	float getVerticalDistance() { return maxYVal - minYVal; }	// returns height of mesh

	// Fields of Mesh class
	//
	vector<glm::vec3> verts;									// holds all position vertices of the mesh
	vector<glm::vec3> nVerts;									// holds all normal verticies of mesh
	vector<Triangle> triangles;									// holds all triangles of the mesh
	float maxYVal = -std::numeric_limits<float>::infinity();	// holds a vector with the maximum value in the y axis
	float minYVal = std::numeric_limits<float>::infinity();		// holds a vector with the minimum value in the y axis
	glm::mat4 meshTransMatrix;									// contains transformation matrix to be stored for mesh

};

// Joint class
//
class Joint : public Sphere {
public:
	// Defined constructor of Joint class
	Joint(string n, glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) {
		position = p;
		radius = r;
		diffuseColor = diffuse;
		name = n;
	}

	// Defined constructor of Joint class that only defines name
	Joint(string n) {
		name = n;
	}

	// Default constructor of Joint class
	Joint() {
		radius = 0.1;
		diffuseColor = ofColor::red;	// set joint's default color to red
	}

	// Assigns attatchedMesh to Joint
	void attatchMesh(Mesh* mesh) {
		attatchedMesh = mesh;
		hasMesh = true;
	}

	// Returns name of attatchedMesh
	string getMeshName()
	{ 
		if (hasMesh) {
			return attatchedMesh->getName();
		}
		else {
			return "no mesh";
		}
	}

	// Returns name of the joint
	string getName() { return name; }

	// Draws joint and bone connecting it to its parent (if joint has a parent)
	void draw() {
		// Calls the super class version of the draw method in order to draw
		//  the sphere
		Sphere::draw();

		// Draw bone from current joint to parent if current Joint has a parent
		if (parent) {
			// vector pointing from the current joint to the parent
			glm::vec3 jointToParent = glm::normalize(parent->getPosition() - this->getPosition());
			// vector representing default direction of cone
			glm::vec3 coneDir = glm::vec3(0, 1, 0);
			
			// rotation matrix to be applied to the cone
			glm::mat4 rotate;
			// sets the rotation matrix to be applied to the cone
			if (jointToParent.x == 0 && jointToParent.z == 0 && jointToParent.y <= -0.999) {
				// sets rotate matrix to parent's rotation (plus 180 in z axis) if jointToParent vector is parallel
				//  and opposite of the cone's default direction
				rotate = glm::eulerAngleYXZ(glm::radians(parent->rotation.y), glm::radians(parent->rotation.x),
					glm::radians(parent->rotation.z + 180.0f));
			}
			else if (jointToParent.x == 0 && jointToParent.z == 0 && jointToParent.y >= 0.999) {
				// sets rotate matrix to parent's rotation if jointToParent vector is parallel to the cone's default direction
				rotate = glm::eulerAngleYXZ(glm::radians(parent->rotation.y), glm::radians(parent->rotation.x),
					glm::radians(parent->rotation.z));
			}
			else {
				// sets rotation matrix to align with jointToParent vector
				rotate = rotateToVector(coneDir, jointToParent);
			}

			// vector pointing from the parent to the current Joint
			glm::vec3 parentToJoint = glm::normalize(this->getPosition() - parent->getPosition());
			// distance between the current joint and its parent
			float distance = glm::distance(this->getPosition(), parent->getPosition());
			// specifies position to translate cone to
			glm::vec3 transPos = parent->getPosition() + distance / 2 * parentToJoint;
			// translation matrix: sends cone to halfway between current joint and parent
			glm::mat4 translate = glm::translate(glm::mat4(1.0), transPos);
			// fields used to instantiate cone
			float coneHeight = distance - (this->getRadius() + parent->getRadius());
			float coneRadius = 0.05;
			// draw cone with transformations applied
			ofPushMatrix();
			ofMultMatrix(translate * rotate);
			ofSetColor(ofColor::blue);
			ofDrawCone(coneRadius, coneHeight);
			ofPopMatrix();

			// Checks if the current joint has a mesh attatched to it and if it does draw that mesh
			//  over the cone representing the bone
			if (hasMesh) {
				// Specifies default direction that the mesh faces
				glm::vec3 meshDir = glm::vec3(0, -1, 0);
				
				// Rotation matrix to be applied to mesh
				glm::mat4 meshRotate;
				// sets the rotation matrix to be applied to the mesh
				if (jointToParent.x == 0 && jointToParent.z == 0 && jointToParent.y <= -0.999) {
					// sets meshRotate matrix to parent's rotation if jointToParent vector is parallel to the mesh's default direction
					meshRotate = glm::eulerAngleYXZ(glm::radians(parent->rotation.y), glm::radians(parent->rotation.x),
						glm::radians(parent->rotation.z));
				}
				else if (jointToParent.x == 0 && jointToParent.z == 0 && jointToParent.y >= 0.999) {
					// sets meshRotate matrix to parent's rotation (plus 180 in z-axis) if jointToParent vector is parallel 
					// and opposite of the mesh's default direction
					meshRotate = glm::eulerAngleYXZ(glm::radians(parent->rotation.y), glm::radians(parent->rotation.x),
						glm::radians(parent->rotation.z + 180.0f));
				}
				else {
					// sets meshRotate matrix to align with jointToParent vector
					meshRotate = rotateToVector(meshDir, jointToParent);
				}

				// Increments/Decrements position of attatchedMesh by yOffset in y direction
				attatchedMesh->position.y = yOffset;
				// Stores new transformation matrix of mesh
				attatchedMesh->meshTransMatrix = translate * meshRotate * attatchedMesh->getMatrix();
			}
		}
	}
	// defines offset in y direction to change attatched mesh
	float yOffset = 0.0;
	// defines default name of a Joint instance
	string name = "default";
	// tells whether current Joint has a mesh attatched or not
	bool hasMesh = false;
	// holds mesh to be attached to current Joint
	Mesh* attatchedMesh;
};

class ofApp : public ofBaseApp {

public:
	// default openframeworks methods
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	// Scene Object Related Methods
	//
	bool mouseToDragPlane(int x, int y, glm::vec3 &point);				// projects a mouse point in screen space to a 3D point on a plane normal to view axis of camera
	static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);// draws 1 unit length lines at origin in postive direction of each axis
	void printChannels(SceneObject *);									// prints out specified scene object's position, rotation, and scale fields
	bool objSelected() { return (selected.size() ? true : false); };	// returns boolean value detailing if a scene object is currently selected
	void printCurrentObjRot();											// prints current local rotation field of object

	// Joint Related Methods
	//
	void addJoint();							// adds a joint to the scene
	void loadObjFile(string fileName);			// loads mesh obj file into scene
	string getNewName(string newName);			// selects name for joint to be added
	void deleteJoint();							// deletes selected joint
	void createFile();							// creates script file for current set of joints
	void loadScriptFile(string fileName);		// loads specified script file

	// Ray Tracing and Lighting Related Methods
	//
	// adds phong shading to given pixel in scene
	ofColor phong(Ray ray, const glm::vec3 & point, const glm::vec3 & normal, const ofColor diffuse, const ofColor specular, float power);
	// adds lambert shading to given pixel in scene
	ofColor lambert(Ray ray, const glm::vec3 &point, const glm::vec3 &normal, const ofColor diffuse);
	// adds Light instances to lights vector
	void addLight(PointLight* newLight) { lights.push_back(newLight); }
	// checks ray fired from object to light for intersction with other SceneObjects
	bool shadowCheck(Ray ray, glm::vec3 intersection, glm::vec3 normal, glm::vec3 lightPosition);
	// draws RenderCam view to ofImage instance
	void rayTrace();

	// Camera and View Related Fields
	//
	// toggles drawing of RenderCam, ViewPlane, and Frustom on and off
	bool bHide = true;
	// toggles preview of rendered image on and off
	// only shows image if render has been called
	bool bShowImage = false;
	// camera that can move about the scene starting from RenderCam POV
	ofEasyCam  mainCam;
	// shows view of the scene from the side
	ofCamera sideCam;
	// provides preview of what RenderCam sees
	ofCamera previewCam;
	// set to current camera either mainCam, sideCam, or previewCam
	ofCamera  *theCam;
	// light in viewer (but no rendered image)
	ofLight light1;

	// Scene and RayTracing Related Fields
	//
	// set up one render camera to render image
	RenderCam renderCam;
	// image to write to in order to save on to disk
	ofImage image;
	// second image to preview
	ofImage prevImage;
	// holds image to map to plane
	ofImage planeTexture;
	// floor of scene
	Plane* floor;
	// backWall of scene
	Plane* backWall;
	// holds the scene object to be rendered by ray tracer
	vector<SceneObject *> meshScene;
	// to add light objects to the scene
	vector<Light *> lights;
	// dimensions of the image to be rendered
	int imageWidth = 1200;
	int imageHeight = 800;
	// dimensions of the textureImage
	int textureWidth = 1000;
	int textureHeight = 1000;
	// variables to be passed into the intersect method calls in raytrace method
	glm::vec3 intersectPt;
	glm::vec3 intersectNormal;
	// power of phong shading
	float phongPower;
	// GUI slider
	ofxFloatSlider power;
	ofxFloatSlider intensity;
	ofxToggle smoothMesh;
	ofxPanel gui;
	// states
	bool bDrag = false;
	bool bAltKeyDown = false;
	bool bRotateX = false;
	bool bRotateY = false;
	bool bRotateZ = false;
	glm::vec3 lastPoint;

	// Joint Related Fields
	//
	// tracks number of joints added to the scene
	int jointCount = 0;	
	// holds currently selected Joint
	vector<Joint *> selected;
	// holds joints to be drawn in viewer
	vector<Joint *> joints;

	// Mesh Related Fields
	//
	// holds mesh that is unattatched to a joint to be used
	//  as a reference for creating a skeleton (not drawn
	//  by raytracer)
	Mesh* referenceMesh;
	// tracks the number of meshes added to the scene
	int numMeshes = 0;

};
