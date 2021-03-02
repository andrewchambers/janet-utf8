(declare-project
  :name "utf8"
  :author "Andrew Chambers"
  :license "MIT"
  :url "https://github.com/andrewchambers/janet-utf8"
  :repo "git+https://github.com/andrewchambers/janet-utf8.git")

(declare-native
  :name "utf8"
  :source ["utf8.c"])

# Some dirty patches to make all our utf8 proc symbols private, this also helps us build on windows.

(rule "build/utf8proc.h" ["utf8proc/utf8proc.h"]
      (os/mkdir "build")
      (spit "build/utf8proc.h"
            (->> (slurp "utf8proc/utf8proc.h")
                 (string/replace-all "\nUTF8PROC_DLLEXPORT extern" "\nstatic ")
                 (string/replace-all "\nUTF8PROC_DLLEXPORT " "\nstatic "))))

(rule "build/utf8proc.c" ["utf8proc/utf8proc.c"]
      (os/mkdir "build")

      (spit "build/utf8proc.c"
            (->> (slurp "utf8proc/utf8proc.c")
                 (string/replace-all "\nUTF8PROC_DLLEXPORT " "\nstatic ")
                 (string/replace-all "#include \"utf8proc_data.c\"" "#include \"../utf8proc/utf8proc_data.c\""))))


(def utf8-obj-file
  (if (= (os/which) :windows)
    "build\\utf8.obj"
    "build/utf8.o"))

(add-dep utf8-obj-file "build/utf8proc.h")
(add-dep utf8-obj-file "build/utf8proc.c")
(add-dep utf8-obj-file "utf8proc/utf8proc_data.c")
