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

#include "ConsoleSystem.hpp"
#include "RenderSystem.hpp"

#include "Simulation/Simulation.hpp"
#include "Simulation/Settings.hpp"

#include "Debug/DebugRenderHelper.hpp"

using namespace crimild;

ConsoleSystem::ConsoleSystem( void )
	: System( "Console System" )
{
    registerMessageHandler< messaging::DidRenderScene >( [ this ]( messaging::DidRenderScene const & ) {
        onDidRenderScene();
    });
}

ConsoleSystem::~ConsoleSystem( void )
{

}

bool ConsoleSystem::start( void )
{	
	if ( !System::start() ) {
		return false;
	}

    // the console is enabled ONLY if a valid system font is provided
    auto font = AssetManager::getInstance()->get< Font >( AssetManager::FONT_SYSTEM );
    Console::getInstance()->setEnabled( font != nullptr );

	return true;
}

void ConsoleSystem::stop( void )
{
	System::stop();
}

void ConsoleSystem::onDidRenderScene( void )
{
    auto renderer = Simulation::getInstance()->getRenderer();
    
    if ( renderer == nullptr ) {
        return;
    }

    auto console = getConsole();
    if ( console->isEnabled() && console->isActive() ) {
        auto output = console->getOutput( 30 );
        DebugRenderHelper::renderText( output, Vector3f( -0.95f, 0.95f, 0.0f ), RGBAColorf( 1.0f, 1.0f, 1.0f, 1.0f ) );
    }
}

