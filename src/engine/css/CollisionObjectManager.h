
//	This file is part of The Drwalin Engine project
// Copyright (C) 2018-2020 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef COLLISION_OBJECT_MANAGER_H
#define COLLISION_OBJECT_MANAGER_H

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btTransform.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

#include <memory>
#include <string>

#include "CollisionShape.h"

class CollisionObjectManager {
public:
	
	static btCollisionObject* CreateCollisionObject(std::shared_ptr<CollisionShape> shape, btTransform transform);
	static btCollisionObject* CreateRigidBody(std::shared_ptr<CollisionShape> shape, btTransform transform, float mass, btVector3 inertia = btVector3(0.0f,0.0f,0.0f));
	static btCollisionObject* CreateGhostObject(std::shared_ptr<CollisionShape> shape, btTransform transform);
	static btCollisionObject* CreatePairCachingGhostObject();
};

#endif

