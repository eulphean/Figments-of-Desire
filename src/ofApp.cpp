#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  
  box2d.init();
  box2d.setGravity(0, 0.2);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  
  // Boundaries
  ofRectangle bounds;
  bounds.x = 0; bounds.y = 0;
  bounds.width = ofGetWidth(); bounds.height = ofGetHeight();
  box2d.createBounds(bounds);
  
  idx = 1;
}

//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
  
  updateAgentProps();
  
  for (auto &a : agents) {
    a.update();
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  for (auto a: agents) {
    a.draw(showSoftBody);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
}

void ofApp::createAgent() {
  Agent a;
  a.setup(box2d, agentProps);
  agents.push_back(a);
}

void ofApp::mousePressed(int x, int y, int button) {
   for (auto &a: agents) {
    a.setTarget(x, y);
  }
}

void ofApp::clearAgents() {
  // Clear agent's soft bodies
  for (auto &a: agents) {
    a.clean();
  }
  
  // Empty the vector
  agents.clear();
}

void ofApp::setupGui() {
    gui.setup();
    gui.add(meshRows.setup("Mesh Rows", 20, 1, 100));
    gui.add(meshColumns.setup("Mesh Columns", 20, 1, 100));
    gui.add(meshWidth.setup("Mesh Width", 50, 10, ofGetWidth()));
    gui.add(meshHeight.setup("Mesh Height", 50, 10, ofGetHeight()));
    gui.add(vertexRadius.setup("Vertex Radius", 5, 1, 30));
    gui.add(vertexDensity.setup("Vertex density", 0.5, 0, 5));
    gui.add(vertexBounce.setup("Vertex bounce", 0.5, 0, 1));
    gui.add(vertexFriction.setup("Vertex friction", 0.5, 0, 1));
    gui.add(jointFrequency.setup("Joint frequency", 4.f, 0.f, 20.f ));
    gui.add(jointDamping.setup("Joint damping", 1.f, 0.f, 5.f));
    gui.add(showSoftBody.setup("Show soft body", 0.f));
  
  
    gui.loadFromFile("InterMesh.xml");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    clearAgents();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a.addForce();
    }
  }
}

void ofApp::exit() {
  gui.saveToFile("InterMesh.xml");
}
