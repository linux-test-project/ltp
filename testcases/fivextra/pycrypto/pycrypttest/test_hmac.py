#
# Test script for Crypto.Hash.HMAC
#

from sancho.unittest import TestScenario, parse_args, run_scenarios
from Crypto.Hash import HMAC, MD5, SHA

tested_modules = [ "Crypto.Hash.HMAC" ]

class HMACTest (TestScenario):

    def setup (self):
	pass

    def shutdown (self):
	pass

    def check_original (self):
        "Check original, pre-1.9 HMAC interface"
        self.test_stmt('HMAC.HMAC(MD5)')
        h = HMAC.HMAC(MD5)
        key1 = "\x0b" * 16 ; data1 = 'Hi There'
        digest1 = ('\x92\x94\x72\x7a\x36\x38\xbb\x1c'
                   '\x13\xf4\x8e\xf8\x15\x8b\xfc\x9d')
        key2 = 'Jefe' ; data2 = "what do ya want for nothing?"
        digest2 = ('\x75\x0c\x78\x3e\x6a\xb0\xb5\x03'
                   '\xea\xa8\x6e\x31\x0a\x5d\xb7\x38')

        key3 = '\xAA' * 16 ; data3 = '\xDD' * 50
        digest3 = ('\x56\xbe\x34\x52\x1d\x14\x4c\x88'
                   '\xdb\xb8\xc7\x33\xf0\xe8\xb3\xf6')
        
        self.test_val('h.hash(key1, [data1])', [digest1])
        self.test_val('h.hash(key2, [data2])', [digest2])
        self.test_val('h.hash(key3, [data3])', [digest3])

        h = HMAC.HMAC(SHA)


if __name__ == "__main__":
    (scenarios, options) = parse_args()
    run_scenarios(scenarios, options)
