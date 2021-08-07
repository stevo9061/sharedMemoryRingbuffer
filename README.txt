
1. Please, at first run 'make' in the directory.

2. Execute the program 'read' first, afterwards start 'write'. 
   With the argument '-m[SIZE]' the size of the ring buffer can be selected.

Example:

./read -m2 < Philosophen.txt &
./write -m2 > out.txt



The usage of the script is as follows:

1. ./sender_test.sh
2. ./receiver_test.sh

Subsequently, a test file (receiver_test_output.txt) is generated.
