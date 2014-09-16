Cody Lieu
cal53
CS 310
Lab 1
9/11/2014

I spent approximately 10 hours on this assignment. I enjoyed it a lot and feel that it has improved my understanding of the C programming language significantly.

For dmalloc, I followed a first fit approach and split the block into an allocated block and a leftover header if there was enough space, and just allocated the whole block if there wasn't enough space. I checked for cases of the allocated block being in the beginning, middle, and end of the freelist.

For dfree, I iterate through the freelist checking for the first instance of the freed block address being lower than freelist_head's address because this is where it is supposed to be inserted. I check for the edge case of the block being inserted at the end of the list.

For coalesce, I check if freelist_head's previous block is right behind it and if its next block is right next to it and coalesce accordingly.

I implemented coalesce in O(1) time and have free in O(n) time.

My HeapManager passes all of the new tests and has an accuracy of 78% for test_stress and test_stress_time.
