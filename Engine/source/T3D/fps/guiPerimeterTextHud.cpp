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

// ----------------------------------------------------------------------------  
// A gui control that displays health or energy information.  Based upon the   
// stock healthBar control but rewritten by M.Hall to display a numerical value.  
// ----------------------------------------------------------------------------  
  
#include "platform/platform.h"  
#include "gui/core/guiControl.h"  
#include "console/consoleTypes.h"  
#include "T3D/gameBase/gameConnection.h"  
#include "T3D/shapeBase.h"  
#include "gfx/gfxDrawUtil.h"  
  
class GuiPerimeterTextHud : public GuiControl  
{  
   typedef GuiControl Parent;  
  
   bool mShowFrame;  
   bool mShowFill;  
  
   ColorF mFillColor;  
   ColorF mFrameColor;  
   ColorF mTextColor;  
 
   F32 mValue;  
  
public:  
   GuiPerimeterTextHud();  
  
   void onRender(Point2I, const RectI &);  
   static void initPersistFields();  
   DECLARE_CONOBJECT(GuiPerimeterTextHud);  
   DECLARE_CATEGORY("Gui Game");  
   DECLARE_DESCRIPTION("Shows the perimeter, on which current\n"  
      "PlayerObjectType control object is looking at, as a numerical\n"
      "text display.");  
};  
  
// ----------------------------------------------------------------------------  
  
IMPLEMENT_CONOBJECT(GuiPerimeterTextHud);  
  
//TODO DocClass
ConsoleDocClass(GuiPerimeterTextHud,  
   "@brief Shows the perimeter of the current PlayerObjectType control object.\n\n"  
   "This control only works if a server connection exists and it's control object "  
   "is a PlayerObjectType. If either of these requirements is false, the control is not rendered.\n\n"  
  
   "@tsexample\n"  
      "\n new GuiPerimeterTextHud()"  
      "{\n"  
      "   fillColor = \"0.0 0.0 0.0 0.5\"; // Fills with a transparent black color\n"  
      "   frameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"  
      "   textColor = \"0.0 1.0 0.0 1.0\" // Solid green text color\n"  
      "   showFill = \"true\";\n"  
      "   showFrame = \"true\";\n"  
     "   profile = \"GuiBigTextProfile\";\n"  
      "};\n"  
   "@endtsexample\n\n"  
  
   "@ingroup GuiGame\n"  
);  
  
GuiPerimeterTextHud::GuiPerimeterTextHud()  
{  
   mShowFrame = mShowFill = true;  
  
   mFillColor.set(0, 0, 0, 0.5);  
   mFrameColor.set(1, 1, 1, 1);  
   mTextColor.set(0, 1, 0, 1);  
}  
  
void GuiPerimeterTextHud::initPersistFields()  
{  
   addGroup("Colors");       
   addField("fillColor", TypeColorF, Offset(mFillColor, GuiPerimeterTextHud), "Color for the background of the control.");  
   addField("frameColor", TypeColorF, Offset(mFrameColor, GuiPerimeterTextHud), "Color for the control's frame.");  
   addField("textColor", TypeColorF, Offset(mTextColor, GuiPerimeterTextHud), "Color for the text on this control.");  
   endGroup("Colors");          
  
   addGroup("View");      
   addField("showFill", TypeBool, Offset(mShowFill, GuiPerimeterTextHud), "If true, draw the background.");  
   addField("showFrame", TypeBool, Offset(mShowFrame, GuiPerimeterTextHud), "If true, draw the frame.");  
   endGroup("View");    
    
   Parent::initPersistFields();  
}  
  
// ----------------------------------------------------------------------------  

#include <iostream>
 
void GuiPerimeterTextHud::onRender(Point2I offset, const RectI &updateRect)  
{ 
   VectorF vector;
   MatrixF matrix;
   int deg;
   // Must have a connection and player control object  
   GameConnection* conn = GameConnection::getConnectionToServer();
 
   if (!conn)  
      return;  

   ShapeBase* control = dynamic_cast<ShapeBase*>(conn->getControlObject());  

   if (!control || !(control->getTypeMask() & PlayerObjectType))  
      return;  
  
   GFXDrawUtil* drawUtil = GFX->getDrawUtil();
  
   // If enabled draw background first  
   if (mShowFill)  
      drawUtil->drawRectFill(updateRect, mFillColor);  
   
   matrix = control->getTransform();

   matrix.setPosition(Point3F(0.0f, 0.0f, 0.0f));

   vector.x = 1.0f;
   vector.y = 1.0f;
   vector.z = 0.0f;
 
   matrix.mulP(vector); 
  
   deg = ((int)(mRadToDeg(mAtan2(vector.x, vector.y)) + 0.5f) + 180 + 135) % 360;

   std::cout << "x = " << vector.x << "; y = " << vector.y << std::endl;

   char buf[4];
   dSprintf(buf, sizeof(buf), "%d", deg);
 
   offset.x += (getBounds().extent.x - mProfile->mFont->getStrWidth((const UTF8 *)buf)) / 2;    
   offset.y += (getBounds().extent.y - mProfile->mFont->getHeight()) / 2;    
  
   ColorF tColor = mTextColor;   
    
   drawUtil->setBitmapModulation(tColor);    
   drawUtil->drawText(mProfile->mFont, offset, buf);    
   drawUtil->clearBitmapModulation();    
  
   // If enabled draw the border last  
  if (mShowFrame)  
	drawUtil->drawRect(updateRect, mFrameColor);  
}  
