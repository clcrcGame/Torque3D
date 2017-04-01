function PerimeterGui::toggle(%this)
{
      if (%this.isAwake())
         Canvas.popDialog(%this);
      else
         Canvas.pushDialog(%this);
}
