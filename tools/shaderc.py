
import argparse
import re
import json
import os

def resolve_uniform_name(material, name):
    return "Material_%s_%s" % (material["name"], name)

def resolve_material_directive(_in, material=None):
    m = re.search("^#material$", _in, re.MULTILINE)
    if m is None:
        return _in

    if not material:
        return _in

    uniforms = []
    if "uniforms" in material:
        uniforms = material["uniforms"]

    # Creates the definitions of the uniforms.
    uniforms_def = []
    for uniform in uniforms:
        uniforms_def.append("uniform %s %s;" % (uniform["type"], resolve_uniform_name(material, uniform["name"])))
    uniforms_def = "\n".join(uniforms_def)

    (start_idx, to_idx) = m.span()

    mat_dir = os.path.dirname(material["path"])
    mat_code_file = open(os.path.join(mat_dir, material["code"]), "rt")
    mat_code = mat_code_file.read()

    mat_code = re.sub(
        "([^a-zA-Z_$]*)material\\.([a-zA-Z_$][a-zA-Z_$0-9]*)",
        r"\1%s" % resolve_uniform_name(material, r"\2"),
        mat_code
    )

    mat_code = uniforms_def + "\n" + mat_code

    return _in[:start_idx] + mat_code + _in[to_idx:]


def parse_shader(_in, material=None):

    compiled = _in
    compiled = resolve_material_directive(compiled, material)

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
                        help="The material .json file to use that will be replaced in the #material directive."
                        )

    parser.add_argument("--out_shader", "-o",
                        type=argparse.FileType("wt"),
                        required=True,
                        help="The file where the compiled shader will be written."
                        )

    args = parser.parse_args()

    _in = args.in_shader

    _material = None
    if args.in_material:
        _material = json.loads(args.in_material.read())
        _material["path"] = args.in_material.name
        _material["name"] = os.path.splitext(os.path.basename(_material["path"]))[0]

    compiled_shader = parse_shader(_in.read(), _material)
    args.out_shader.write(compiled_shader)
