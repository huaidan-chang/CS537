# Running P1
Ensure you are currently in the memcached folder (if you do `ls`, you should see memcached.c and Makefile, among others).
Run tests using `~cs537-1/tests/p1a/run-tests.sh`. When you run tests, you should see your score in the last line (out
of 95).

You will see a list of tests being run. The general format of the test is `test_<name>_p<points>`. So `test_c_div_1_p20`
is a test that is worth 20 points and checks the div method. If a test fails, read the exception message carefully.
It will usually guide you to where you went wrong. To understand more about the errors, look at the section below.

The final test (worth 5 points) is not tested by default because it takes ~2 minutes to run. When you feel confident
about the rest of the tests, run ` ~cs537-1/tests/p1a/run-tests.sh --full` to run the final test as well. This will give
you a score out of 100.

# Scoring.
There are no hidden test cases. We will run your submission using `run-tests.sh --full` and that will be your score.
We will copy over the t/ folder and the Makefile, so you don't need to copy them over in your turnin folder.


# Understanding errors thrown by testing framework.
Here's an example of a failed test you might see if you run
the test cases without modifying memcached:

```
    test_c_div_1_p20 (__main__.TestP1A)
    test simple div command ... ERROR

    ======================================================================
    ERROR: test_c_div_1_p20 (__main__.TestP1A)
    test simple div command
    ----------------------------------------------------------------------
    Traceback (most recent call last):
      File "/home/cs537-1/tests/p1a/test-p1a.py", line 174, in test_c_div_1_p20
        answer = int(answer.strip())
    ValueError: invalid literal for int() with base 10: b'ERROR'
```

This means that the `test_c_div_1_p20` failed. You can read more about it in 
`/home/cs537-1/tests/p1a/test-p1a.p`, line 174.

Go to that line in that file, and you'll find this snippet of test:
```
        self.conn.write(b'set p1adiv 0 3600 5\r\n')
        self.conn.write(b'90252\r\n')

        _ = self.conn.read_until(b'STORED\r\n')

        self.conn.write(b'div p1adiv 3\r\n')
        answer = self.conn.read_until(b'\r\n')
        answer = int(answer.strip())
        self.assertEqual(answer, 30084)
```

So it seems that the test is setting p1adiv to 90252. It reads your server's output until STORED.
Then it calls `div p1adiv 3` and tries to read and parse the output as an integer. It then tries
to confirm that the answer your server returned is indeed 90252/3 == 30084.

The traceback shows that the test was trying to parse "ERROR" as an integer. What that means is your
server returned ERROR instead of the value. You can input the same commands into telnet to reproduce the
bug, and figure out a solution using breakpoints/gdb.

