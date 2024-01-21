Checkpoint 1 Writeup
====================

My name: Logan Schreier

My SUNet ID: logansch

I collaborated with: mattjv22

I would like to thank/reward these classmates for their help: mattjv22

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

I was surprised by or edified to learn that: processing strings char by char is insanely slow, and even more slow is the sort() function, especially if sorting a large vector like I was :3

Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

Implementation Challenges:
Finding a fast way to extract non-overlap within a string that had overlap; keeping track of which indices in the overall payload were already accounted for; avoiding memory/use of uninitialized value errors (thanks valgrind, and Michael Chang!); avoiding the storage of data that's outside the bounds of tempStorage. Finally, I had to rework my entire approach after my first implementation (which went char-by-char) was ridiculously slow and caused me to fail win test 0. 

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: 

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
