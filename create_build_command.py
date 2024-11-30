import glob


src_dirs = ["src", "src/chunking", "src/clucene-wrapper", "src/fsl", "src/index", "src/recipe", "src/storage", "src/utils", "src/reverse_mapping"]
include_dirs = ["/usr/include/glib-2.0", "/usr/lib/x86_64-linux-gnu/glib-2.0/include", "src/lucene++_include", "src/lucene++_include/lucene++", "src/lucene++_include/lucene++/core", "src/lucene++_include/lucene++/contrib"]
include_dirs += src_dirs
libs = ["pthread", "lucene++", "lucene++-contrib", "ssl", "db", "crypto",
        "boost_system", "boost_random", "boost_regex"]

command = "g++-9 -w -fpermissive --std=c++14 -g3 -Wl,--copy-dt-needed-entries" # -D _GLIBCXX_USE_CXX11_ABI=0"

command += " -I" + " -I".join(include_dirs)

for src_dir in src_dirs:
    c_dir = src_dir + "/*.c"
    cpp_dir = src_dir + "/*.cpp"

    if glob.glob(c_dir):
        command += " " + c_dir
    if glob.glob(cpp_dir):
        command += " " + cpp_dir

# source files should always come before linked libraries

# special glib addition
command += " $(pkg-config --libs glib-2.0)"

command += " -l" + " -l".join(libs)

command += ' -o build/destor'

print(command)

