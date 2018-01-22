//
//  GFXMain.cpp
//  GoldsprintsFX
//
//  Created by Charlie Whitney on 8/27/14.
//
//

#include "app/GFXMain.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gfx;

GFXMain::GFXMain(){
    
}

GFXMain::~GFXMain(){
    CI_LOG_I("GFX main shutting down");
}

void GFXMain::setup(){
    // INIT --------------------------------------------------------------
    mModel = Model::getInstance();
    mGlobal = GFXGlobal::getInstance();
    
    mSerialReader = SerialReaderRef( new SerialReader() );
    mSerialReader->setup();
    
    mStateManager = StateManager::getInstance();
    
    // VIEWS --------------------------------------------------------------
    mNav			= std::make_shared<NavBarView>();
    mRaceView		= std::make_shared<RaceView>();
    mRosterView		= std::make_shared<RosterView>();
    mSettingsView	= std::make_shared<SettingsView>();
    
    mNav->setup();
    mRaceView->setup();
    mRosterView->setup();
    
    // EVENTS --------------------------------------------------------------
    getWindow()->getSignalKeyDown().connect(std::bind(&GFXMain::onKeyDown, this, std::placeholders::_1));
    
    mStateManager->signalOnRaceFinished.connect( bind( &GFXMain::onRaceFinished, this ) );
    
    mStateManager->signalOnStateChange.connect( bind( &GFXMain::onAppStateChaged, this, std::placeholders::_1 ) );
    mStateManager->signalOnRaceStateChange.connect( bind( &GFXMain::onRaceStateChanged, this, std::placeholders::_1 ) );
    
    mStateManager->signalRacerFinish.connect( [&](int _id, int _finishMillis, int _finishTicks){
        mModel->playerData[_id]->setFinished(_finishMillis, _finishTicks);
    });
    
    // START --------------------------------------------------------------
    mStateManager->changeAppState( APP_STATE::RACE, true );
    mStateManager->changeRaceState( RACE_STATE::RACE_STOPPED, true );
}

void GFXMain::reloadShaders()
{
    mRaceView->reloadShader();
}

void GFXMain::onRaceFinished() {
    console() << "GFXMAIN :: RACE FINSIHED" << endl;
    mSerialReader->stopRace();
    mStateManager->changeRaceState( RACE_STATE::RACE_COMPLETE );
}

void GFXMain::onAppStateChaged( APP_STATE as ) {
    
}

void GFXMain::onRaceStateChanged( RACE_STATE rc ){
    
    if( rc == RACE_STATE::RACE_STARTING ){
        if(GFXGlobal::getInstance()->currentRaceType == RACE_TYPE_DISTANCE ){
            mSerialReader->setRaceType(RACE_TYPE_DISTANCE);
            mSerialReader->setRaceLengthTicks( mModel->getRaceLengthTicks() );
        }else{
            mSerialReader->setRaceType(RACE_TYPE_TIME);
            mSerialReader->setRaceDuration( mModel->getRaceTimeSeconds() );
        }
        
		mModel->resetPlayers();
        mSerialReader->startRace();
    }
    
    else if( rc == RACE_STATE::RACE_STOPPED ){
        mSerialReader->stopRace();
     //   mModel->resetPlayers();
        mModel->elapsedRaceTimeMillis = 0.0;
    }
}

void GFXMain::onKeyDown(KeyEvent event)
{

    if(event.getCode() == KeyEvent::KEY_SPACE && StateManager::getInstance()->getCurrentAppState() == gfx::APP_STATE::RACE){
        if(StateManager::getInstance()->getCurrentRaceState() == gfx::RACE_STATE::RACE_STOPPED){
            mStateManager->changeRaceState( RACE_STATE::RACE_STARTING );
        }else{
            mStateManager->changeRaceState( RACE_STATE::RACE_STOPPED );
        }
    }
    
    if( !event.isAccelDown() && !event.isControlDown() ){
        return;
    }
    else if( event.getChar() == 'm'){
        mSerialReader->setMockMode();
    }
}

void GFXMain::resetPlayerData(){
    for( int i=0; i<mModel->playerData.size(); i++){
        mModel->playerData[i]->reset();
        mModel->playerData[i]->setRollerDiameter(mModel->getRollerDiameterMm());
    }
}

void GFXMain::update()
{
    double start_t, end_t;
    start_t = getElapsedSeconds();
    mSerialReader->update();
    end_t = getElapsedSeconds();
//    console() << "SerialReader update time :: " << start_t << " -- " << (end_t - start_t) << endl;
    
    mRaceView->update();
    mRosterView->update();
    mSettingsView->update();
}

void GFXMain::draw( const Rectf &drawRect ){
//    mGlobal->setScale(drawRect);
    
    mRaceView->draw();
    mRosterView->draw();
    mSettingsView->draw();
    
    mNav->draw();
}
