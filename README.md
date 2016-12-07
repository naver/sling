# Sling Webkit
------------

Sling WebKit is one of the WebKit (r203260) based rendering engine for multi platform.

The following markups are supported.

* HTML5
* Javascript
* CSS3
* XML
* WebGL

The following platforms are supported.

* Android
* Windows

# Demo
-----------
* Graphics Benchmark : Chrome vs Sling
https://youtu.be/IyQWcJd52pA
 
* JavaScript Benchmark : Chrome vs Sling
https://youtu.be/LfL8H-7QgKk
 
* Web Page Navigation Performance : Chrome vs Sling
https://youtu.be/h0pWlxH2QNo
 
* GFX Tool for Sling
https://youtu.be/8QlZQV3dvUs

# Building
-----------

## Step-1 : Installing Development Tool
Reference the Installing Development Tools section below.

https://webkit.org/webkit-on-windows/#installing-developer-tools

## Step-2 : Setup the Git Repository
```
git config --global user.name "John Smith"
```
```
git config --global user.email "johnsmith@example.com"
```
```
git clone https://github.com/naver/sling
cd webkit
```

## Step-3 : Generation Visual Studio Solution files

### using command-line
```
mkdir WebKitBuild
cd WebKitBuild
cmake -G "Visual Studio 14 2015 Win64" -D PORT=WinCairo ..
```
or

### using CMake-gui
```
3-1) Select sling/webkit directory in "Where is the source code" field.
3-2) Input sling/webkt/WebKitBuild directory name in "Where to build the binaries" field.
3-3) Modify PORT name to "WinCairo".
3-4) Click Configure button.
3-5) If the configure is done, Click Generate button.
```
![cmake-gui](https://cloud.githubusercontent.com/assets/2087774/20919413/08e30d1a-bbdf-11e6-9ed8-37b2a127e5df.png)


## Step-4 : Build
- Open webkit.sln in WebKitBuild
- Build Solution x64 Debug/Release.

## Step-5 : Launch MiniBrowser
- Opent WebKitBuild/bin64/MiniBrowser.exe

# Contributing
------------
1. Clone this.
2. Create a branch (`git checkout -b my_markup`)
3. Commit your changes (`git commit -am "Added Snarkdown"`)
4. Push to the branch (`git push origin my_markup`)
5. Open a [Pull Request][1]
6. Enjoy a refreshing Diet Coke and wait
