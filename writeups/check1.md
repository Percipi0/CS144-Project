Checkpoint 1 Writeup
====================

My name: Logan Schreier

My SUNet ID: logansch

I collaborated with: mattjv22

I would like to thank/reward these classmates for their help: mattjv22

This lab took me about [30] hours to do. I [did not] attend the lab session.

I was surprised by or edified to learn that: processing strings char by char is insanely slow, and even more slow is the sort() function in comparison to a simple insert-where-needed approach, especially if sorting a large vector like I was :3 

Describe Reassembler structure and design: I use the classic 2-string approach wherein one string (called storage) acts as a buffer and stores our characters, and the other (storage_bitmap) acts as a kind of bitmap of 0's and 1's to indicate which positions in storage contain values we will need. I am a huge fan of this approach as it is FAR simpler to read and debug than my other attempts, my first of which involved using a deque that was populated with tuples of type <uint64, char>; the uint64 stored the index of the char. This was, of course, ridiculously slow, and also convoluted due to the fact that I'd have to scan the deque for overlap before adding a new char. Furthermore, I used the sort() function on the deque which made my slow implementation much slower... My second implementation cut out the sort() function, and I instead linearly inserted elements in their proper place. I also modified the deque to store tuples of type <uint64, string, uint64> where the first uint describes the first index of the string, and the second uint64 describes the last index of the string. This approach helped me delete overlapping strings quicker, because I could simply look at the two uints in a given tuple to determine the indices they covered, instead of taking the length of the string and calculating the indices held within every single time I tried to find overlap. However I had to scrap this entire approach after at least 10 hours of debugging it because of some horrible suite of bugs I couldn't even properly locate - it was far too difficult to read and therefore debug. So I went with the much simpler and much more elegant 2-string approach. The 2-string approach has its benefits in being easy to read, and in easily dealing with overlap - if we have first_index = 2 and string = "c", if we then get first_index = 1 and string = "bc", we can simply replace instead of deleting the c, like what I would have done in my first 2 attempts. The 2-string approach also took me a fraction of the time to code versus my second deque<uint,string,uint> approach - perhaps 75% less time. Most of the gains in time to implement came from the ease of debugging. Another strength of my approach is that the storage string is never longer than the initial storage capacity we're given. I considered another implementation that doubled the size of the storage string when our index went out of bounds, however I chose not to because it would have wasted a ton of space - after one size increase we'd already expect to see a full half of the string go entirely unused. Finally, a potential drawback of the 2-string approach is that I could have used an actual bitmap, where bit one maps to the first index of our storage string and so on, but instead I used a string for ease of use. This makes it far slower to mark whether a given index is occupied or not, whereas using a bitmap would enable a simple bitflip to do this job quicker and in a less computationally expensive manner.

Implementation Challenges:
Finding a way to extract non-overlap within a string that had overlap; keeping track of which indices in the overall payload were already accounted for; avoiding memory/use of uninitialized value errors (thanks valgrind, and Michael Chang!); avoiding the storage of data that's outside the bounds of tempStorage. Finally, I had to rework my entire approach after my first implementation (which went char-by-char) was ridiculously slow and caused me to timeout the wintest. I then tried another approach where I stored not just individual chars, but full strings within tuples organized in a deque. It was absolutely bug-infested, and I couldn't pass the win tests. So I restrarted again, this time with the 2-string approach, and passed everything up to the speed test. I was then getting out-of-bounds errors in Valgrind due to indexing outside of the bitmap's bounds, so I added some modulus math - index % storage capacity -  to properly index the reassembler's buffer when required.

Remaining Bugs:
Hopefully none! I ran the tests through Valgrind too and did not locate any errors.

- Optional: I had unexpected difficulty with: working with my deque strategy. Iterating over it and locating overlap to remove was very time-consuming and full of potential for error. So I had to wipe it and start over with the 2-string strategy. I also had plenty of trouble with my first strategy of storing individual chars - it was excruciatingly slow!

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
