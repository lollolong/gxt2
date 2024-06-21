import os
import json
from subprocess import call

# american
# chinese
# chinesesimp
# french
# german
# italian
# japanese
# korean
# mexican
# polish
# portuguese
# russian
# spanish
LANGUAGE = "american"

GXT2CONV_PATH = r"gxt2conv.exe"
DLC_TEXT_DIR = r"DLC Text"
UPDATE_TEXT_DIR = r"Game Text"
PATCH_TEXT_DIR = r"Patch Text"

def convert_gxt2_files(base_dir, language_subdir):
    for subdir, _, files in os.walk(base_dir):
        if language_subdir in subdir:
            for file in files:
                if file.endswith('.gxt2'):
                    gxt2_file_path = os.path.join(subdir, file)
                    call([GXT2CONV_PATH, gxt2_file_path])

def merge_json_files(base_dir, language_subdir, merged_data):
    for subdir, _, files in os.walk(base_dir):
        if language_subdir in subdir:
            for file in files:
                if file.endswith('.json'):
                    json_file_path = os.path.join(subdir, file)
                    try:
                        with open(json_file_path, 'r', encoding='utf-8') as f:
                            data = json.load(f)
                            if data:  # Ensure the data is not None
                                merged_data.update(data)
                    except json.JSONDecodeError:
                        print(f"Skipping invalid JSON file: {json_file_path}")
                    except Exception as e:
                        print(f"Error reading file {json_file_path}: {e}")

if __name__ == "__main__":
    output_json_file = f"{LANGUAGE}_rel.json"
    merged_data = {}


    convert_gxt2_files(DLC_TEXT_DIR, os.path.join(LANGUAGE))
    convert_gxt2_files(UPDATE_TEXT_DIR, f"{LANGUAGE}_rel")
    convert_gxt2_files(PATCH_TEXT_DIR, f"{LANGUAGE}_rel")
    
    merge_json_files(DLC_TEXT_DIR, os.path.join(LANGUAGE), merged_data)
    merge_json_files(UPDATE_TEXT_DIR, f"{LANGUAGE}_rel", merged_data)
    merge_json_files(PATCH_TEXT_DIR, f"{LANGUAGE}_rel", merged_data)
    
    with open(output_json_file, 'w', encoding='utf-8') as f:
        json.dump(merged_data, f, indent=4, ensure_ascii=False)
