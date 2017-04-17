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

#include "platform/platform.h"
#include "T3D/gui3DItem.h"

#include "core/stream/bitStream.h"
#include "math/mMath.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "collision/boxConvex.h"
#include "collision/earlyOutPolyList.h"
#include "collision/extrudedPolyList.h"
#include "math/mPolyhedron.h"
#include "math/mathIO.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "ts/tsShapeInstance.h"
#include "console/engineAPI.h"

#include "T3D/gameBase/gameConnection.h"

const F32 sRotationSpeed = 6.0f;        // Secs/Rotation
const F32 sAtRestVelocity = 0.15f;      // Min speed after collision
const S32 sCollisionTimeout = 15;       // Timout value in ticks

// Client prediction
static F32 sMinWarpTicks = 0.5 ;        // Fraction of tick at which instant warp occures
static S32 sMaxWarpTicks = 3;           // Max warp duration in ticks

F32 Gui3DItem::mGravity = -20.0f;

const U32 sClientCollisionMask = (TerrainObjectType     |
                                  StaticShapeObjectType |
                                  VehicleObjectType     |  
                                  PlayerObjectType);

const U32 sServerCollisionMask = (sClientCollisionMask);

const S32 Gui3DItem::csmAtRestTimer = 64;

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(Gui3DItemData);

ConsoleDocClass( Gui3DItemData,
   "@brief Stores properties for an individual Gui3DItem type.\n\n"   

   "Gui3DItems represent an object in the world, usually one that the player will interact with.  "
   "One example is a health kit on the group that is automatically picked up when the player "
   "comes into contact with it.\n\n"

   "Gui3DItemData provides the common properties for a set of Gui3DItems.  These properties include a "
   "DTS or DAE model used to render the Gui3DItem in the world, its physical properties for when the "
   "Gui3DItem interacts with the world (such as being tossed by the player), and any lights that emit "
   "from the Gui3DItem.\n\n"

   "@tsexample\n"
	   "datablock Gui3DItemData(HealthKitSmall)\n"
      "{\n"
	   "   category =\"Health\";\n"
	   "   className = \"HealthPatch\";\n"
	   "   shapeFile = \"art/shapes/items/kit/healthkit.dts\";\n"
	   "   gravityMod = \"1.0\";\n"
	   "   mass = 2;\n"
	   "   friction = 1;\n"
	   "   elasticity = 0.3;\n"
      "   density = 2;\n"
	   "   drag = 0.5;\n"
	   "   maxVelocity = \"10.0\";\n"
	   "   emap = true;\n"
	   "   sticky = false;\n"
	   "   dynamicType = \"0\"\n;"
	   "   lightOnlyStatic = false;\n"
	   "   lightType = \"NoLight\";\n"
	   "   lightColor = \"1.0 1.0 1.0 1.0\";\n"
	   "   lightTime = 1000;\n"
	   "   lightRadius = 10.0;\n"
      "   simpleServerCollision = true;"
      "   // Dynamic properties used by the scripts\n\n"
      "   pickupName = \"a small health kit\";\n"
	   "   repairAmount = 50;\n"
	   "};\n"
   "@endtsexample\n"

   "@ingroup gameObjects\n"
);


Gui3DItemData::Gui3DItemData()
{
   shadowEnable = true;


   friction = 0;
   elasticity = 0;

   sticky = false;
   gravityMod = 1.0;
   maxVelocity = 25.0f;

   density = 2;
   drag = 0.5;

   lightOnlyStatic = false;
   lightType = Gui3DItem::NoLight;
   lightColor.set(1.f,1.f,1.f,1.f);
   lightTime = 1000;
   lightRadius = 10.f; 

   simpleServerCollision = true;
}

ImplementEnumType( Gui3DItemLightType,
   "@brief The type of light the Gui3DItem has\n\n"
   "@ingroup gameObjects\n\n")
   { Gui3DItem::NoLight,           "NoLight",        "The item has no light attached.\n" },
   { Gui3DItem::ConstantLight,     "ConstantLight",  "The item has a constantly emitting light attached.\n" },
   { Gui3DItem::PulsingLight,      "PulsingLight",   "The item has a pulsing light attached.\n" }
EndImplementEnumType;

void Gui3DItemData::initPersistFields()
{
   addField("friction",          TypeF32,       Offset(friction,           Gui3DItemData), "A floating-point value specifying how much velocity is lost to impact and sliding friction.");
   addField("elasticity",        TypeF32,       Offset(elasticity,         Gui3DItemData), "A floating-point value specifying how 'bouncy' this Gui3DItemData is.");
   addField("sticky",            TypeBool,      Offset(sticky,             Gui3DItemData), 
      "@brief If true, Gui3DItemData will 'stick' to any surface it collides with.\n\n"
      "When an item does stick to a surface, the Gui3DItem::onStickyCollision() callback is called.  The Gui3DItem has methods to retrieve "
      "the world position and normal the Gui3DItem is stuck to.\n"
      "@note Valid objects to stick to must be of StaticShapeObjectType.\n");
   addField("gravityMod",        TypeF32,       Offset(gravityMod,         Gui3DItemData), "Floating point value to multiply the existing gravity with, just for this Gui3DItemData.");
   addField("maxVelocity",       TypeF32,       Offset(maxVelocity,        Gui3DItemData), "Maximum velocity that this Gui3DItemData is able to move.");

   addField("lightType",         TYPEID< Gui3DItem::LightType >(),      Offset(lightType, Gui3DItemData), "Type of light to apply to this Gui3DItemData. Options are NoLight, ConstantLight, PulsingLight. Default is NoLight." );
   addField("lightColor",        TypeColorF,    Offset(lightColor,         Gui3DItemData),
      "@brief Color value to make this light. Example: \"1.0,1.0,1.0\"\n\n"
      "@see lightType\n");
   addField("lightTime",         TypeS32,       Offset(lightTime,          Gui3DItemData), 
      "@brief Time value for the light of this Gui3DItemData, used to control the pulse speed of the PulsingLight LightType.\n\n"
      "@see lightType\n");
   addField("lightRadius",       TypeF32,       Offset(lightRadius,        Gui3DItemData), 
      "@brief Distance from the center point of this Gui3DItemData for the light to affect\n\n"
      "@see lightType\n");
   addField("lightOnlyStatic",   TypeBool,      Offset(lightOnlyStatic,    Gui3DItemData), 
      "@brief If true, this Gui3DItemData will only cast a light if the Gui3DItem for this Gui3DItemData has a static value of true.\n\n"
      "@see lightType\n");

   addField("simpleServerCollision",   TypeBool,  Offset(simpleServerCollision,    Gui3DItemData), 
      "@brief Determines if only simple server-side collision will be used (for pick ups).\n\n"
      "If set to true then only simple, server-side collision detection will be used.  This is often the case "
      "if the item is used for a pick up object, such as ammo.  If set to false then a full collision volume "
      "will be used as defined by the shape.  The default is true.\n"
      "@note Only applies when using a physics library.\n"
      "@see TurretShape and ProximityMine for examples that should set this to false to allow them to be "
      "shot by projectiles.\n");

   Parent::initPersistFields();
}

void Gui3DItemData::packData(BitStream* stream)
{
   Parent::packData(stream);
}

void Gui3DItemData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(Gui3DItem);

ConsoleDocClass( Gui3DItem,
   "@brief Base Gui3DItem class. Uses the Gui3DItemData datablock for common properties.\n\n"   

   "Gui3DItems represent an object in the world, usually one that the player will interact with.  "
   "One example is a health kit on the group that is automatically picked up when the player "
   "comes into contact with it.\n\n"

   "@tsexample\n"
      "// This is the \"health patch\" dropped by a dying player.\n"
      "datablock Gui3DItemData(HealthKitPatch)\n"
      "{\n"
      "   // Mission editor category, this datablock will show up in the\n"
      "   // specified category under the \"shapes\" root category.\n"
      "   category = \"Health\";\n\n"
      "   className = \"HealthPatch\";\n\n"
      "   // Basic Gui3DItem properties\n"
      "   shapeFile = \"art/shapes/items/patch/healthpatch.dts\";\n"
      "   mass = 2;\n"
      "   friction = 1;\n"
      "   elasticity = 0.3;\n"
      "   emap = true;\n\n"
      "   // Dynamic properties used by the scripts\n"
      "   pickupName = \"a health patch\";\n"
      "   repairAmount = 50;\n"
      "};\n\n"

	   "%obj = new Gui3DItem()\n"
	   "{\n"
	   "	dataBlock = HealthKitSmall;\n"
	   "	parentGroup = EWCreatorWindow.objectGroup;\n"
	   "	static = true;\n"
	   "	rotate = true;\n"
	   "};\n"
	"@endtsexample\n\n"

   "@see Gui3DItemData\n"

   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( Gui3DItem, onStickyCollision, void, ( const char* objID ),( objID ),
   "@brief Informs the Gui3DItem object that it is now sticking to another object.\n\n"
   "This callback is only called if the Gui3DItemData::sticky property for this Gui3DItem is true.\n"
   "@param objID Object ID this Gui3DItem object.\n"
   "@note Server side only.\n"
   "@see Gui3DItem, Gui3DItemData\n"
);

IMPLEMENT_CALLBACK( Gui3DItem, onEnterLiquid, void, ( const char* objID, F32 waterCoverage, const char* liquidType ),( objID, waterCoverage, liquidType ),
   "Informs an Gui3DItem object that it has entered liquid, along with information about the liquid type.\n"
   "@param objID Object ID for this Gui3DItem object.\n"
   "@param waterCoverage How much coverage of water this Gui3DItem object has.\n"
   "@param liquidType The type of liquid that this Gui3DItem object has entered.\n"
   "@note Server side only.\n"
   "@see Gui3DItem, Gui3DItemData, WaterObject\n"
);

IMPLEMENT_CALLBACK( Gui3DItem, onLeaveLiquid, void, ( const char* objID, const char* liquidType ),( objID, liquidType ),
   "Informs an Gui3DItem object that it has left a liquid, along with information about the liquid type.\n"
   "@param objID Object ID for this Gui3DItem object.\n"
   "@param liquidType The type of liquid that this Gui3DItem object has left.\n"
   "@note Server side only.\n"
   "@see Gui3DItem, Gui3DItemData, WaterObject\n"
);


Gui3DItem::Gui3DItem()
{
   mTypeMask |= ItemObjectType | DynamicShapeObjectType;
   mDataBlock = 0;

   mVelocity = VectorF(0,0,0);
   mAtRest = true;
   mAtRestCounter = 0;
   mInLiquid = false;
   delta.warpTicks = 0;
   delta.dt = 1;
   mCollisionObject = 0;
   mCollisionTimeout = 0;
   mPhysicsRep = NULL;
   mConvex.init(this);
   mWorkingQueryBox.minExtents.set(-1e9, -1e9, -1e9);
   mWorkingQueryBox.maxExtents.set(-1e9, -1e9, -1e9);

   mLight = NULL;

   mSubclassItemHandlesScene = false;
}

Gui3DItem::~Gui3DItem()
{
   SAFE_DELETE(mLight);
}


//----------------------------------------------------------------------------

bool Gui3DItem::onAdd()
{


   if (!Parent::onAdd() || !mDataBlock)
      return false;

   mObjToWorld.getColumn(3,&delta.pos);

   if( !isHidden() && !mSubclassItemHandlesScene )
      addToScene();

   if (isServerObject())
   {
      if (!mSubclassItemHandlesScene)
         scriptOnAdd();
   }
   else if (mDataBlock->lightType != NoLight)
   {
      mDropTime = Sim::getCurrentTime();
   }

   return true;
}

void Gui3DItem::_updatePhysics()
{
   SAFE_DELETE( mPhysicsRep );
}

bool Gui3DItem::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<Gui3DItemData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
      return false;

   if (!mSubclassItemHandlesScene)
      scriptOnNewDataBlock();

   return true;
}

void Gui3DItem::onRemove()
{

   SAFE_DELETE( mPhysicsRep );

   if (!mSubclassItemHandlesScene)
   {
      scriptOnRemove();
      removeFromScene();
   }

   Parent::onRemove();
}

void Gui3DItem::onDeleteNotify( SimObject *obj )
{

}

// Lighting: -----------------------------------------------------------------

void Gui3DItem::registerLights(LightManager * lightManager, bool lightingScene)
{
   if(lightingScene)
      return;

   if(mDataBlock->lightOnlyStatic)
      return;

   F32 intensity;
   switch(mDataBlock->lightType)
   {
      case ConstantLight:
         intensity = mFadeVal;
         break;

      case PulsingLight:
      {
         S32 delta = Sim::getCurrentTime() - mDropTime;
         intensity = 0.5f + 0.5f * mSin(M_PI_F * F32(delta) / F32(mDataBlock->lightTime));
         intensity = 0.15f + intensity * 0.85f;
         intensity *= mFadeVal;  // fade out light on flags
         break;
      }

      default:
         return;
   }

   // Create a light if needed
   if (!mLight)
   {
      mLight = lightManager->createLightInfo();
   }   
   mLight->setColor( mDataBlock->lightColor * intensity );
   mLight->setType( LightInfo::Point );
   mLight->setRange( mDataBlock->lightRadius );
   mLight->setPosition( getBoxCenter() );

   lightManager->registerGlobalLight( mLight, this );
}


//----------------------------------------------------------------------------

Point3F Gui3DItem::getVelocity() const
{
   return mVelocity;
}

void Gui3DItem::setVelocity(const VectorF& vel)
{
   mVelocity = Point3F(0, 0, 0); 
}

void Gui3DItem::applyImpulse(const Point3F&,const VectorF& vec)
{

}

void Gui3DItem::setCollisionTimeout(ShapeBase* obj)
{

}


//----------------------------------------------------------------------------
#include <iostream>
void Gui3DItem::processTick(const Move* move)
{
   GameConnection* conn;
   MatrixF matrix;
   MatrixF rotMatrix;
   ShapeBase* control;
   Point3F tmp;
   VectorF tmp_vect;
   VectorF rot;

   if ( isServerObject() )
      return;

   conn = GameConnection::getConnectionToServer();

   Parent::processTick(move);

   if (!conn)
       return;

   control = dynamic_cast<ShapeBase*>(conn->getControlObject());

   if (!control || !(control->getTypeMask() & PlayerObjectType))  
      return;  

   //Move compass to the player
   matrix = control->getTransform();

   tmp = matrix.getPosition();

   matrix.setPosition(Point3F(0.0f, 0.0f, 0.0f));

   tmp_vect.x = addX;
   tmp_vect.y = addY;
   tmp_vect.z = addZ;

   rot.x = 1.0f;
   rot.y = 1.0f;
   rot.z = 1.0f;

   matrix.mulP(tmp_vect);
   matrix.mulP(rot);

   tmp.x += tmp_vect.x;
   tmp.y += tmp_vect.y;
   tmp.z += tmp_vect.z;

   matrix.setPosition(tmp);

   rotMatrix.set(EulerF(0.0, 0.0, 0.0));
   matrix.mul(rotMatrix);

   rotMatrix.set(EulerF(0.0, 0.0, mAtan2(rot.y, rot.x) - mDegToRad(45.0)));
   matrix.mul(rotMatrix);

   setTransform(matrix); 
 
   if (isServerObject())
      std::cout << "This is server object.\n";
   else
      std::cout << "This is client obj.\n";	 
}

void Gui3DItem::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
   if ( isMounted() )
      return;

   MatrixF mat = mRenderObjToWorld;
   setRenderTransform(mat);

}


//----------------------------------------------------------------------------

void Gui3DItem::setTransform(const MatrixF& mat)
{
   Parent::setTransform(mat);

   setMaskBits(RotationMask | PositionMask | NoWarpMask);
}


//----------------------------------------------------------------------------
void Gui3DItem::updateWorkingCollisionSet(const U32 mask, const F32 dt)
{
}

void Gui3DItem::updateVelocity(const F32 dt)
{
   mVelocity   = Point3F(0, 0, 0);
}


void Gui3DItem::updatePos(const U32 /*mask*/, const F32 dt)
{
}


//----------------------------------------------------------------------------

static MatrixF IMat(1);

bool Gui3DItem::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F&, const SphereF&)
{
      return false;
}


//----------------------------------------------------------------------------

U32 Gui3DItem::packUpdate(NetConnection *connection, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(connection,mask,stream);

   if (stream->writeFlag(mask & InitialUpdateMask)) {
      stream->writeRangedF32(addX, 0.0f, 10.0f, 10);
      stream->writeRangedF32(addY, 0.0f, 10.0f, 10);
      stream->writeRangedF32(addZ, 0.0f, 10.0f, 10);
   }

   return retMask;
}

void Gui3DItem::unpackUpdate(NetConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection,stream);

   if (stream->readFlag()) {
      addX = stream->readRangedF32(0.0f, 10.0f, 10);
      addY = stream->readRangedF32(0.0f, 10.0f, 10);
      addZ = stream->readRangedF32(0.0f, 10.0f, 10);
   }
}


//----------------------------------------------------------------------------

bool Gui3DItem::_setAddX(void *object, const char *index, const char *data)
{
   Gui3DItem *i = static_cast<Gui3DItem*>(object);
   i->addX = dAtof(data);
   i->setMaskBits(InitialUpdateMask | PositionMask);
   return true;
}

bool Gui3DItem::_setAddY(void *object, const char *index, const char *data)
{
   Gui3DItem *i = static_cast<Gui3DItem*>(object);
   i->addY = dAtof(data);
   i->setMaskBits(InitialUpdateMask | PositionMask);
   return true;
}

bool Gui3DItem::_setAddZ(void *object, const char *index, const char *data)
{
   Gui3DItem *i = static_cast<Gui3DItem*>(object);
   i->addZ = dAtof(data);
   i->setMaskBits(InitialUpdateMask | PositionMask);
   return true;
}

void Gui3DItem::initPersistFields()
{
   addGroup("Misc");	

   addProtectedField("addX", TypeF32, Offset(addX, Gui3DItem), &_setAddX, &defaultProtectedGetFn, "Sets X position of compass\n");
   addProtectedField("addY", TypeF32, Offset(addY, Gui3DItem), &_setAddY, &defaultProtectedGetFn, "Sets Y position of compass\n");
   addProtectedField("addZ", TypeF32, Offset(addZ, Gui3DItem), &_setAddZ, &defaultProtectedGetFn, "Sets Z position of compass\n");
   endGroup("Misc");

   Parent::initPersistFields();
}

void Gui3DItem::consoleInit()
{

}

//----------------------------------------------------------------------------

void Gui3DItem::prepRenderImage( SceneRenderState* state )
{
   Parent::prepRenderImage( state );
}

void Gui3DItem::buildConvex(const Box3F& box, Convex* convex)
{
	return;
}

void Gui3DItem::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

}