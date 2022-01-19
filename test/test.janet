(import /build/utf8)

(def valid?-positive-tests [
    "a"
    "abc\xc3\xb1"
    "\xc3\xb1"
    "\xe2\x82\xa1"
    "\xf0\x90\x8c\xbc"
])

(def valid?-negative-tests [
    "abc\xc3\x28"
    "\xc3\x28"
    "\xa0\xa1"
    "\xe2\x28\xa1"
    "\xe2\x82\x28"
    "\xf0\x28\x8c\xbc"
    "\xf0\x90\x28\xbc"
    "\xf0\x28\x8c\x28"
    "\xf8\xa1\xa1\xa1\xa1"
    "\xfc\xa1\xa1\xa1\xa1\xa1"
])

(each tc valid?-positive-tests
  (unless (utf8/valid? tc)
    (errorf "%v should be valid" tc)))

(each tc valid?-negative-tests
  (when (utf8/valid? tc)
    (errorf "%v should be invalid" tc)))

(assert (= (utf8/to-lower "ABCÄ") "abcä"))
(assert (= (utf8/to-upper "abcä") "ABCÄ"))
# Test smalloc code path
(assert (= (utf8/to-lower "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"))

(assert (deep= (utf8/to-codepoints "abc😀") @[97 98 99 128512]))

# Basic sanity test.
(assert (= (utf8/normalize "abc😀") "abc😀"))

# Test from- and to-codepoints
(def s "ěščřžýáíé")
(assert (= s (utf8/from-codepoints (utf8/to-codepoints s))))
