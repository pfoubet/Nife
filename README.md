
        nife - Networking Industrial Forth-like Environment.

Overview

     Nife is a programming language based on the principles set of Forth
     defined by Charles H. Moore in the 1960s. It does not include the
     full functionality of Forth, that is the reason why it is considered
     as a "Forth-like" language.

     His ambition is to offer people who are not computer scientists, and
     need to make measurements, control of remote devices to monitor industrial
     processes, handling of large collections of data, make calculations, of
     filtering, statistics, can easily make their needs in a Linux environment
     at low cost.

     The simplicity of this language that anyone can understand how it works
     in a few minutes and totally control a few hours to a week at most.
     It can also be used more modestly as a super calculator, just to make its
     accounts or calculations of matrix inversion. The public concerned is
     very large.

     It keeps the forth the following characteristics:
        - Mode operation interpretation: as a shell.
        - Loading source files and compilation and execution.
        - Using stacks (digital, logical and characters) requiring the use of
          notation called "reverse-Polish."
        - How to write new features.
     
     It adds the following possibilities:
        - A true "context-free" language to compile code on the fly.
        - Installation compatible on all Unix-like systems including Linux.
        - Ease of interfacing to additional commands in a program Nife use many
          free tools available in a Linux environment (eg xgraph).
        - Integration of a consistent mathematical library to easily perform
          calculations on a large number of data.
        - Ability to easily add other functions to the base library.
        - Automatic recognition of cards or devices compatible with the library
          COMEDI (http://www.comedi.org) to make data acquisition, signal
          processing, automatic pilot. This is one of its sides "industrial".
        - Variable definition of variable geometry (which may in turn contain
          a scalar, an array of numeric values, a boolean, a string ...) and
          can also become executable variables (ie referring to the code that
          will be called when using the variable). This aspect gives the term
          Nife dynamic language with all that this implies (code compact and
          fast, scalable performance).
        - Management of multi-tasking with the command task that can initiate
          parallel execution of functions, control (whether it is complete,
          refer to the consoles, delete, etc.).
        - Nife programs can communicate with each other via the communication
          tools offered by traditional Unix, internally (within the same system)
          or external interface based on TCP/IP. For this, it has its own
          protocol STSP (stack to stack protocol) for exchanging data directly
          from a numerical stack to an other one. This is one of its aspects
          "Networking".
        - The possible opening towards the use of other devices through the
          integration of specific drivers.


How to compile and install nife

     Download the nife source code, then:
     tar zxvf nife-x.y.z.tar.gz
     cd nife-x.y.z
     ./configure
     make

     WARNING !!
     make install is not preconized in beta version !!

     You can use it by positionning in the source directory and call the
     command :
     cd src 
     ./nife

     You can also lauch the benchmark test :
     ./test.sh


Web Page

        http://www.nife-project.org

Mailing List and Bug Reports

        nife-project.org hosts all the nife-related mailing-lists.
        + help-nife@nife-project.org is for those seeking to get help without
          wanting to hear about the technical details of its development.

        + nife-devel@nife-project.org is the list used by the people
          that make Nife and a general development discussion list, with
          moderate traffic.

        To subscribe, go directely to :
        http://nife-project.org/general/newsletter/subone/view-subscribe

        For general bug reports, send a description of the problem to
        nife@seriane.fr or directly to the development list.

Current Status

        nife is still in beta version.

   Patrick Foubet (patrick.foubet@nife-project.org)


