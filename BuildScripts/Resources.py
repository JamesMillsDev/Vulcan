import json, shutil
from pathlib import Path

ignore_file_extensions = []

def ignore_content(path, files):
    ignored = set()

    for file in files:
        if Path(file).suffix[1:].lower() in ignore_file_extensions:
            ignored.add(file)

    return ignored

def _split(lst, n):
    k, m = divmod(len(lst), n)
    return [lst[i * k + min(i, m):(i + 1) * k + min(i + 1, m)] for i in range(n)]

def _pack_group(resources : list[Path], output_name : str, output_path : Path, index : int, parent_dir : Path):
    # Open the new resource file for writing
    resource_file = open(output_path / Path(output_name + str(index) + ".res"), mode="+wb")

    # Iterate over every resource file and open it for read
    for file_path in resources:
        file = open(file_path, "rb")
        
        # Write the relative file path to the file along with its length
        relative_path = file_path.relative_to(parent_dir)
        resource_file.write(len(str(relative_path)).to_bytes(length=4, byteorder="little"))
        resource_file.write(bytearray(str(relative_path), "utf-8"))

        # Write the binary length and the binary data
        bin = file.read()
        resource_file.write(len(bin).to_bytes(length=4, byteorder="little"))
        resource_file.write(bin)

        file.close()

    resource_file.close()

def pack(project_name : str, directories : json, script_dir : Path, output : str, group_count : int = 3):
    # Get all files in the input directory and their subdirectories
    content_dir = directories["content"]
    solution_dir = script_dir / directories["solution"]
    input_dir : Path = Path(solution_dir) / Path(project_name) / Path(content_dir)

    paths = list(input_dir.rglob("*"))
    resources = _split(list(filter(lambda file: file.is_file() and file.suffix.lower()[1:] not in ignore_file_extensions, paths)), group_count)

    # Validate the output directory
    output_path : Path = solution_dir / project_name / Path(output)
    output_path.mkdir(parents=False, exist_ok=True)

    mappings = {}

    index : int = 0
    for resource_group in resources:
        # Store the mappings relative to the root dir
        mappings[index] = [str(file.relative_to(input_dir)) for file in resource_group]

        # Pack this resource data into its file and increment the integer
        _pack_group(resource_group, output, output_path, index, input_dir)
        index += 1

    # Write the file mappings to the mappings file
    resource_map_file = open(output_path / Path(output + ".mappings"), mode="+wt")
    resource_map_file.write(str(mappings))
    resource_map_file.close()

def copy(project_name : str, out_dir : str, copy_dir : Path, directories : json, script_dir : Path):
    config_dir = directories["config"]
    dlls_dir = directories["dlls"]
    solution_dir = script_dir / directories["solution"]

    input_dir = Path(solution_dir) / Path(project_name) / Path(copy_dir)
    output_dir = Path(out_dir) / Path(copy_dir)
    
    shutil.copytree(input_dir, output_dir, dirs_exist_ok=True, ignore=ignore_content)

    input_dir = Path(solution_dir) / Path(project_name) / Path(config_dir)
    output_dir = Path(out_dir) / Path(config_dir)

    shutil.copytree(input_dir, output_dir, dirs_exist_ok=True, ignore=ignore_content)