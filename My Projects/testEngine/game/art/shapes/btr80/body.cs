
singleton TSShapeConstructor(BodyDae)
{
   baseShape = "./body.dae";
   loadLights = "0";
};

function BodyDae::onLoad(%this)
{
   %this.setNodeTransform("hub4", "1.88447 -3 0.431915 -0.000138756 0.707209 0.707004 3.14114", "1");
   %this.setNodeTransform("bn_ShockRearRight", "1.38494 -3 0.581919 -0.295615 -0.514654 0.804825 1.93877", "1");
}
