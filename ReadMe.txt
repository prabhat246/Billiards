
********************************************************************************************************************
					Assignment 2
	 		     CSL:859 Advanced Computer Graphics

				Simulation of Billards Game

				By       : Prabhat Kumar Gupta
				Entry no : 2007 MCS 2895

						&
				By       : Nidhi Arora
				Entry no : 2007 MCS 2913

			It Requires OpenGL and Cg to compile and execute.
********************************************************************************************************************

This folder Includes a Visual studio Project 'Billiards.dsp'. which can also be accessed through workspace
'Billiards.dsw'. 

The source code Files are.

1. Billiards.c : Sourcecode that contains main function. All the features are implemented in this code including 
		 rendering of Table, balls, stick, with mouse and keyboard control. Keyboard control are as
		 follows;
		-----------------------------------------------
		Control:		Action
		-----------------------------------------------
		Spacebar	Enable Stick and Strike
		zRotate 	Stick Left
		xRotate 	Stick Right
		n		Zoom In
		m		Zoom Out
		1Camera 	1
		2Camera 	2
		3Camera 	3
		4Camera 	4
		-----------------------------------------------
		Left Click	Enable Stick and Strike
		Right Click	Menu Bar
		-----------------------------------------------


2. Cg_Matrix_Operations.h	
		 This file contains all required matrix operations to perform transformations. It also includes 
		 functions to implement viewspace.

3. FragmentLightening.cg
		 It is the fragment Program.

4. PhongsLightening.cg
		 This implements Phong's Lightening Model.

********************************************************************************************************************
Other files

5. Billiards.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. 

6. Billiards.cpp
    This is the main application source file.

********************************************************************************************************************

Other standard files:

7. StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Billiards.pch and a precompiled types file named StdAfx.obj.

********************************************************************************************************************
