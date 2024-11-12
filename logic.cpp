#include "logic.h"
#include "storage_element.h"
#include "logic_element.h"
#include "bird.h"
#include "bullet.h"
#include "points.h"
#include "effects.h"

// Forward Declared
int random(int min, int max);



/******************************************************************************
* LOGIC :: CONSTRUCTOR
******************************************************************************/
Logic::Logic(Interface* iface) : gun(storage.gun, Position(750, 750)), 
    time(storage.time), logStandard(), logFloater(), logSinker(),
    logCrazy(), logPellet(), logBomb(), logShrapnel(), logMissile(),
    logFragment(), logStreak(), logExhaust() 
{
	pInterface = iface;
}

/******************************************************************************
* LOGIC :: ADVANCE
******************************************************************************/
void Logic::advance()
{
    
    for (auto it = getElementsBegin(); it != getElementsEnd(); ++it){
        LogicElement * logicElement = (*it)->getLogicElementPtr();
        logicElement->advance(*it, storage);
    }
}

/******************************************************************************
* LOGIC :: SPAWN
******************************************************************************/
void Logic::spawn()
{
    
    double size;
    auto it = storage.beginBird();
    switch (time.level())
    {
       // in level 1 spawn big birds occasionally
       case 1:
          size = 30.0;
          // spawns when there is nothing on the screen
          if (*it == nullptr && random(0, 15) == 1)
              storage.add(new Standard(size, 7.0, 10, pInterface, this));
          
          
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
              storage.add(new Standard(size, 7.0, 10, pInterface, this));
          break;
          
       // two kinds of birds in level 2
       case 2:
          size = 25.0;
          // spawns when there is nothing on the screen
          if (*it == nullptr && random(0, 15) == 1)
            storage.add(new Standard(size, 7.0, 12, pInterface, this));

          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
            storage.add(new Standard(size, 5.0, 12, pInterface, this));
          // spawn every 3 seconds
          if (random(0, 3 * 30) == 1)
             storage.add(new Sinker(size, 4.5, 20 , pInterface, this));
          break;
       
       // three kinds of birds in level 3
       case 3:
          size = 20.0;
          // spawns when there is nothing on the screen
          if (*it == nullptr && random(0, 15) == 1)
              storage.add(new Standard(size, 5.0, 15, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
              storage.add(new Standard(size, 5.0, 15, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
              storage.add(new Sinker(size, 4.0, 22, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
              storage.add(new Floater(size, 5.0, 15, pInterface, this));
          break;
          
       // three kinds of birds in level 4
       case 4:
          size = 15.0;
          // spawns when there is nothing on the screen
          if (*it == nullptr && random(0, 15) == 1)
            storage.add(new Standard(size, 4.0, 18, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
            storage.add(new Standard(size, 4.0, 18, pInterface, this));
            // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
            storage.add(new Sinker(size, 3.5, 25, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
            storage.add(new Floater(size, 4.0, 25, pInterface, this));
          // spawn every 4 seconds
          if (random(0, 4 * 30) == 1)
            storage.add(new Crazy(size, 4.5, 30, pInterface, this));
          break;
          
       default:
          break;
    }
}

/******************************************************************************
* LOGIC :: INPUT
******************************************************************************/
void Logic::input(const UserInput & ui)
{
    // reset the game
    if (time.isGameOver() && ui.isSpace())
    {
       time.reset();
//       score.reset();
//       hitRatio.reset();
       return;
    }

    // gather input from the interface
    gun.interact(ui.isUp() + ui.isRight(), ui.isDown() + ui.isLeft());
    StorageBullet *p = nullptr;

    // a pellet can be shot at any time
    if (ui.isSpace())
       p = new Pellet(gun.getAngle(), 15.0, pInterface, this);
    // missiles can be shot at level 2 and higher
    else if (ui.isM() && time.level() > 1)
       p = new Missile(gun.getAngle(), 10.0, pInterface, this);
    // bombs can be shot at level 3 and higher
    else if (ui.isB() && time.level() > 2)
       p = new Bomb(gun.getAngle(), 10.0, pInterface, this);
    
    // NOT IMPLEMENTED
    //bullseye = ui.isShift();

    // add something if something has been added
    if (nullptr != p)
        storage.add(p);
   
    // MISSILE IS NOT IMPLEMENTED
    // send movement information to all the bullets. Only the missile cares.
    // for (auto bullet : bullets)
    //    bullet->input(ui.isUp() + ui.isRight(), ui.isDown() + ui.isLeft(), ui.isB());
}

/******************************************************************************
* LOGIC :: HIT DETECTION
******************************************************************************/
void Logic::hitDetection()
{
    // hit detection
    for (auto bird = storage.beginBird(); *bird != *storage.endBird(); ++bird)
       for (auto bullet = storage.beginBullet(); *bullet != *storage.endBullet(); ++bullet)
       {
           StorageElement * birdElement = *bird;
           StorageElement * bulletElement = *bullet;
           if (!birdElement->getIsDead() && !bulletElement->getIsDead() &&
               birdElement->getRadius() + bulletElement->getRadius() >
               minimumDistance(birdElement->getPosition(), birdElement->getVelocity(),
                               bulletElement->getPosition(), bulletElement->getVelocity()))
           {
              for (int i = 0; i < 25; i++)
                 storage.add(new Fragment(bulletElement->getPosition(), bulletElement->getVelocity(),pInterface, this));
               birdElement->getLogicElementPtr()->kill(birdElement);
               bulletElement->getLogicElementPtr()->kill(bulletElement);
               // NOT SURE HOW WE ARE USING THIS
               // hitRatio.adjust(1);
               //bulletElement->setValue(-(birdElement->getPoints()));
               bulletElement->getValue() = int(birdElement->getPoints());
               //birdElement->setPoints(0);
               birdElement->getPoints() = 0;
           }
       }
}

/******************************************************************************
* LOGIC :: REMOVE DEAD ELEMENTS
 ******************************************************************************/
void Logic::removeDeadElements()
{
    // remove the zombie birds
    for (auto it = storage.beginBird(); (*it) != *storage.endBird(); ++it)
       if ((*it)->getIsDead())
       {
          if ((*it)->getPoints())
             storage.add(new Points((*it)->getPosition(), (*it)->getPoints()));
          // ARE WE HANDLING SCORE?
          // score.adjust((*it)->getPoints());
          // Fixed Memory Leak
          delete (*it);
          // ^^^^^^^^^^^^^^^^^
          // NEEDS TO BE IMPLEMENTED
          // it = storage.erase(it);
       }
       else
          ++it;
        
    // remove zombie bullets
    for (auto it = storage.beginBullet(); (*it) != *storage.endBullet(); ++it )
       if ((*it)->getIsDead())
       {
           (*it)->getLogicElementPtr()->kill(*it);
          int value = -(*it)->getValue();
          storage.add(new Points((*it)->getPosition(), value));
//          score.adjust(value);
          // Fixed Memory Leak
          delete (*it);
          // ^^^^^^^^^^^^^^^^^
           // STORAGE ERASE IS NOT DEFINED
//          it = storage.erase(it);
       }
       else
          ++it;
    // NOT IMPLEMENTED
    //
//    // remove zombie fragments
//    for (auto it = effects.begin(); it != effects.end();)
//        if ((*it)->isDead()) {
//          // Fixed Memory Leak
//          delete (*it);
//          // ^^^^^^^^^^^^^^^^^
//          it = effects.erase(it);
//        }
//        else
//          ++it;
//
//    // remove expired points
//    for (auto it = points.begin(); it != points.end();)
//       if ((*it).isDead())
//          it = points.erase(it);
//       else
//          ++it;
}

/******************************************************************************
* LOGIC :: GET FRAMES LEFT
******************************************************************************/
int Logic::getFramesLeft()
{
	// TODO: Placeholder - Remove
	return 0;
}

/******************************************************************************
* LOGIC :: GET LEVEL NUMBER
******************************************************************************/
int Logic::getLevelNumber()
{
    return time.level();
}
/******************************************************************************
* LOGIC :: GET GUN ANGLE
******************************************************************************/
double Logic::getGunAngle()
{
    return gun.getAngle();
}

/******************************************************************************
* LOGIC :: IS PLAYING
******************************************************************************/
bool Logic::isPlaying()
{
    return time.isPlaying();
}

/******************************************************************************
* LOGIC :: IS GAME OVER
******************************************************************************/
bool Logic::isGameOver()
{
    return time.isGameOver();
}

/******************************************************************************
* LOGIC :: GET PERCENT LEFT
******************************************************************************/
double Logic::getPercentLeft()
{
    return time.percentLeft();
}

/******************************************************************
 * RANDOM
 * This function generates a random number.
 *
 *    INPUT:   min, max : The number of values (min <= num <= max)
 *    OUTPUT   <return> : Return the integer
 ****************************************************************/
int random(int min, int max)
{
   assert(min < max);
   int num = (rand() % (max - min)) + min;
   assert(min <= num && num <= max);

   return num;
}
