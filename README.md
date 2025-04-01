# Authors:\
-Paweł Deorowicz / chess engine creator.  
-Stanisław Laszczak / graphics designer.  
-Bartłomieł Kominek / networking coder.  
-Konrad Bartoszewicz / bug tester.  
-Jeremiasz Gorawski / bug tester.  


# Known Issues:  
-our program works with Visual Studio 2022  
-our program does not work with Visual Studio Code  
-our program uses Standard ISO C++20 (/std:c++20)  
-while creating std.cpp you compile it more than once it could not work (that does not mean you need to delete it)  
-if there will be problems with libraries we suggest to reinstal them and possibly redo the project  
-if the program calls abort it is most likely the .png files not loalding error

# Instructions to build part/1.  
After creating your project:  
-in your project files (C:your/file/path) create a folder:Libraries.  
-in which we will create two more folders:  
-include and lib.  

# Download opengl:  
to avoid any problems while including glad and GLFW3 libraries it is advised to download:  
-(https://github.com/Kitware/CMake/releases/download/v4.0.0/cmake-4.0.0-windows-x86_64.msi) for Windows x64.  
-(https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip)  
-(https://glad.dav1d.de/) API gl version 4.5 Profle core.  
-extract glfw-3.4 folder form glfw-3.4.zip.  
 ## Run cmake:  
  -use the glfw-3.4 folder as a source code.  
  -next we need a build path which we will make ourselfs.  
  -click on Browse Build when a window shows up open glfw-3.4 folder:  
   -in it create a new folder called build and now select that folder as a build path.  
  -click on configure and wait for it to finish (it should not take long)  
  -now when another window showed up use default settings and click finish.  
  -new settings should pop up on cmake and you still want to use deafult settings as your choise.  
  -click on configure than generate and wait fo it to finish.  
 ## In Visual Studio and files
 -go to C:path/to/your/folder/glfw-3.4/build folder next and from it open GLFW.sln in Visual Studio.  
 -in Visual Studio Solution Sxplorer right click on your GLFW solution file and select Build Solution or click f7.  
 -open glfw folder than src and debug.  
 -copy glfw3.lib file in to your lib folder 
 -open glfw folder than include.  
 -copy GLFW in to your include folder  
 -open glad.zip folder than include.  
 -copy glad and KHR folders in to youe include folder  
 -open glad.zip folder than src.  
 -copy glad.c in to your main folder  
 -in visual studio go to project -> properties -> Configuration Properties -> VC++ Directories  
 -open "Include Directories" setting and add your include folder  
 -open "Libraly Directories" setting and add your lib folder  
 -in visual studio go to project -> properties -> Configuration Properties -> Linker -> input  
 -open "Additional Dependencies" setting and type "glfw3.lib" and in the next line "opengl32.lib"  
 -drag your glad.c file in to your source files in visual studio  
    
 
# Download boost:  
-(https://archives.boost.io/release/1.87.0/source/boost_1_87_0.zip)  
-click the link and let it download  
-when it downloads unzip it in any directory  
-copy the path of your unziped file ex. C:\path\to\your\file\boost_1_87_0  
-now in visual studio go to project -> properties -> Configuration Properties -> C/C++ -> General  
- open "Addicional Header Files" setting and add there the path that you copied earlier


# Instructions to build part/2.  
-firstly to your project folder add all of the .cpp .h .frag .vert and .png files  
-than add all of the .cpp files to visual studio source files  
-add all of the .h files to visual studio header files  

# Graphic:
-in visual studio solution explorer look for "Resource Files"  
-there create two new filters called "Shaders" and "Textures"  
-in "Shaders" add two existing files called "default.vert" and "default.frag"  
-and in the "Textures" one add all of the .png files  

 



