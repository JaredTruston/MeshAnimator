// This file provides definitions for the Ray, SceneObject, Sphere,
// Plane, ViewPlane, and RenderCam classes.
// - author: Jared Bechthold 
// - starter files provided by Professor Kevin Smith

#pragma once

#include "ofMain.h"
#include "glm/gtx/euler_angles.hpp"
#include <glm/gtx/intersect.hpp>

//  General Purpose Ray class 
//
class Ray {
public:
	// ray constrcutor
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }

	// default ray constructor
	Ray() { this->p = glm::vec3(0, 0, 0); this->d = glm::vec3(0, 0, 0); }

	// draws the ray
	void draw(float t) { ofDrawLine(p, p + t * d); }

	// returns value along the ray for any value of t
	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	// two quantities represent the array
	//	position where the ray starts in space
	//	direction where the ray is fired
	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//	(AKA SurfaceObject)
class SceneObject {
public:
	// every SceneObject has draw() and intersect() methods to be overloaded
	// pure virtual funcs - must be overloaded
	// draws the scene object
	virtual void draw() = 0;
	// determines is ray intersects with scene object (to be overriden)
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	// returns the color of the scene object
	virtual ofColor getColor(glm::vec3 intersectPt) { return diffuseColor; }
	// method to be overridden in the Joint/Mesh class to return the joint's/mesh's name
	virtual string getName() { return "no name"; }
	// method to be overridden in the Sphere class to return the sphere's radius
	virtual float getRadius() { return 0.0; }
	// method to be overridden in the Join class to return attatched Mesh's name
	virtual string getMeshName() { return "no mesh"; }

	// commonly used transformations
	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}
	glm::mat4 getScaleMatrix() {
		return (glm::scale(glm::mat4(1.0), glm::vec3(scale.x, scale.y, scale.z)));
	}

	// returns the matrix transformations exclusive to current scene object 
	glm::mat4 getLocalMatrix() {
		// get the local transformations + pivot
		//
		glm::mat4 scale = getScaleMatrix();
		glm::mat4 rotate = getRotateMatrix();
		glm::mat4 trans = getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-pivot.x, -pivot.y, -pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(pivot.x, pivot.y, pivot.z));

		return (trans * post * rotate * pre * scale);
	}

	// returns complete set of current scene object's matrix transformations
	// concatenated with parent's transformations
	glm::mat4 getMatrix() {

		// if we have a parent (we are not the root),
		// concatenate parent's transform (this is recursive)
		// 
		if (parent) {
			glm::mat4 M = parent->getMatrix();
			return (M * getLocalMatrix());
		}
		else return getLocalMatrix();  // priority order is SRT
	}

	// get current Position in World Space
	//
	glm::vec3 getPosition() {
		return (getMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}

	// set position (pos is in world space)
	//
	void setPosition(glm::vec3 pos) {
		position = glm::inverse(getMatrix()) * glm::vec4(pos, 1.0);
	}

	// resets SceneObjects position/orientation matrices back to default
	//
	void resetMatrices() {
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);
	}

	// return a rotation  matrix that rotates one vector to another
	//
	glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2);

	//  Hierarchy 
	//
	void addChild(SceneObject *child) {
		childList.push_back(child);
		child->parent = this;
	}

	// hierachy implementation: each scene object (that is not a root)
	// has a parent whose transformations are also applied to it
	SceneObject *parent = NULL;        // if parent = NULL, then this obj is the ROOT
	vector<SceneObject *> childList;

	// position/orientation of scene object
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale

	// rotate pivot
	//
	glm::vec3 pivot = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	// UI parameters
	//
	bool isSelectable = true;

	// determines if a mesh scene object uses smooth shading
	bool smoothShading = true;

	// default name of a scene object
	string name = "SceneObject";
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	// Sphere constructor that sets the position, raidus, and color of the Sphere
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	// Default Sphere constructor
	Sphere() {}

	// tests for intersection of the Sphere (at position in World Space) with a Ray
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, this->getPosition(), radius, point, normal));
	}

	// draws the Sphere
	void draw() {
		// get the current transformation matrix of the sphere
		glm::mat4 m = getMatrix();

		// set sphere to be filled
		ofFill();

		// push the current stack matrix and multiply this object's
		// matrix. now all the vertices will be transformed by this
		// matrix
		//
		ofPushMatrix();
		ofMultMatrix(m);
		ofDrawSphere(radius);
		ofPopMatrix();
	}

	// returns the radius of the sphere
	float getRadius() { return radius; }

	// radius of the Sphere
	float radius = 1.0;
};

//  General purpose plane 
//
class Plane : public SceneObject {
public:
	// Plane constructor that sets point, normal, color, height, and width
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		isSelectable = false;
		plane.rotateDeg(-90, 1, 0, 0);
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
	}
	// default Plane constructor
	Plane() {
		plane.rotateDeg(-90, 1, 0, 0);
		isSelectable = false;
	}

	// applies texture image to the plane
	void applyTexture(ofImage textureToApply) {
		textureImg = textureToApply;
		textureApplied = true;
	}

	// sets amount of tiles in x and y direction for texture mapping
	void setTiles(int x, int y) {
		tilesX = x;
		tilesY = y;
	}

	// overrdes getColor to handle textureMapping
	ofColor getColor(glm::vec3 intersectPt) {
		// check if texture is applied and plane is orthogonal to positive y axis
		if (textureApplied && normal == glm::vec3(0, 1, 0)) {
			// get minimum point on plane
			glm::vec3 minimum = glm::vec3(position.x - width / 2, position.y, position.z - height / 2);
			// get x and y coordinates of current point on plane in plane's own 2d coordinate system with max height/width
			//  equal to the number of tiles in y and x direction respectively
			float nX = ((intersectPt.x - minimum.x) / width) * tilesX;
			float nY = ((intersectPt.z - minimum.z) / height) * tilesY;
			// get pixel coordinates of textureImg for current nX and nY point
			float i = nX * textureImg.getWidth() - 0.5;
			float j = nY * textureImg.getHeight() - 0.5;
			// get color of current point in plane (apply modulus for repeating pattern)
			return textureImg.getColor(fmod(i, textureImg.getWidth()), fmod(j, textureImg.getHeight()));
		}
		else if (textureApplied && normal == glm::vec3(0, 0, 1)) {
			// get minimum point on plane
			glm::vec3 minimum = glm::vec3(position.x - width / 2, position.y - height / 2, position.z);
			// get x and y coordinates of current point on plane in plane's own 2d coordinate system with max height/width
			//  equal to the number of tiles in y and x direction respectively
			float nX = ((intersectPt.x - minimum.x) / width) * tilesX;
			float nY = ((intersectPt.y - minimum.y) / height) * tilesY;
			// get pixel coordinates of textureImg for current nX and nY point
			float i = nX * textureImg.getWidth() - 0.5;
			float j = nY * textureImg.getHeight() - 0.5;
			// get color of current point in plane (apply modulus for repeating pattern)
			return textureImg.getColor(fmod(i, textureImg.getWidth()), fmod(j, textureImg.getHeight()));
		}
		else {	// return assigned diffuse color if no texture is applied
			return diffuseColor;
		}
	}

	// tests for intersection of Plane with a Ray
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	// returns the Plane's normal
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	// draws the Plane
	void draw() {
		ofSetColor(diffuseColor);
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		material.begin();
		material.setDiffuseColor(diffuseColor);
		plane.drawFaces();
		material.end();
	}

	// used for drawing the Plane
	ofPlanePrimitive plane;
	ofMaterial material;
	// holds vector normal to the Plane
	glm::vec3 normal = glm::vec3(0, 1, 0);
	// dimensions of the Plane
	float width = 20;
	float height = 20;
	// detects if a texture has been applied to the plane (initialized false)
	bool textureApplied = false;
	// holds texture image (if applied)
	ofImage textureImg;
	// holds amount of tiles used in texture mapping in x and y direction
	//  default to 10 each
	int tilesX = 10;
	int tilesY = 10;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	// constructor for ViewPlane defining p0 as bottom left corner
	// and p1 as top right corner
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }
	// default constructor for ViewPlane create reasonable defaults (6x4 aspect)
	ViewPlane() {
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // ViewPlane currently limited to Z axis orientation
	}

	// sets the min and max points of the ViewPlane
	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	// returns the width and height of the ViewPlane
	float getAspect() { return width() / height(); }
	// converts coordinates on ViewPlane to 3D world space
	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]
	// draws the ViewPlane
	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	// returns the width and height of the ViewPlane
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}
	// returns the corners of the ViewPlane
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }


	glm::vec2 min, max;	// defines the boundaries of the ViewPlane
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	// default Constructor for the RenderCam
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
		boxDimension = 1.0;
	}

	// returns a Ray from the current RenderCam position to the (u, v) position of the ViewPlane
	Ray getRay(float u, float v);
	// draws the RenderCam
	void draw() { ofDrawBox(position, boxDimension); };
	// draws lines connecting camera to the view plane
	void drawFrustum() {
		// line from top left of RenderCam to top left of View Plane
		ofDrawLine(glm::vec3(position.x - boxDimension / 2, position.y + boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.topLeft(), view.position.z));
		// line from bottom left of RenderCam to bottom left of View Plane
		ofDrawLine(glm::vec3(position.x - boxDimension / 2, position.y - boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.bottomLeft(), view.position.z));
		// line from top right of RenderCam to bottom right of View Plane
		ofDrawLine(glm::vec3(position.x + boxDimension / 2, position.y + boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.topRight(), view.position.z));
		// line from bottom right of RenderCam to bottom right of View Plane
		ofDrawLine(glm::vec3(position.x + boxDimension / 2, position.y - boxDimension / 2, position.z - boxDimension / 2), glm::vec3(view.bottomRight(), view.position.z));
	}

	float boxDimension;	// defines the width, length, and height of the RenderCam
	glm::vec3 aim;		// the position that the RenderCam aims at			
	ViewPlane view;		// The camera viewplane, this is the view that we will render 
};
