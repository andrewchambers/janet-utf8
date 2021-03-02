(declare-project
  :name "utf8"
  :author "Andrew Chambers"
  :license "MIT"
  :url "https://github.com/andrewchambers/janet-utf8"
  :repo "git+https://github.com/andrewchambers/janet-utf8.git")

(declare-native
  :name "utf8"
  :source [
    "utf8proc/utf8proc.c"
    "utf8.c"
  ])