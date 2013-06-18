Cinder-Warping
==============

A Cinder block that enables you to easily create editable bi-linear and perspective warps, or a combination of the two.


![Preview](https://raw.github.com/paulhoux/Cinder-Warping/master/PREVIEW.jpg)


Image warping, as used in this block, is the process of manipulating an image so that it can be projected onto flat or curved surfaces without distortion. There are three types of warps available:
* **Perspective warp**: performs image correction for projecting onto flat surfaces in case the projector is not horizontally and/or vertically aligned with the wall. Use it to exactly project your content on a rectangular area on a wall or building, regardless of how your projector is setup. For best results, disable any keystone correction on the projector. Then simply drag the four corners of your content where you want them to be on the wall.
* **Bilinear warp**: inferior to perspective warping on flat surfaces, but allows projecting onto curved surfaces. Simply add control points to the warp and drag them to where you want your content to be. 
* **Perspective-bilinear warp**: a combination of both techniques, where you first drag the four corners of your content to the desired location on the wall, then adjust for curvatures using the additional control points. If you (accidentally) move your projector later, all you need to do is adjust the four corners and the projection should perfectly fit on the curved wall again.


#####Adding this block to Cinder
This block is meant to be used with version 0.8.5 (or higher) of Cinder.

Cinder's tool for setting up empty projects, TinderBox, has been revamped and now supports a neat system for the management of plug-ins called Cinder Blocks. This Warping block supports this new feature. To add this block to your copy of Cinder, so it will be detected automatically by TinderBox:
* Open a command window (or Terminal)
* Switch to the disk containing the Cinder root folder, e.g.: ```d:```
* Browse to the Cinder root folder: ```cd path\to\cinder_master```
* Add the Warping block to the blocks folder: ```git clone https://github.com/paulhoux/Cinder-Warping.git blocks/Warping```

Alternatively, you can download the repository as a [ZIP-file](https://github.com/paulhoux/Cinder-Warping/zipball/master) and manually add the files to a "cinder_master\blocks\Warping" folder.



#####Dependency on OpenCV
This block relies on Cinder's optional OpenCV block,so make sure you've installed it in Cinder's block folder:
* Open a command window (or Terminal)
* Switch to the disk containing the Cinder root folder, e.g.: ```d:```
* Browse to the Cinder root folder: ```cd path\to\cinder_master```
* Add the OpenCV block to the blocks folder: ```git clone https://github.com/cinder/Cinder-OpenCV.git blocks/OpenCV```


#####Creating a sample application using Tinderbox
For more information on how to use the block with TinderBox, please follow this link:
https://forum.libcinder.org/#Topic/23286000001617049


#####To-Do's
* Support for call-backs or lambda's when iterating over all warps
* Out-of-the-box support for multiple windows
* Support for other warp types, e.g. WarpSpherical


#####Copyright notice
Copyright (c) 2010-2013, Paul Houx - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

Cinder-Warping is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 
Cinder-Warping is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
