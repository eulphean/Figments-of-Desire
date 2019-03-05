#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetCircleResolution(20);
  
  box2d.init();
  box2d.setGravity(0, 0);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  startRepelling = false;
  
  // Boundaries
  ofRectangle bounds;
  bounds.x = 0; bounds.y = 0;
  bounds.width = ofGetWidth(); bounds.height = ofGetHeight();
  box2d.createBounds(bounds);
  
  agentNum = 0;
  
  //serial.setup("/dev/cu.usbmodem1411", 9600);
}


//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
//
//  handleSerial();
  updateAgentProps();
  
  for (auto &a : agents) {
    a.update();
  }
  
  // Should I create interAgent joints?
  if (collidingBodies.size()>0) {
    // We have pending bodies to create a joint with.
    auto jointList = collidingBodies[0]->GetJointList();
    auto j = std::make_shared<ofxBox2dJoint>();
    j->setup(box2d.getWorld(), collidingBodies[0], collidingBodies[1], agentProps.jointPhysics.x, agentProps.jointPhysics.y);
    j->setLength(ofRandom(50, 100));
    interAgentJoints.push_back(j);
    collidingBodies.clear();
  }
  
  // Joint removal condition
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
      auto f = c->getReactionForce(ofGetElapsedTimef());
      if (f.length() > 5) {
        box2d.getWorld()->DestroyJoint(c->joint);
        return true;
      }

      return false;
  });
  
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Draw InterAgentJoints
  for (auto j: interAgentJoints) {
    ofSetColor(ofColor::green);
    j->draw();
  }
  
  for (auto a: agents) {
    a.draw(true);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
        // [Summary] Data is sent from the Arduino byte by byte.
        // In this while loop, we read the data byte by byte until
        // there is incoming data. Every character we are sending from
        // there is a byte wide. We keep constructing the buffer until
        // we reach the new line character, which determines that we are
        // done reading the first set of data.
    
        // Read the byte.
        char b = serial.readByte();
      
        // End of line character.
        if (b == '\n')
        {
            // Skip
            cout<< "New line" << "\n";
        }
        else
        {
            // If it's not the line feed character, add it to the buffer.
            cout << "Received: " << b << "\n";
            startRepelling = b - '0';
            cout << "Replling: " << startRepelling << "\n";
        }
    }
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
  agentProps.agentId = agentNum; 
}

void ofApp::createAgent() {
  Agent a;
  a.setup(box2d, agentProps);
  agents.push_back(a);
  agentNum++;
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
  
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence. 
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  if(e.a != NULL && e.b != NULL) {
    if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
      // Extract vertex data.
      VertexData* aData = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
      VertexData* bData = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
      
      if (aData->agentId != bData->agentId) {
          collidingBodies.clear();
          auto bodyA = e.a->GetBody();
          auto bodyB = e.b->GetBody();
        
          // Condense this in a function
          bool a = canJoin(bodyA, aData->agentId);
          bool b = canJoin(bodyB, bData->agentId);
        
          if (a && b) {
            // Reached here?
            collidingBodies.push_back(bodyA);
            collidingBodies.push_back(bodyB);
          }
        }
      }
    }
}

bool ofApp::canJoin(b2Body* body, int curAgentId) {
  // If it joins anything except itself, then it cannot join.
  auto curEdge = body -> GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge && curEdge != curEdge -> next) {
    // Get the other body and check if its agentId is same as
    // bodyBAgentId
    auto otherAgentId = reinterpret_cast<VertexData*>(curEdge->other->GetUserData())->agentId;
    
    // It's not the same body? That means it's jointed with someone else.
    if (otherAgentId != curAgentId) {
      return false;
    }
    
    curEdge = curEdge -> next;
  }
  
  return true;
}

void ofApp::cleanInterAgentJoints() {
  // Actually destroy the joint... Else, the joint disappears but it still sit between the bodies.
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> j){
      box2d.getWorld()->DestroyJoint(j->joint);
      return true;
  });
  
  interAgentJoints.clear();
  collidingBodies.clear();
}

void ofApp::enableRepulsion() {
    // Enable repelling on the agent.
    for (auto &j: interAgentJoints) {
      auto bodyA = j->joint->GetBodyA();
      auto bodyB = j->joint->GetBodyB();
      
      // Agent A (centroid will be the repulsion point)
      auto agentIdA = reinterpret_cast<VertexData*>(bodyA -> GetUserData()) -> agentId;
      auto &agentA = agents.at(agentIdA);
      auto centroidA = agentA.getCentroid();
      
      // Agent B (centroid will be the repulsion point)
      auto agentIdB = reinterpret_cast<VertexData*>(bodyB -> GetUserData()) -> agentId;
      auto &agentB = agents.at(agentIdB);
      auto centroidB = agentB.getCentroid();
      
      // Set a repulsion target for agent A
      agentA.setRepulsionTarget(centroidB);
      agentB.setRepulsionTarget(centroidA);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    collidingBodies.clear();
    cleanInterAgentJoints();
    clearAgents();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a.setRandomForce();
    }
  }
  
  if (key == 'j') {
    cleanInterAgentJoints();
  }
  
  if (key == 'r') {
    enableRepulsion();
  }
}

void ofApp::mousePressed(int x, int y, int button) {
   for (auto &a: agents) {
    a.setAttractionTarget(glm::vec2(x, y));
  }
}

void ofApp::exit() {
  gui.saveToFile("InterMesh.xml");
}


//
//      auto v = (bodyA->GetPosition() - bodyB->GetPosition());
//      auto v2 = b2Vec2(v.x * 2, v.y * 5);
//
//      // Get bodyA's agentId
//      // apply force on bodyB's vertices away from the centroid of bodyA
//
//      // Apply force on bodyA in the opposite direction of bodyB
//      bodyA->ApplyForceToCenter(v2, true);
//
//      // Apply force on bodyB in the opposite direction of bodyA
//      v2 = b2Vec2(-v.x * 2, -v.y * 5);
//      bodyB->ApplyForceToCenter(v2, true);



//  auto it1 = interAgentHistory.find(vId);
//  if (it1 != interAgentHistory.end()) {
//    // Does it have the agentid?
//    auto a = it1->second;
//    for (int i = 0; i < a->size(); i++) {
//      if (agentId == a->at(i)) {
//        return true;
//      }
//    }
//
//    return false;
//  } else {
//    return false;
//  }
