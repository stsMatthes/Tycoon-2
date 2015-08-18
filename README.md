STS Tycoon-2 Distribution Guide
===============================

Contents:
---------

-   [Version](#ver)
-   [Licence](#lic)
-   [Hard- and software requirements for Tycoon-2](#req)
-   [Installing Tycoon-2](#ins)
    -   [Installation for Unix](#sol)
    -   [Installation for Windows](#win)
    -   [Installing JAM/MR](#jam)
-   [Organisation of the Tycoon-2 System](#org)
-   [**WARNING:** Syntax-changes](#syn)
-   [Running Tycoon-2](#run)
-   [Configuring and Compiling the Tycoon-2 virtual machine](#compile)
-   [Simple Toplevel Examples](#exp)
-   [Using Tycoon-2 and its Tools](#use)
    -   [The Class-Loader](#classloader)
    -   [The Type-Checker](#typechecker)
    -   [The Class-Browser](#classbrowser)
    -   [The Debugger](#debugger)
    -   [Tycoon-2 Virtual Machine (TVM) Options](#tvmopt)
-   [Simple Class Examples (Piggybank)](#cls)
-   [Static Class Library Overview](#app)
-   [Sample Programs](#prg)
-   [FAQ](#faq)
-   [Advanced Topics](#adv)
    -   [Porting Tycoon-2 to a new Platform](#port)
    -   [Bootstrapping a Tycoon-2 System](#boot)

------------------------------------------------------------------------

Version
-------

The current Version of Tycoon-2, which is described in this documentation, is **Version 1.0.3**.

------------------------------------------------------------------------

License
-------

Tycoon-2 is published under the **GNU GENERAL PUBLIC LICENSE (GPL)**. See [here](LICENSE.txt) (local version) or at <http://www.opensource.org/gpl-license.html> for the agreement.

------------------------------------------------------------------------

Hard- and software requirements for Tycoon-2
--------------------------------------------

Tycoon-2 was tested on Unix platforms (Solaris 5.x, HP-UX 10.x and Linux) and Windows (NT 4.0, 2000, XP). A minimum of 64 MB RAM is recommended for all platforms. The distribution will use about 100 MB of diskspace on installation and tends to grow while using, depending on the size of the persistent stores.

To use the Tycoon-2 STML web pages ([\[GaWi97\]](http://www.sts.tu-harburg.de/papers/STS-Rep-1997.html#GaWi97), [\[MaWi97\]](http://www.sts.tu-harburg.de/papers/STS-Rep-1997.html#MaWi97)), you need to have a current version of James Clark's SGML parser **nsgmls** installed. Nsgmls can be downloaded from <http://www.jclark.com/sp/>. The distribution was tested with nsgmls version 1.3.4.

For compiling the Tycoon-2 virtual machine, a platform-independent make-tool, called **JAM**, is used. A slightly customized version of JAM version 2.2 is included in this Tycoon-2 distribution (in the directory `contrib/Jam-2.2`). JAM comes from [Perforce Software](http://www.perforce.com/jam/jam.html). For Windows JAM needs the additional tools `rm.exe`, `touch.exe` and `echo.exe` from the [Cygnus](http://www.cygnus.com/misc/gnu-win32/) GNU Tools for Win32. These tools are also bundled with the Tycoon-2 distribution and can be found in the directory `contrib/win32`.

For building the Tycoon-2 virtual machine on Windows, either the free Borland C++ compiler or the Microsoft Visual C++ Compiler v 5.0 or higher is needed. For Unix systems, **cc** or **gcc** can be used. The binaries for Linux are compiled for the glibc2. If you still have got a libc5 system, you have to recompile the TVM (see [below](#compile)).

**Windows 95/98**: Windows platforms earlier than Windows **NT** 4.0 are not supported any longer for Tycoon-2. You can use Tycoon-2 with Windows 95/98, but because of thread-synchronisation problems on that platform, you will have following severe limitations:

1.  No garbage collection is available
2.  No multithreaded applications

On Windows 95 you need the WinSockets 2.0 update from Microsoft. All security functions from tosSecurity (e.g. getuid, getgid, ...) are only available under Windows NT and later.

------------------------------------------------------------------------

### Installation for Windows

**Steps:**

1.  Add the directory `contrib\win32` to your path, or copy all files from `contrib\win32` into a directory in your PATH. This directory contains some tools needed by jam (rm, touch, cygecho) and the cygwin dlls they require, and some dll's needed by the Tycoon-2 runtime.

### Installing JAM/MR

If you need (or want) to compile the Tycoon-2 virtual machine, you need to install a version of Jam (version &gt;= 2.2). You can find an full Jam distribution on the directory `contrib/Jam-2.2`. Simple copy the Jam executable from the `contrib/Jam-2.2/bin.xxxx` subdirectory (xxxx = your platform) into a directory in your path.

Further notes for recompiling a new Jam-Version can be found in [README.JAM](jam/README.JAM.html). Information for writing Jamfiles can be found in the [JAM Documentation](jam/Jam.html) provided by Perforce Software, Inc.

------------------------------------------------------------------------

Organisation of the Tycoon-2 System
-----------------------------------

NOTE: Alle pathnames in this documentation are Unix-pathnames, i.e. the separator is "/". For Windows, the backslash "\\" is used. In most cases, Tycoon-2 (and even Jam) convert them transparently.

All files and (sub-)directories are contained in one directory called **tycoon-2** which is installed from the archieve **tycoon2-102.tgz**. This directory contains all further directories and files, and is organized as follows:

**tycoon2/**  
This is the directory for the *normal usage*. It contains:

**Clean.ts**  
This is the initial persistent store. Hint: Use only a backup copy of the initial store, e.g. **TL2.ts**; if you damage your working-store or want to start over from scratch, you can easily restore from the initial status.

**TL2.ts**  
This is your working persistent store, a copy of the initial store **Clean.ts**.

**Jamrules / Jamfile**  
The Jam files for compiling the Tycoon-2 virtual machine. The Jamrules contains all the compiler flags, e.g for debugging or tracing the TVM.

**lib/**  
This directory contains dynamic loadable libraries both for Unix and for Windows, separated in subdirectories named **Linux\_i386** and **Solaris2\_SPARC** etc. They are used by the TVM only.

**lib/sgml/**  
Contains files needed to parse SGML (CATALOG and document type definitions).

**tools/**  
This directory contains some additional tools for setting up the environment variables. These shell-scripts in the **Win32** and **Unix** subdirectories are explained in section [Running Tycoon-2](#run).

**src/**  
Contains all sources: Sources of the Tycoon-2 virtual machine in **tm/** and **tycoonOS/**, and all Tycoon-2 sources (standard library, compiler, etc.) in **TL2/**.

**bin/**  
Contains executable files in platform-specific subdirectories.

**bootstrap/**

This is the directory for the *bootstrapping* the system. All files and directories following the organisation of the `tycoon2` directory, limited to all that, what is really needed by a bootstrap. You will not need it unless you want to change the virtual machine or the compiler. For informations on bootstrapping see [below](#boot).

**contrib/**

Contains some additional tools like Jam, the MS Windows C-runtime DLL's, and a Tycoon-2 mode for **XEmacs**.

**doc/**

Documentation for the Tycoon-2 system (under construction). Also contains some code examples. Currently, the best documentation are the published [papers and theses](entry.html#pub).

------------------------------------------------------------------------

WARNING: Syntax changes
-----------------------

Since Tycoon-2 version 1.0.2, we have completed the transition to the Tycoon-2 syntax version 1.0 as described in [\[GaWi98\]](http://www.sts.tu-harburg.de/papers/STS-Rep-1998.html#GaWi98). Some papers and documentation may still use an older version of the surface syntax. The old syntax is no longer understood by the system, but a conversion tool is included (see "tycoon2/src/TL2/Compile/Sugar/Sugar2.tyc").

------------------------------------------------------------------------

Running Tycoon-2
----------------

This section describes how to start the Tycoon-2 system. Once installed as described above (section [Installing Tycoon-2](#ins)), at the beginning each session the environment (variables, path etc.) has to be set up. This is accomplished by the shell-script **tools/Unix/def\_tl2\_sh** on Unix systems and by **tools/Win32/def\_tl2.bat** on Windows (If you want to compile the tycoon runtime or virtual machine, ensure that you have set the MS Visual Studio environment variables or BCCROOT set before invoking def\_tl2.bat). The scripts are setting up shell variables which are used by Tycoon-2, like $PATH, $HOX\_HOME (which is the mentioned root directory), or $HOX\_HOST (which contains the current platform).

First change the current directory to your Tycoon-2 working directory (`tycoon-2/tycoon2` for normal development, `tycoon-2/bootstrap` for using the bootstrap system), then execute the script in this place ($HOX\_HOME will be set to the current working directory).

      Unix:                                  Windows:
      cd xxx/tycoon-2/tycoon2                cd xxx\tycoon-2\tycoon2
      source tools/Unix/def_tl2_sh           tools\win32\def_tl2
        

If you use a C shell under UNIX, you should use the `tools/Unix/def_tl2_csh` script. If you have problems under Windows with the MSVCNT environment variable (set by `tools/Win32/def_tl2.bat`) see further hints on [README.JAM](jam/README.JAM.html)

The next step is to invoke Tycoon-2 with a persistent store. On all platforms, the call is:

    tycoon2 TL2.ts
        

After a short loading time, the Tycoon-2 Toplevel prompt will appear. The next steps are described in section [Simple Toplevel Examples](#exp).

------------------------------------------------------------------------

Configuring and Compiling the Tycoon-2 virtual machine
------------------------------------------------------

If you do not have binary files for your desired platform, or if you modify the Tycoon-2 virtual machine (e.g. the C sources under `tycoon-2/tycoon2/src/tm`) you have to compile new Tycoon-2 binaries with Jam. First of all, you have to set the environment variables for the machine variant you want to recompile (bootstrap or development machine) as described in [Running Tycoon-2](#run). On Windows platforms ensure that you the MS Visual Studio environment variables are set.

Then set up your configuration. This is done by editing the compiler options in the Jamrules file (e.g. turn on TVM tracing flags). Available options are:

BUILDMODE  
controls C compiler optimization level and debug information. "rt" for optimized runtime version, "debug" for debuggable version.

BOOTSTRAP  
Include code to transform a dump file into an executable store. Needed for bootstrap. No performance impact.

ASSERTIONS  
Enable assertions in the Tycoon Virtual Machine. May avoid store corruption, and may produce better diagnostics than a SEGFAULT. Slight performance impact.

TVM\_TRACE
   
Include code to enable tracing of calls between components, as described in [\[WMB99\]](http://www.sts.tu-harburg.de/papers/STS-Pub-1999.html#WMB99). Serious performance impact for traced code, minimal impact for other threads.

TSP\_SUPERCOMPONENTS  
Store the supercomponent of each object in the object header. Required for the UML99 demo. Increases the store size by 15%. A store produced by a machine with TSP\_SUPERCOMPONENTS enabled cannot be read by a machine without TSP\_SUPERCOMPONENTS and vice versa. When you change this option, you need to create a new store from a bootstrap dump file.

TSP\_DEBUG  
Debugging for the Tycoon Store. \#\#\#

TVM\_DEBUG  
Debugging for the Tycoon Virtual Machine. \#\#\#

TMDEBUG\_TEST  
\#\#\#

Finally, you change to the directory containing the Jamrules file (`tycoon-2/tycoon2`, or `tycoon-2/bootstrap`) and invoke jam there:

    cd xxx/tycoon-2/tycoon2
    jam
        

You can also clean up all currently generated binary files:

    cd xxx/tycoon-2/tycoon2
    jam clean
        

------------------------------------------------------------------------

Simple Toplevel Examples
------------------------

After starting Tycoon-2, the Toplevel prompt (**|&gt;**) will appear.

    sun02\> tycoon2 TL2.ts
    System re-started.
    1
    |>
        

**HINT:** All commands and statements in the toplevel must end with a question mark (**?**). The toplevel won't react and start execution of the typed text until it sees the question mark. So if the system seems to be "hanging", try typing another question mark.

The most comfortable way to use the toplevel is using the supplied tl2-mode for Emacs (see contrib/xemacs/). Second to that, you can copy and paste code lines from a text-editor into a shell window.

A number of simple, ready-to-type-in code examples are provided [here](simple-toplevel-examples.html). They cover literals, operators, function objects, blocks, scope, visibility and control structures. They are intended to get familiar with the syntax and the usage of the toplevel of Tycoon-2. There is also a [text-only-version](simple-toplevel-examples.tyc) of these examples. The latter file can be found in the distribution at **doc/codeExamples/demo.tyc**

Tycoon-2 can always be aborted simply by pressing **ctrl-c**. In this case, the persistent store will not be saved and and all changes since the last commit will be lost.

To save the Tycoon-2 persistent store, at the toplevel type

    |> tycoon.saveSystem ?
            

Instead of typing long sequences of often used command in the toplevel again and again, the toplevel can read its command from a file. To execute a file containing Tycoon-2 statements (e.g. the mentioned **test.tyc**), type

    |> DO load "Test/test.tyc"?
        

The default current directory for **DO load** commands is the **src/TL2/** directory and is defined by the shell environment variable **TL2\_PATH**. If you want to load the code examples, add the code Examples directory to the TL2\_PATH:

    sh:  TL2_PATH=../doc/codeExamples:$TL2_PATH
    csh: setenv TL2_PATH ../doc/codeExamples:$TL2_PATH
    cmd: set TL2_PATH=..\doc\codeExamples;%TL2_PATH%
            

Note: There is a convention to name files containing scripts with the extension **.tyc**. In contrast, files containing class-definitions have the same name as the class definition they contain, with the extension **.tc** (see below).

------------------------------------------------------------------------

Using Tycoon-2 and its Tools
----------------------------

Software develoment in Tycoon-2 is not normally done by typing entire classes into the toplevel prompt, although this would be possible. Normally, in object-oriented development in Tycoon-2, classes are written in files (having extension .tc) and are stored somewhere in **src/TL2/**. Similar to other programming languages and systems, the classloader (compiler) and typechecker is invoked by a call on the toplevel of Tycoon-2. Compiled classes are stored in Tycoon-2's *persistent object store*, which is saved in **TL2.ts**. The usage of the tools for developing software is discussed in the following subsections.

------------------------------------------------------------------------

### The Class Loader

#### What is the task of the Class Loader?

The classloader is a tool to check which classes-files in the **src/TL2/** directory have been changed since last loading and compilation and to load and compile them.
 The classloader is accessible as an object named `tycoon.tl.loader`.

#### How to use the Class Loader

1.  To be able to use the classloader, each class definition has to be in its own class-file with extension **.tc**. In Tycoon-2, class names by convention start with an uppercase letter, and continue with lower- and uppercase letters mixed, e.g. **FooBar**. It is **strongly** recommended that class name and class file name are identical, e.g. the class **Humble** has to be written in a class file named **Humble.tc**. Since both Window NT and all Unix OS support long filenames, we encourage to give meaningful names to classes. Additionally, there is a naming convention for metaclasses; their name is usually postfixed with **Class**, e.g. the metaclass (if one is defned) of the class **Humble** is named **HumbleClass** and therefore stored in the file **HumbleClass.tc** in the same directory as Humble.tc. It is advisable to create a new subdirectory in the **src/TL2/** directory for own new projects, e.g. **/src/TL2/MyProject**. By convention directory-names start with upper case letters.
2.  For file- and pathnnames, the classloader assumes **src/TL2/** as implicit current directory, so all file- and pathnames should be expressed relative to this default. This directory is defined in the shell environment variable **TL2\_PATH** and is set up by the **def\_tl2** starting script described above. The following examples will make this obvious.
3.  The next step is to tell the classloader, where are your class files located. This is done by either directly by

        tycoon.tl.loader.registerClassFile("Humble", "MyProject/Humble.tc") ?
                

    or a little bit more comfortably by

        tycoon.tl.loader.registerFile("MyProject/Humble.tc") ?
                

    which assumes **Humble** as classname. It is possible to process all class files in a directory (all files ending with .tc) at once by the command

        tycoon.tl.loader.registerDirectory("MyProject") ?
                

    The classloader prints a message "Building Foo" for each unknown class. If the filename of an already registered class has changed, a warning containing the old and new filename is printed.

4.  The last step did only register the classes. The next step is to start the loader, which actually parses and compiles the classes. This is done by

        tycoon.tl.loader.update ?
                

    Now the loader will read and parse all classes changed since last compilation (a) (even these, which were registered a long time ago) and it is checked, if all needed classes (superclasses, metaclasses etc.) are known (registered) (b). After this, all necessary classes are compiled in a special dependency order (c). If an error occurs, copiling is aborted, but already compiled classes won't be lost. After (hopefully) fixing the bug, the nect update can be started and compilation starts where at the point where the error occured.
     The output while doing so looks as follows:

    1.  For each read source class file the path is printed out:

            [./MyProject/Foo.tc...
            ]
            [./MyProject/Humble.tc...
            ]
                        

    2.  The loader checks for neede classes:

            [Checking for unknown class names...]
                        

        Here error messages may be printed out.

    3.  The loader compiles all parsed classes in a dependency order, i.e. it tries to compile subclasses and metaclasses before the class itself. The loader also complains about circular relationships. While compiling, following messages are printed:

            [Building Foo
             recomputing cpl in class Bar
             ...
            ]
                        

        plus potential error messages.

5.  If class files are deleted, the loader complains about missing files. To switch off checking a class, type:

        tycoon.tl.loader.unregisterClass("Foo") ?
                

    This turns off checking the class **Foo** whereever the class file was located before. Since once compiled classes reside in the persistent store, they cannot be deleted permanently. A workaround to get almost rid of a useless class is to overwrite it by an empty definition. This is done most easily directly on the toplevel by a statement of the following form:

        class Foo metaclass AbstractClass ?
                

    This is important for typechecking and helps to find classes which still make use of Foo, since normally such an declaration will lead to typechecking errors in the dependent classes.

#### Use of scripts

If there are many files and directories to be registered, for example for a big package or project with many subdirectories, it is recommended to place all "registerDirectory" statements in one **.tyc** file residing in he root directory of the projects sources. A good convention is to name this script **make&gt;project-name&lt;.tyc**, e.g. **src/TL2/MyProject/makeMyProject.tyc**. It is **strongly disencouraged** to place class definitions into this files!

There are sometimes classes which need explicit initialization after compilation, but just once, for example *singletons*. We recommend to place such initializations in a separate .tyc file, which is placed in the same directory as the make-script, having the prefix **init**, e.g. **src/TL2/MyProject/initMyProject.tyc**.

Both types of scripts may easilsy be executed by the **DO load ...;** statement. The main advantage of this scheme is that packages can easily be transported to another Tycoon-2 system and be compiled and initialized.

A full example of both a make and an init script might look as follows: **makeMyProject.tyc**:

        ;; DO load "MyProject/makeMyProject.tyc" ?

        tycoon.tl.loader.registerDirectory("MyProject/Storage") ?
        tycoon.tl.loader.registerDirectory("MyProject/Application") ?
        tycoon.tl.loader.registerDirectory("MyProject/MyFramework/Basic") ?
        tycoon.tl.loader.registerDirectory("MyProject/MyFramework/Advanced") ?
        tycoon.tl.loader.registerDirectory("MyProject/Utilities") ?
        tycoon.tl.loader.registerDirectory("MyProject/Test") ?

        tycoon.tl.loader.update ?

        tycoon.tl.typeChecker.check ?

        ;; tycoon.saveSystem ?
        

The **initMyProject.tyc** could look like this:

        ;; DO load "MyProject/initMyProject.tyc" ?

        ;; create singletons, init some slots

        ITCSystem.init ?
        MyFrameworkGlobal.init ?
        Test.init ?

        MyFrameworkGlobal.instance.mustTrace:=true ?

        Currency.init ?
        Currency.setRates("DM", "NOK", 4.0) ?
        

------------------------------------------------------------------------

### The Type-Checker

#### Usage

The Tycoon-2 typechecker operates completly indepent from the classloader. It is similar to the classloader accessable as

    tycoon.tl.typeChecker
        

This object has several slots and methods, which are listes below:

-   `check` checks all classes that are changed by the classloader. No special order is applicated. The typechecker runs until it encounters the first error. Example:

        tycoon.tl.typeChecker.check ?
                

    This is also the most common usage of the typechecker.

-   `checkClass(c :ClassPublic)` checks the class `c` unconditionally (even if the class was not an the agenda). Example:

        tycoon.tl.typeChecker.checkClass(String) ?
                

-   `ignore(c :ClassPublic)` removes class `c` from the agenda. This can be used for example if the typechecker is known to be buggy or if a class is known to have type-errors, but this can be ignored for some reasons. Note that changing any class on which depends `c` re-inserts the class `c` into the agenda again. Example:

        tycoon.tl.typeChecker.ignore(TWLGram) ?
                

-   `agenda ` is the agenda. Example:

        tycoon.tl.typeChecker.agenda ?
                

-   `dependencies:` If any class or pool-method is chaged, all dependent classes are entered into the agenda. The dependencies for a can be viewed by `tycoon.tl.typeChecker.dependencies["String"]`. Example:

        tycoon.tl.typeChecker.dependencies["Humble"] ?
                

    prints out a Set of all Classnames dependent from **Humble**.

-   `maxDepth` is a slot describing the maximum of internal calls in the typechecker. The default value is 10000, which can be too small for very complex classes. Example:

        tycoon.tl.typeChecker.maxDepth := big number ?
                

-   `tycoon.tl.typeChecker.logSubTypeTests := true` switchs on a kind of "Super-Verbose-Mode" and is intended for debugging the typechecker, dito the slots `logTypeChecks, logValueChecks`.

------------------------------------------------------------------------

### The Class-Browser

(\#\#\# HW : todo)

Can be used to browse the inheritance structure. When clicking on a class name, the class's interface including all inherited methods, docstrings, pre- and postconditions is shown. Type names are hyperlinks to further interface views. (\#\#\# AX : todo)

------------------------------------------------------------------------

### The Debugger

The TVM contains some basic debugging functionality. It is currently used for generating sequence diagrams from running code, in the Development/Browse Objects pages of the HttpdAdmin application. To use this feature, you need to mark all objects that should appear in the sequence diagram using "isTracedComponent := true". Navigate to some interesting object using the object browser and type some message in the text field, then click on "trace" (not "eval"). (\#\#\# AX : todo)

(\#\#\# HW : todo)

------------------------------------------------------------------------

### Tycoon-2 Virtual Machine (TVM) Options

The Tycoon-2 virtual machine supports some diagnostic output in error situations. These options can be set by defining the shell environment variable `HOX_DEBUG`:

`setenv HOX_DEBUG opt[=par[,par]*][,opt[=par[,par]*]]* `

<table>
<colgroup>
<col width="33%" />
<col width="33%" />
<col width="33%" />
</colgroup>
<thead>
<tr class="header">
<th align="left">opt</th>
<th align="left">par</th>
<th align="left">Explanation</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td align="left">all</td>
<td align="left">-</td>
<td align="left">Activates all Messages</td>
</tr>
<tr class="even">
<td align="left">BackTrace</td>
<td align="left"></td>
<td align="left">Specifies, which exceptions generate a backtrace</td>
</tr>
<tr class="odd">
<td align="left"></td>
<td align="left">AllExceptions</td>
<td align="left">all</td>
</tr>
<tr class="even">
<td align="left"></td>
<td align="left">Type</td>
<td align="left">Typconversion for C-Calls</td>
</tr>
<tr class="odd">
<td align="left"></td>
<td align="left">MustBeBoolean</td>
<td align="left">if (?:) or assert</td>
</tr>
<tr class="even">
<td align="left"></td>
<td align="left">DLLOpen</td>
<td align="left"></td>
</tr>
<tr class="odd">
<td align="left"></td>
<td align="left">DLLCall</td>
<td align="left"></td>
</tr>
<tr class="even">
<td align="left"></td>
<td align="left">DivisionByZero</td>
<td align="left"></td>
</tr>
<tr class="odd">
<td align="left"></td>
<td align="left">IndexOutOfBounds</td>
<td align="left"></td>
</tr>
<tr class="even">
<td align="left">StoreGrow</td>
<td align="left">-</td>
<td align="left">Signalizes, when and how much the store grows</td>
</tr>
<tr class="odd">
<td align="left">ObjectCount</td>
<td align="left"></td>
<td align="left">Changes in count of objects grouped by class</td>
</tr>
<tr class="even">
<td align="left"></td>
<td align="left">-</td>
<td align="left">without par: after each Store grow</td>
</tr>
<tr class="odd">
<td align="left"></td>
<td align="left">CountOnGC</td>
<td align="left">after each garbage collection</td>
</tr>
</tbody>
</table>

The following example will cause the TVM to report a stack backtrace every time a exception of type `DoesNotUnderstand` or `IndexOutOfBounds` occurs.

    setenv HOX_DEBUG BackTrace=DoesNotUnderstand,IndexOutOfBounds
        

------------------------------------------------------------------------

Simple Class Examples (Piggybank)
---------------------------------

To illustrate the usage of TL2-classes there are some example classes called the *Piggybank* provided.

[Counter.tc](doc/codeExamples/demo/Counter.tc)

[Counter2.tc](doc/codeExamples/demo/Counter2.tc)

[Counter2Class.tc](doc/codeExamples/demo/Counter2Class.tc)

[CounterWithFun.tc](doc/codeExamples/demo/CounterWithFun.tc)

[CycleCounter.tc](doc/codeExamples/demo/CycleCounter.tc)

[DoubleCounter.tc](doc/codeExamples/demo/DoubleCounter.tc)

[OrderedCounter.tc](doc/codeExamples/demo/OrderedCounter.tc)

[PiggyBank.tc](doc/codeExamples/demo/PiggyBank.tc)

[ResetableCounter.tc](doc/codeExamples/demo/ResetableCounter.tc)

[ResetableDoubleCounter.tc](doc/codeExamples/demo/ResetableDoubleCounter.tc)

These examples are part of the Tycoon-2 distribution; they can be found in the directory **doc/codeExamples/demo/**.

------------------------------------------------------------------------

Static Class Library Overview
-----------------------------

The following three links lead to an graphical overview of the class hierarchy. The hierachy is displayed by a Java applet. **WARNING: The second and the third applet take a LONG while to load!**

[Class Browser (only a subset)](doc/classes/tycoon2DAGPart.html)

[Class Browser (only a subset with its metaclasses -- takes a while)](doc/classes/tycoon2DAGPartWithMetaClasses.html)

[Class Browser (all -- takes a very LONG while)](doc/classes/tycoon2DAG.html)

------------------------------------------------------------------------

(\#\#\# HW, AX : replace this by the new online version (?))

FAQ
---

Frequently asked questions - and their answers. Currently, there is only a german version available. Click [here](doc/faq.md) to read the FAQ.

(\#\#\# HW : extend FAQ and translate)

------------------------------------------------------------------------

Advanced Topics
---------------

This section covers advanced aspects of the Tycoon-2 system.

------------------------------------------------------------------------

### Porting Tycoon-2 to a new Platform

(\#\#\# AW : todo; See [preliminary german version](doc/portierung.html))

------------------------------------------------------------------------

### Bootstrapping a Tycoon-2 System

see [preliminary outdated german version](doc/bootstrap/bootstrap.txt) (\#\#\# AX, AW : todo)

------------------------------------------------------------------------

------------------------------------------------------------------------

|                                                                                     |                                             |
|-------------------------------------------------------------------------------------|---------------------------------------------|
| [![Software Systems Institute](/icons/STS-Logo.gif)](http://www.sts.tu-harburg.de/) | 02-jun-1999 [holm wegner](/~ho.wegner)      
                                                                                        04-nov-2003 [axel wienberg](/~ax.wienberg)  |


