Checkpoint 0 Writeup
====================

My name: Logan Schreier

My SUNet ID: logansch

I collaborated with: mattjv22

I would like to credit/thank these classmates for their help: mattjv22

This lab took me about [10] hours to do. I [did not] attend the lab session.

My secret code from section 2.1 was: 875597

I was surprised by or edified to learn that: string_views exist and are useful in situations where we want to work with a string without modifying it at all.

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.] I used a deque to serve as the buffer due to push_front and push_back being useful and quick (O(1)). I use I also considered using an array, but figured it would be too slow. Writer:push uses push_back and Reader:pop uses pop_front which makes for a circular buffer. 
ByteStream throughput was .47Gbit/s, and so I figured it was a solid implementation given that the minimum throughput was .1Gbit/s. A benefit of this implementation is that it was rather simple to implement, although I know it was not as fast as it could have been.

- Optional: I had unexpected difficulty with: the peek function, as I did not understand string_view. I dealt with some gnarly memory leaks! 

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [provide Pull Request URL]