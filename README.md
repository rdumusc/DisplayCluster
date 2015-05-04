# DisplayCluster

A collaborative software for driving large display walls.

## Documentation

The documentation is available at
[bluebrain.github.io](http://bluebrain.github.io/)

## Features

DisplayCluster provides the following functionality:
* Interactively view media such as high-resolution imagery and video
* Receive content from remote sources such as laptops / desktops or
  high-performance remote visualization machines using the
  [Deflect library](https://github.com/BlueBrain/Deflect.git)

## Building from Source

```
  git clone https://github.com/BlueBrain/DisplayCluster.git
  mkdir DisplayCluster/build
  cd DisplayCluster/build
  cmake ..
  make
```

Or using Buildyard:

```
  git clone https://github.com/Eyescale/Buildyard.git
  cd Buildyard
  git clone https://github.com/BlueBrain/config.git config.bluebrain
  make DisplayCluster
```

## Original Project

This version of DisplayCluster is a fork of the original project by the Texas
Advanced Computing Center, Austin:

https://github.com/TACC/DisplayCluster
