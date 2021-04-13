import shutil
import os
import shaderc


def resolve_src_path(sym_path):
    [prefix, rge_path] = sym_path.split("/", 1)

    # print(prefix, rge_path)

    if prefix == ".rge":
        # If the symbolic path starts with .rge then .rge must be replaced with ${RGE_HOME} (from cmake).
        return os.path.join(os.environ['RGE_HOME'], rge_path)
    else:
        # Otherwise the path is a game-relative path and thus must be replaced with game's src dir (from cmake).
        return os.path.join(os.environ['GAME_SRC_DIR'], sym_path)


def resolve_dst_path(sym_path):
    # For the destination, the sym_path is kept as is and just moved to the game's binary directory.
    return os.path.join(os.environ['GAME_BIN_DIR'], sym_path)


def resolve_dst_path_and_mkdir(sym_path):
    dst_path = resolve_dst_path(sym_path)
    dst_dir = os.path.dirname(dst_path)
    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)
    return dst_path


def copy(resources, resource_name, resource, data):
    src_path = resolve_src_path(resource['path'])
    dst_path = resolve_dst_path_and_mkdir(resource['path'])

    print(f"[RGE] Copying: {resource_name} -> {dst_path}")

    shutil.copy(src_path, dst_path)


def compile_shader(resources, resource_name, resource, data):
    src_path = resolve_src_path(resource['path'])
    dst_path = resolve_dst_path_and_mkdir(resource['path'])

    out = shaderc.proc_shader(src_path)
    with open(dst_path, "wt") as out_f:
        out_f.write(out)


def compile_material_shader(resources, resource_name, resource, data):
    shader_name = data['shader']
    material_name = data['material']
    material_shader_name = resource_name

    shader_src_path = resolve_src_path(resources['Shader'][shader_name]['path'])
    material_src_path = resolve_src_path(resources['Material'][material_name]['path'])
    material_shader_dst_path = resolve_dst_path_and_mkdir(resource['path'])

    print(f"[RGE] Compiling material shader: {material_shader_name} -> {material_shader_dst_path}")

    out = shaderc.proc_material_shader(shader_src_path, material_src_path)
    with open(material_shader_dst_path, 'w') as out_f:
        out_f.write(out)


def generate(resources):
    for category_name, category in resources.items():
        for resource_name, resource in category.items():
            if 'generator' in resource:
                generator = resource['generator']

                if 'type' not in generator:
                    print("`generator` found but no `type` specified.")
                    raise

                if generator['type'] not in globals():
                    print("Generator `%s` is unknown." % generator['type'])

                # print("Generating %s (through `%s`)..." % (resource_name, generator['type']))
                gen_func = globals()[generator['type']]
                gen_func(resources, resource_name, resource, generator)
            else:
                print(f"[RGE] Resource without generator: {resource_name}")
