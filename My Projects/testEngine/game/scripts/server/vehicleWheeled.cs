//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// This file contains script methods unique to the WheeledVehicle class.  All
// other necessary methods are contained in "../server/scripts/vehicle.cs" in
// which the "generic" Vehicle class methods that are shared by all vehicles,
// (flying, hover, and wheeled) can be found.

// Parenting is in place for WheeledVehicleData to VehicleData.  This should
// make it easier for people to simply drop in new (generic) vehicles.  All that
// the user needs to create is a set of datablocks for the new wheeled vehicle
// to use.  This means that no (or little) scripting should be necessary.

function WheeledVehicleData::onAdd(%this, %obj)
{
   Parent::onAdd(%this, %obj);

   // Setup the car with some tires & springs
   for (%i = %obj.getWheelCount() - 1; %i >= 0; %i--)
   {
      %obj.setWheelTire(%i, DefaultCarTire);
      %obj.setWheelSpring(%i, DefaultCarSpring);
      %obj.setWheelPowered(%i, false);
   }

   // Steer with the front tires
   %obj.setWheelSteering(0, 1);
   %obj.setWheelSteering(1, 1);

   // Only power the two rear wheels... assuming there are only 4 wheels.
   %obj.setWheelPowered(2, true);
   %obj.setWheelPowered(3, true);
}

function WheeledVehicleData::onCollision(%this, %obj, %col, %vec, %speed)
{
   // Collision with other objects, including items
}

// Used to kick the players out of the car that your crosshair is over
function serverCmdcarUnmountObj(%client, %obj)
{
   %obj.unmount();
   %obj.setControlObject(%obj);

   %ejectpos = %obj.getPosition();
   %ejectpos = VectorAdd(%ejectpos, "0 0 5");
   %obj.setTransform(%ejectpos);

   %ejectvel = %obj.mVehicle.getVelocity();
   %ejectvel = VectorAdd(%ejectvel, "0 0 10");
   %ejectvel = VectorScale(%ejectvel, %obj.getDataBlock().mass);
   %obj.applyImpulse(%ejectpos, %ejectvel);
}

// Used to flip the car over if it manages to get stuck upside down
function serverCmdflipCar(%client)
{
   %car = %client.player.getControlObject();

   if (%car.getClassName() $= "WheeledVehicle")
   {
      %carPos = %car.getPosition();
      %carPos = VectorAdd(%carPos, "0 0 3");

      %car.setTransform(%carPos SPC "0 0 1 0");
   }
}

function serverCmdsetPlayerControl(%client)
{
     %client.setControlObject(%client.player);
}

function serverCmddismountVehicle(%client)
{
   %car = %client.player.getControlObject();   
   %passenger = %car.getMountNodeObject(0);
   %car.unmountObject(%passenger);
   %passenger.mVehicle = "";

 
   %car_transform = %car.getTransform();
   %exit_pos = %car.getExitPosition("LeftUpperDoorMesh2");
   
   //This rotates exit_pos.
   %exit_pos = MatrixMulVector(%car_transform, %exit_pos);
   //And this sets proper position.
   %exit_pos = VectorAdd(%exit_pos, %car_transform); 
   //And additional height to drop point because floor/collision detection may
   //fail.
   %exit_pos = VectorAdd(%exit_pos, "0 0 2");

   %passenger.setTransform(%exit_pos);
}

function serverCmdmountVehicle(%client)
{
   %player = %client.player;
   %obj = %player.getPointedObject();
   %point = %player.getPointedPoint();//TODO: return RayInfo?
   // Mount vehicles

   echo("1");
   if (%obj.getType() & $TypeMasks::GameBaseObjectType)
   {
      %db = %obj.getDataBlock();
      echo("2");
      if ((%db.getClassName() $= "WheeledVehicleData") && %obj.mountable &&
	  %player.mountVehicle)
      {	
         echo(%point);
         echo("3");
	 %car_inverse_transform = %obj.getInverseTransform();
	 %point = MatrixMulVector(%car_inverse_transform, %point);
         %point = VectorAdd(%point, %car_inverse_transform);

         echo(%point);

	 if (%db.pointLocatedInDoorBounding(%point)) { 
            echo("4");
            // Only mount drivers for now.
            ServerConnection.setFirstPerson(0);
            
            // For this specific example, only one person can fit
            // into a vehicle
            %mount = %obj.getMountNodeObject(0);         
            if(%mount)
               return;
            
            // For this specific FPS Example, always mount the player
            // to node 0
            %node = 0;
            %obj.mountObject(%player, %node);
            %player.mVehicle = %obj;
         }
      }
   }
}
