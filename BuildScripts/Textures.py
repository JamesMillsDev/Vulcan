import subprocess, json, os
from pathlib import Path

ignore_file_extensions = []
channel_names = [ 'R', 'G', 'B', 'A' ]

def format_for(meta : json):
    format_str : str = ""
    channels : json = meta["channels"]
    channel_count : int = channels["count"]
    channel_size : int = channels["size"]

    for i in range(0, channel_count):
        format_str += channel_names[i] + str(channel_size)

    format_str += "_"

    if channel_size > 8:
        format_str += "SFLOAT"
    else:
        match meta["type"]:
            case "color":
                format_str += "SRGB"
            case "normal":
                format_str += "UNORM"

    return format_str

def get_mip_settings(mip_settings : json, gen : bool):
    if gen != True:
        return []
    
    settings : list[str] = [ "--generate-mipmap" ]

    settings.extend(["--mipmap-filter", mip_settings["filter"]])
    settings.extend(["--mipmap-filter-scale", mip_settings["scale"]])
    settings.extend(["--mipmap-wrap", mip_settings["wrap"]])

    return settings

def transcode(project_name : str, ktx_sdk : str, texture_extensions : list[str], transcoder : json, directories : json, script_dir : Path):
    texture_dir = directories["textures"]
    content_dir = directories["content"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(content_dir) / Path(texture_dir)
    collected_files = [
        file for file in input_dir.rglob("*")
        if file.is_file() and file.suffix.lower()[1:] in texture_extensions
    ]

    transcoder_executable = Path(os.path.expandvars(ktx_sdk)) / Path("bin") / Path(transcoder["name"])

    for file in collected_files:
        with open(str(file) + ".meta") as json_data:
            meta = json.load(json_data)

            format : str = format_for(meta)
            srgb : bool = meta["srgb"] == True
            hdr : bool = meta["hdr"] == True
            gen_mips : bool = meta["mipmap"]["generate"] == True
            normal_mode : bool = meta["type"] == "normal"
            mip_settings : list[str] = get_mip_settings(meta["mipmap"], gen_mips)

            out_file = Path(input_dir) / Path(str(file.stem))
        
            print('[INFO] Transcoding: ' + file.stem + file.suffix.lower() + ' -> ' + file.stem + '.ktx2')

            run = [
                str(transcoder_executable)
            ]

            run.extend(["create"])
            run.extend(["--format", format])
            run.extend(["--encode", "uastc-hdr-4x4" if hdr else "uastc"])
            run.extend(["--assign-tf", "srgb" if srgb else "linear"])
            if hdr == True:
                out_file = str(out_file) + ".exr"
            else:
                out_file = str(out_file) + ".ktx2"

            run.extend(mip_settings)

            run.extend([str(file), str(out_file)])

            subprocess.run(run)