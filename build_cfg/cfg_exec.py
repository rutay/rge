import argparse
import json
import os

import res_gen


# https://stackoverflow.com/questions/7204805/how-to-merge-dictionaries-of-dictionaries
def merge(a, b, path=None):
    if path is None:
        path = []
    for key in b:
        if key in a:
            if isinstance(a[key], dict) and isinstance(b[key], dict):
                merge(a[key], b[key], path + [str(key)])
            elif a[key] == b[key]:
                pass  # same leaf value
            else:
                raise Exception('Conflict at %s' % '.'.join(path + [str(key)]))
        else:
            a[key] = b[key]
    return a


def load_cfg(cfg_files: []):
    cfg = {}
    for cfg_file in cfg_files:
        print("Loading: `%s`..." % cfg_file.name)
        rel_cfg = json.load(cfg_file)
        merge(cfg, rel_cfg)
    return cfg


def create_resources_def_hpp(resources):
    _dir = os.path.join(os.environ['GAME_SRC_DIR'], ".gen_cfg")
    path = os.path.join(_dir, "resources_def.hpp")

    print("Generating `%s`..." % path)

    if not os.path.exists(_dir):
        os.mkdir(_dir)

    enum_classes = []
    resources_init = []

    category_idx = 0
    for category_name, category in resources.items():

        enum_classes.extend([
            "enum class " + category_name + " {",
            "None = %d * 0x01000000," % category_idx
        ])
        for resource_name, resource in category.items():
            enum_classes.extend([
                resource_name + ","
            ])

            resources_init.extend([
                "rge::ResourceManager::register_resource_desc(%s::%s, \"%s\");" % (
                    category_name, resource_name, resource["path"])
            ])

            if "bin_assoc" in resource:
                bin_assoc = resource['bin_assoc']
                resources_init.extend([
                    "rge::ResourceManager::register_resource_binary_assoc(%s, %s, %s::%s);" % (
                        "::".join(bin_assoc['a']),
                        "::".join(bin_assoc['b']),
                        category_name,
                        resource_name
                    )
                ])

        enum_classes.extend([
            "};",
            ""
        ])

        category_idx += 1

    lines = [
        "#pragma once",
        "",
        "#include \"resources/resource_manager.hpp\"",
        "",
        "namespace rge::resources {",
        "",
        *enum_classes,
        "",
        "inline static void init() {",
        *resources_init,
        "}",
        "}"
    ]

    f = open(path, "wt")
    f.write("\n".join(lines))
    f.close()


def generate_resources(resources):
    res_gen.generate(resources)


def on_cmake_configure(cfg):
    create_resources_def_hpp(cfg['resources'])


def on_post_build(cfg):
    generate_resources(cfg['resources'])


def main():
    parser = argparse.ArgumentParser(description="RGE configuration executor.")

    parser.add_argument("--config-files", "-c",
                        type=argparse.FileType('r'),
                        nargs="+",
                        required=True)

    parser.add_argument("--on", "-o")

    args = parser.parse_args()

    cfg = load_cfg(args.config_files)

    if args.on in globals() and callable(globals()[args.on]):
        globals()[args.on](cfg)
    else:
        print("Can't find a callable func for: `%s`." % args.on)


if __name__ == "__main__":
    main()
