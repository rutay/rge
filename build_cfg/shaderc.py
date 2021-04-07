import re
import typing


def preprocess_material_directive(shader_code: str, material_code: str) -> str:
    i = 0
    for match in re.finditer("^#material$", shader_code, re.MULTILINE):
        i += 1

        if i > 1:
            raise Exception("Found more than one #material directive.")

        (start_idx, to_idx) = match.span()
        shader_code = shader_code[:start_idx] + material_code + shader_code[to_idx:]

    return shader_code


def preprocess_material_shader(shader_code: str, material_code: str) -> str:
    out = preprocess_material_directive(shader_code, material_code)

    return out


def process_material_shader(shader_f: typing.IO, material_f: typing.IO, material_shader_f: typing.IO):
    # todo compile the shader to spir-v in case of desktop applications?

    out = preprocess_material_shader(shader_f.read(), material_f.read())
    material_shader_f.write(out)
