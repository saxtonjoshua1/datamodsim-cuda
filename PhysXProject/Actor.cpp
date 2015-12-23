#include "Actor.h"

Actor::Actor(PxPhysics *physicsEngine, string name, PxGeometry *shape, 
	PxMaterial *material, float density) {
	this->name = name;

	body = physicsEngine->createRigidDynamic(PxTransform(PxVec3(0, 0, 0)));
	body->createShape(*shape, *material);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	body->setName(name.c_str());
}

Actor::~Actor() {
	body->release();
}

Actor::operator PxRigidDynamic*() {
	return body;
}

void Actor::SetPosition(float x, float y, float z) {
	body->setGlobalPose(PxTransform(PxVec3(x, y, z)));
}
PxVec3 Actor::GetPosition() {
	return body->getGlobalPose().p;
}

void Actor::SetLinearVelocity(float x, float y, float z) {
	body->setLinearVelocity(PxVec3(x, y, z));
}

void Actor::SetPose(PxVec3 pos, PxQuat rot) {
	body->setGlobalPose(PxTransform(pos, rot));
}

bool Actor::IsSleeping() const {
	return body->isSleeping();
}