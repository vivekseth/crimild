/*
 * Copyright (c) 2013, Hernan Saez
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SkinnedMeshComponent.hpp"

#include "Rendering/SkinnedMesh.hpp"
#include "SceneGraph/Node.hpp"
#include "Visitors/Apply.hpp"
#include "Debug/DebugRenderHelper.hpp"

using namespace crimild;

SkinnedMeshComponent::SkinnedMeshComponent( void )
{
	setAnimationParams( 0.0f, -1.0f, true, 1.0f );
}

SkinnedMeshComponent::SkinnedMeshComponent( SharedPointer< SkinnedMesh > const &skinnedMesh )
	: ContainerComponent< SharedPointer< SkinnedMesh >>( skinnedMesh )
{
	setAnimationParams( 0.0f, -1.0f, true, 1.0f );
}

SkinnedMeshComponent::~SkinnedMeshComponent( void )
{

}

void SkinnedMeshComponent::start( void )
{
	ContainerComponent< SharedPointer< SkinnedMesh >>::start();

	_time = 0;
}

void SkinnedMeshComponent::update( const Clock &c )
{
	ContainerComponent< SharedPointer< SkinnedMesh >>::update( c );

	_time += c.getDeltaTime();

	auto mesh = get();
	
	auto skeleton = mesh->getSkeleton();
	
	auto animationState = mesh->getAnimationState();
	animationState->getJointPoses().resize( skeleton->getJoints().getJointCount() );

	auto currentClip = skeleton->getClips()[ _currentAnimation ];

	float firstFrame = _firstFrame;
	float lastFrame = _lastFrame >= 0.0f ? _lastFrame : currentClip->getDuration();
	bool loop = _loop;

	float timeInTicks = _time * _timeScale * currentClip->getFrameRate();
	float duration = lastFrame - firstFrame;
	float animationTime = firstFrame +  Numericf::clamp( fmod( timeInTicks, duration ), 0.0f, duration );

	if ( !loop && timeInTicks > ( lastFrame - firstFrame ) ) {
		animationTime = lastFrame;
	}

	getNode()->perform( Apply( [mesh, skeleton, currentClip, animationState, animationTime]( Node *node ) {

		Transformation modelTransform;

		if ( currentClip->getChannels().find( node->getName() ) ) {
			auto channel = currentClip->getChannels()[ node->getName() ];

			Transformation tTransform;
			channel->computePosition( animationTime, tTransform.translate() );

			Transformation rTransform;
			channel->computeRotation( animationTime, rTransform.rotate() );

			float scale;
			channel->computeScale( animationTime, scale );
			Transformation sTransform;
			sTransform.setScale( scale );

			modelTransform.computeFrom( rTransform, sTransform );
			modelTransform.computeFrom( tTransform, modelTransform );
		}
		else {
			modelTransform = node->getLocal();
		}
		
		// Transformation worldTransform = modelTransform;
		// worldTransform.computeFrom( node->getParent()->getWorld(), modelTransform );

		auto joint = skeleton->getJoints().find( node->getName() );
		if ( joint != nullptr ) {
			Transformation t;
			t.computeFrom( node->getParent()->getWorld(), modelTransform );
			t.computeFrom( t, joint->getOffset() );
			// t.computeFrom( skeleton->getGlobalInverseTransform(), t );
			animationState->getJointPoses()[ joint->getId() ] = t.computeModelMatrix();
		}

		// if ( false && self->_bones.boneMap.find( node->getName() ) != self->_bones.boneMap.end() ) {
		// 	unsigned int boneIndex = self->_bones.boneMap[ node->getName() ];
		// 	worldTransform.computeFrom( worldTransform, self->_bones.boneOffsets[ boneIndex ] );
		// 	worldTransform.computeFrom( self->getGlobalInverseTranspose(), worldTransform );
		// }

		// node->setWorld( worldTransform );
		node->setLocal( modelTransform );
		// node->setWorldIsCurrent( true );
	}));		
}

void SkinnedMeshComponent::setAnimationParams( float firstFrame, float lastFrame, bool loop, float timeScale )
{
	_firstFrame = firstFrame;
	_lastFrame = lastFrame;
	_loop = loop;
	_timeScale = timeScale;
}

void SkinnedMeshComponent::renderDebugInfo( Renderer *renderer, Camera *camera )
{
	std::vector< Vector3f > lines;
	auto self = this;
	getNode()->perform( Apply( [&lines, self]( Node *node ) {
		if ( node->hasParent() ) {
			// if ( self->getBones().boneMap.find( node->getName() ) != self->getBones().boneMap.end() ) {
				lines.push_back( node->getParent()->getWorld().getTranslate() );
				lines.push_back( node->getWorld().getTranslate() );
			// }
		}
	}));

	DebugRenderHelper::renderLines( renderer, camera, &lines[ 0 ], lines.size(), RGBAColorf( 1.0f, 0.0f, 0.0f, 1.0f ) );
}
