#pragma once

#include "core/math.h"
#include "physics.h"

struct collider_union;
struct rigid_body_global_state;
struct broadphase_collision;

struct contact_info
{
	vec3 point;
	float penetrationDepth;
};

struct collision_contact
{
	// Don't change the order here.
	vec3 point;
	float penetrationDepth;
	vec3 normal;
	uint32 friction_restitution; // Packed as 16 bit int each. The packing makes it more convenient for the SIMD code to load the contact data.
	uint16 rbA;
	uint16 rbB;
};

struct collision_constraint
{
	vec3 relGlobalAnchorA;
	vec3 relGlobalAnchorB;
	vec3 tangent;

	vec3 tangentImpulseToAngularVelocityA;
	vec3 tangentImpulseToAngularVelocityB;
	vec3 normalImpulseToAngularVelocityA;
	vec3 normalImpulseToAngularVelocityB;

	float impulseInNormalDir;
	float impulseInTangentDir;
	float effectiveMassInNormalDir;
	float effectiveMassInTangentDir;
	float bias;
};

struct non_collision_interaction
{
	uint16 rigidBodyIndex;
	uint16 otherIndex;
	physics_object_type otherType;
};

struct narrowphase_result
{
	uint32 numContacts;
	uint32 numNonCollisionInteractions;
};

narrowphase_result narrowphase(collider_union* worldSpaceColliders, broadphase_collision* possibleCollisions, uint32 numPossibleCollisions,
	collision_contact* outContacts, non_collision_interaction* outNonCollisionInteractions);

void initializeCollisionVelocityConstraints(rigid_body_global_state* rbs, collision_contact* contacts, collision_constraint* collisionConstraints, uint32 numContacts, float dt);
void solveCollisionVelocityConstraints(collision_contact* contacts, collision_constraint* constraints, uint32 count, rigid_body_global_state* rbs);





#define COLLISION_SIMD_WIDTH 8

struct simd_collision_batch
{
	float relGlobalAnchorA[3][COLLISION_SIMD_WIDTH];
	float relGlobalAnchorB[3][COLLISION_SIMD_WIDTH];
	float normal[3][COLLISION_SIMD_WIDTH];
	float tangent[3][COLLISION_SIMD_WIDTH];

	float normalImpulseToAngularVelocityA[3][COLLISION_SIMD_WIDTH];
	float tangentImpulseToAngularVelocityA[3][COLLISION_SIMD_WIDTH];
	float normalImpulseToAngularVelocityB[3][COLLISION_SIMD_WIDTH];
	float tangentImpulseToAngularVelocityB[3][COLLISION_SIMD_WIDTH];

	float effectiveMassInNormalDir[COLLISION_SIMD_WIDTH];
	float effectiveMassInTangentDir[COLLISION_SIMD_WIDTH];
	float friction[COLLISION_SIMD_WIDTH];
	float impulseInNormalDir[COLLISION_SIMD_WIDTH];
	float impulseInTangentDir[COLLISION_SIMD_WIDTH];
	float bias[COLLISION_SIMD_WIDTH];

	uint16 rbAIndices[COLLISION_SIMD_WIDTH];
	uint16 rbBIndices[COLLISION_SIMD_WIDTH];
};

struct simd_collision_constraint
{
	simd_collision_batch* batches;
	uint32 numBatches;
};

struct simd_collision_metrics
{
	uint32 simdWidth;
	uint32 numContacts;
	uint32 numBatches;
	float averageFillRate;
};

void initializeCollisionVelocityConstraintsSIMD(memory_arena& arena, rigid_body_global_state* rbs, collision_contact* contacts, uint32 numContacts, 
	uint16 dummyRigidBodyIndex, simd_collision_constraint& outConstraints, float dt, simd_collision_metrics& metrics);
void solveCollisionVelocityConstraintsSIMD(simd_collision_constraint& constraints, rigid_body_global_state* rbs);
