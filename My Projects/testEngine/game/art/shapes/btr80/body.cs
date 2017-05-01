
singleton TSShapeConstructor(BodyDae)
{
   baseShape = "./body.dae";
   loadLights = "0";
   neverImport = "EnvironmentAmbientLightEnvironmentAmbientLight";
   lodType = "TrailingNumber";
};

function BodyDae::onLoad(%this)
{
   %this.setNodeTransform("hub4", "1.88447 -3 0.431915 -0.000138756 0.707209 0.707004 3.14114", "1");
   %this.setNodeTransform("bn_ShockRearRight", "1.38494 -3 0.581919 -0.295615 -0.514654 0.804825 1.93877", "1");
   %this.setNodeParent("Mesh35", "Armature");
   %this.setNodeParent("Mesh36", "Armature");
   %this.setNodeParent("Mesh34", "Armature");
   %this.setNodeParent("Mesh33", "Armature");
   %this.setNodeParent("Mesh32", "Armature");
   %this.setNodeParent("Mesh31", "Armature");
   %this.setNodeParent("Mesh30", "Armature");
   %this.setNodeParent("Mesh29", "Armature");
   %this.setNodeParent("Mesh28", "Armature");
}
