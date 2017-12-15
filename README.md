# Hand Gesture Recognition

A detailed introduction to this system can be found at https://arxiv.org/abs/1704.07296 and https://arxiv.org/abs/1709.08945.

A demo video can be found at https://youtu.be/4n9F7iJJ2TY.


It is a beta version now, and works more stable than the pervious versions.

The detection of the hand region is based on skin color, while the background substraction is provided in order to deal with bad light condition but it is not must-be-set.
The analysis of gesture is now based on contour and convexity defect extraction. It works not quite accurately.

V gesture for single click.
W gesture for dragging.
Four gesture for right click.
Five gesture for double click.

Now it only works on Mac OS.

In the next version, it is considered to introduce caffe and use CNN to recognize gestures.
