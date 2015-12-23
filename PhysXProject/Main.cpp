#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <PxPhysicsAPI.h>
#include <pvd\PxVisualDebugger.h>
#include <windows.h>


#include "CollisionHandler.h"
#include "FilterGroup.h"
#include "Actor.h"

#pragma comment(lib, "PhysX3CHECKED_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x86.lib")
#pragma comment(lib, "PhysXVisualDebuggerSDKCHECKED.lib")

using namespace physx;
using namespace std;

static CollisionHandler *collisionHandler = nullptr;
static PxScene* gScene =nullptr;
PxPhysics* physics = nullptr;
static PxFoundation* foundation = nullptr;
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
Actor *ball;
vector<PxRigidStatic*> world;
vector<PxVec2> pegPositions;

//#include <malloc.h>
static void* myPlatformAlignedAlloc(size_t size)
{
	return _aligned_malloc(size, 16);
}

static void myPlatformAlignedFree(void* ptr)
{
	_aligned_free(ptr);
}

static PxFilterFlags GroupFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return PxFilterFlag::eDEFAULT;
}

void setupFiltering(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask)
{
	PxFilterData filterData;
	filterData.word0 = filterGroup; // word0 = own ID
	filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;
	const PxU32 numShapes = actor->getNbShapes();
	PxShape** shapes = (PxShape**)myPlatformAlignedAlloc(sizeof(PxShape*)*numShapes);
	actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}
	myPlatformAlignedFree(shapes);
}

void InitializePhysX()
{
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
	debugger::comm::PvdConnectionManager* pvdcm = physics->getPvdConnectionManager();
	if (pvdcm != NULL)
	{
		debugger::comm::PvdConnection* conn = PxVisualDebuggerExt::createConnection(
			pvdcm, "localhost", 5425, 10000, PxVisualDebuggerConnectionFlag::eDEBUG);
		if (conn)
			conn->release();
		physics->getVisualDebugger()->setVisualizeConstraints(true);
		physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
		//PxInitExtensions(*gPhysicsSDK);
	}
	// Create the scene
	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);

	if (!sceneDesc.cpuDispatcher)
	{
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	if (!sceneDesc.filterShader) {
		sceneDesc.filterShader = GroupFilterShader;
	}

	collisionHandler = new CollisionHandler();
	sceneDesc.simulationEventCallback = collisionHandler;

	gScene = physics->createScene(sceneDesc);
}

void PlacePeg(float x, float y) {
	PxMaterial *mat = physics->createMaterial(0.5f, 0.5f, 0.0f);
	PxCapsuleGeometry geom(0.5f, 2.5f);
	PxShape *shape = physics->createShape(geom, *mat);
	PxVec3 pos(x, 7.5f+0.707f*y, 0.707f*y); //0.707 = sin(45) = cos(45)
	PxTransform trans(pos, PxQuat(PxPi / 2.0f, PxVec3(0, 1, 1).getNormalized()));
	PxRigidStatic *peg = PxCreateStatic(*physics, trans, *shape);
	gScene->addActor(*peg);
	world.push_back(peg);
}

void LoadPegs(string fname) {
	ifstream file(fname);
	int numPegs; 
	file >> numPegs;
	for(int i=0; i<numPegs; ++i) {
		float x, y;
		file >> x >> y;
		pegPositions.push_back(PxVec2(x,y));
	}
	file.close();
}

void PlacePegs() {
	for(auto p = pegPositions.begin(); p!=pegPositions.end(); ++p) {
		PlacePeg(p->x, p->y);
	}
}

void EstablishActors(float ballX)
{
	float radius = 1.0f;
	float density = 3.0f;

	//static friction, dynamic friction, restitution
	PxMaterial* ballMaterial = physics->createMaterial(0.5f, 0.5f, 0.25f);
	PxMaterial* backMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);

	PxGeometry *backGeom = new PxBoxGeometry(10.0f, 10.0f, 0.25f);
	PxShape *backShape = physics->createShape(*backGeom, *backMaterial);
	PxTransform backTrans(PxVec3(0,7.5f,0), PxQuat(PxPi/4.0f, PxVec3(1,0,0)));
	PxRigidStatic *back = PxCreateStatic(*physics, backTrans, *backShape);
	gScene->addActor(*back);
	world.push_back(back);

	PxGeometry *leftGeom = new PxBoxGeometry(0.25f, 10.0f, 3.0f);
	PxShape *leftShape = physics->createShape(*leftGeom, *backMaterial);
	PxTransform leftTrans(PxVec3(10.25f, 7.5f, 0), PxQuat(PxPi / 4.0f, PxVec3(1, 0, 0)));
	PxRigidStatic *left = PxCreateStatic(*physics, leftTrans, *leftShape);
	gScene->addActor(*left);
	world.push_back(left);

	PxGeometry *rightGeom = new PxBoxGeometry(0.25f, 10.0f, 3.0f);
	PxShape *rightShape = physics->createShape(*rightGeom, *backMaterial);
	PxTransform rightTrans(PxVec3(-10.25f, 7.5f, 0), PxQuat(PxPi / 4.0f, PxVec3(1, 0, 0)));
	PxRigidStatic *right = PxCreateStatic(*physics, rightTrans, *rightShape);
	gScene->addActor(*right);
	world.push_back(right);

	PlacePegs();

	PxGeometry *ballShape = new PxSphereGeometry(radius);
	ball = new Actor(physics, "Ball1", ballShape, ballMaterial, density);
	ball->SetPosition(ballX, 16.0f, 6.5f);
	gScene->addActor(**ball);
	setupFiltering(*ball, FilterGroup::BALL, FilterGroup::BALL | FilterGroup::FLOOR);
}

void DisplayState()
{
	int count = gScene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC);

	PxActor** actors = new PxActor*[count];
	gScene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC, actors, count);
	for (int i = 0; i < count; i++)
	{
		cout << ((PxRigidDynamic*)(actors[i]))->getGlobalPose().p.x << ", " <<
			((PxRigidDynamic*)(actors[i]))->getGlobalPose().p.y << ", " <<
			((PxRigidDynamic*)(actors[i]))->getGlobalPose().p.z << " | ";
	}
	cout << "\n";
}



void ClearWorld() {
	delete ball;
	while(!world.empty()) {
		world.back()->release();
		world.pop_back();
	}
}

void ShutDownPhysX()
{
	gScene->release();
	physics->release();
	foundation->release();
}

float toRange(float proportion, float min, float max) {
	return min + proportion * (max - min);
}

void UpdateCompletion(int percent) {
	COORD coord = {11, 3};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	cout << percent << "% complete";
}

int main(void)
{
	InitializePhysX();

	int numSimulations;
	string pegFileName;
	string outputFileName;
	
	cout << "Number of simulations to run? ";
	cin >> numSimulations;

	cout << "Peg file name? ";
	cin >> pegFileName;
	
	cout << "Output file name? ";
	cin >> outputFileName;
	
	LoadPegs(pegFileName);
	ofstream outputFile(outputFileName);
	outputFile << "Test ID,Drop Point,Result" << endl;

	cout << "Simulation 0% complete";

	for(int x = 0; x < numSimulations; ++x) {
		float dropPoint = toRange((float)x / (float)numSimulations, -9.0f, 9.0f);
		EstablishActors(dropPoint);
		while (true)
		{
			gScene->simulate(1.0f / 60.0f); // simulate 60 fps
			gScene->fetchResults(true);

			if (ball->IsSleeping()) {
				outputFile << x << "," << dropPoint << ",NULL" << endl;
				break;
			} else if (ball->GetPosition().y <= 0) {
				if (ball->GetPosition().z >= 5) {
					//Ball fell behind the board, probably a peg too close to the top.
					outputFile << x << "," << dropPoint << ",NULL" << endl;
				} else {
					outputFile << x << "," << dropPoint << "," << ball->GetPosition().x << endl;
				}
				break;
			}
		}
		ClearWorld();

		int complete = (int)(((float)x / (float)numSimulations) * 100.0f);
		UpdateCompletion(complete);
	}
	UpdateCompletion(100);
	cout << endl;
	ShutDownPhysX();
	system("pause");
	return 0;
}