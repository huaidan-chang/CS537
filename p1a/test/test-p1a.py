#!/usr/bin/env python3

from genericpath import isfile
from telnetlib import Telnet
import os
import subprocess
import random
import time
import unittest
import signal

HOST = 'localhost'
PORT = random.randint(1025, 65535)
PID_FILE = '/tmp/p1a-test-memcached.%s.pid' % os.getlogin()
BINARY_PATH = './memcached'

pid = None

# https://stackoverflow.com/a/49567288
class timeout:
  def __init__(self, seconds, error_message=None):
    if error_message is None:
      error_message = 'test timed out after {}s.'.format(seconds)
    self.seconds = seconds
    self.error_message = error_message

  def handle_timeout(self, signum, frame):
    raise Exception("timeout: " + self.error_message)

  def __enter__(self):
    signal.signal(signal.SIGALRM, self.handle_timeout)
    signal.alarm(self.seconds)

  def __exit__(self, exc_type, exc_val, exc_tb):
    signal.alarm(0)

def _kill_memcached_if_running():
    if not os.path.isfile(PID_FILE): return
    with open(PID_FILE, 'r') as f:
        try:
            pid = int(f.read().strip())
            # log('Killing memcached process with pid: %d' % pid, 'INFO')
            os.kill(pid, 9)
        except Exception as e:
            log('Unable to read/kill memcached server: %s' % e, 'WARN')
    if os.path.isfile(PID_FILE):
        os.remove(PID_FILE)

def _restart_memcached(memcached_path):
    _kill_memcached_if_running()
    if TestP1A.proc is not None:
        TestP1A.proc.wait()
    time.sleep(0.2)
    if not os.path.exists(memcached_path):
        raise Exception('Memcached binary not found at: %s. Check that `make all` has compiled successfully.' % memcached_path)

    for _ in range(3):
        global PORT, pid
        process = subprocess.Popen([memcached_path, '-p', str(PORT)])
        pid = process.pid
        with open(PID_FILE, 'w') as f:
            f.write("%d\r\n" % pid)
        time.sleep(1)

        if process.poll() is None: break
        PORT = random.randint(1025, 65535)
    else:
        raise Exception("Unable to start ./memcached server. 99% of the time it means that either the compilation failed or your server crashed as soon as it started. Please check manually. 1% of the time it could be due to overloaded CSL machines, please try again in 10 seconds. This is almost never the case.") 

    TestP1A.proc = process

def _check_alive():
    """ check if pid is alive"""
    if TestP1A.proc.poll() is not None: return False
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False

def log(msg, level='INFO'):
    print('[%s]: %s' % (level, msg))

def _full():
    return os.environ.get("FULL_TEST_MODE", '0') == '1'

class TestP1A(unittest.TestCase):
    proc = None

    @classmethod
    def setUpClass(cls):
        print()
        log("Beginning tests ...", "INFO")
        _restart_memcached(BINARY_PATH)
        cls.conn = Telnet(HOST, PORT)
        cls.total_score = 0

    def setUp(self):
        self.assertTrue(_check_alive(), 'Memcached server is not running')

    @classmethod
    def tearDownClass(self):
        print()

        if not _full():
            log("Not running full test mode for quicker testing. Maximum score limited to 95. When rest of tests pass, run with --full to get final score out of 100.", 'WARN')

        maximum = 100 if _full() else 95
        log('Total points: %d/%d' % (TestP1A.total_score, maximum), 'INFO')

        _kill_memcached_if_running()

    def test_a_mult_p30(self):
        """ test simple mult command """
        with timeout(5):
            score = 30
            self.conn.write(b'set p1amult 0 3600 1\r\n')
            self.conn.write(b'5\r\n')
            _ = self.conn.read_until(b'STORED\r\n')

            self.conn.write(b'mult p1amult 20\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 100)

            self.conn.write(b'mult p1amult 20\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 2000)

            self.conn.write(b'mult p1amult 0\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 0)


            self.conn.write(b'set p1amult-1 0 3600 1\r\n')
            self.conn.write(b'0\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'mult p1amult-1 1250\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 0)

            TestP1A.total_score += score

    def test_b_mult_2_p5(self):
        """ test mult command with invalid input """
        with timeout(5):
            score = 5
            self.conn.write(b'set p1amult-2 0 3600 1\r\n')
            self.conn.write(b'5\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'mult p1amult-2 -5\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            TestP1A.total_score += score

    def test_c_div_1_p20(self):
        """ test simple div command """

        with timeout(5):
            score = 20
            self.conn.write(b'set p1adiv 0 3600 5\r\n')
            self.conn.write(b'90252\r\n')

            _ = self.conn.read_until(b'STORED\r\n')

            self.conn.write(b'div p1adiv 3\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 30084)

            TestP1A.total_score += score

    def test_d_div_2_p10(self):
        """ more div tests. check integer division. """
        with timeout(5):
            score = 10
            self.conn.write(b'set p1adiv-1 0 3600 5\r\n')
            self.conn.write(b'12590\r\n')
            _ = self.conn.read_until(b'STORED\r\n')

            self.conn.write(b'div p1adiv-1 10\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 1259)

            self.conn.write(b'div p1adiv-1 10\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 125)

            self.conn.write(b'div p1adiv-1 100\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 1)

            self.conn.write(b'div p1adiv-1 10\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 0)

            self.conn.write(b'div p1adiv-1 5\r\n')
            answer = self.conn.read_until(b'\r\n')
            answer = int(answer.strip())
            self.assertEqual(answer, 0)

            TestP1A.total_score += score

    def test_e_div_3_p5(self):
        """ test div by 0 and negative numbers """
        with timeout(5):
            score = 5

            self.conn.write(b'set p1adiv-2 0 3600 5\r\n')
            self.conn.write(b'12590\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'div p1adiv-2 0\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'set p1adiv-2 0 3600 5\r\n')
            self.conn.write(b'12590\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'div p1adiv-2 -10\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            TestP1A.total_score += score

    def test_x_basic_regression_p10(self):
        """ test that basic original functionality is retained. """

        with timeout(5):
            score = 10
            self.conn.write(b'set p1a 0 3600 5\r\n')
            self.conn.write(b'hello\r\n')
            _ = self.conn.read_until(b'STORED\r\n')

            self.conn.write(b'get p1a\r\n')
            answer = self.conn.read_until(b'\r\n').strip()
            answer = self.conn.read_until(b'\r\n').strip()
            self.assertEqual(answer, b'hello')

            self.conn.write(b'set p1a 0 3600 1\r\n')
            self.conn.write(b'7\r\n')
            _ = self.conn.read_until(b'STORED\r\n')

            self.conn.write(b'incr p1a 2\r\n')
            answer = self.conn.read_until(b'\r\n').strip()
            self.assertEqual(answer, b'9')

            self.conn.write(b'decr p1a 7\r\n')
            answer = self.conn.read_until(b'\r\n').strip()
            self.assertEqual(answer, b'2')
            
            self.conn.write(b'decr p1a 7\r\n')
            answer = self.conn.read_until(b'\r\n').strip()
            self.assertEqual(answer, b'0')

            TestP1A.total_score += score

    def test_y_bound_and_value_checking_p15(self):
        """ test that your server doesn't crash on malformed/bad input. """
        with timeout(15):
            score = 15

            self.conn.write(b'set p1adiv-2 0 3600 3\r\n')
            self.conn.write(b'123\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'div p1adiv-2\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'set p1adiv-2 0 3600 3\r\n')
            self.conn.write(b'123\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'mult p1adiv-2\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'div p1adiv-404\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'mult p1adiv-404\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'div p1adiv-404 5\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'mult p1adiv-404 5\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'set p1adiv-2 0 3600 5\r\n')
            self.conn.write(b'hello\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'mult p1adiv-2 5\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            self.conn.write(b'set p1adiv-2 0 3600 5\r\n')
            self.conn.write(b'world\r\n')
            _ = self.conn.read_until(b'STORED\r\n')
            self.conn.write(b'div p1adiv-2 5\r\n')
            time.sleep(0.4) # We don't really care what you answer here. You should gracefully error out.
            self.assertTrue(_check_alive(), "Your server crashed")
            _restart_memcached(BINARY_PATH)
            TestP1A.conn = Telnet(HOST, PORT)

            TestP1A.total_score += score


    @unittest.skipUnless(_full(), "For final 5 points, run with --full in the end. Disabled by default for faster testing.")
    def test_z_complete_regression_testing_p5(self):
        """ run memcached's existing tests to ensure everything else is safe and sound. """
        with timeout(8*60):
            score = 5
            folder = os.path.dirname(BINARY_PATH)
            make_test_process = subprocess.Popen(["make", "test"], cwd=folder, env=dict(os.environ, PARALLEL='6'), stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            out = make_test_process.stdout.read()
            make_test_process.stdout.close()
            self.assertTrue('All tests successful.' in out.decode('utf-8'), "memcached's internal tests failed. Run `make test` to figure out which test failed.")

            TestP1A.total_score += score
            make_test_process.wait()


if __name__ == '__main__':
    BINARY_PATH = os.environ.get('MEMCACHED_BINARY_PATH', './memcached')
    unittest.main()
