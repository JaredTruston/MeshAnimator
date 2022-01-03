# MeshAnimator
Project created in C++ with openFrameworks library. Allows users to create animatable skeleton that 3d meshes (in the form of .obj files) can be mapped onto.

User can create joints of the skeleton with the press of the 'J' key. Joints can be clicked on to be selected. Adding a new joint while one is selected creates
a hierarchy. The new joint will be a child of the selected joint and a cone is drawn between the joints to represent a bone. While a child joint is selected, a 
mesh (obj file) can be dragged into the scene where it will be mapped to the bone between both the parent and child joint. While a joint is selected, if the user
holds down on the left mouse button and drags the mouse across the screen, the position of the joint will be transformed. If the user holds the left mouse button
and drags the mouse while also holding down on the 'X', 'Y', or 'Z' keys, then the joint will rotate about the respective axis. Any transformations (including both
position translation and rotation) will be applied to child joints within the hierachy of the selected joints as well as any of the mapped 3d meshes.
