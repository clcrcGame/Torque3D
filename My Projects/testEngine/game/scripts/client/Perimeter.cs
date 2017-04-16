function PerimeterGui::toggle(%this)
{
      if (%this.isAwake())
         Canvas.popDialog(%this);
      else
         Canvas.pushDialog(%this);
	 new Gui3DItem(FSCompass) {
			static = true;
		        dataBlock = "Compass";
			addX = "0.0";
			addY = "0.2";
			addZ = "1.35";
	   };
}
