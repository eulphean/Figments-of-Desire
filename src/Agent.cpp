#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps) {
  // Post-process
  _filters.push_back(new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y));
  _filters.push_back(new PixelateFilter(agentProps.meshSize.x, agentProps.meshSize.y));
  
  attractTargetPos = glm::vec2(ofRandom(150, ofGetWidth() - 200), ofRandom(50 , 250));
  
  // Populate color slots.
  populateSlots();
  createTexture(agentProps.meshSize); // Design the look of this creature.
  
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  
  // Calculate a targetPerceptionRad based on the size of the mesh
  auto area = agentProps.meshSize.x * agentProps.meshSize.y;
  targetPerceptionRad = sqrt(area/PI);
  
  // Force weights for various body activities. 
  attractWeight = 0.06;
  randWeight = 0.3;
  centrifugalWeight = 0.5;
  
  // Healths to keep track when to execute something again.
  tickleHealth = 50;
  
  applyRandomForce = false;
  attractTarget = true;
  repelTarget = false;
  applyCentrifugalForce = false;
  maxInterAgentJoints = ofRandom(1, vertices.size());
}

void Agent::update() {
  if (partner != NULL) {
    attractWeight = ofRandom(0.2, 0.4);
  }
  
  // Use box2d circle to update the mesh.
  updateMesh();
  
  // Apply behavioral forces on the body.
  applyBehaviors();
}

void Agent::draw(bool debug, bool showTexture) {
  // Draw the meshes.
  // Draw the soft bodies.
  ofPushStyle();
    for(auto v: vertices) {
      ofPushMatrix();
        ofTranslate(v->getPosition());
        ofSetColor(ofColor::red);
        ofFill();
        ofDrawCircle(0, 0, v->getRadius());
      ofPopMatrix();
    }
  ofPopStyle();
  
  if (showTexture) {
  // Bind the fbo.
    _filters[_currentFilter]->begin();
    fbo.getTexture().bind();
    mesh.draw();
    fbo.getTexture().unbind();
    _filters[_currentFilter]->end();
  } else {
    ofPushStyle();
    for(auto j: joints) {
      ofPushMatrix();
        ofSetColor(ofColor::green);
        j->draw();
      ofPopMatrix();
    }
    ofPopStyle();
  }

  if (debug) {
    auto centroid = mesh.getCentroid();
    ofPushMatrix();
      ofTranslate(centroid);
      ofNoFill();
      ofSetColor(ofColor::white);
      ofDrawCircle(0, 0, targetPerceptionRad);
    ofPopMatrix();
  }
}

int Agent::getMaxInterAgentJoints() {
  return maxInterAgentJoints;
}

void Agent::clean(ofxBox2d &box2d) {
  // Remove joints.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  // Remove vertices
  ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> c){
    return true;
  });

  joints.clear();
  vertices.clear();
}

void Agent::createTexture(ofPoint meshSize) {
  // Create a simple fbo. 
  fbo.allocate(meshSize.x, meshSize.y, GL_RGBA);
  fbo.begin();
    ofClear(0, 0, 0, 0);
    ofBackground(colorSlots.at(0));
    const int firstRecs = 10;
    // Create the slots in the fbo.
    for (int i = 0; i < maxSlots; i++) {
      int numRecs = firstRecs * (i+1);
      ofSetColor(colorSlots.at(i));
      for (int j = 0; j < numRecs; j++) {
        auto x = ofRandom(0, fbo.getWidth());
        auto y = ofRandom(0, fbo.getHeight());
        //ofDrawRectangle(x, y, x+ofRandom(10, 15), y+ofRandom(10, 15));
        ofDrawCircle(x, y, 5);
      }
    }
  fbo.end();
}

void Agent::mutateTexture() {
  // Populate slots again
  populateSlots();
  ofPoint p = ofPoint(fbo.getWidth(), fbo.getHeight());
  createTexture(p);
}

void Agent::populateSlots() {
  colorSlots.clear();
  for (int i = 0; i < maxSlots; i++) {
    int randIdx = ofRandom(colors.size());
    colorSlots.push_back(colors.at(randIdx));
  }
}

void Agent::applyBehaviors() {
  handleTickle();
  handleAttraction();
  handleRepulsion();
  handleCentrifugalForce();
}

void Agent::handleCentrifugalForce() {
  if (applyCentrifugalForce) {
    // Apply force away from centroid
    for (auto &v : vertices) {
      if (ofRandom(1) < 0.4) {
         v->addAttractionPoint({mesh.getCentroid().x, mesh.getCentroid().y}, centrifugalWeight);
      } else {
        v->addRepulsionForce(mesh.getCentroid().x, mesh.getCentroid().y, centrifugalWeight);
      }
      v->setRotation(ofRandom(150));
    }
    applyCentrifugalForce = false;
  }
}

void Agent::handleTickle() {
  // Check for tickle health.
  if (tickleHealth == 0) {
    applyRandomForce = true;
    tickleHealth = 100;
    randWeight = 0.3;
  } else {
    tickleHealth -= 0.1;
  }
  
  // Random force/ Force all vertices.
  if (applyRandomForce) {
    // force()
    for (auto &v: vertices) {
      glm::vec2 force = glm::vec2(ofRandom(-5, 5), ofRandom(-5, 5));
      v -> addForce(force, randWeight);
    }
    applyRandomForce = false;
  }
}

void Agent::handleAttraction() {
  // New target? Add an impulse in that direction.
  if (attractTarget) {
    // seek()
    for (auto &v: vertices) {
        v->addAttractionPoint(attractTargetPos.x, attractTargetPos.y, attractWeight);
        v->setRotation(ofRandom(180, 360));
    }
    attractTarget = false;
  }
  
  // Pick a new target location once it starts slowing down.
  glm::vec2 avgVel;
  for (auto v : vertices) {
    avgVel += glm::vec2(v->getVelocity().x, v->getVelocity().y);
  }
  avgVel = avgVel/vertices.size();
  if (abs(avgVel.g) < ofRandom(0.1, 0.3)) {
    // Pick a new target.
    auto x = targetPerceptionRad * sin(ofRandom(360)); auto y = targetPerceptionRad * cos(ofRandom(360));
    attractTargetPos = glm::vec2(mesh.getCentroid().x, mesh.getCentroid().y) + glm::vec2(x, y);
    attractWeight = ofRandom(0.1, 0.3);
    attractTarget = true;
  }
}

void Agent::handleRepulsion() {
  // Initiate a repel on all the vertices and then stop.
  // Repulsion impulse and then stop.
  if (repelTarget) {
    if (ofGetElapsedTimeMillis() - repelTotalTimer < 20000) {
      if (ofGetElapsedTimeMillis() - repelIntervalTimer > 2000) {
        // Repel and reset time
        cout << "Repulsion";
        for (auto &v: vertices) {
          auto pos = glm::vec2(repelTargetAgent->getCentroid().x, repelTargetAgent->getCentroid().y);
          v->addRepulsionForce(pos.x, pos.y, 2.0);
        }
        repelIntervalTimer = ofGetElapsedTimeMillis(); // Reset timer
      }
    } else {
      repelTarget = false;
      // Check if we are still connected???
      // Need to do that check somewhere..
      repelTotalTimer = ofGetElapsedTimeMillis();
    }
  }
}

glm::vec2 Agent::getCentroid() {
  return mesh.getCentroid();
}

ofMesh& Agent::getMesh() {
  return mesh;
}

void Agent::setAttractionTarget(glm::vec2 target) {
  attractTargetPos = target;
  attractTarget = true;
}

void Agent::setRepulsionTarget(Agent *targetAgent, int newTargetAgentId) {
  if (newTargetAgentId == repelTargetAgentId) {
    // Don't do anything
  } else {
    repelTargetAgentId = newTargetAgentId;
    repelTargetAgent = targetAgent;
    repelTarget = true;
    repelTotalTimer = ofGetElapsedTimeMillis();
    repelIntervalTimer = ofGetElapsedTimeMillis();
    // Start the timer as well.
  }
}

void Agent::setRandomForce(float avgForceWeight) {
  applyRandomForce = true;
  randWeight = avgForceWeight;
  tickleHealth = 0; 
}

void Agent::setCentrifugalForce(float avgWeight) {
  applyCentrifugalForce = true;
  centrifugalWeight = avgWeight;
}

std::shared_ptr<ofxBox2dCircle> Agent::getRandomVertex() {
  int randV = ofRandom(vertices.size());
  auto v = vertices[randV];
  return v;
}

void Agent::createMesh(AgentProperties agentProps) {
  //auto a = ofRandom(50, ofGetWidth() - 50); auto b = ofRandom(50, ofGetHeight() - 50);
  //auto a = ofPoint(ofRandom(60, 100), ofGetHeight() - agentProps.meshSize.y * ofRandom(2, 5));
  auto a = ofPoint(150, ofGetHeight()/3);
  auto meshOrigin = glm::vec2(a.x, a.y);
  
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshDimensions.x;
  int nCols = agentProps.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = agentProps.meshSize.x;
  int h = agentProps.meshSize.y;
  
  auto texture = fbo.getTexture();
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = meshOrigin.x + w * x / (nCols - 1);
      float iy = meshOrigin.y + h * y / (nRows - 1);
     
      mesh.addVertex({ix, iy, 0});
      
      float texX = ofMap(ix - meshOrigin.x, 0, fbo.getTexture().getWidth(), 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy - meshOrigin.y, 0, fbo.getTexture().getHeight(), 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord({texX, texY});
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void Agent::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius); // ofRandom(3, agentProps.vertexRadius)
    vertex -> setFixedRotation(true);
    vertex -> setData(new VertexData(this)); // Data is passed with current Agent's pointer
    vertices.push_back(vertex);
  }
  
  int meshRows = agentProps.meshDimensions.x;
  int meshColumns = agentProps.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void Agent::updateMesh() {
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
}

// Agent partners. 
void Agent::setPartner(Agent *a) {
  partner = a;
}

Agent *Agent::getPartner() {
  return partner;
}

void Agent::nextFilter() {
  _currentFilter ++;
  if (_currentFilter>=_filters.size()) _currentFilter = 0;
}
