# Hammer

Build system based on Boost.Build v2 ideas, but written in C++.

## How to try it

1. Download release from Github.
1. Unpack it somewhere in a PATH.
1. [Create user-config.ham](#configuration)
1. [Setup libs warehouse.](#warehouse)
1. [Build examples.](#examples)
1. [Build hammer using hammer.](#Building Hammer by Hammer)

## Configuration
When you run hammer it will try to load configuration file placed at:

* **Linux**: $HOME/user-config.ham
* **Window**: %USERPROFILE%\\user-config.ham

In this config file you can configure your toolsets and [warehouse](#warehouse).
By default hammer will try to autoconfigure.
This means it will try to find usable toolsets at predefined paths and configure them for usage.
Currently it knows only **gcc** and **msvc** toolsets.

Autoconfiguration will try to do the following:

* on **Linux**: If /usr/bin/gcc exists configure gcc-system toolset.
* on **Windows**: search at:

   * C:\\Program Files\\Microsoft Visual Studio 11.0\\VC
   * C:\\Program Files\\Microsoft Visual Studio 12.0\\VC
   * C:\\Program Files\\Microsoft Visual Studio 14.0\\VC

and configure msvc-11.0/msvc-12.0/msvc-14.0 respectively.

If you have your toolset installed in some custom path you can show hammer where it is by writing one-liner in **user-config.ham**:

    use-toolset gcc : 10 : "/usr/local/bin/gcc-10" ;
or

    use-tooset msvc : 14.0 : "D:\\Development\\Microsoft Visual Studio 14.0\\VC" ;

## Warehouse

Warehouse is collection of libraries/packages that hammer can download and provide to developer similar to how other programming language package managers does.
To configure warehouse you should put this line into **user-config.ham**:

    setup-warehouse libs : "https://dl.bintray.com/darkangel-ua/hammer" ;

This line will add a batch of Boost libraries plus some others.
Because hammer uses external tools to download and unpack packages there is some additional dependencies you need to be able to use it:

* **Linux**: tar, bunzip2, curl - probably **every** Linux distributions has installed by default.
* **Windows**: Same here - tar.exe, bunzip2.exe and curl.exe. You need to download and put them (+dependencies) in a PATH. They can be downloaded from:
  * [cURL](https://curl.haxx.se/download.html#Win32) - you need version with SSL support.
  * [tar](http://gnuwin32.sourceforge.net/packages/gtar.htm)
  * [bzip2](http://gnuwin32.sourceforge.net/packages/bzip2.htm)

## Examples

The root folder has 'examples' directory where couple of simple examples placed.
Try to build them to verify that everything configured and you can actualy build something.
All build artifacts will be deep in '.hammer' folder.
Hammer uses MD5 hashes to ensure that different builds will not interfere with each others,
so path to final so/dll/lib/exe will be a bit cryptic and it begins with '.hammer'. On linux system it looks like hidden file,
so be aware.

One thing that you need to know is when compiling examples/use-lib-from-warehouse Hammer should ask about downloading
a batch of libs. Just press 'Y' and [Return] :)

## Building Hammer by Hammer

If you was able to build examples/use-lib-from-warehouse than it's time to build Hammer using Hammer :)