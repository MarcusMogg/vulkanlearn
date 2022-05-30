# coding=utf-8

import shutil
import subprocess
import os

shader_path = os.path.abspath("./shaders")
tex_path = os.path.abspath("./texture")
res_path = os.path.abspath("./resources")

build_path = os.path.abspath("./out/build/x64-debug")


def TraverseDir(dirpath, relativepath, func):
    for x in os.listdir(dirpath):
        abspath = os.path.join(dirpath, x)
        if os.path.isdir(abspath):
            TraverseDir(abspath, os.path.join(relativepath, x), func)
        elif os.path.isfile(abspath):
            func(abspath, relativepath, x)
        else:
            print(x)


def BuildGlsl(filepath, relativepath, filename):
    suffix = os.path.splitext(filename)[1]
    if suffix in [".h", ".cpp", ".cc"]:
        return
    buildpath = os.path.join(build_path, relativepath, filename)
    # create if not exist
    os.makedirs(os.path.split(buildpath)[0], exist_ok=True)
    param = "glslc {0} -o {1}".format(
        filepath, buildpath)
    print(param)
    subprocess.call(param, shell=True)


def CopyImage(filepath, relativepath, filename):
    suffix = os.path.splitext(filename)[1]
    if suffix in [".h", ".cpp", ".cc"]:
        return
    buildpath = os.path.join(build_path, relativepath, filename)
    # create if not exist
    os.makedirs(os.path.split(buildpath)[0], exist_ok=True)
    param = "copy {0} {1}".format(
        filepath, buildpath)
    print(param)
    shutil.copy(filepath, buildpath)


TraverseDir(shader_path, "shaders", BuildGlsl)
TraverseDir(tex_path, "texture", CopyImage)
TraverseDir(res_path, "resources", CopyImage)
