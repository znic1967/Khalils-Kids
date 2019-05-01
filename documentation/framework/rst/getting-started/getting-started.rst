Getting Started
=================

Supported Operating Systems
----------------------------
Need a running version of Ubuntu 16.04 LTS or above, or Mac OS X (OS X Yosemite
and above), or Windows 10 with WSL installed.

Installing WSL for Windows 10 Users
------------------------------------
Windows 10 users must install WSL in order to work with the development
environment. To do so follow these steps:

  https://docs.microsoft.com/en-us/windows/wsl/install-win10

Installing Serial Device Driver for Windows 10 and Mac Users
-------------------------------------------------------------
Windows 10 and Mac users will need to install the serial drivers on their
computer to be able to communicate with the SJTwo board. Follow this link and
download the driver for your computer.

  https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

Installation
-------------

**Step 0**
  Open a terminal (also called commandline) and go to a directory where you
  would like to download SJSU-Dev2.

  .. note::
    On Windows it is advised to install in the :code:`/mnt/c/` directory as you
    will have access to the files from your desktop. To go to this folder type
    :code:`cd /mnt/c` into your terminal and press enter.

**Step 1**
  Download the repository.

  Go to https://github.com/kammce/SJSU-Dev2 and click on the
  "Clone or download" drop down button.

  Put the zip file in the directory you chose in step 0 and unzip it there.

**Step 2**
  Change directory into **SJSU-Dev2**

  .. code-block:: bash

    cd SJSU-Dev2

**Step 3**
  Run :code:`setup` script.

  .. code-block:: bash

    ./setup

  This will install all of the necessary files and programs that SJSU-Dev2 needs
  in order to build your code. The downloaded files are placed in the :code:`tools/` folder.

**Step 4**
  Done!

Building and Loading the "Hello World" Application
---------------------------------------------------

**Step 0**
  From the root of the project/repository directory

  .. code-block:: bash

    cd firmware/HelloWorld

**Step 1**
  Run

  .. code-block:: bash

    make application

  This will take all of the relevant source code files to your project and generate a binary file that can be loaded onto your board. These files can be found in the :code:`build/application` folder with the project.

  .. tip::
    use the :code:`make` by itself to get additional information on the
    different features of the build script.

**Step 2**
  To load the binary file into your board, run the following make command:

  .. code-block:: bash

    make flash

  .. tip::
    If you run this command without first building, this command will build
    your project and then flash it. So you want to build then immediately flash,
    you can skip the step above.

  .. note::
    If :code:`make flash` couldn't find your device, try running it again. If
    it continues to fail then it could a few things such as, (1) you haven't
    installed the driver yet or (2) the board may not have the bootloader
    installed on it, which allows the board to be programmed. To install the
    bootloader, follow the steps at the bottom of this page.

**Step 3**
  To view output messages and interact with the board, go to this URL and following the setup instructions:

     https://kammce.github.io/Telemetry

  Once there, use the drop down menu to find and select the serial device that corresponds to your board press the **Connect** button.

  .. note::
    You know you have selected the right serial device if you see the web
    terminal print "Hyperload Version".

**Step 4**
  Done!!

Creating your own Project
--------------------------
Copy and rename the **HelloWorld** folder to make a new project. This project
folder must contain a :code:`source` folder, :code:`env.mk` file, and a
:code:`Makefile` to work properly.

Burning the Hyperload Bootloader on SJTwo (NOT CURRENTLY SUPPORTED)
---------------------------------------------------------------------
Move into the :code:`firmware/Hyperload/` folder and run :code:`make burn`

.. warning::
  Only do this if you cannot run :code:`make flash` or you want to upgrade
  your bootloader. Typically you shouldnt have to do this though.
