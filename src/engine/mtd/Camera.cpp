
//	This file is part of The Drwalin Engine project
// Copyright (C) 2018-2020 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef CAMERA_CPP
#define CAMERA_CPP

#include "../css/Camera.h"
#include "../css/Engine.h"

#include "../lib/Math.hpp"

void Camera::UseTarget() {
	if(target) {
		engine->GetWindow()->GetVideoDriver()->setRenderTarget(target, true, true, 0);
	} else {
		engine->GetWindow()->GetVideoDriver()->setRenderTarget(0, true, true, 0);
	}
}

irr::video::ITexture *Camera::GetTexture() {
	return target;
}

bool Camera::IsMainTarget() {
	if(target)
		return false;
	return true;
}

irr::scene::ICameraSceneNode *Camera::GetCameraNode() {
	return sceneNode;
}

btTransform Camera::GetTransform() const {
	return btTransform(GetRotation(), GetWorldPosition());
}

btQuaternion Camera::GetRotation() const {
	return parentTransformation.getRotation() *Math::MakeQuaternionFromEuler(rotation);
}

btQuaternion Camera::GetFlatRotation() const {
	return parentTransformation.getRotation() *btQuaternion(btVector3(0, 1, 0), -rotation.y());
}

btVector3 Camera::GetUpVector() const {
	return btTransform(GetRotation()) *btVector3(0, 1, 0);
}

btVector3 Camera::GetFlatRightVector() const {
	return btTransform(GetFlatRotation()) *btVector3(-1, 0, 0);
}

btVector3 Camera::GetRightVector() const {
	return btTransform(GetRotation()) *btVector3(-1, 0, 0);
}

btVector3 Camera::GetFlatLeftVector() const {
	return btTransform(GetFlatRotation()) *btVector3(1, 0, 0);
}

btVector3 Camera::GetLeftVector() const {
	return btTransform(GetRotation()) *btVector3(1, 0, 0);
}

btVector3 Camera::GetFlatForwardVector() const {
	return btTransform(GetFlatRotation()) *btVector3(0, 0, 1);
}

btVector3 Camera::GetForwardVector() const {
	return btTransform(GetRotation()) *btVector3(0, 0, 1);
}

btVector3 Camera::GetPosition() const {
	return position;
}

btVector3 Camera::GetEulerRotation() const {
	return rotation;
}

btVector3 Camera::GetWorldPosition() const {
	return parentTransformation * position;
}

void Camera::UpdateCameraView() {
	if(rotation.m_floats[0] > Math::PI/2.0)
		rotation.m_floats[0] = Math::PI/2.0;
	else if(rotation.m_floats[0] < -Math::PI/2.0)
		rotation.m_floats[0] = -Math::PI/2.0;
	
	if(rotation.m_floats[1] > Math::PI*2.0)
		rotation.m_floats[1] -= Math::PI*2.0;
	else if(rotation.m_floats[1] < -Math::PI*2.0)
		rotation.m_floats[1] += Math::PI*2.0;
	
	if(rotation.m_floats[2] > Math::PI*2.0)
		rotation.m_floats[2] -= Math::PI*2.0;
	else if(rotation.m_floats[2] < -Math::PI*2.0)
		rotation.m_floats[2] += Math::PI*2.0;
	
	sceneNode->setTarget(Math::GetIrrVec(GetWorldPosition() + GetForwardVector()));
	sceneNode->setUpVector(Math::GetIrrVec(GetUpVector()));
	sceneNode->setPosition(Math::GetIrrVec(GetWorldPosition()));
}

void Camera::RotateCameraToLookAtPoint(const btVector3 &worldPoint, bool smooth) {
	btVector3 dstCameraRotation(0,0,0);
	{
		btVector3 dstLookingDirection = (worldPoint - GetWorldPosition()).normalized();
		btVector3 forwardVector = GetForwardVector().normalized();
		btVector3 flatForward = forwardVector;
		flatForward.m_floats[1] = 0.0f;
		flatForward.normalize();
		btVector3 flatDstLookingDirection = dstLookingDirection;
		flatDstLookingDirection.m_floats[1] = 0.0f;
		flatDstLookingDirection.normalize();
		
		float dot = flatForward.normalized().dot(flatDstLookingDirection.normalized());
		dstCameraRotation.m_floats[1] = ((dot<=-1 || dot>=1) ? 0.001f : acos(dot))*(flatDstLookingDirection.dot(GetFlatLeftVector()) > 0.0f ? -1.0f : 1.0f);
		dot = forwardVector.normalized().dot((Math::MakeTransformFromEuler(btVector3(0, -dstCameraRotation.y(), 0))*dstLookingDirection).normalized());
		dstCameraRotation.m_floats[0] = ((dot<=-1 || dot>=1) ? 0.001f : acos(dot))*(dstLookingDirection.y() > forwardVector.y() ? -1.0f : 1.0f);
	}
	
	if(smooth)
		Rotate(dstCameraRotation*engine->GetDeltaTime()*3.11f*Math::PI);
	else
		Rotate(dstCameraRotation);
}

void Camera::SetRelativePosition(btVector3 src) {
	position = src;
	UpdateCameraView();
}

void Camera::SetRotation(btVector3 src) {
	rotation = src;
	UpdateCameraView();
}

void Camera::Move(btVector3 src) {
	this->position += src;
	this->UpdateCameraView();
}

void Camera::Rotate(btVector3 src) {
	this->rotation += src;
	this->UpdateCameraView();
}

void Camera::SetCameraParentTransform(btTransform transform) {
	this->parentTransformation = transform;
	this->UpdateCameraView();
}

void Camera::SetFOV(float fovDegrees) {
	if(this->fovDegrees != fovDegrees) {
		this->fovDegrees = fovDegrees;
		this->sceneNode->setFOV(this->fovDegrees*Math::PI/180.0f);
	}
}

void Camera::SetRenderTargetSize(unsigned width, unsigned height) {
	if(this->target) {
		if(this->target->getOriginalSize() == irr::core::dimension2d<unsigned>(width, height)) {
			return;
		}
		this->engine->GetWindow()->GetVideoDriver()->setRenderTarget(0, true, true, 0);
		this->target->drop();
		this->engine->GetWindow()->GetVideoDriver()->removeTexture(this->target);
		this->target = NULL;
		this->target = this->engine->GetWindow()->GetVideoDriver()->addRenderTargetTexture(irr::core::dimension2d<unsigned>(width, height));//, "RTT1");
	}
	this->sceneNode->setAspectRatio(float(width)/float(height));
}

Camera::Camera(Engine *engine, bool textured, unsigned w, unsigned h, irr::scene::ICameraSceneNode *cameraNode) {
	this->engine = engine;
	this->target = NULL;
	
	this->fovDegrees = 70.0f;
	
	this->sceneNode = cameraNode;
	if(textured)
		SetRenderTargetSize(w, h);
	
	this->position = btVector3(0, 0, 0);
	this->rotation = btVector3(0, 0, 0);
	this->parentTransformation = btTransform(btQuaternion(btVector3(0, 1, 0), 0), btVector3(0, 0, 0));
}

Camera::~Camera() {
	this->position = btVector3(0, 0, 0);
	this->rotation = btVector3(0, 0, 0);
	this->parentTransformation = btTransform(btQuaternion(btVector3(0, 1, 0), 0), btVector3(0, 0, 0));
}

#endif
