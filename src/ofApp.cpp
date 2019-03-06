#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetCircleResolution(20);
  ofDisableArbTex();
  
  box2d.init();
  box2d.setGravity(-0.5, 0.5);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  startRepelling = false;
  debug = false;
  
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
  
  for (auto &a : agents) {
    a.update();
  }

  updateAgentProps();
  
//  handleSerial();

  // Should create/destroy interAgentJoints?
  interAgentJointCreateDestroy();
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofPushStyle();
    // Draw InterAgentJoints
    for (auto j: interAgentJoints) {
      ofSetColor(ofColor::green);
      j->draw();
    }
  ofPopStyle();
  
  for (auto a: agents) {
    a.draw(debug);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::interAgentJointCreateDestroy() {
  // Joint creation based on when two bodies collide at certain vertices. 
  if (collidingBodies.size()>0) {
    // We have pending bodies to create a joint with.
    auto jointList = collidingBodies[0]->GetJointList();
    auto j = std::make_shared<ofxBox2dJoint>();
    j->setup(box2d.getWorld(), collidingBodies[0], collidingBodies[1], frequency, damping); // Use the interAgentJoint props.
    j->setLength(40);
    interAgentJoints.push_back(j);
    collidingBodies.clear();
  }
  
  // Joint destruction based on a predetermined force between agents.
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
      auto f = c->getReactionForce(ofGetElapsedTimef());
      if (f.length() > maxJointForce) {
        box2d.getWorld()->DestroyJoint(c->joint);
        return true;
      }

      return false;
  });
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
  agentNum = agents.size();
  cout << "Agent Num: " << agentNum;
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("Inter Mesh Settings");
  
    // Mesh parameters.
    meshParams.setName("Mesh Params");
    meshParams.add(meshRows.set("Mesh Rows", 5, 5, 100)); // Add the current value
    meshParams.add(meshColumns.set("Mesh Columns", 5, 5, 100));
    meshParams.add(meshWidth.set("Mesh Width", 100, 10, ofGetWidth()/2));
    meshParams.add(meshHeight.set("Mesh Height", 100, 10, ofGetHeight()/2));
  
    // Vertex parameters
    vertexParams.setName("Vertex Params");
    vertexParams.add(vertexRadius.set("Vertex Radius", 6, 1, 30));
    vertexParams.add(vertexDensity.set("Vertex Density", 1, 0, 5));
    vertexParams.add(vertexBounce.set("Vertex Bounce", 0.3, 0, 1));
    vertexParams.add(vertexFriction.set("Vertex Friction", 1, 0, 1));
  
    // Joint parameters
    jointParams.setName("Joint Params");
    jointParams.add(jointFrequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    jointParams.add(jointDamping.set("Joint Damping", 1.0f, 0.0f, 5.0f));
  
    // InterAgentJoint parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(frequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    interAgentJointParams.add(damping.set("Joint Damping", 1.0f, 0.0f, 10.0f));
    interAgentJointParams.add(maxJointForce.set("Max Joint Force", 6.f, 1.f, 20.0f));
  
    settings.add(meshParams);
    settings.add(vertexParams);
    settings.add(jointParams);
    settings.add(interAgentJointParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence. 
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  if (agents.size() > 0) {
    if(e.a != NULL && e.b != NULL) {
      if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
        // Extract vertex data.
        VertexData* aData = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
        VertexData* bData = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
        
        if (e.a->GetBody() && e.b->GetBody()) {
          // Should these 2 bodies bond???
          // [WARNING] Assuming both agentNum is the agent's index in the array. Bad Assumption
          // Figure out a better solution soon.
          // Evaluate color slots of both these agents.
          if (shouldBond(aData->agentId, bData->agentId)) {
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
    }
}

//
bool ofApp::shouldBond(int agentA, int agentB) {
  if (agentA != agentB) { // Hope it's not the same agent
    // Agent A and Agent B should not have any interAgentJoints
    // Go through each vertex of A and check if each of the vertices can Join
    // Go through each vertex of B and check if each of the vertices can Join

    auto agent1 = agents.at(agentA);
    auto agent2 = agents.at(agentB);
    
    // Checks to make sure if bonding is happening, it's only between agentA and agentB.
    for (auto v : agent1.vertices) {
      int otherAgentId = findOtherAgent(v->body, agentA);
      if (otherAgentId == agentA || otherAgentId == agentB) {
        // Cool. Keep going through each other.
      } else {
        // No bonding.
        return;
      }
    }
    
    for (auto v : agent2.vertices) {
      int otherAgentId = findOtherAgent(v->body, agentB);
      if (otherAgentId == agentB || otherAgentId == agentA) {
        // Cool. Keep going through each other.
      } else {
        // No bonding.
        return;
      }
    }

    // How many common colors are between the two arrays?
    int commonColorsNum = 0;
    int uncommonColorsNum = 0;
    bool a; bool b;
    
    // Agent 1
    auto colors = agent1.colors;
    for (int i = 0; i < colors.size(); i++) {
      a = ofContains(agent1.colorSlots, colors.at(i));
      b = ofContains(agent2.colorSlots, colors.at(i));
      if (a & b) {
        commonColorsNum++;
      }
      
      if (!a && !b) {
        // Color isn't in a and b.
      } else {
        // It's in one but not the other.
        uncommonColorsNum++;
      }
    }
    
    // Should be some common and some uncommon colors.
    return commonColorsNum >= 3 && uncommonColorsNum >=2;
  }
  
  return false;
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

int ofApp::findOtherAgent(b2Body *body, int curAgentId) {
  if (body == NULL) {
    return -1;
  }
  
  auto curEdge = body -> GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge && curEdge != curEdge -> next) {
    // Get the other body and check if its agentId is same as
    // bodyBAgentId
    if (curEdge->other == NULL) {
      ofLogWarning("A Null Pointer came around");
    }
    
    auto otherAgentId = reinterpret_cast<VertexData*>(curEdge->other->GetUserData())->agentId;
    // It's not the same body? That means it's jointed with someone else.
    if (otherAgentId != curAgentId) {
      return otherAgentId;
    }
    
    curEdge = curEdge -> next;
  }
  
  return curAgentId;
}

void ofApp::cleanInterAgentJoints() {
  // Actually destroy the joint... Else, the joint disappears but it still sit between the bodies.
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> j){
      box2d.getWorld()->DestroyJoint(j->joint);
      return true;
  });
  
  cout << "InterAgentJoints cleared." << endl;
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
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
    // middle of a step function. Disabling and renabling the events work as a good solution for now.
    box2d.disableEvents();
    
    collidingBodies.clear();
    cleanInterAgentJoints();
    for (auto &a : agents) {
      a.clean(box2d);
    }
    agents.clear();
    agentNum = 0;
    
    box2d.enableEvents();
  }
  
  if (key == 'j') {
    box2d.disableEvents();
    cleanInterAgentJoints();
    box2d.enableEvents();
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
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
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
