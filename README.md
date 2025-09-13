# Proc
A diagram editor styled after the diagrams shown in [Picturing Quantum Processes](https://www.cs.ox.ac.uk/people/aleks.kissinger/PQP.pdf).

This is a work in progress, and features are added as I progress through the book, so don't expect too much for now!

## Guide
The main interactive unit in the app is a "process", which will make a lot of sense if you end up reading the book. Processes are just boxes that can be connected together with wires, like string diagrams.

The keybinds below are defaults, but can be changed by editing `config/keybind.txt` (see `config/example-keybind.txt`).

- Ctrl-click the background to create a process.
- Click and drag processes to move them around.

- Processes can be selected by being clicked, and wires can be selected by clicking one of the green boxes at the endpoints.
    - Select multiple processes and wire by ctrl-clicking.
    - Click and drag from the background to select multiple processes.
- Clicking any part of the background will de-select any selected processes/wires.

- Each process has a green box than can be clicked to create a new wire. Click on another process (or the same process) to connect the two with a wire.
- Wires can be moved by dragging the endpoint to a new place.
    - Drag the wire-end to another wire-end to move the wire to that place, pushing other wires to the right.
    - Draw the wire-end to a process to move the wire to the right-most end of the process.

- Use Ctrl-C to copy selected processes and Ctrl-V to paste them at the mouse's position.
- Press Ctrl-D to delete any selected processes/wires.

- Use the mouse-wheel to zoom in and out.
- Right-click and drag to pan.

- Enter text by selecting some processes and typing. Backspace deletes from the end of the characters.
- Press the "Tab" key when certain processes are selected to change their appearance. These special processes are:
    - Processes with only one input, only one output, or no inputs/outputs can be toggled to be invisible. This is useful if you want to have dangling wires or text labels.
        - Ex. If you wanted to draw a bare wire, you would connect two processes, and then toggle both processes to be invisible.
        - Ex. For a text label, create a process with no ins/outs, enter some text, then toggle the process to be invisible.
    - Processes with exactly 2 inputs or outputs can be toggled to look like a wire that changes directions (cups and caps).
    - Processes with exactly 1 input and 1 output will just look like a wire.
- Ctrl-M will toggle on/off "rounded shapes" mode (Rounded shapes are still a bit wonky with their shape and sizing).
