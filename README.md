# drop-tracer
The drop-tracer is software that can simulate the growth of cave forms, such as stalactites.

This can be useful when researching how cave forms develop, or can be used as a tool to generate realistic models of cave forms.

The software supports:

- The creation of cracked "base" rock models that act as a substrate on which the cave forms can grow.
- The simulation of water seeping through the cracks of the rock, depositing calcite and growing the cave forms
- The conversion of the base rock models and simulation results to 2D and 3D graphic that can be viewed or printed

INSTALLATION
------------

Use these commands:

    git clone https://github.com/jariarkko/drop-tracer.git
    cd drop-tracer
    make clean all
    sudo make install

USAGE
-----

The software is a command-line tool that runs under Unix systems. The command syntax is:

    drop-tracer [mode-options] [options]

Where mode-options is one of:

    --create-rock         Generate a rock base model through which water can flow
    --simulate            Run a simulation of water flowing through a model
    --image               Convert a selected slice of the model to a 2D image
    --model               Convert a selected slice of the model to a 3D model that can be printed


And options is one of:

    General options:
    
    --debug               Turn on debugging
    --no-debug            Turn off debugging (default)
    --unit                Sets the unit in the model in terms of how many subdivisions of a
                          meter there are. E.g., --unit 1000 implies 1mm accuracy
    --xsize               The x size of the model in units
    --ysize               The y size of the model in units
    --zsize               The z size (height) of the model in units
    --input               Input model file
    --output              Output model or image file
    --seed		  Provide a random seed, which may be needed when
                          if test runs need to be repeated deterministically

    
    Options used with --create-rock:
    
    --simple-crack        Generates a simple crack with no side cracks
    --fractal-crack       Generates a fractal crack, i.e., a main crack with side cracks
    --uniform             Sets the main crack to be equally wide throughout the model
    --non-uniform         Sets the main crack to be widest in the middle of the model
    --crack-width         Sets the width of the crack in its widest position
    --crack-growth-steps  Sets the number of steps in crack growth in the non-uniform
                          cracks
    --fractal-shrink      Sets the fractal diminishing factor, i.e., how much the next 
                          level fractal form is smaller than the current form
    --fractal-levels      Maximum level of fractal recursion
    --fractal-cardinality Sets the fractal splitting factor, i.e., how many next level
                          side fractals are generated from the current form
    --vertical-crack      Sets the crack to be created vertically (as viewed from an
                          image generated with -image). This is the default.
    --horizontal-crack    Sets the crack to be created horizontally


    Options used with --simulate:

    --rounds              Sets the number of rounds. Use suffix "M" as shorthand for
                          million and "B" for billion. For instance, 250M is 250
                          million simulation rounds.
    --drop-frequency      How many simulation rounds there are between drops of water
    --drop-size           Sets the radius of a drop of water in mm (in floating point
                          form, for instance as in "--drop-size 0.1")

                          
    Options used with --image:

    --imagez              Sets the z value when exporting a model to an image file; the
                          image will be from that z height in the model, while x and y
                          form the image
    --imagey              Sets the y value when exporting a model to an image file; the
                          image will be from that y position in the model, while z and x
                          form the image
    --imagex              Sets the x value when exporting a model to an image file; the
                          image will be from that x position in the model, while z and y
                          form the image

    
EXAMPLES
--------

This command creates a simple 10 units wide crack, putting the resulting base model in the file base.mod:

    drop-tracer --create-rock --simple-crack --creation-parameter 10 --output base.mod

This command converts the above base model into an image, putting the result in base.jpg:

    drop-tracer --image --output base.jpg

And this command runs a simulation for a million rounds:

    drop-tracer --simulate --rounds 1M --input base.mod --output result.mod

CONTRIBUTORS
------------

The software has been written by Jari Arkko.
