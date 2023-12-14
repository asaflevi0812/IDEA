import glob


src_dirs = ["src", "src/chunking", "src/clucene-wrapper", "src/fsl", "src/index", "src/recipe", "src/storage", "src/utils", "src/reverse_mapping"]
include_dirs = ["src/lucene++_include", "src/lucene++_include/lucene++", "src/lucene++_include/lucene++/core", "src/lucene++_include/lucene++/contrib"]
include_dirs += src_dirs
libs = ["boost_system", "boost_random", 
        "pthread", "lucene++", "lucene++-contrib", "ssl", "db", "crypto"]

command = "g++-7 -w -fpermissive --std=c++14 -Ofast -g3"

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