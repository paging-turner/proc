# Proc
A diagram editor styled after the diagrams shown in [Picturing Quantum Processes](https://www.cs.ox.ac.uk/people/aleks.kissinger/PQP.pdf).

This is a work in progress, and features are added as I progress through the book, so don't expect too much for now!

## Guide
The main interactive unit in the app is a "process", which will make a lot of sense if you end up reading the book. Processes are just boxes that can be connected together with wires, like string diagrams.

- Ctrl-click the background to create a process.
- Click and drag processes to move them around.
- Each process has a green box than can be clicked to create a new wire. Click on another process (or the same process) to connect the two with a wire.
- Processes can be selected by being clicked, and wires can be selected by clicking one of the green boxes at the endpoints.
- Clicking any part of the background will de-select any selected processes/wires.
- With a process selected, press the "I" key to start "text insertion mode", which allows you to type text that appears inside the process. Exit text-insert mode by clicking somewhere on the backgroun.
- Pressing the "backspace" key deletes any selected process/wire.
- Pressing the "tab" key when certain processes are selected will change their appearance. These special processes are:
    - Processes with only one input, only one output, or no inputs/outputs can be toggled to be invisible. This is useful if you want to have dangling wires or text labels.
        - Ex. If you wanted to draw a bare wire, you would connect two processes, and then toggle both processes to be invisible.
        - Ex. For a text label, create a process with no ins/outs, enter some text, then toggle the process to be invisible.
    - Processes with exactly 2 inputs or outputs can be toggled to look like a wire that changes directions (cups and caps).
