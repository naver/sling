## Sling
Sling is an open source Web content engine for browsers and other applications.
It is forked from WebKit (r203260).

Our goal is to be a lightweight web content engine available through all major platforms and IOT devices.
![goals](https://cloud.githubusercontent.com/assets/2087774/20997226/f1f71768-bd46-11e6-9c1c-d9ffb3db63cf.PNG)

The following markups are supported.

* HTML5
* Javascript
* CSS3
* XML
* WebGL

The following platforms are supported.

* Android
* Windows

The special features

* Support chrome extension (developing)
* Support DWrite font Rendering on Windows  
* Support SPDY and QUIC
* Support GFX Tool for Graphics Debugging

## Demo
* Graphics Benchmark : Chrome vs Sling
https://youtu.be/IyQWcJd52pA
 
* JavaScript Benchmark : Chrome vs Sling
https://youtu.be/LfL8H-7QgKk
 
* Web Page Navigation Performance : Chrome vs Sling
https://youtu.be/rb4BqhWF6Wo
 
* GFX Tool for Sling
https://youtu.be/8QlZQV3dvUs

## Latest Release (2016/12/08)
[**Downloads MiniBrowser-Sling-win-x64**](https://github.com/naver/sling/blob/master/release/release-20161206.zip)<br/>
In preparation for our next release, we have added a release android. 

## Schedule
* All features have already implemented. But we need time to refactor them for opening codes. Please wait~!!
![schedule](https://cloud.githubusercontent.com/assets/2087774/20997227/f6a45c4e-bd46-11e6-9226-46fd3827b5be.PNG)

## Building

**Step-1 : Installing Development Tool**
Reference the Installing Development Tools section below.

https://webkit.org/webkit-on-windows/#installing-developer-tools

**Step-2 : Generating Visual Studio Solution files**

* **using command-line**
```
mkdir WebKitBuild
cd WebKitBuild
cmake -G "Visual Studio 14 2015 Win64" -D PORT=WinCairo ..
```

* **using CMake-gui**
```
3-1) Select sling/webkit directory in "Where is the source code" field.
     Input sling/webkt/WebKitBuild directory name in "Where to build the binaries" field.
3-2) Modify PORT name to "WinCairo".
3-3) Click Configure button.
3-4) If the configure is done, Click Generate button.
```
![cmake-gui](https://cloud.githubusercontent.com/assets/2087774/20919413/08e30d1a-bbdf-11e6-9ed8-37b2a127e5df.png)


**Step-3 : Build**
- Open webkit.sln in WebKitBuild
- Build Solution x64 Debug/Release.

**Step-4 : Launch MiniBrowser**
- Opent WebKitBuild/bin64/MiniBrowser.exe

## Contributing
We are always thrilled to receive pull requests, and do our best to process them as fast as possible.
