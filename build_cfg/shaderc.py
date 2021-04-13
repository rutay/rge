import os
import re


# ------------------------------------------------------------------------------------------------
# Shader
# ------------------------------------------------------------------------------------------------


def preproc_include_dir(src_path: str, shader_code: str) -> str:
    dir_name = os.path.abspath(os.path.join(src_path, os.pardir))
    out = shader_code

    for match in re.finditer(r"[^\n]*#include \"([^\"]*)\"[^\n]*", shader_code, re.MULTILINE):
        dep_name = match.groups()[0]
        dep_path = os.path.join(dir_name, dep_name)

        if os.path.exists(dep_path) and os.path.isfile(dep_path):
            with open(dep_path, "rt") as dep_file:
                dep_txt = dep_file.read()
                (start_idx, to_idx) = match.span()
                out = out[:start_idx] + dep_txt + out[to_idx:]
        else:
            raise ValueError(f"Can't open file: {dep_path} requested by #include.")

    return out


def preproc_shader(shader_path: str, shader_code: str) -> str:
    out = shader_code
    out = preproc_include_dir(shader_path, out)
    return out


def proc_shader(shader_path: str) -> str:
    with open(shader_path, "rt") as shader_f:
        shader_code = shader_f.read()

        out = shader_code
        out = preproc_shader(shader_path, out)
        return out


# ------------------------------------------------------------------------------------------------
# MaterialShader
# ------------------------------------------------------------------------------------------------

def preproc_material_dir(shader_code: str, material_code: str) -> str:
    i = 0
    for match in re.finditer(r"^#material$", shader_code, re.MULTILINE):
        i += 1

        if i > 1:
            raise ValueError("#material directive is repeated more than once.")

        (start_idx, to_idx) = match.span()
        shader_code = shader_code[:start_idx] + material_code + shader_code[to_idx:]

    return shader_code


def preproc_material_shader(shader_path: str, shader_code: str, material_path: str, material_code: str) -> str:
    out = shader_code
    out = preproc_shader(shader_path, out)
    out = preproc_material_dir(out, material_code)
    return out


def proc_material_shader(shader_path: str, material_path: str) -> str:
    # todo compile the shader to spir-v in case of desktop applications?

    with open(shader_path, "rt") as shader_f, open(material_path, "rt") as material_f:
        shader_code = shader_f.read()
        material_code = material_f.read()

        out = shader_code
        out = preproc_shader(shader_path, out)
        out = preproc_material_shader(shader_path, out, material_path, material_code)
        return out
