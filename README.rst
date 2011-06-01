====
grav
====

`grav` is a GL-based application for receiving & displaying many videos,
using RTP. Primarily intended for videoconferencing/telepresence/etc
applications, all videos are displayed in a canvas-style window, with a
globe displayed in the background for geographical correlation, as well
as automatic grouping, both based on RTCP metadata.

.. important::

    For more info, see the grav trac instance at https://grav.rc.rit.edu.  This
    README file is the 'authoritative source' for documentation [1]_, but the
    trac instance is where to go for filing/finding bug tickets/issues.

Compilation
===========

Compiling on Linux
------------------

1. Make sure to get all the dependencies (see below). Here's an example for
   Ubuntu or Debian-style Linux distros::

      apt-get install git-core scons subversion cmake ffmpeg libpng12-dev \
           libpng12-0 wx2.8-headers libwxgtk2.8-0 libwxgtk2.8-dev libftgl-dev \
           libftgl2 python-dev libglu1-mesa libglu1-mesa-dev  libglew1.5-dev \
           libglew1.5 libavcodec-dev libswscale-dev libavutil-dev \
           libpthread-stubs0-dev

   Or on a redhat based distro::

      yum install git scons subversion cmake libpng-devel libpng wxGTK-devel \
           wxGTK ftgl-devel ftgl python-devel mesa-libGLU-devel mesa-libGLU \
           glew-devel glew

   Note that Fedora doesn't ship with ffmpeg, which greatly enhances codec support.
   You can either build it from source as is detailed below in step 2, or you can
   do the following to acquire it from ``rpmfusion.org``::

      yum localinstall --nogpgcheck \
           http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-rawhide.noarch.rpm \
           http://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-rawhide.noarch.rpm
      yum install ffmpeg ffmpeg-devel

2. **IF** you need to compile ffmpeg (OPTIONAL - you can get ffmpeg from a
   package but note you need the newer .52 libavcodec version)...

   1. Get ``ffmpeg 0.5`` from `ffmpeg's website <http://ffmpeg.org>`_,
      do::

         ./configure --enable-gpl --enable-postproc --enable-swscale \
              --enable-pthreads --enable-debug --enable-shared

      then compile & sudo make install (make install will probably install
      to ``/usr/local/lib`` by default).  Note that if you also have a system
      copy of ffmpeg installed, there may be conflicts.  See [2]_.

3. SVN checkout the UCL common library from here:
   https://mediatools.cs.ucl.ac.uk/repos/mmedia e.g.::

        MMEDIA_URL=https://mediatools.cs.ucl.ac.uk/repos/mmedia/common/trunk
        svn co $MMEDIA_URL ucl-common

   1. In the common lib directory, do a standard::

        ./configure
        sudo make install

   2. Or, if you don't want to install (will go to ``/usr/local/lib`` by
      default) just do make and copy the .a file to the eventual `grav` build
      directory or somewhere CMake can find it.

4. SVN checkout VPMedia from here:  https://svn.ci.uchicago.edu/svn/if-media/::

        svn co https://svn.ci.uchicago.edu/svn/if-media/VPMedia VPMedia

   1. In the VPMedia directory, do::

        scons configure
        scons
        sudo scons install

   2. If you don't want to install this, follow the same instructions for the
      common lib, except with scons to build.

5. Obtain `grav` Source

   1. If you want the `grav` source in order to **install and use** `grav`, get
      a snapshot tarball from https://github.com/adhesion/grav/tarball/master::

       TAR_URL=http://github.com/adhesion/grav/tarball/master
       wget --no-check-certificate -O grav-latest.tar.gz $TAR_URL
       tar -xzvf grav-latest.tar.gz

   2. **OR**, if you want the `grav` source in order to **contribute to or
      develop** `grav`, fork and clone the upstream repository located at
      http://github.com/adhesion/grav.  You may want to read the `general github
      documentation <http://help.github.com/>`_ and their `documentation on
      forking projects <http://help.github.com/fork-a-repo/>`_ in particular.

6. Cmake `grav`

   1. Start the CMake GUI (cmake-gui)

      1. Select target (most likely Unix makefiles for Linux), point its
         source directory to the top-level `grav` directory (ie, the one
         that has ``include/``, ``src/`` and ``CMakeLists.txt``), and point the
         build directory to a new directory (for ease of use), ie
         ``mkdir Build/`` and point CMake's build dir there.
      2. Click configure, which will tell you if any dependencies
         are missing (on Linux, lots of the automatically found X11
         libs are optional, so don't worry if those are missing), and
         allow you to change any build settings - it is recommended to
         enter a build type into ``CMAKE_BUILD_TYPE`` so that various
         cflags get set sanely (ie, to enable -g debug symbols and #def
         for enabling debug keys for debug, or -O optimizations for
         release). The supported build types are ``DEBUG``,
         ``RELWITHDEBINFO``, ``RELEASE``, and ``MINSIZEREL``. Click
         configure again to solidify settings and click generate to
         generate Makefiles.
      3. Go to the build directory you specified and run make.
   2. Or run Cmake in a Build Directory like so. Note build variables can
      be set on the command line in cmake::

       cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG \
                -DCMAKE_VERBOSE_MAKEFILE=True /path/to/gravroot
       make

7. To run `grav`, run from the top level dir so it can find
   the resources there (ie, if your build dir is ``Build/``, run
   ``Build/grav [options]``), or, you can do a ``make install``
   which will copy the resources and binary to the location
   specified by the ``CMAKE_INSTALL_PREFIX``.

Dependencies
------------

Keep in mind, for the library dependencies you'll need the -dev
package (headers) as well as the regular runtime library.

Tools:

    * Subversion
    * CMake
    * Scons preferably version 1.2 or later, not sure if older
      versions will work
    * git (git-core)

For VPMedia:

    * UCL common lib (UCL version recommended, if-media version also
      works but is not actively developed)
    * ffmpeg: libavcodec.52, libswscale (see above - make sure you have
      the .52 version of libavcodec or you'll need to compile yourself)
    * libpthread
    * libdv (for DV support, which is not really working/supported
      at the moment in `grav`. essentially optional)
    * libtheora (optional)

From `grav`:

    * GL
    * GLU
    * GLEW:

      - Note this means ~1.5 or later, probably 1.5.8. 1.3.5 (provided
        version on CentOS 5.5, for example) does not prevent GLX from
        getting included afterwards, which causes a conflict with
        wxGLCanvas. Therefore, if you don't have GLEW ~1.5 or above in
        your repositories, you'll have to compile it from source - get
        a source tarball from here:  http://glew.sourceforge.net/ and
        compiling it should be a simple make install.
      - Ubuntu 10.10 also has this issue, glx.h from both mesa and nvidia
        have a different #define which screws up the include prevention
        that GLEW 1.5.2 does in glxew.h. Solution is the same, get
        GLEW 1.5.8 and compile & install.

    * FTGL (in turn requires libfreetype)
    * libpng
    * wx 2.8 (including OpenGL support)
    * python 2.6 (for AG venue client control, could be optional
      but isn't optional in the build process yet)

Usage
=====

Command Line Usage
------------------
::

  Usage: grav [-h] [-vr] [-v] [-vpv] [-t] [-nt] [-np] [-es] [-bf] [-ht <str>] [-fps <num>] [-fs] [-am]
              [-ga] [-avl] [-arav <num>] [-agvs] [-a <str>] [-vk <str>] [-ak <str>] [-sx <num>]
              [-sy <num>] [-sw <num>] [-sh <num>] video address...
    -h, --help                                    displays this help message
    -vr, --version                                print version string
    -v, --verbose                                 verbose command line output for grav
    -vpv, --vpmedia-verbose                       verbose command line output for VPMedia (network/RTP/
                                                  decoding backend)
    -t, --threads                                 threading separation of graphics and network/decoding
                                                  (this is the default, option left in for legacy purposes)
    -nt, --no-threads                             disables threading separation of graphics and network/decoding
    -np, --no-python                              disables python tools, including Access Grid integration
    -es, --enable-shaders                         enable GLSL shader-based colorspace conversion if it would
                                                  be available (experimental, may not look as good, adds CPU
                                                  usage to rendering thread)
    -bf, --use-buffer-font                        enable buffer font rendering method - may save memory and be
                                                  better for slower machines, but doesn't scale as well CPU-wise
                                                  for many objects
    -ht, --header=<str>                           header string
    -fps, --framerate=<num>                       framerate for rendering
    -fs, --fullscreen                             start in fullscreen mode
    -am, --automatic                              automatically focus on single objects, rotating through the
                                                  list at regular intervals
    -ga, --gridauto                               rearrange all objects in grid on source add/remove
    -avl, --available-video-list                  add supplied video addresses to available list, rather than
                                                  immediately connect to them
    -arav, --auto-rotate-available-video=<num>    rotate through available video sessions every [num] seconds
    -agvs, --get-ag-venue-streams                 grab video sessions from Access Grid venue client, if running
    -a, --audio=<str>                             RTP audio session address (only used for highlighting/centering
                                                  corresponding video)
    -vk, --video-key=<str>                        encryption key for initial video sessions
    -ak, --audio-key=<str>                        encryption key for initial audio sessions
    -sx, --start-x=<num>                          initial X position for main window
    -sy, --start-y=<num>                          initial Y position for main window
    -sw, --start-width=<num>                      initial width for main window
    -sh, --start-height=<num>                     initial height for main window

Keyboard Shortcuts
------------------
::

         (backspace)    Clear selection.
            (escape)    Quit.
                   +    Upscale selected objects.
                   -    Downscale selected objects.
                   =    Upscale selected objects.
                   F    Rearrange objects to focus on selected objects.
                   G    Toggle site grouping.
                   H    Print this help message to the commandline.
                   L    Toggle group locks.
                   M    Mute selected objects.
                   N    Scale selected videos to native size.
                   P    Arrange objects around the perimeter of the screen.
                   R    Arrange objects into a grid.
                   T    Rearrange groups.
                   U    Update group names.
                   X    Toggle rendering of selected objects.
       alt + (enter)    Toggle fullscreen.
             alt + A    Toggle 'automatic' mode (rotating focus)
             alt + R    Toggle runway visibility.
            ctrl + A    Select all.
            ctrl + I    Invert selection.
            ctrl + Q    Quit.
            ctrl + V    Toggle venue client controller visibility.
           shift + F    Fullscreen selected object (includes border and text).
           shift + N    Scale all videos to native size.
    shift + ctrl + D    Toggle graphics debugging information.
    shift + ctrl + F    Fullscreen selected object (video/inner contents of object).

General
-------

All video streams in the multicast group(s) you are connected to will
automatically be displayed. To move objects, you can click on them and
click on a destination, or click-and-drag. For selecting multiple objects,
click-and-drag starting from empty space for a box selection, or ``ctrl-click``
on a video to add it to the selection. You can invert a selection with
``ctrl-i``. Resize selected objects with ``+`` (or ``=``) and ``-``.

Videos
------

In addition to being manipulated as described above, video objects can be
muted (will not decode or process) with ``m`` or have their rendering disabled
with ``x`` (will still decode/process in the background but only display the
last frame). If the runway is visible, muting videos will send them there (see
"Runway" section below for info). Videos can be scaled to their native size
with ``n`` (size all videos to native size with ``shift-n``). Right-clicking
with videos selected will bring up an option for a properties window which
will display resolution, codec, and metadata info.

Layouts
-------

grav includes a few automatic layout algorithms. Press ``r`` to rearrange
objects in a grid. Press ``p`` to arrange objects in a perimeter around
the earth. Press ``f`` with objects selected to focus on selected objects
in the center and move others around them. ``shift-f`` and ``ctrl-shift-f``
will fullscreen a selected object, including the border and excluding the
border respectively.

``alt-a`` will enable automatic mode. When automatic mode is on, grav will
focus on a single object, and focus will automatically rotate between objects
at a regular interval. Automatic mode can also be toggled in the view menu.

Groups
------

Videos can be grouped by siteID (metadata that comes from Access Grid).
Press ``g`` to enable siteID groups - videos will be added to their groups
automatically. Pressing g again will disable siteID grouping, dissociate
videos from their siteID groups and delete the siteID groups. Press ``l``
on a selected group to unlock it - unlocking allows you to move a group's
members independently of it, and its unlocked status will be noted in its
displayed name. Note that resizing a group will automatically rearrange
its members if it is in the locked state. You can also manually rearrange
internal group members by pressing ``t``.

Session Management
------------------

All addresses listed on the command line will be added as video sessions.
Sessions can be added or removed at runtime with the side window.  Sessions
can also be temporarily disabled via the right-click menu - disabling a
session will not process the incoming packets but you will still receive
the data.

Video sessions can also be added to an "available video" list, meaning they
will not be automatically connected to on startup, and when in this list
only one can be connected to at a time. Adding ``-avl`` on the command line
will added given sessions to the available video list. Available video sessions
can be rotated through manually via the right-click menu on the rotated video
group in the side window, or automatically every X seconds with the ``-arav``
(seconds) command line option. In addition, sessions can be shifted between
the main list and the available video list via the right-click menu in the side
window.

Runway
------

The runway is a side area for muted videos (press m on a video to mute it).
To remove a video from the runway, simply drag it out of the runway area.
The runway will be hidden when turning on automatic mode, or you can manually
enable/disable it with ``alt-r``.

Venue Client Controller
-----------------------

If you have the Access Grid toolkit installed, grav can control the Access
Grid Venue Client, mainly via changing venues. To bring up the Venue Client
Controller menu, either press ``ctrl-v`` or toggle it via the entry in the
view menu. If you do not have Access Grid installed, do not have a Venue
Client running, or if a running Venue Client is not in a venue, this option
will not be available.

In the Venue Client controller menu, different venue connections will be
shown. Double click on a venue to signal the Venue Client to move to that
venue. Doing so will also automatically grab any video addresses in that
venue and add them to the main list of connected video sessions.

To automatically grab video addresses from a running Venue Client when grav
is started, use the command line option ``-agvs``.

Notes
=====

`grav` (C) 2011 Rochester Institute of Technology.

Authored by `Andrew Ford <http://github.com/adhesion>`_ with
contributions from `Ralph Bean <http://github.com/ralphbean>`_.

`grav` is free software, released under the GNU GPL. See COPYING for details.

This software uses libraries from the FFmpeg project under the GPLv3.

Earth imagery by NASA from the
`Visible Earth project
<http://visibleearth.nasa.gov/view_detail.php?id=2430>`_.

.. FOOTNOTES:

.. [1] README.rst authority(!)

   This README.rst file is the 'authoritative source' for
   documentation.  The information is duplicated in some place on the `grav`
   trac instance at http://grav.rc.rit.edu, but trac can handle the
   reStructuredText (.rst) format used here and should be updated with changes
   made here first.

   To inform trac to render a block of text (the whole page!) as rST, surround
   the block with::

       {{{
       #!rst

       <document goes here>

       }}}

   This feature of trac requires that the python ``docutils`` package is
   installed on the server.  This is already installed on
   http://grav.rc.rit.edu, but if another instance is being installed somewhere,
   the ``docutils`` package can be installed with the following command::

      easy_install docutils

   See http://trac.edgewall.org/wiki/WikiRestructuredText for more information.

.. [2] Gotcha:  multiple ffmpegs.

   Having another copy of ffmpeg installed on your machine (things
   like VLC and mplayer might depend on it) will cause conflicts,
   ie, when running `grav` or anything that wants to link to the new
   ffmpeg 0.5 in ``/usr/local/lib``, you will get a "symbol lookup
   error", probably looking for av_gcd or similar in your system
   copy of ffmpeg in ``/usr/lib``. To temporarily fix this, run::

       export LD_LIBRARY_PATH=/usr/local/lib

   or whichever directory you installed ffmpeg in. Put that command
   into your startup script if you want to not have to do that each
   time, though that might break other things that dynamically link
   to ffmpeg, if they rely on an older version.
