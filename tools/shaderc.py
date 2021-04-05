
import argparse
import re

def resolve_material_directive(shader_code, material_code):
    i = 0
    for match in re.finditer("^#material$", shader_code, re.MULTILINE):
        i += 1

        if i > 1:
            raise Exception("Found more than one #material directive.")

        (start_idx, to_idx) = match.span()
        shader_code = shader_code[:start_idx] + material_code + shader_code[to_idx:]

    return shader_code


def parse_shader(shader_code, material_code=None):
    compiled = shader_code
    compiled = resolve_material_directive(compiled, material_code)

    return compiled


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Shader utility for RGE")

    parser.add_argument("--in_shader", "-i",
                        type=argparse.FileType("rt"),
                        required=True,
                        help="The shader file to be compiled."
                        )

    parser.add_argument("--in_material", "-m",
                        type=argparse.FileType("rt"),
                        help="The material .mat file to use that will be replaced in the #material directive."
                        )

    parser.add_argument("--out_shader", "-o",
                        required=True,
                        help="The file where the compiled shader will be written."
                        )

    args = parser.parse_args()

    _in_shader = args.in_shader
    _in_material = args.in_material

    _out_shader = args.out_shader
    _out_shader = open(_out_shader, 'w')

    compiled_shader = parse_shader(_in_shader.read(), _in_material.read())
    _out_shader.write(compiled_shader)
